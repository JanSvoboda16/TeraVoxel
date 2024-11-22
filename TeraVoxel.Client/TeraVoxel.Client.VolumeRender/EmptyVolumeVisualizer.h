#pragma once
#include "VolumeVisualizerBase.h"
#include "VolumeVisualizerSettingsBase.h"

class EmptyVolumeVisualizerSettings: public VolumeVisualizerSettingsBase
{

};

class EmptyVolumeVisualizer: public VolumeVisualizerBase
{
public:
	EmptyVolumeVisualizer(std::shared_ptr<Camera> camera, std::shared_ptr<VolumeLoaderFactory> volumeLoderFactory) : VolumeVisualizerBase(camera, volumeLoderFactory) {}
	void ComputeFrameInternal(std::shared_ptr<unsigned char[]>& framebuffer, int downscale, const std::shared_ptr<MultiLayeredFramebufferBase>& multiLayeredFramebuffer) override {
		
		auto sizes = this->_camera->GetScreenSize();
		auto frameBufferSize = sizes[0] * sizes[1] * 4;

		for (size_t i = 0; i < frameBufferSize; i++)
		{
			framebuffer[i] = 0;
		}

	}
	bool DataChanged() override { return false; }
};

