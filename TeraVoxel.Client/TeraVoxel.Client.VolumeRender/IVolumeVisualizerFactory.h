#pragma once
#include "VolumeVisualizerBase.h"

class IVolumeVisualizerFactory
{
public:
	virtual std::unique_ptr<VolumeVisualizerBase> Create(const std::shared_ptr<Camera> &camera, const std::shared_ptr<VolumeLoaderFactory>& volumeLoaderFactory, const std::shared_ptr<MeshNode>& meshNode) = 0;
};

