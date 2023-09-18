#pragma once
#include "IVolumeVisualizerFactory.h"
#include "FastRayCastingVolumeVisualizer.h"

template <typename T>
class FastRCVolumeVisualizerFactory: public IVolumeVisualizerFactory<T>
{
public:
	FastRCVolumeVisualizerFactory(std::shared_ptr<FastRCVolumeVisualizerSettings> settings)
	{
		_settings = settings;
	}

	std::unique_ptr<VolumeVisualizerBase<T>> Create(std::shared_ptr<Camera> camera, std::shared_ptr<VolumeObjectMemory<T>> memory) override {
		return std::unique_ptr<VolumeVisualizerBase<T>>((VolumeVisualizerBase<T>*) new FastRayCastingVolumeVisualizer<T>(camera, memory, _settings));
	}

private:
	std::shared_ptr<FastRCVolumeVisualizerSettings> _settings;
};

