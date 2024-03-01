#pragma once
#include "CPURayCastingVolumeVisualizerBase.h"
#include "ColorMappingTable.h"
#include "CPURCVolumeVisualizerSettings.h"
#include "CPUMeshVisualizer.h"
#include "SeedVolumeSelector.h"
#include "MarchingCubesSurfaceExtractor.h"

class CPURayCastingVolumeVisualizer : public CPURayCastingVolumeVisualizerBase
{
public:
	CPURayCastingVolumeVisualizer(const std::shared_ptr<Camera>& camera, const std::shared_ptr<VolumeLoaderFactory>& volumeLoaderFactory, const std::shared_ptr<CPURCVolumeVisualizerSettings>& settings, const std::shared_ptr<MeshNode>& meshNode) : 
		CPURayCastingVolumeVisualizerBase(camera, volumeLoaderFactory, meshNode), _meshVisualizer(meshNode, camera)
	{
		_settings = settings;
	}

	bool DataChanged() override;

private:
	void ComputeFrameInternal(int downscale) override;
	void MixColors(float& r, float& g, float& b, float& a, const float ra, const float ga, const float ba, const float ca);
	
	template <typename T>
	void ComputePartOfFrame(int threads, int threadIndex, int framebufferWidth, int framebufferHeight, int downscale, const std::shared_ptr<CPURayCastingVolumeObjectMemory<T>>& memory);
	template <typename T>
	color ComputeRay(int x, int y, const std::shared_ptr<CPURayCastingVolumeObjectMemory<T>>& memory);
	template <typename T>
	bool DataChangedTemplated();
	template <typename T>
	void CoumputeFrameInternalTemplated(int downscale);

	std::shared_ptr<CPURCVolumeVisualizerSettings> _settings;
	std::atomic<int> _reneringPosition = 0;
	CPURCVolumeVisualizerSettings _settingsCopy;
	CPUMeshVisualizer _meshVisualizer;
	std::shared_ptr<MultiLayeredFramebuffer> _meshFramebuffer;
};
