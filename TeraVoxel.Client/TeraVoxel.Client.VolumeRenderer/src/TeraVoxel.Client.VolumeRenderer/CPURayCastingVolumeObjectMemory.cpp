/*
 * Author: Jan Svoboda
 * University: BRNO UNIVERSITY OF TECHNOLOGY, FACULTY OF INFORMATION TECHNOLOGY
 */
#include "TeraVoxel.Client.VolumeRenderer/CPURayCastingVolumeObjectMemory.h"
#include <TeraVoxel.Client.Core/Logger.h>

template<typename T>
CPURayCastingVolumeObjectMemory<T>::~CPURayCastingVolumeObjectMemory()
{
	delete _volumeLoader.release();

	ProcessDelete(_volumes);

	ProcessDelete(_lowResolutionVolumes);

	ProcessDelete(_volumesToDelete);
}

template <typename T>
CPURayCastingVolumeObjectMemory<T>::CPURayCastingVolumeObjectMemory(const std::shared_ptr<Camera>& camera, const std::shared_ptr<VolumeLoaderFactory>& volumeLoaderFactory)
{
	_datasetInfo = volumeLoaderFactory->GetDatasetInfo();
	_camera = camera;
	xSegmentCount = _datasetInfo.sizeX / _datasetInfo.segmentSize;
	ySegmentCount = _datasetInfo.sizeY / _datasetInfo.segmentSize;
	zSegmentCount = _datasetInfo.sizeZ / _datasetInfo.segmentSize;
	_oneDivSegmentSize = 1.0 / _datasetInfo.segmentSize;
	_segmentSize = _datasetInfo.segmentSize;
	_segmentSizeShifter = (int)(log2(_segmentSize) + 0.5);
	_volumeLoader = std::unique_ptr<VolumeLoaderBaseGenericBase<T>>(dynamic_cast<VolumeLoaderBaseGenericBase<T>*>(volumeLoaderFactory->Create(SettingsContext::GetInstance().loadingThreadCount).release()));

	_segmentCount = (uint64_t)xSegmentCount * (uint64_t)ySegmentCount * (uint64_t)zSegmentCount;
	_volumes.resize(_segmentCount);
	_tickets.resize(_segmentCount, nullptr);

	_used = std::vector<std::atomic<bool>>(_segmentCount);
	_lowResolutionVolumes.resize(_segmentCount);
	maxSegmentIndex = xSegmentCount * ySegmentCount * zSegmentCount - 1;

	// Preload low quality segments
	Preload(SettingsContext::GetInstance().preloadingThreadCount.load(std::memory_order::acquire));
	_volumeLoader->BindOnBlockLoaded([this]() { _memoryChanged.store(true); return true; });
}

template <typename T>
long CPURayCastingVolumeObjectMemory<T>::GetBlockRequiredMemory(int downscale)
{
	short downscaleDividerReq = (short)pow(2, downscale);
	return (long)pow((_segmentSize / downscaleDividerReq), 3) * sizeof(T);
}

template <typename T>
void CPURayCastingVolumeObjectMemory<T>::Preload(short threadCount)
{
	_volumeLoader->Preload(3, threadCount);

	int count;
	while (true)
	{
		auto volume = _volumeLoader->TakeFirstLoaded(count);
		if (count == 0)
		{
			break;
		}
		int_fast16_t segmentIndex = volume->x + volume->y * xSegmentCount + volume->z * xSegmentCount * ySegmentCount;

		_volumes[segmentIndex] = nullptr;
		_lowResolutionVolumes[segmentIndex] = volume.release();
	}
}

template <typename T>
__forceinline float CPURayCastingVolumeObjectMemory<T>::GetPriority(int xIndex, int yIndex, int zIndex)
{
	auto vecPos = _camera->DeshrinkVector(Vector3f(xIndex, yIndex, zIndex) + Vector3f(0.5f, 0.5f, 0.5f));
	return (vecPos - _camera->GetShrankPosition()).norm();
}

