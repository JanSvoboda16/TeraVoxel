#pragma once
#include <memory>
#include "VolumeLoaderGenericBase.h"
#include "../TeraVoxel.Client.Core/SettingsContext.h"

class VolumeLoaderFactory
{
public:
	virtual ~VolumeLoaderFactory() {}
	virtual std::unique_ptr<VolumeLoaderGenericBase> Create(int threadCount = SettingsContext::GetInstance().loadingThreadCount) = 0;
	ProjectInfo GetProjectInfo() { return _projectInfo; }
	VolumeLoaderFactory(const ProjectInfo& projectInfo) : _projectInfo(projectInfo) { }

protected:
	ProjectInfo _projectInfo;
};

