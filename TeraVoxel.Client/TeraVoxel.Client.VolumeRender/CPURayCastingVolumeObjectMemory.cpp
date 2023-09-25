/*
 * Author: Jan Svoboda
 * University: BRNO UNIVERSITY OF TECHNOLOGY, FACULTY OF INFORMATION TECHNOLOGY
 */
#include "CPURayCastingVolumeObjectMemory.h"
#include "../TeraVoxel.Client.Core/Logger.h"


template<typename T>
CPURayCastingVolumeObjectMemory<T>::~CPURayCastingVolumeObjectMemory()
{
	delete _volumeLoader.release();

	ProcessDelete(_volumes);

	ProcessDelete(_lowResolutionVolumes);

	ProcessDelete(_volumesToDelete);
}

template <typename T>
CPURayCastingVolumeObjectMemory<T>::CPURayCastingVolumeObjectMemory(const std::shared_ptr<Camera>& camera, const ProjectInfo& projectInfo, const std::shared_ptr<VolumeLoaderFactory<T>>& volumeLoaderFactory)
{
	_projectInfo = projectInfo;
	_camera = camera;
	xSegmentCount = projectInfo.sizeX / projectInfo.segmentSize;
	ySegmentCountCount = projectInfo.sizeY / projectInfo.segmentSize;
	zSegmentCount = projectInfo.sizeZ / projectInfo.segmentSize;
	_oneDivSegmentSize = 1.0 / _projectInfo.segmentSize;
	_segmentSize = _projectInfo.segmentSize;
	_segmentSizeShifter = (int)(log2(_segmentSize) + 0.5);
	_volumeLoader = volumeLoaderFactory->Create(projectInfo, SettingsContext::GetInstance().loadingThreadCount);

	_segmentCount = (uint64_t)xSegmentCount * (uint64_t)ySegmentCountCount * (uint64_t)zSegmentCount;
	_volumes.resize(_segmentCount);

	_lowResolutionVolumes.resize(_segmentCount);
	maxSegmentIndex = xSegmentCount * ySegmentCountCount * zSegmentCount - 1;

	// Preload low quality segments
	Preload(SettingsContext::GetInstance().preloadingThreadCount.load(std::memory_order::acquire));
	_volumeLoader->BindOnSegmentLoaded([this]() { _memoryChanged.store(true); });
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
		int_fast16_t segmentIndex = volume->x + volume->y * xSegmentCount + volume->z * xSegmentCount * ySegmentCountCount;
		_volumes[segmentIndex] = new VolumeSegment<T>(volume->x, volume->y, volume->z);
		_lowResolutionVolumes[segmentIndex] = volume.release();
	}
}

