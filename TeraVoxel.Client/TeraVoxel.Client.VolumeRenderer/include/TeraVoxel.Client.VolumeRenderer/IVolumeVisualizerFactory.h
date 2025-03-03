/*
 * Author: Jan Svoboda
 * University: BRNO UNIVERSITY OF TECHNOLOGY, FACULTY OF INFORMATION TECHNOLOGY
 */
#pragma once
#include "TeraVoxel.Client.VolumeRenderer/VolumeVisualizerBase.h"

class IVolumeVisualizerFactory
{
public:
	virtual std::unique_ptr<VolumeVisualizerBase> Create(const std::shared_ptr<Camera> &camera, const std::shared_ptr<VolumeLoaderFactory>& volumeLoaderFactory) = 0;
};

