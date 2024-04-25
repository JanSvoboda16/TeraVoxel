#pragma once
#include "IVolumeVisualizerFactory.h"
#include "CPURayCastingVolumeVisualizer.h"

class CPURCVolumeVisualizerFactory : public IVolumeVisualizerFactory
{
public:
	CPURCVolumeVisualizerFactory(std::shared_ptr<CPURCVolumeVisualizerSettings> settings)
	{
		_settings = settings;
	}

	std::unique_ptr<VolumeVisualizerBase> Create(const std::shared_ptr<Camera>& camera, const std::shared_ptr<VolumeLoaderFactory>& volumeLoaderFactory, const std::shared_ptr<MeshNode>& meshNode) override
	{
		return std::make_unique<CPURayCastingVolumeVisualizer>(camera, volumeLoaderFactory, _settings, meshNode);
	}

private:
	std::shared_ptr<CPURCVolumeVisualizerSettings> _settings;
};