template <typename T>
__forceinline int CPURayCastingVolumeObjectMemory<T>::GetRequiredDownscale(int xIndex, int yIndex, int zIndex)
{
	auto vecPos = _camera->DeshrinkVector(Vector3f(xIndex, yIndex, zIndex) + Vector3f(0.5f, 0.5f, 0.5f));
	auto dist = (vecPos - _camera->GetPosition()).norm();
	float quality = _camera->GetScreenSize().x() / (2 * tanf(_camera->GetViewAngle() * 0.5) * dist) * _camera->GetVoxelSizeMean();
	auto actualDownscale = 0;

	if (quality <= 0.125)
	{
		actualDownscale = 3;
	}
	else if (quality <= 0.25)
	{
		actualDownscale = 2;
	}
	else if (quality <= 0.5)
	{
		actualDownscale = 1;
	}
	return actualDownscale;
}

template <typename T>
void CPURayCastingVolumeObjectMemory<T>::ProcessDelete(std::vector<VolumeBlock<T>*>& volumes)
{
	int size = volumes.size();
	for (size_t i = 0; i < size; i++)
	{		
		auto volume = volumes.back();
		if (volume == nullptr)
		{
			continue;
		}
		MemoryContext::GetInstance().memoryInfoWriteMutex.lock();
		MemoryContext::GetInstance().usedMemory -= GetBlockRequiredMemory(volume->downscale);
		MemoryContext::GetInstance().memoryInfoWriteMutex.unlock();
		delete volume;
		volumes.pop_back();
	}
}

template <typename T>
void CPURayCastingVolumeObjectMemory<T>::Revalidate()
{
	bool ramAlmostFull = MemoryContext::GetInstance().usedMemory.load(std::memory_order::acquire) + 140000000 > MemoryContext::GetInstance().maxMemory.load(std::memory_order_acquire);
	int countOfOkQuality= 0;
	int countVisible = 0;

	for (size_t z = 0; z < zSegmentCount; z++)
	{
		for (size_t y = 0; y < ySegmentCount; y++)
		{
			for (size_t x = 0; x < xSegmentCount; x++)
			{
				auto index = x + y * xSegmentCount + z * xSegmentCount * ySegmentCount;
				VolumeBlock<T>* vol = _volumes[index];

				int requiredDownscale = GetRequiredDownscale((x << _segmentSizeShifter) + _segmentSize / 2, (y << _segmentSizeShifter) + _segmentSize / 2, (z << _segmentSizeShifter) + _segmentSize / 2);


				auto ticket = _tickets[index];

				// Volume not updated yet
				if (ticket != nullptr && ticket->state == RequestState::Loaded)
				{
					continue;
				}

				if (ticket != nullptr && (_tickets[index]->state == RequestState::UnableToLoadOrSkipped))
				{
					_tickets[index] = nullptr;
					ticket = nullptr;
				}

				if (!ramAlmostFull) 
				{
					if (ticket == nullptr)
					{
						if (vol == nullptr && _used[index].load(std::memory_order_relaxed)
							|| (vol != nullptr && _used[index].load(std::memory_order_relaxed) && vol->downscale > requiredDownscale))
						{
							float priority = GetPriority((x << _segmentSizeShifter) + _segmentSize / 2, (y << _segmentSizeShifter) + _segmentSize / 2, (z << _segmentSizeShifter) + _segmentSize / 2);

							_tickets[index] = _volumeLoader->LoadAsync(x, y, z, requiredDownscale, priority);
						}
					}
					else
					{
						float priority = GetPriority((x << _segmentSizeShifter) + _segmentSize / 2, (y << _segmentSizeShifter) + _segmentSize / 2, (z << _segmentSizeShifter) + _segmentSize / 2);

						ticket->mutex.lock();

						ticket->needed = _used[index].load(std::memory_order_relaxed) && (vol == nullptr || vol->downscale > requiredDownscale);
						ticket->downscale = requiredDownscale;
						ticket->priority = priority;

						ticket->mutex.unlock();
					}
				}
				
			}
		}
	}

	int segmentCount = xSegmentCount * ySegmentCount * zSegmentCount;

	Logger::GetInstance()->LogEvent("VolumeObjectMemory", "QualityInfo", std::to_string(countOfOkQuality / (double)countVisible));

	// Downscale N of segments with higher quality
	if (ramAlmostFull)
	{
		DownscaleWithHigherQuality(10);
	}

	ramAlmostFull = MemoryContext::GetInstance().usedMemory.load(std::memory_order::relaxed) + 140000000 > MemoryContext::GetInstance().maxMemory.load(std::memory_order::acquire);

	// Delete N of not used segments
	if (ramAlmostFull)
	{
		DeleteNotUsed(10);
	}

	// Usage variables setting
	for (size_t z = 0; z < zSegmentCount; z++)
	{
		for (size_t y = 0; y < ySegmentCount; y++)
		{
			for (size_t x = 0; x < xSegmentCount; x++)
			{
				auto index = x + y * xSegmentCount + z * xSegmentCount * ySegmentCount;

				_used[index].store(false, std::memory_order_release);
			}
		}
	}

	ProcessDelete(_volumesToDelete);
	std::cout << "Used data memory:" << MemoryContext::GetInstance().usedMemory.load(std::memory_order::relaxed) << " B \n";
}