template <typename T>
__forceinline int CPURayCastingVolumeObjectMemory<T>::GetRequiredDownscale(int xIndex, int yIndex, int zIndex)
{
	auto vecPos = _camera->DeshrinkVector(Vector3f(xIndex, yIndex, zIndex));
	auto dist = (vecPos - _camera->GetPosition()).norm(); // Absolute distance
	float quality = _camera->GetScreenSize().x() / (2 * tanf(_camera->GetViewAngle() * 0.5) * dist) * _camera->GetMaxVoxelSize();
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
void CPURayCastingVolumeObjectMemory<T>::ProcessDelete(std::vector<VolumeSegment<T>*>& volumes)
{
	int size = volumes.size();
	for (size_t i = 0; i < size; i++)
	{
		auto volume = volumes.back();
		if (volume->data != nullptr)
		{
			MemoryContext::GetInstance().memoryInfoWriteMutex.lock();
			MemoryContext::GetInstance().usedMemory -= GetBlockRequiredMemory(volume->actualDownscale);
			MemoryContext::GetInstance().memoryInfoWriteMutex.unlock();
		}
		delete volume;
		volumes.pop_back();
	}
}

template <typename T>
void CPURayCastingVolumeObjectMemory<T>::Revalidate()
{
	bool ramAlmostFull = MemoryContext::GetInstance().usedMemory.load(std::memory_order::acquire) + 50000000 > MemoryContext::GetInstance().maxMemory.load(std::memory_order_acquire);

	for (size_t z = 0; z < zSegmentCount; z++)
	{
		for (size_t y = 0; y < ySegmentCountCount; y++)
		{
			for (size_t x = 0; x < xSegmentCount; x++)
			{
				auto index = x + y * xSegmentCount + z * xSegmentCount * ySegmentCountCount;
				VolumeSegment<T>* vol = _volumes[index];


				if (vol->used.load(std::memory_order_relaxed))
				{
					int lastRequiredDownscale = GetRequiredDownscale((x << _segmentSizeShifter) + _segmentSize / 2, (y << _segmentSizeShifter) + _segmentSize / 2, (z << _segmentSizeShifter) + _segmentSize / 2);

					// Important for refresh after zooming on the loaded scene -> step depend on quality
					if (vol->lastRequiredDownscale != lastRequiredDownscale)
					{
						vol->lastRequiredDownscale = lastRequiredDownscale;
						_memoryChanged.store(true, std::memory_order_release);
					}

					if (vol->actualDownscale > lastRequiredDownscale && !ramAlmostFull)
					{
						if (vol->waitsToBeReloaded.load(std::memory_order::acquire))
						{
							if (vol->futureDownscale > lastRequiredDownscale)
							{
								vol->futureDownscale.store(lastRequiredDownscale, std::memory_order::release);
							}
						}
						else
						{
							vol->futureDownscale.store(lastRequiredDownscale, std::memory_order::release);
							_volumeLoader->AddToStack(vol);
						}
					}
				}
			}
		}
	}

	int segmentCount = xSegmentCount * ySegmentCountCount * zSegmentCount;

	// Downscale N of segments with higher quality
	if (ramAlmostFull)
	{
		DownscaleWithHigherQuality(10);
	}

	ramAlmostFull = MemoryContext::GetInstance().usedMemory.load(std::memory_order::relaxed) + 50000000 > MemoryContext::GetInstance().maxMemory.load(std::memory_order::acquire);

	// Delete N of not used segments
	if (ramAlmostFull)
	{
		DeleteNotUsed(10);
	}

	// Usage variables setting
	for (size_t z = 0; z < zSegmentCount; z++)
	{
		for (size_t y = 0; y < ySegmentCountCount; y++)
		{
			for (size_t x = 0; x < xSegmentCount; x++)
			{
				auto index = x + y * xSegmentCount + z * xSegmentCount * ySegmentCountCount;
				VolumeSegment<T>* vol = _volumes[index];

				if (!vol->used.load(std::memory_order_acquire))
				{
					vol->unusedCount.fetch_add(1, std::memory_order_acq_rel);
				}
				else
				{
					vol->unusedCount.store(0, std::memory_order_release);
					vol->used.store(false, std::memory_order_release);
				}
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
		int_fast16_t segmentIndex = volume->x + volume->y * xSegmentCount + volume->z * xSegmentCount * ySegmentCountCount;
		if (_volumes[segmentIndex] != nullptr)
		{
			_volumesToDelete.push_back(_volumes[segmentIndex]);
		}
		_volumes[segmentIndex] = volume.release();
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
		VolumeSegment<T>* vol = _volumes[i];

		if (vol->data != nullptr)
		{
			if (!vol->used.load(std::memory_order_acquire) && !vol->waitsToBeReloaded.load(std::memory_order_acquire))
			{
				_volumesToDeleteMutex.lock();
				auto newVolume = new VolumeSegment<T>(vol->x, vol->y, vol->z);
				_volumesToDelete.push_back(vol);
				_volumes[i] = newVolume;
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
		VolumeSegment<T>* vol = _volumes[i];
		if (vol->data != nullptr)
		{
			// The quality is better than we need 
			if (!vol->waitsToBeReloaded.load(std::memory_order_acquire) && vol->actualDownscale < vol->lastRequiredDownscale)
			{
				// NOT BEING RELOADED -> only this thread has access to this block

				// Downscale a block to the required quality
				int downscaledSegmentSize = _segmentSize >> vol->lastRequiredDownscale;
				T* downscaledData = new T[downscaledSegmentSize * downscaledSegmentSize * downscaledSegmentSize];
				int actualSegmentSize = _segmentSize >> vol->actualDownscale;
				int downscaledVoxelCount = downscaledSegmentSize * downscaledSegmentSize * downscaledSegmentSize;
				int downscale = vol->lastRequiredDownscale - vol->actualDownscale;
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
				MemoryContext::GetInstance().usedMemory -= GetBlockRequiredMemory(vol->actualDownscale) - GetBlockRequiredMemory(vol->lastRequiredDownscale);
				MemoryContext::GetInstance().memoryInfoWriteMutex.unlock();

				// Swapping data
				delete vol->data;
				vol->data = downscaledData;
				vol->actualDownscale = vol->lastRequiredDownscale;
				vol->futureDownscale = vol->lastRequiredDownscale;

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
	uint_fast32_t segmentIndex = xSegment + ySegent * xSegmentCount + zSegment * xSegmentCount * ySegmentCountCount;

	VolumeSegment<T>* volume = nullptr;
	volume = _volumes[segmentIndex];

	if (!volume->used.load(std::memory_order::acquire))  // This condition is important for optimalization
	{
		volume->used.store(true, std::memory_order::seq_cst); // seq_cst cause faster change in all threads -> less calling
	}

	if (volume->data == nullptr)
	{
		volume = _lowResolutionVolumes[segmentIndex]; // If a volume segment is not loaded we use a low resolution one
	}

	downscale = volume->actualDownscale;

	auto index = Serialization::GetZCurveIndex(
		(xIndex % _segmentSize) >> downscale, // >> downscale ==  / 2^downscale
		(yIndex % _segmentSize) >> downscale,
		(zIndex % _segmentSize) >> downscale
	);

	return volume->data[index];
}

template <typename T>
ProjectInfo CPURayCastingVolumeObjectMemory<T>::GetProjectInfo()
{
	return _projectInfo;
}

template<typename T>
std::array<int, 3> CPURayCastingVolumeObjectMemory<T>::GetDataSizes()
{
	return { _projectInfo.dataSizeX, _projectInfo.dataSizeY, _projectInfo.dataSizeZ };
}

template CPURayCastingVolumeObjectMemory<uint8_t>;
template CPURayCastingVolumeObjectMemory<uint16_t>;
template CPURayCastingVolumeObjectMemory<uint32_t>;
template CPURayCastingVolumeObjectMemory<uint64_t>;
template CPURayCastingVolumeObjectMemory<float>;
template CPURayCastingVolumeObjectMemory<double>;
template CPURayCastingVolumeObjectMemory<int8_t>;
template CPURayCastingVolumeObjectMemory<int16_t>;
template CPURayCastingVolumeObjectMemory<int32_t>;
template CPURayCastingVolumeObjectMemory<int64_t>;
