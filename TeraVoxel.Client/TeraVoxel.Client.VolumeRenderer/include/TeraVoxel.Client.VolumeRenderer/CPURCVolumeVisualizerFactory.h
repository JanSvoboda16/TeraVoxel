#pragma once
#include "TeraVoxel.Client.VolumeRenderer/IVolumeVisualizerFactory.h"
#include "TeraVoxel.Client.VolumeRenderer/CPURayCastingVolumeVisualizer.h"

class CPURCVolumeVisualizerFactory : public IVolumeVisualizerFactory
{
public:
	CPURCVolumeVisualizerFactory(std::shared_ptr<CPURCVolumeVisualizerSettings> settings)
	{
		_settings = settings;
	}

	std::unique_ptr<VolumeVisualizerBase> Create(const std::shared_ptr<Camera>& camera, const std::shared_ptr<VolumeLoaderFactory>& volumeLoaderFactory) override
	{
		return std::make_unique<CPURayCastingVolumeVisualizer>(camera, volumeLoaderFactory, _settings);
	}

private:
	std::shared_ptr<CPURCVolumeVisualizerSettings> _settings;
};

