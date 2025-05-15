/*
 * Author: Jan Svoboda
 * University: BRNO UNIVERSITY OF TECHNOLOGY, FACULTY OF INFORMATION TECHNOLOGY
 */
#pragma once
#include "TeraVoxel.Client.VolumeRenderer/VolumeLoaderFactory.h"
#include "TeraVoxel.Client.VolumeRenderer/NetVolumeLoader.h"

class NetVolumeLoaderFactory : public VolumeLoaderFactory
{
public:

	NetVolumeLoaderFactory(const ProjectManager& projectManager, const ProjectInfo& projectInfo) :
		VolumeLoaderFactory(projectInfo.ToBlockBasedDatasetInfo()),
		_projectinfo(projectInfo),
		_projectManager(projectManager)
	{ }

	std::unique_ptr<VolumeLoaderBase> Create(int threadCount) override
	{
		return CALL_TEMPLATED_FUNCTION2(CreateInternal, _datasetInfo.dataType, threadCount);
	}

private:

	ProjectManager _projectManager;
	ProjectInfo _projectinfo;

	template <typename T>
	std::unique_ptr<VolumeLoaderBase> CreateInternal(int threadCount)
	{
		return std::make_unique<NetVolumeLoader<T>>(_projectinfo, threadCount, _projectManager);
	}
};

