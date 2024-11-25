#pragma once
#include "IVolumeVisualizerFactory.h"
#include "GPURayCastingVolumeVisualizer.h"

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
