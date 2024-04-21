#pragma once
#include "VolumeLoaderFactory.h"
#include "NetVolumeLoader.h"
#include "../TeraVoxel.Client.Core/TemplatedFunctionCaller.h"


class NetVolumeLoaderFactory : public VolumeLoaderFactory
{
public:

	NetVolumeLoaderFactory(const ProjectManager& projectManager, const ProjectInfo& projectInfo) : VolumeLoaderFactory(projectInfo)
	{
		_projectManager = projectManager;
	}

	std::unique_ptr<VolumeLoaderGenericBase> Create(int threadCount) override
	{
		return CALL_TEMPLATED_FUNCTION(CreateInternal, _projectInfo.dataType.c_str(), threadCount);
	}

private:

	ProjectManager _projectManager;

	template <typename T>
	std::unique_ptr<VolumeLoaderGenericBase> CreateInternal(int threadCount)
	{
		return std::make_unique<NetVolumeLoader<T>>(_projectInfo, threadCount, _projectManager);
	}
};

