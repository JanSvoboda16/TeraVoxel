#pragma once
#include "IVolumeVisualizerFactory.h"
#include "EmptyVolumeVisualizer.h"

class EmptyVolumeVisualizerFactory: public IVolumeVisualizerFactory
{
public:
	EmptyVolumeVisualizerFactory(std::shared_ptr<EmptyVolumeVisualizerSettings> settings){
		_settings = settings;
	}

	std::unique_ptr<VolumeVisualizerBase> Create(const std::shared_ptr<Camera>& camera, const std::shared_ptr<VolumeLoaderFactory>& volumeLoaderFactory, const std::shared_ptr<MeshNode>& meshNode) override {
		return std::make_unique<EmptyVolumeVisualizer>(camera, volumeLoaderFactory, meshNode);
	}

private:
	std::shared_ptr<EmptyVolumeVisualizerSettings> _settings;
};

