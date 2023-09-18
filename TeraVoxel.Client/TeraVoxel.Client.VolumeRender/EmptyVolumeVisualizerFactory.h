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
	std::unique_ptr<VolumeVisualizerBase<T>> Create(std::shared_ptr<Camera> camera, std::shared_ptr<VolumeObjectMemory<T>> memory) override {
		return std::unique_ptr<VolumeVisualizerBase<T>>((VolumeVisualizerBase<T>*) new EmptyVolumeVisualizer<T>(camera, memory));
	}

private:
	std::shared_ptr<EmptyVolumeVisualizerSettings> _settings;
};