template<typename T>
void CPURayCastingVolumeObjectMemory<T>::Prepare()
{
	int count;
	while (true)
	{
		auto volume = _volumeLoader->TakeFirstLoaded(count);
		if (count == 0)
		{
			break;
		}
		int_fast16_t segmentIndex = volume->x + volume->y * xSegmentCount + volume->z * xSegmentCount * ySegmentCount;
		if (_volumes[segmentIndex] != nullptr)
		{
			_volumesToDelete.push_back(_volumes[segmentIndex]);
		}
		_volumes[segmentIndex] = volume.release();
		_tickets[segmentIndex] = nullptr;
	}
}

template<typename T>
void CPURayCastingVolumeObjectMemory<T>::FlushCachedData()
{
	ProcessDelete(_volumes);
}

template <typename T>
void CPURayCastingVolumeObjectMemory<T>::DeleteNotUsed(int maxCount)
{
	int count = 0;
	for (size_t i = 0; i < _segmentCount; i++)
	{
		VolumeBlock<T>* vol = _volumes[i];

		if (vol != nullptr)
		{
			if (!_used[i].load(std::memory_order_acquire))
			{
				_volumesToDeleteMutex.lock();
				_volumesToDelete.push_back(vol);
				_volumes[i] = nullptr;
				_volumesToDeleteMutex.unlock();

				count++;
				if (count == maxCount)
				{
					break;
				}

				_memoryChanged.store(true, std::memory_order_release);
			}
		}
	}
}

