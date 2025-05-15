/*
 * Author: Jan Svoboda
 * University: BRNO UNIVERSITY OF TECHNOLOGY, FACULTY OF INFORMATION TECHNOLOGY
 */
#include "TeraVoxel.Client.VolumeRenderer/VolumeLoaderGenericBase.h"

template<typename T>
VolumeLoaderBaseGenericBase<T>::VolumeLoaderBaseGenericBase(const BlockBasedDatasetInfo& datasetInfo, int threadCount) : VolumeLoaderBase(datasetInfo)
{
	_segmentCountX = datasetInfo.sizeX / datasetInfo.segmentSize;
	_segmentCountY = datasetInfo.sizeY / datasetInfo.segmentSize;
	_segmentCountZ = datasetInfo.sizeZ / datasetInfo.segmentSize;
	_threadCount = threadCount;

	for (size_t i = 0; i < threadCount; i++)
	{
		_loadingTreads.push_back(std::async(std::launch::async, &VolumeLoaderBaseGenericBase::LoadingTask, this));
	}
}

template<typename T>
VolumeLoaderBaseGenericBase<T>::~VolumeLoaderBaseGenericBase()
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
void VolumeLoaderBaseGenericBase<T>::LoadingTask()
{
	std::shared_ptr<VolumeBlockRequestTicket> loadingTicket;

	while (!_endLoopingThreads)
	{
		_ticketsMutex.lock();
		if (_tickets.size() > 0)
		{			
			std::shared_ptr<VolumeBlockRequestTicket> minPriorityTicket = _tickets.front();
			for (std::shared_ptr<VolumeBlockRequestTicket>& ticket : _tickets)
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
						data = LoadBlockData(coords.x(), coords.y(), coords.z(), loadingDownscale);
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

				VolumeBlock<T>* newVolume = new VolumeBlock<T>(coords.x(), coords.y(), coords.z());
				// New segment inicialization
				newVolume->data = data;
				newVolume->downscale = loadingDownscale;

				_loadedSegmentsMutex.lock();
				_loadedSegments.push(std::unique_ptr<VolumeBlock<T>>(newVolume));
				_loadedSegmentsMutex.unlock();

				bool success = _onSegmentLoaded();

				loadingTicket->mutex.lock();
				loadingTicket->state = success ? RequestState::Loaded : RequestState::UnableToLoadOrSkipped;
				loadingTicket->mutex.unlock();
			}
			else
			{
				loadingTicket->mutex.lock();
				loadingTicket->state = RequestState::UnableToLoadOrSkipped;
				loadingTicket->mutex.unlock();
				MemoryContext::GetInstance().memoryInfoWriteMutex.unlock();
			}
		}
	}
}

template<typename T>
std::shared_ptr<VolumeBlockRequestTicket> VolumeLoaderBaseGenericBase<T>::LoadAsync(int x, int y, int z, int downscale, float priority)
{
	_ticketsMutex.lock();

	auto ticket = std::make_shared<VolumeBlockRequestTicket>();
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
void VolumeLoaderBaseGenericBase<T>::Preload(int downscale, int threadCount)
{
	std::vector<std::future<void>> threads;
	for (size_t i = 0; i < threadCount; i++)
	{
		threads.push_back(std::async(std::launch::async, &VolumeLoaderBaseGenericBase::PreloadTask, this, i, threadCount, downscale));
	}
	for (size_t i = 0; i < threadCount; i++)
	{
		threads[i].get();
	}
}

template<typename T>
std::unique_ptr<VolumeBlock<T>> VolumeLoaderBaseGenericBase<T>::TakeFirstLoaded(int& count)
{
	_loadedSegmentsMutex.lock();

	count = _loadedSegments.size();
	std::unique_ptr<VolumeBlock<T>> value(nullptr);

	if (count > 0)
	{
		value = std::move(_loadedSegments.front());
		_loadedSegments.pop();
	}

	_loadedSegmentsMutex.unlock();
	return value;
}

template<typename T>
std::unique_ptr<VolumeBlock<T>> VolumeLoaderBaseGenericBase<T>::LoadSync(int x, int y, int z, int downscale)
{
	auto volume = std::make_unique<VolumeBlock<T>>(x, y, z);
	volume->downscale = downscale;

	for (size_t i = 0; i < 100; i++)
	{
		try
		{
			volume->data = LoadBlockData(x, y, z, downscale);			

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
void VolumeLoaderBaseGenericBase<T>::PreloadTask(short threadIndex, short threadCount, int downscale)
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
		auto volume = new VolumeBlock<T>(x, y, z);


		for (size_t i = 0; i < 100; i++)
		{
			try
			{
				volume->data = LoadBlockData(x, y, z, downscale);
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
		_loadedSegments.push(std::unique_ptr<VolumeBlock<T>>(volume));
		_loadedSegmentsMutex.unlock();
	}
}

template<typename T>
uint64_t VolumeLoaderBaseGenericBase<T>::GetBlockRequiredMemory(int downscale)
{
	short downscaleDividerReq = (short)pow(2, downscale);
	return (uint64_t)pow((_datasetInfo.segmentSize / downscaleDividerReq), 3) * sizeof(T);
}

template VolumeLoaderBaseGenericBase<uint8_t>;
template VolumeLoaderBaseGenericBase<uint16_t>;
template VolumeLoaderBaseGenericBase<uint32_t>;
template VolumeLoaderBaseGenericBase<uint64_t>;
template VolumeLoaderBaseGenericBase<float>;
template VolumeLoaderBaseGenericBase<int8_t>;
template VolumeLoaderBaseGenericBase<int16_t>;
template VolumeLoaderBaseGenericBase<int32_t>;
template VolumeLoaderBaseGenericBase<int64_t>;