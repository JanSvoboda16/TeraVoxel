#pragma once
#include "CPURayCastingVolumeVisualizerBase.h"
#include "ColorMappingTable.h"
#include "CPURCVolumeVisualizerSettings.h"

template <typename T>
class CPURayCastingVolumeVisualizer : public CPURayCastingVolumeVisualizerBase<T>
{
public:
	CPURayCastingVolumeVisualizer(const std::shared_ptr<Camera> &camera, const ProjectInfo& projectInfo, const std::shared_ptr<VolumeLoaderFactory<T>>& volumeLoaderFactory, const std::shared_ptr<CPURCVolumeVisualizerSettings> &settings) : CPURayCastingVolumeVisualizerBase<T>(camera, projectInfo, volumeLoaderFactory) {
		_settings = settings;
	}
	
	bool DataChanged() override;

private:
	void DisplayPoint(Vector3f point); // usefull for testing direct projection
	virtual void ComputeFrameInternal(int downscale) override;
	void ComputePartOfFrame(int threads, int threadIndex, int downscale);
	color ComputeRay(int x, int y);
	std::shared_ptr<CPURCVolumeVisualizerSettings> _settings;
	std::atomic<int> _reneringPosition = 0;
	CPURCVolumeVisualizerSettings _settingsCopy;
};
