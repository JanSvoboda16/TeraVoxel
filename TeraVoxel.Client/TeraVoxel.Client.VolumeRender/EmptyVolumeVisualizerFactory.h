#pragma once
#include "IVolumeVisualizerFactory.h"
#include "EmptyVolumeVisualizer.h"

template <typename T> 
class EmptyVolumeVisualizerFactory: IVolumeVisualizerFactory<T>
{
public:
	EmptyVolumeVisualizerFactory(std::shared_ptr<EmptyVolumeVisualizerSettings> settings){
		_settings = settings;
	}
	std::unique_ptr<VolumeVisualizerBase<T>> Create(const std::shared_ptr<Camera>& camera, const ProjectInfo& projectInfo, const std::shared_ptr<VolumeLoaderFactory<T>>& volumeLoaderFactory) override {
		return std::unique_ptr<VolumeVisualizerBase<T>>((VolumeVisualizerBase<T>*) new EmptyVolumeVisualizer<T>(camera, projectInfo, volumeLoaderFactory));
	}

private:
	std::shared_ptr<EmptyVolumeVisualizerSettings> _settings;
};

