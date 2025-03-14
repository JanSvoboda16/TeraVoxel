/*
 * Author: Jan Svoboda
 * University: BRNO UNIVERSITY OF TECHNOLOGY, FACULTY OF INFORMATION TECHNOLOGY
 */

#pragma once
#include <memory>
#include <map>
#include "TeraVoxel.Client.VolumeRenderer/VolumeCacheGenericBase.h"
#include "TeraVoxel.Client.VolumeRenderer/VolumeLoaderBase.h"
#include "TeraVoxel.Client.VolumeRenderer/VolumeLoaderFactory.h"
#include <TeraVoxel.Client.Core/TemplatedFunctionCaller.h>
#include <NeuroVoxel/Common/DataType.h>

template <typename T>
class VolumeCache : public VolumeCacheGenericBase
{
public:
	VolumeCache(const std::shared_ptr<VolumeLoaderFactory> &loaderFac) : 
		_volumeLoader(std::dynamic_pointer_cast<VolumeLoaderBase<T>>(std::shared_ptr<VolumeLoaderGenericBase>(loaderFac->Create(1)))), 
		VolumeCacheGenericBase(loaderFac->GetDatasetInfo()) { }

	~VolumeCache() {
		Flush();
	}

	T GetValue(size_t x, size_t y, size_t z) 
	{
		auto datasetInfo = _volumeLoader->GetDatasetInfo();
		T value;
		uint16_t blockIdX = x / datasetInfo.segmentSize;
		uint16_t blockIdY = y / datasetInfo.segmentSize;
		uint16_t blockIdZ = z / datasetInfo.segmentSize;

		int xb = x % datasetInfo.segmentSize;
		int yb = y % datasetInfo.segmentSize;
		int zb = z % datasetInfo.segmentSize;

		int key = blockIdX + blockIdY * datasetInfo.segmentSize + blockIdZ * datasetInfo.segmentSize * datasetInfo.segmentSize;

		std::shared_ptr<VolumeSegment<T>> block = nullptr;
		if (_lastSegmentId == key) 
		{
			block = _lastSegment;
		}
		else if (_cache.contains(key))
		{
			block = _cache[key];
			_lastSegment = block;			
			_lastSegmentId = key;
		}
		else
		{
			block = std::shared_ptr<VolumeSegment<T>>(_volumeLoader->LoadSync(blockIdX, blockIdY, blockIdZ, 0));
			_cache.emplace(key, block);
			_lastSegment = block;
			_lastSegmentId = key;
		}	

		value = block->data[Serialization::GetZCurveIndex(xb, yb, zb)];

		return value;
	}	

	void Flush() override 
	{
		auto datasetInfo = _volumeLoader->GetDatasetInfo();
		MemoryContext::GetInstance().memoryInfoWriteMutex.lock();
		MemoryContext::GetInstance().usedMemory -= _cache.size() * datasetInfo.segmentSize * datasetInfo.segmentSize * datasetInfo.segmentSize * sizeof(T);
		MemoryContext::GetInstance().memoryInfoWriteMutex.unlock();
		_cache.clear();
	}

private:
	std::shared_ptr<VolumeLoaderBase<T>> _volumeLoader;
	std::map<int, std::shared_ptr<VolumeSegment<T>>> _cache;

	int _lastSegmentId = -1;
	std::shared_ptr<VolumeSegment<T>> _lastSegment = nullptr;

};


class VolumeCacheFactory {
public:
	VolumeCacheFactory() = delete;

	static std::unique_ptr<VolumeCacheGenericBase> VolumeCacheCreate(const std::shared_ptr<VolumeLoaderFactory>& loaderFac) {
		return CALL_TEMPLATED_FUNCTION2(VolumeCacheCreateTemplated, loaderFac->GetDatasetInfo().dataType, loaderFac);
	}
private:
	template <typename T>
	static std::unique_ptr<VolumeCacheGenericBase> VolumeCacheCreateTemplated(const std::shared_ptr<VolumeLoaderFactory>& loaderFac) {
		return std::make_unique<VolumeCache<T>>(loaderFac);
	}
};
