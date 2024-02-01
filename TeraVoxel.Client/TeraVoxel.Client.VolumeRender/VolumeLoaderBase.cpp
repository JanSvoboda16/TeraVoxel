#include "VolumeLoaderBase.h"

template<typename T>
VolumeLoaderBase<T>::VolumeLoaderBase(const ProjectInfo& projectInfo, int threadCount)
{
	_projectInfo = projectInfo;
	_segmentCountX = projectInfo.sizeX / projectInfo.segmentSize;
	_segmentCountY = projectInfo.sizeY / projectInfo.segmentSize;
	_segmentCountZ = projectInfo.sizeZ / projectInfo.segmentSize;
	_threadCount = threadCount;

	for (size_t i = 0; i < threadCount; i++)
	{
		_loadingTreads.push_back(std::async(std::launch::async, &VolumeLoaderBase::LoadingTask, this));
	}
}

template<typename T>
VolumeLoaderBase<T>::~VolumeLoaderBase()
{
	// end loading threads and wait until ended
	_endLoopingThreads = true;
	_loadingTreads.clear();

	_loadedSegmentsMutex.lock();
	for (size_t i = 0; i < _loadedSegments.size(); i++)
	{
		MemoryContext::GetInstance().memoryInfoWriteMutex.lock();
		MemoryContext::GetInstance().usedMemory
			-= GetBlockRequiredMemory(_loadedSegments.front()->actualDownscale);
		MemoryContext::GetInstance().memoryInfoWriteMutex.unlock();

		_loadedSegments.pop();
	}
	_loadedSegmentsMutex.unlock();
}

template <typename T>
void VolumeLoaderBase<T>::LoadingTask()
{

	VolumeSegment<T>* volume;

	while (!_endLoopingThreads)
	{
		_segmentsToLoadMutex.lock();
		if (_segmentsToLoad.size() > 0)
		{			
			//int segmentIndex = 0;
			//int iterationIndex = 0;
			VolumeSegment<T>* minPrioritySegment = _segmentsToLoad.front();
			for (VolumeSegment<T>* segment :_segmentsToLoad )
			{
				if (segment->priority < minPrioritySegment->priority)
				{
					minPrioritySegment = segment;
					//segmentIndex = iterationIndex;
				}
				//iterationIndex++;
			}
			volume = minPrioritySegment;
			_segmentsToLoad.remove(volume);
		}
		else
		{
			volume = nullptr;
		}
		_segmentsToLoadMutex.unlock();

		if (volume == nullptr)
		{
			std::this_thread::sleep_for(std::chrono::milliseconds(100));
		}
		else
		{
			auto futureDownscale = volume->futureDownscale.load(std::memory_order::relaxed);
			auto unusedCount = volume->unusedCount.load(std::memory_order::relaxed);
			auto x = volume->x; auto y = volume->y; auto z = volume->z;
			auto requiredMemory = GetBlockRequiredMemory(futureDownscale);

			MemoryContext::GetInstance().memoryInfoWriteMutex.lock();
			if (unusedCount < 2 && (requiredMemory + MemoryContext::GetInstance().usedMemory.load(std::memory_order::acquire) <= MemoryContext::GetInstance().maxMemory.load(std::memory_order::acquire)) && unusedCount < 2)
			{
				Logger::GetInstance()->LogEvent("VolumeLoaderBase", "MemoryInfo", std::to_string(MemoryContext::GetInstance().usedMemory));
				MemoryContext::GetInstance().usedMemory += requiredMemory;
				MemoryContext::GetInstance().memoryInfoWriteMutex.unlock();

				// Data loading
				T* data;
				while (true)
				{
					try
					{
						data = LoadSegment(x, y, z, futureDownscale);
						break;
					}
					catch (const std::exception& ex)
					{
						if (_endLoopingThreads)
						{
							return;
						}
						std::this_thread::sleep_for(std::chrono::milliseconds(100));
					}
				}

				VolumeSegment<T>* newVolume = new VolumeSegment<T>(x, y, z);
				// New segment inicialization
				newVolume->data = data;
				newVolume->actualDownscale = futureDownscale;
				newVolume->futureDownscale.store(futureDownscale, std::memory_order::relaxed);
				newVolume->requiredDownscale = futureDownscale;
				newVolume->unusedCount.store(0, std::memory_order::relaxed);
				newVolume->used.store(true, std::memory_order::relaxed);
				newVolume->waitsToBeReloaded.store(false, std::memory_order::relaxed);

				_loadedSegmentsMutex.lock();
				_loadedSegments.push(std::unique_ptr<VolumeSegment<T>>(newVolume));
				_loadedSegmentsMutex.unlock();

				_onSegmentLoaded();
			}
			else
			{
				//FUTURE DOWNSCALE only valid when waitsToBeReloaded == true;
				volume->waitsToBeReloaded.store(false, std::memory_order::relaxed);
				MemoryContext::GetInstance().memoryInfoWriteMutex.unlock();
			}
		}
	}
}

