#pragma once
#include <memory>
#include <map>
#include "VolumeCacheGenericBase.h"
#include "VolumeLoaderBase.h"
#include "VolumeLoaderFactory.h"
#include "../TeraVoxel.Client.Core/TemplatedFunctionCaller.h"

template <typename T>
class VolumeCache : public VolumeCacheGenericBase
{
public:
	VolumeCache(const std::shared_ptr<VolumeLoaderFactory> &loaderFac) : 
		_volumeLoader(std::dynamic_pointer_cast<VolumeLoaderBase<T>>(std::shared_ptr<VolumeLoaderGenericBase>(loaderFac->Create(1)))), 
		VolumeCacheGenericBase(loaderFac->GetProjectInfo()) { }

	~VolumeCache() {
		Flush();
	}

	T GetValue(size_t x, size_t y, size_t z) 
	{
		T value;
		uint16_t blockIdX = x / _projectInfo.segmentSize;
		uint16_t blockIdY = y / _projectInfo.segmentSize;
		uint16_t blockIdZ = z / _projectInfo.segmentSize;

		int xb = x % _projectInfo.segmentSize;
		int yb = y % _projectInfo.segmentSize;
		int zb = z % _projectInfo.segmentSize;

		int key = blockIdX + blockIdY * _projectInfo.segmentSize + blockIdZ * _projectInfo.segmentSize * _projectInfo.segmentSize;

		if (_cache.contains(key))
		{
			auto block = _cache[key];
			value = block->data[Serialization::GetZCurveIndex(xb, yb, zb)];
		}
		else
		{
			auto block = std::shared_ptr<VolumeSegment<T>>(_volumeLoader->LoadSync(blockIdX, blockIdY, blockIdZ, 0));
			_cache.emplace(key, block);
			value = block->data[Serialization::GetZCurveIndex(xb, yb, zb)];
		}	

		return value;
	}	

	void Flush() override {
		MemoryContext::GetInstance().memoryInfoWriteMutex.lock();
		MemoryContext::GetInstance().usedMemory -= _cache.size() * _projectInfo.segmentSize * _projectInfo.segmentSize * _projectInfo.segmentSize * sizeof(T);
		MemoryContext::GetInstance().memoryInfoWriteMutex.unlock();
		_cache.clear();
	}

private:
	std::shared_ptr<VolumeLoaderBase<T>> _volumeLoader;
	std::map<int, std::shared_ptr<VolumeSegment<T>>> _cache;
};


class VolumeCacheFactory {
public:
	VolumeCacheFactory() = delete;

	static std::unique_ptr<VolumeCacheGenericBase> VolumeCacheCreate(const std::shared_ptr<VolumeLoaderFactory>& loaderFac) {
		return CALL_TEMPLATED_FUNCTION(VolumeCacheCreateTemplated, loaderFac->GetProjectInfo().dataType.c_str(), loaderFac);
	}
private:
	template <typename T>
	static std::unique_ptr<VolumeCacheGenericBase> VolumeCacheCreateTemplated(const std::shared_ptr<VolumeLoaderFactory>& loaderFac) {
		return std::make_unique<VolumeCache<T>>(loaderFac);
	}
};