template <typename T>
void CPURayCastingVolumeObjectMemory<T>::DownscaleWithHigherQuality(int maxCount)
{
	int count = 0;
	for (size_t i = 0; i < _segmentCount; i++)
	{
		VolumeBlock<T>* vol = _volumes[i];
		if (vol != nullptr)
		{
			Eigen::Vector3i XYZ = Serialization::IdxToXYZ(i, Eigen::Vector3i(xSegmentCount, ySegmentCount, zSegmentCount));
			auto requiredDownscale = GetRequiredDownscale((XYZ.x() << _segmentSizeShifter) + _segmentSize / 2, (XYZ.y() << _segmentSizeShifter) + _segmentSize / 2, (XYZ.z() << _segmentSizeShifter) + _segmentSize / 2);
			// The quality is better than we need 
			if (vol->downscale < requiredDownscale)
			{
				// NOT BEING RELOADED -> only this thread has access to this block

				// Downscale a block to the required quality
				int downscaledSegmentSize = _segmentSize >> requiredDownscale;
				T* downscaledData = new T[downscaledSegmentSize * downscaledSegmentSize * downscaledSegmentSize];
				int actualSegmentSize = _segmentSize >> vol->downscale;
				int downscaledVoxelCount = downscaledSegmentSize * downscaledSegmentSize * downscaledSegmentSize;
				int downscale = requiredDownscale - vol->downscale;
				int downscaleVoxCount = (size_t)1 << downscale; // 1 << downscale == 2 ^ downscale
				downscaleVoxCount = downscaleVoxCount * downscaleVoxCount * downscaleVoxCount;

				// Downscaling in Z-curve
				for (size_t j = 0; j < downscaledVoxelCount; j++)
				{
					double sum = 0;
					for (size_t k = 0; k < downscaleVoxCount; k++)
					{
						sum += vol->data[j * downscaleVoxCount + k];
					}

					downscaledData[j] = sum / downscaleVoxCount;
				}

				// Recomputing RAM 
				MemoryContext::GetInstance().memoryInfoWriteMutex.lock();
				MemoryContext::GetInstance().usedMemory -= GetBlockRequiredMemory(vol->downscale) - GetBlockRequiredMemory(requiredDownscale);
				MemoryContext::GetInstance().memoryInfoWriteMutex.unlock();

				// Swapping data
				delete vol->data;
				vol->data = downscaledData;
				vol->downscale = requiredDownscale;

				count++;
				if (count == maxCount)
				{
					break;
				}

				_memoryChanged.store(true, std::memory_order_release);
			}
		}
	}
}

template <typename T>
__forceinline T CPURayCastingVolumeObjectMemory<T>::GetValue(uint_fast16_t xIndex, uint_fast16_t yIndex, uint_fast16_t zIndex, int& downscale)
{
	uint_fast16_t xSegment = xIndex >> _segmentSizeShifter;
	uint_fast16_t ySegent = yIndex >> _segmentSizeShifter;
	uint_fast16_t zSegment = zIndex >> _segmentSizeShifter;
	uint_fast32_t segmentIndex = xSegment + ySegent * xSegmentCount + zSegment * xSegmentCount * ySegmentCount;

	VolumeBlock<T>* volume = nullptr;
	volume = _volumes[segmentIndex];

	if (!_used[segmentIndex].load(std::memory_order::acquire))  // This condition is important for optimalization
	{
		_used[segmentIndex].store(true, std::memory_order::seq_cst); // seq_cst cause faster change in all threads -> less calling
	}

	if (volume == nullptr)
	{
		volume = _lowResolutionVolumes[segmentIndex]; // If a volume segment is not loaded we use a low resolution one
	}

	downscale = volume->downscale;

	auto index = Serialization::GetZCurveIndex(
		(xIndex % _segmentSize) >> downscale, // >> downscale ==  / 2^downscale
		(yIndex % _segmentSize) >> downscale,
		(zIndex % _segmentSize) >> downscale
	);

	return volume->data[index];
}

template <typename T>
DatasetInfo CPURayCastingVolumeObjectMemory<T>::GetDatasetInfo()
{
	return _datasetInfo;
}

template<typename T>
std::array<int, 3> CPURayCastingVolumeObjectMemory<T>::GetDataSizes()
{
	return { _datasetInfo.dataSizeX, _datasetInfo.dataSizeY, _datasetInfo.dataSizeZ };
}

template CPURayCastingVolumeObjectMemory<uint8_t>;
template CPURayCastingVolumeObjectMemory<uint16_t>;
template CPURayCastingVolumeObjectMemory<uint32_t>;
template CPURayCastingVolumeObjectMemory<uint64_t>;
template CPURayCastingVolumeObjectMemory<float>;
template CPURayCastingVolumeObjectMemory<int8_t>;
template CPURayCastingVolumeObjectMemory<int16_t>;
template CPURayCastingVolumeObjectMemory<int32_t>;
template CPURayCastingVolumeObjectMemory<int64_t>;
