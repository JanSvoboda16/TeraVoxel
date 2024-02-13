#pragma once
#include "IVolumeVisualizerFactory.h"
#include "EmptyVolumeVisualizer.h"

template <typename T> 
class EmptyVolumeVisualizerFactory: public IVolumeVisualizerFactory<T>
{
public:
	EmptyVolumeVisualizerFactory(std::shared_ptr<EmptyVolumeVisualizerSettings> settings){
		_settings = settings;
	}
	std::unique_ptr<VolumeVisualizerBase<T>> Create(const std::shared_ptr<Camera>& camera, const ProjectInfo& projectInfo, const std::shared_ptr<VolumeLoaderFactory<T>>& volumeLoaderFactory, const std::shared_ptr<MeshNode>& meshNode) override {
		return std::make_unique<EmptyVolumeVisualizer<T>>(camera, projectInfo, volumeLoaderFactory, meshNode);
	}

private:
	std::shared_ptr<EmptyVolumeVisualizerSettings> _settings;
};