template<typename T>
void VolumeLoaderBase<T>::AddToStack(VolumeSegment<T>* segment)
{
	_segmentsToLoadMutex.lock();

	segment->waitsToBeReloaded.store(true, std::memory_order::release);
	_segmentsToLoad.push_back(segment);

	_segmentsToLoadMutex.unlock();
}

template<typename T>
void VolumeLoaderBase<T>::Preload(int downscale, int threadCount)
{
	std::vector<std::future<void>> threads;
	for (size_t i = 0; i < threadCount; i++)
	{
		threads.push_back(std::async(std::launch::async, &VolumeLoaderBase::PreloadTask, this, i, threadCount, downscale));
	}
	for (size_t i = 0; i < threadCount; i++)
	{
		threads[i].get();
	}
}

template<typename T>
std::unique_ptr<VolumeSegment<T>> VolumeLoaderBase<T>::TakeFirstLoaded(int& count)
{
	_loadedSegmentsMutex.lock();

	count = _loadedSegments.size();
	std::unique_ptr<VolumeSegment<T>> value(nullptr);

	if (count > 0)
	{
		value = std::move(_loadedSegments.front());
		value->waitsToBeReloaded.store(false, std::memory_order::relaxed);
		_loadedSegments.pop();
	}

	_loadedSegmentsMutex.unlock();
	return value;
}

template<typename T>
void VolumeLoaderBase<T>::PreloadTask(short threadIndex, short threadCount, int downscale)
{
	int segmentCount = _segmentCountX * _segmentCountY * _segmentCountZ;
	for (size_t i = threadIndex; i < segmentCount; i += threadCount)
	{
		uint64_t requiredMemory = GetBlockRequiredMemory(downscale);
		MemoryContext::GetInstance().memoryInfoWriteMutex.lock();
		MemoryContext::GetInstance().usedMemory += requiredMemory;
		if (MemoryContext::GetInstance().usedMemory.load(std::memory_order::acquire) > MemoryContext::GetInstance().maxMemory.load(std::memory_order::acquire))
		{
			MemoryContext::GetInstance().usedMemory -= requiredMemory;
			//TODO Memory access type -> maybe relaxed?
		}
		MemoryContext::GetInstance().memoryInfoWriteMutex.unlock();

		auto z = i / (_segmentCountX * _segmentCountY);
		auto mod = i % (_segmentCountX * _segmentCountY);
		auto y = mod / _segmentCountX;
		auto x = mod % _segmentCountX;
		auto volume = new VolumeSegment<T>(x, y, z);


		for (size_t i = 0; i < 100; i++)
		{
			try
			{
				volume->data = LoadSegment(x, y, z, downscale);
				break;
			}
			catch (const std::exception& ex)
			{				
				if (i == 99)
				{
					throw ex;
				}

				std::this_thread::sleep_for(std::chrono::milliseconds(100));
			}			
		}		

		volume->actualDownscale = downscale;
		volume->futureDownscale = downscale;
		volume->requiredDownscale = downscale;

		_loadedSegmentsMutex.lock();
		_loadedSegments.push(std::unique_ptr<VolumeSegment<T>>(volume));
		_loadedSegmentsMutex.unlock();
	}
}

template<typename T>
uint64_t VolumeLoaderBase<T>::GetBlockRequiredMemory(int downscale)
{
	short downscaleDividerReq = (short)pow(2, downscale);
	return (uint64_t)pow((_projectInfo.segmentSize / downscaleDividerReq), 3) * sizeof(T);
}

template VolumeLoaderBase<uint8_t>;
template VolumeLoaderBase<uint16_t>;
template VolumeLoaderBase<uint32_t>;
template VolumeLoaderBase<uint64_t>;
template VolumeLoaderBase<float>;
template VolumeLoaderBase<double>;
template VolumeLoaderBase<int8_t>;
template VolumeLoaderBase<int16_t>;
template VolumeLoaderBase<int32_t>;
template VolumeLoaderBase<int64_t>;