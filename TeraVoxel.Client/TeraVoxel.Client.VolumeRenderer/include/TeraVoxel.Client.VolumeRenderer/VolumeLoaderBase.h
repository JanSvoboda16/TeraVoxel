#pragma once
#include <memory>
#include <stack>
#include <queue>
#include <thread>
#include <future>
#include <functional>
#include <list>
#include "TeraVoxel.Client.VolumeRenderer/VolumeSegment.h"
#include <TeraVoxel.Client.Core/MemoryContext.h>
#include "TeraVoxel.Client.VolumeRenderer/Serialization.h"
#include <TeraVoxel.Client.Core/Logger.h>
#include "TeraVoxel.Client.VolumeRenderer/VolumeLoaderGenericBase.h"

template <typename T>
struct ComparePriority
{
	bool operator()(VolumeSegment<T>* lhs, VolumeSegment<T>* rhs)
	{
		return lhs->priority.load(std::memory_order_acquire) > rhs->priority.load(std::memory_order_acquire);
	}
};

template <typename T>
class VolumeLoaderBase : public VolumeLoaderGenericBase
{

public:
	VolumeLoaderBase(const BlockBasedDatasetInfo& datasetInfo, int threadCount);
	virtual ~VolumeLoaderBase();
	std::shared_ptr<VolumeSegmentRequestTicket> LoadAsync(int x, int y, int z, int downscale, float priority);
	void Preload(int downscale, int threadCount);
	std::unique_ptr<VolumeSegment<T>> TakeFirstLoaded(int& count);
	std::unique_ptr<VolumeSegment<T>> LoadSync(int x, int y, int z, int downscale);

	void BindOnSegmentLoaded(std::function<void(void)> function) { _onSegmentLoaded = function; }

	uint64_t GetBlockRequiredMemory(int downscale);
protected:
	std::list<std::shared_ptr<VolumeSegmentRequestTicket>> _tickets;
	std::queue<std::unique_ptr<VolumeSegment<T>>> _loadedSegments;
	std::mutex _ticketsMutex;
	std::mutex _loadedSegmentsMutex;
	
	int _segmentCountX, _segmentCountY, _segmentCountZ, _threadCount;

	std::list<std::future<void>> _loadingTreads;
	bool _endLoopingThreads = false;
	std::function<void(void)> _onSegmentLoaded = [=]() {};

	void PreloadTask(short threadIndex, short threadCount, int downscale);
	
	virtual T* LoadSegmentData(int x, int y, int z, int downscale) = 0;

	void LoadingTask();
};

