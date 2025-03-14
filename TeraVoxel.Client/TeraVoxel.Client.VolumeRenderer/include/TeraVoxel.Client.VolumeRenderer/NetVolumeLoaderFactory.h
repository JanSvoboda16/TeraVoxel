#pragma once
#include "TeraVoxel.Client.VolumeRenderer/VolumeLoaderFactory.h"
#include "TeraVoxel.Client.VolumeRenderer/NetVolumeLoader.h"
#include <TeraVoxel.Client.Core/TemplatedFunctionCaller.h>


class NetVolumeLoaderFactory : public VolumeLoaderFactory
{
public:

	NetVolumeLoaderFactory(const ProjectManager& projectManager, const ProjectInfo& projectInfo) :
		VolumeLoaderFactory(projectInfo.ToBlockBasedDatasetInfo()),
		_projectinfo(projectInfo),
		_projectManager(projectManager)
	{ }

	std::unique_ptr<VolumeLoaderGenericBase> Create(int threadCount) override
	{
		return CALL_TEMPLATED_FUNCTION2(CreateInternal, _datasetInfo.dataType, threadCount);
	}

private:

	ProjectManager _projectManager;
	ProjectInfo _projectinfo;

	template <typename T>
	std::unique_ptr<VolumeLoaderGenericBase> CreateInternal(int threadCount)
	{
		return std::make_unique<NetVolumeLoader<T>>(_projectinfo, threadCount, _projectManager);
	}
};

