#include "VolumeVisualizerBase.h"

#include "ColorMappingTable.h"
#include "CPURCVolumeVisualizerSettings.h"
#include <cuda_runtime.h>
#include <device_launch_parameters.h>
#include "GPURayCastingVolumeTexture.cuh"

class GPURayCastingVolumeVisualizer : public VolumeVisualizerBase
{
public:
	GPURayCastingVolumeVisualizer(const std::shared_ptr<Camera>& camera, const std::shared_ptr<VolumeLoaderFactory>& volumeLoaderFactory, const std::shared_ptr<CPURCVolumeVisualizerSettings>& settings);
	~GPURayCastingVolumeVisualizer();

	bool DataChanged() override;

private:
	cudaTextureObject_t _texObj;

	cudaArray_t _textureArrayDevice;

	GPURayCastingVolumeTexture<float>* _texture;

	template <typename T>
	void CoumputeFrameInternalTemplated(int downscale);

	// Dědí se přes VolumeVisualizerBase.
	void ComputeFrameInternal(std::shared_ptr<unsigned char[]>& framebuffer, int downscale, const std::shared_ptr<MultiLayeredFramebufferBase>& multiLayeredFramebuffer) override;
};
