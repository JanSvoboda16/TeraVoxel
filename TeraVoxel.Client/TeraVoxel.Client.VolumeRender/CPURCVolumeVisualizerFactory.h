#pragma once
#include "IVolumeVisualizerFactory.h"
#include "CPURayCastingVolumeVisualizer.h"

template <typename T>
class CPURCVolumeVisualizerFactory: public IVolumeVisualizerFactory<T>
{
public:
	CPURCVolumeVisualizerFactory(std::shared_ptr<CPURCVolumeVisualizerSettings> settings)
	{
		_settings = settings;
	}

	std::unique_ptr<VolumeVisualizerBase<T>> Create(const std::shared_ptr<Camera>& camera, const ProjectInfo& projectInfo, const std::shared_ptr<VolumeLoaderFactory<T>>& volumeLoaderFactory) override {
		return std::unique_ptr<VolumeVisualizerBase<T>>((VolumeVisualizerBase<T>*) new CPURayCastingVolumeVisualizer<T>(camera, projectInfo, volumeLoaderFactory, _settings));
	}

private:
	std::shared_ptr<CPURCVolumeVisualizerSettings> _settings;
};

