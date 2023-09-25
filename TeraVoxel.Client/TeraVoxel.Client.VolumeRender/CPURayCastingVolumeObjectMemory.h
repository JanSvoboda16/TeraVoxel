/*
 * Author: Jan Svoboda
 * University: BRNO UNIVERSITY OF TECHNOLOGY, FACULTY OF INFORMATION TECHNOLOGY
 */
#pragma once
#include "Camera.h"
#include "VolumeSegment.h"
#include "../TeraVoxel.Client.Core/ProjectManager.h"
#include "../TeraVoxel.Client.Core/ProjectInfo.h"
#include "../TeraVoxel.Client.Core/SettingsContext.h"
#include "VolumeLoaderFactory.h"
#include <mutex>
#include <future>
#include <stack>
#include <thread>
#include <array>
#include <chrono>
#include <queue>
#include <bit>
	
template <typename T>
class CPURayCastingVolumeObjectMemory
{
public:
	~CPURayCastingVolumeObjectMemory();
	CPURayCastingVolumeObjectMemory(const std::shared_ptr<Camera> &camera, const ProjectInfo& projectInfo, const std::shared_ptr<VolumeLoaderFactory<T>>& volumeLoaderFactory);
		
	/// <summary>
	/// Gets a value at a position in the memory
	/// </summary>
	/// <param name="xIndex">Index x</param>
	/// <param name="yIndex">Index y</param>
	/// <param name="zIndex">Index z</param>
	/// <param name="downscale">Required data downscale</param>
	/// <returns>Value</returns>
	T GetValue(uint_fast16_t xIndex, uint_fast16_t yIndex, uint_fast16_t zIndex, int& downscale);
		
	/// <summary>
	/// Returns an information about the loaded project
	/// </summary>
	/// <returns>Information</returns>
	ProjectInfo GetProjectInfo();

	std::array<int,3> GetDataSizes();
		
	/// <summary>
	/// Revalidates data according to the current camera view
	/// </summary>
	void Revalidate();
	
	/// <summary>
	/// Prepares data for loading
	/// </summary>
	void Prepare();

	/// <summary>
	/// Memory changed since last calling
	/// </summary>
	/// <returns>true if changed</returns>
	bool MemoryChanged() { bool res = _memoryChanged.load(std::memory_order_relaxed); _memoryChanged = false; return res; }
	
	void FlushCachedData();
private:
	std::vector<VolumeSegment<T>*> _volumesToDelete;		// Volume segments that will bee deleted
	std::mutex _reloadStackMutex, _volumesToDeleteMutex;	// Mutexes
	std::vector<VolumeSegment<T>*> _lowResolutionVolumes;					// Low resolution volume segments
		
	// For other architectures std::atomic should be used
	std::vector<VolumeSegment<T>*> _volumes;								// Volume segments
		
	std::shared_ptr<Camera> _camera;	// Scene camera	
	ProjectInfo _projectInfo;					// Informations about the curent project
	std::unique_ptr<VolumeLoaderBase<T>> _volumeLoader;

	uint_fast16_t xSegmentCount, ySegmentCountCount, zSegmentCount; // Count of volume segments in each axis
	uint_fast32_t maxSegmentIndex;									// Max index of the volume segment

	float _oneDivSegmentSize = 0; 
	uint16_t _segmentSize = 0;
	uint8_t _segmentSizeShifter = 0;		// Used for segent size multiplication/division by shifting		
	uint64_t _segmentCount = 0;
		
	std::atomic<bool> _memoryChanged = false;			// Data in memory changed			
	std::atomic<bool> _endLoopingThreads = false;		// Used for ending looping threads

	/// <summary>
	/// Gets the amount of memory needed to store a segment with a given downscale
	/// </summary>
	/// <param name="Downscale">downscale</param>
	/// <returns>Memory in bytes</returns>
	long GetBlockRequiredMemory(int downscale);

	/// <summary>
	/// Gets downscale required for a voxel on a given position
	/// </summary>
	/// <param name="xIndex">X position</param>
	/// <param name="yIndex">Y position</param>
	/// <param name="zIndex">Z position</param>
	/// <returns>Downscale index</returns>
	int GetRequiredDownscale(int xIndex, int yIndex, int zIndex);

	/// <summary>
	/// Deletes all segments in array
	/// </summary>
	/// <param name="volumes">Array of segments</param>
	void ProcessDelete(std::vector<VolumeSegment<T>*> &volumes);

	/// <summary>
	/// Preloads data from server
	/// </summary>
	/// <param name="threadCount">Count of threads</param>
	void Preload(short threadCount);

	/// <summary>
	/// Deletes not used segments
	/// </summary>
	/// <param name="maxCount">Maximal count</param>
	void DeleteNotUsed(int maxCount);

	/// <summary>
	/// Downscales all segments with higher quality than required
	/// </summary>
	/// /// <param name="maxCount">Maximal count</param>
	void DownscaleWithHigherQuality(int maxCount);
};
