#pragma once
#include <memory>
#include "VolumeLoaderGenericBase.h"

class VolumeLoaderFactory
{
public:
	virtual ~VolumeLoaderFactory() {}
	virtual std::unique_ptr<VolumeLoaderGenericBase> Create(int threadCount) = 0;
	ProjectInfo GetProjectInfo() { return _projectInfo; }
	VolumeLoaderFactory(const ProjectInfo& projectInfo) : _projectInfo(projectInfo) { }

protected:
	ProjectInfo _projectInfo;
};

