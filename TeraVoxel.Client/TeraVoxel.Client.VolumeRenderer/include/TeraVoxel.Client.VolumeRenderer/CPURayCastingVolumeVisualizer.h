#pragma once
#include "TeraVoxel.Client.VolumeRenderer/CPURayCastingVolumeVisualizerBase.h"
#include "TeraVoxel.Client.VolumeRenderer/ColorMappingTable.h"
#include "TeraVoxel.Client.VolumeRenderer/CPURCVolumeVisualizerSettings.h"
#include "TeraVoxel.Client.VolumeRenderer/CPUMeshVisualizer.h"
#include "TeraVoxel.Client.VolumeRenderer/SeedVolumeSelector.h"
#include "TeraVoxel.Client.VolumeRenderer/MarchingCubesSurfaceExtractor.h"
#include "TeraVoxel.Client.VolumeRenderer/CPURayCastingVolumeObjectMemory.h"

class CPURayCastingVolumeVisualizer : public RayCastingVolumeVisualizerBase
{
public:
	CPURayCastingVolumeVisualizer(const std::shared_ptr<Camera>& camera, const std::shared_ptr<VolumeLoaderFactory>& volumeLoaderFactory, const std::shared_ptr<CPURCVolumeVisualizerSettings>& settings);

	bool DataChanged() override;

private:
	void ComputeFrameInternal(std::shared_ptr<unsigned char[]>& framebuffer, int downscale, const std::shared_ptr<MultiLayeredFramebufferBase>& multiLayeredFramebuffer) override;
	void MixColors(float& r, float& g, float& b, float& a, const float ra, const float ga, const float ba, const float ca);
	
	template <typename T>
	void ComputePartOfFrame(std::shared_ptr<unsigned char[]>& framebuffer, int threads, int threadIndex, int framebufferWidth, int framebufferHeight, int downscale, const std::shared_ptr<CPURayCastingVolumeObjectMemory<T>>& memory, const std::shared_ptr<CPUMultiLayeredFramebuffer>& multiLayeredFramebuffer);
	template <typename T>
	color ComputeRay(int x, int y, const std::shared_ptr<CPURayCastingVolumeObjectMemory<T>>& memory, const std::shared_ptr<CPUMultiLayeredFramebuffer>& multiLayeredFramebuffer);
	template <typename T>
	bool DataChangedTemplated();
	template <typename T>
	void CoumputeFrameInternalTemplated(std::shared_ptr<unsigned char[]>& framebuffer, int downscale, const std::shared_ptr<MultiLayeredFramebufferBase>& multiLayeredFramebuffer);
	template <typename T>
	void CreateMemory(const std::shared_ptr<Camera>& camera, const std::shared_ptr<VolumeLoaderFactory>& volumeLoaderFactory);

	std::any _memory;	

	std::shared_ptr<CPURCVolumeVisualizerSettings> _settings;
	std::atomic<int> _reneringPosition = 0;
	CPURCVolumeVisualizerSettings _settingsCopy;
};
