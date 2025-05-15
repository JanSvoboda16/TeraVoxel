/*
 * Author: Jan Svoboda
 * University: BRNO UNIVERSITY OF TECHNOLOGY, FACULTY OF INFORMATION TECHNOLOGY
 */
#pragma once
#include "VolumeLoaderGenericBase.h"
#include <future>
#include <TeraVoxel.Client.Core/ProjectManager.h>

/// <summary>
/// Loads NetProject datasets from the server (not NeuroVoxel)
/// </summary>
template <typename T>
class NetVolumeLoader : public VolumeLoaderBaseGenericBase<T>
{
public:
	NetVolumeLoader(const ProjectInfo& projectInfo, int threadCount, const ProjectManager& projectManager);
	~NetVolumeLoader() override;

protected:
	T* LoadBlockData(int x, int y, int z, int downscale) override;
	ProjectManager _projectManager;
	ProjectInfo _projectInfo;
};

