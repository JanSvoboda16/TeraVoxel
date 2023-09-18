#pragma once
#include "VolumeVisualizerBase.h"
#include "VolumeVisualizerSettingsBase.h"

class EmptyVolumeVisualizerSettings: public VolumeVisualizerSettingsBase
{

};

template <typename T>
class EmptyVolumeVisualizer: public VolumeVisualizerBase<T>
{
public:
	EmptyVolumeVisualizer(std::shared_ptr<Camera> camera, std::shared_ptr<VolumeObjectMemory<T>> memory) : VolumeVisualizerBase<T>(camera, memory) {}
	void ComputeFrameInternal(int downscale) override {
		
		auto sizes = this->_camera->GetScreenSize();
		auto frameBufferSize = sizes[0] * sizes[1] * 4;

		for (size_t i = 0; i < frameBufferSize; i++)
		{
			this->_framebuffer[i] = 0;
		}

	}
};

