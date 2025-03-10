#include "TeraVoxel.Client.VolumeRenderer/VolumeLoaderBase.h"

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
			-= GetBlockRequiredMemory(_loadedSegments.front()->downscale);
		MemoryContext::GetInstance().memoryInfoWriteMutex.unlock();

		_loadedSegments.pop();
	}
	_loadedSegmentsMutex.unlock();
}

template <typename T>
void VolumeLoaderBase<T>::LoadingTask()
{
	std::shared_ptr<VolumeSegmentRequestTicket> loadingTicket;

	while (!_endLoopingThreads)
	{
		_ticketsMutex.lock();
		if (_tickets.size() > 0)
		{			
			std::shared_ptr<VolumeSegmentRequestTicket> minPriorityTicket = _tickets.front();
			for (std::shared_ptr<VolumeSegmentRequestTicket>& ticket : _tickets)
			{
				ticket->mutex.lock();

				if (ticket->priority < minPriorityTicket->priority)
				{
					minPriorityTicket = ticket;
				}

				ticket->mutex.unlock();
			}
			loadingTicket = minPriorityTicket;
			_tickets.remove(minPriorityTicket);
		}
		else
		{
			loadingTicket = nullptr;
		}
		_ticketsMutex.unlock();

		if (loadingTicket == nullptr)
		{
			std::this_thread::sleep_for(std::chrono::milliseconds(100));
		}
		else
		{
			loadingTicket->mutex.lock();

			loadingTicket->state = RequestState::BeingLoaded;
			auto loadingDownscale = loadingTicket->downscale;
			bool needed = loadingTicket->needed;
			Eigen::Vector3i coords = loadingTicket->coordinates;
			auto requiredMemory = GetBlockRequiredMemory(loadingDownscale);

			loadingTicket->mutex.unlock();

			MemoryContext::GetInstance().memoryInfoWriteMutex.lock();
			if (needed && (requiredMemory + MemoryContext::GetInstance().usedMemory.load(std::memory_order::acquire) <= MemoryContext::GetInstance().maxMemory.load(std::memory_order::acquire)))
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
						data = LoadSegmentData(coords.x(), coords.y(), coords.z(), loadingDownscale);
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

				VolumeSegment<T>* newVolume = new VolumeSegment<T>(coords.x(), coords.y(), coords.z());
				// New segment inicialization
				newVolume->data = data;
				newVolume->downscale = loadingDownscale;

				_loadedSegmentsMutex.lock();
				_loadedSegments.push(std::unique_ptr<VolumeSegment<T>>(newVolume));
				_loadedSegmentsMutex.unlock();

				_onSegmentLoaded();

				loadingTicket->mutex.lock();
				loadingTicket->state = RequestState::Loaded;
				loadingTicket->mutex.unlock();
			}
			else
			{
				loadingTicket->mutex.lock();
				loadingTicket->state = RequestState::UnableToLoad;
				loadingTicket->mutex.unlock();
				MemoryContext::GetInstance().memoryInfoWriteMutex.unlock();
			}
		}
	}
}

template<typename T>
std::shared_ptr<VolumeSegmentRequestTicket> VolumeLoaderBase<T>::LoadAsync(int x, int y, int z, int downscale, float priority)
{
	_ticketsMutex.lock();

	auto ticket = std::make_shared<VolumeSegmentRequestTicket>();
	ticket->coordinates = Eigen::Vector3i(x, y, z);
	ticket->downscale = downscale;
	ticket->needed = true;
	ticket->priority = priority;
	ticket->state = RequestState::WaitingToBeLoaded;

	_tickets.push_back(ticket);
	_ticketsMutex.unlock();

	return ticket;
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
		_loadedSegments.pop();
	}

	_loadedSegmentsMutex.unlock();
	return value;
}

template<typename T>
std::unique_ptr<VolumeSegment<T>> VolumeLoaderBase<T>::LoadSync(int x, int y, int z, int downscale)
{
	auto volume = std::make_unique<VolumeSegment<T>>(x, y, z);
	volume->downscale = downscale;

	for (size_t i = 0; i < 100; i++)
	{
		try
		{
			volume->data = LoadSegmentData(x, y, z, downscale);			

			uint64_t requiredMemory = GetBlockRequiredMemory(downscale);
			MemoryContext::GetInstance().memoryInfoWriteMutex.lock();
			MemoryContext::GetInstance().usedMemory += requiredMemory;
			MemoryContext::GetInstance().memoryInfoWriteMutex.unlock();

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

	return volume;
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
				volume->data = LoadSegmentData(x, y, z, downscale);
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

		volume->downscale = downscale;

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