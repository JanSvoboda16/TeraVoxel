/*
 * Author: Jan Svoboda
 * University: BRNO UNIVERSITY OF TECHNOLOGY, FACULTY OF INFORMATION TECHNOLOGY
 */
#include "TeraVoxel.Client.VolumeRenderer/VolumeVisualizerBase.h"
#include "TeraVoxel.Client.VolumeRenderer/ColorMappingTable.h"
#include "TeraVoxel.Client.VolumeRenderer/CPURCVolumeVisualizerSettings.h"
#include <cuda_runtime.h>
#include <device_launch_parameters.h>
#include "TeraVoxel.Client.VolumeRenderer/GPURCVolumeVisualizerSettings.h"

class GPURayCastingVolumeMemory;

class GPURayCastingVolumeVisualizer : public VolumeVisualizerBase
{
public:

	GPURayCastingVolumeVisualizer(const std::shared_ptr<Camera>& camera, const std::shared_ptr<VolumeLoaderFactory>& volumeLoaderFactory, const std::shared_ptr<GPURCVolumeVisualizerSettings>& settings);
	~GPURayCastingVolumeVisualizer();

	bool DataChanged() override;

private:
	/// <summary>
	/// Updates entities in the scene if need to be updated
	/// </summary>
	/// <param name="camera_d"></param>
	void UpdateEntities(Camera* camera_d);

	/// <summary>
	/// Remoputes shadows in the scene
	/// </summary>
	/// <param name="camera_d">Device camera pointer</param>
	void UpdateShadowTexture(Camera* camera_d);

	/// <summary>
	/// Computes frame
	/// </summary>
	/// <param name="framebuffer"></param>
	/// <param name="downscale"></param>
	/// <param name="multiLayeredFramebuffer"></param>
	void ComputeFrameInternal(std::shared_ptr<unsigned char[]>& framebuffer, int downscale, const std::shared_ptr<MultiLayeredFramebufferBase>& multiLayeredFramebuffer) override;

	std::unique_ptr<GPURayCastingVolumeMemory> _memory;
	std::shared_ptr<GPURCVolumeVisualizerSettings> _settings;

	MaterialTableItem* _materialTable_d = nullptr;
	int64_t _settingsDeviceVersion = -1;

	// Shadows - Arrays are on host, objects are on GPU
	cudaTextureObject_t* _shadowTextures_h = NULL;
	cudaArray_t* _shadowArrays_h = nullptr;	
	cudaTextureObject_t* _shadowTextures_d = NULL;
	LightSettings* _lightSettings_d = nullptr;
	uint8_t _numShadows = 0;
	uint8_t _shadowSubsampling = 4;
};
