/*
 * Author: Jan Svoboda
 * University: BRNO UNIVERSITY OF TECHNOLOGY, FACULTY OF INFORMATION TECHNOLOGY
 */
#pragma once
#include "TeraVoxel.Client.VolumeRenderer/IVolumeVisualizerFactory.h"
#include "TeraVoxel.Client.VolumeRenderer/GPURayCastingVolumeVisualizer.h"

class GPURCVolumeVisualizerFactory : public IVolumeVisualizerFactory
{
public:
	GPURCVolumeVisualizerFactory(std::shared_ptr<GPURCVolumeVisualizerSettings> settings)
	{
		_settings = settings;
	}

	std::unique_ptr<VolumeVisualizerBase> Create(const std::shared_ptr<Camera>& camera, const std::shared_ptr<VolumeLoaderFactory>& volumeLoaderFactory) override
	{
		return std::make_unique<GPURayCastingVolumeVisualizer>(camera, volumeLoaderFactory, _settings);
	}

private:
	std::shared_ptr<GPURCVolumeVisualizerSettings> _settings;
};
