#include "VolumeVisualizerBase.h"
#include "ColorMappingTable.h"
#include "CPURCVolumeVisualizerSettings.h"
#include <cuda_runtime.h>
#include <device_launch_parameters.h>
#include "GPURayCastingVolumeTexture.cuh"
#include "GPURCVolumeVisualizerSettings.h"

class GPURayCastingVolumeMemory;

class GPURayCastingVolumeVisualizer : public VolumeVisualizerBase
{
public:

	GPURayCastingVolumeVisualizer(const std::shared_ptr<Camera>& camera, const std::shared_ptr<VolumeLoaderFactory>& volumeLoaderFactory, const std::shared_ptr<GPURCVolumeVisualizerSettings>& settings);
	~GPURayCastingVolumeVisualizer();

	bool DataChanged() override;

private:
	template <typename T>
	void CoumputeFrameInternalTemplated(int downscale);

	void UpdateEntities(Camera* camera_d);

	void UpdateShadowTexture(Camera* camera_d);

	// Dědí se přes VolumeVisualizerBase.
	void ComputeFrameInternal(std::shared_ptr<unsigned char[]>& framebuffer, int downscale, const std::shared_ptr<MultiLayeredFramebufferBase>& multiLayeredFramebuffer) override;

	std::unique_ptr<GPURayCastingVolumeMemory> _memory;
	std::shared_ptr<GPURCVolumeVisualizerSettings> _settings;



	MaterialTableItem* _materialTable_d = nullptr;
	int64_t _settingsDeviceVersion = -1;

	// Arrays are on hosts, not objects
	cudaTextureObject_t* _shadowTextures_h = NULL;
	cudaArray_t* _shadowArrays_h = nullptr;

	cudaTextureObject_t* _shadowTextures_d = NULL;

	LightSettings* _lightSettings_d = nullptr;

	uint8_t _numShadows = 0;

	uint8_t _subsamplingFactor = 4;
};
