#pragma once
#include "TeraVoxel.Client.VolumeRenderer/IVolumeVisualizerFactory.h"
#include "TeraVoxel.Client.VolumeRenderer/EmptyVolumeVisualizer.h"

class EmptyVolumeVisualizerFactory: public IVolumeVisualizerFactory
{
public:
	EmptyVolumeVisualizerFactory(std::shared_ptr<EmptyVolumeVisualizerSettings> settings){
		_settings = settings;
	}

	std::unique_ptr<VolumeVisualizerBase> Create(const std::shared_ptr<Camera>& camera, const std::shared_ptr<VolumeLoaderFactory>& volumeLoaderFactory) override {
		return std::make_unique<EmptyVolumeVisualizer>(camera, volumeLoaderFactory);
	}

private:
	std::shared_ptr<EmptyVolumeVisualizerSettings> _settings;
};

