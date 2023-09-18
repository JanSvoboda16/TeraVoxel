#pragma once
#include "RayCastingVolumeVisualizerBase.h"
#include "ColorMappingTable.h"
#include "FastRCVolumeVisualizerSettings.h"

template <typename T>
class FastRayCastingVolumeVisualizer : public RayCastingVolumeVisualizerBase<T>
{
public:
	FastRayCastingVolumeVisualizer(std::shared_ptr<Camera> camera, std::shared_ptr<VolumeObjectMemory<T>> memory, std::shared_ptr<FastRCVolumeVisualizerSettings> settings) : RayCastingVolumeVisualizerBase<T>(camera, memory) {
		_settings = settings;
	}
	

private:
	void DisplayPoint(Vector3f point); // usefull for testing direct projection
	virtual void ComputeFrameInternal(int downscale) override;
	void ComputePartOfFrame(int threads, int threadIndex, int downscale);
	color ComputeRay(int x, int y);
	std::shared_ptr<FastRCVolumeVisualizerSettings> _settings;
	std::atomic<int> _reneringPosition = 0;
	FastRCVolumeVisualizerSettings _settingsCopy;
};
