#pragma once
#include "VolumeLoaderBase.h"
#include <future>
#include "../TeraVoxel.Client.Core/ProjectManager.h"

template <typename T>
class NetVolumeLoader : public VolumeLoaderBase<T>
{
public:
	NetVolumeLoader(const ProjectInfo& projectInfo, int threadCount, const ProjectManager& projectManager);
	~NetVolumeLoader() override;
	

protected:
	T* LoadSegment(int x, int y, int z, int downscale) override;
	ProjectManager _projectManager;	
};

