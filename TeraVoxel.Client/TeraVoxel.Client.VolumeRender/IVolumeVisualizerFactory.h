#pragma once
#include "VolumeVisualizerBase.h"

template<typename T>
class IVolumeVisualizerFactory
{
public:
	virtual std::unique_ptr<VolumeVisualizerBase<T>> Create(const std::shared_ptr<Camera> &camera, const ProjectInfo& projectInfo, const std::shared_ptr<VolumeLoaderFactory<T>>& volumeLoaderFactory) = 0;
};

