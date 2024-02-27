#pragma once
#include <memory>
#include <stack>
#include <queue>
#include <thread>
#include <future>
#include <functional>
#include <list>
#include <memory>
#include "VolumeSegment.h"
#include "../TeraVoxel.Client.Core/MemoryContext.h"
#include "Serialization.h"
#include "../TeraVoxel.Client.Core/Logger.h"
#include "VolumeLoaderGenericBase.h"

template <typename T>
struct ComparePriority
{
	bool operator()(VolumeSegment<T>* lhs, VolumeSegment<T>* rhs)
	{
		return lhs->priority.load(std::memory_order::acquire) > rhs->priority.load(std::memory_order::acquire);
	}
};

template <typename T>
class VolumeLoaderBase : public VolumeLoaderGenericBase
{

public:
	VolumeLoaderBase(const ProjectInfo& projectInfo, int threadCount);
	virtual ~VolumeLoaderBase();
	void AddToStack(VolumeSegment<T>* segment);
	void Preload(int downscale, int threadCount);
	std::unique_ptr<VolumeSegment<T>> TakeFirstLoaded(int& count);
	std::unique_ptr<VolumeSegment<T>> LoadSync(int x, int y, int z, int downscale);

	void BindOnSegmentLoaded(std::function<void(void)> function) { _onSegmentLoaded = function; }
	ProjectInfo GetProjectInfo() { return _projectInfo; }

protected:
	std::list<VolumeSegment<T>*> _segmentsToLoad;
	std::queue<std::unique_ptr<VolumeSegment<T>>> _loadedSegments;
	std::mutex _segmentsToLoadMutex;
	std::mutex _loadedSegmentsMutex;
	
	int _segmentCountX, _segmentCountY, _segmentCountZ, _threadCount;

	std::list<std::future<void>> _loadingTreads;
	bool _endLoopingThreads;
	std::function<void(void)> _onSegmentLoaded = [=]() {};

	void PreloadTask(short threadIndex, short threadCount, int downscale);
	uint64_t GetBlockRequiredMemory(int downscale);
	virtual T* LoadSegmentData(int x, int y, int z, int downscale) = 0;

	void LoadingTask();

};

