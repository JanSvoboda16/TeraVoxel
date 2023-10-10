#pragma once
#include "VolumeLoaderFactory.h"
#include "NetVolumeLoader.h"

template <typename T>
class NetVolumeLoaderFactory : public VolumeLoaderFactory<T>
{
public:
	NetVolumeLoaderFactory(const ProjectManager& projectManager)
	{
		_projectManager = projectManager;
	}

	std::unique_ptr<VolumeLoaderBase<T>> Create(const ProjectInfo& projectInfo, int threadCount) const override
	{
		return std::make_unique<NetVolumeLoader<T>>(projectInfo, threadCount, _projectManager);
	}

private:
	ProjectManager _projectManager;
};

