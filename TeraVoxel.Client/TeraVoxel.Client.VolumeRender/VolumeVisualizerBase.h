#pragma once
#include <memory>
#include "VolumeObjectMemory.h"
#include <thread>

struct color
{
	uint8_t r, g, b, a;
};

template <typename T>
class VolumeVisualizerBase
{
protected:

	std::shared_ptr<Camera> _camera;
	std::shared_ptr<VolumeObjectMemory<T>> _memory;
	std::shared_ptr<unsigned char[]> _framebuffer;

	virtual void ComputeFrameInternal(int downscale) = 0;

public:
	VolumeVisualizerBase(std::shared_ptr<Camera> camera, std::shared_ptr<VolumeObjectMemory<T>> memory);
	void ComputeFrame(std::shared_ptr<unsigned char[]> _framebuffer, int width, int height, int downscale);
};

template<typename T>
inline VolumeVisualizerBase<T>::VolumeVisualizerBase(std::shared_ptr<Camera> camera, std::shared_ptr<VolumeObjectMemory<T>> memory)
{
	_camera = camera;
	_memory = memory;
}

template<typename T>
void VolumeVisualizerBase<T>::ComputeFrame(std::shared_ptr<unsigned char[]> framebuffer, int width, int height, int downscale)
{
	_framebuffer = framebuffer;
	_camera->ChangeScreenSize(width, height);
	_memory->Prepare();

	ComputeFrameInternal(downscale);

	_memory->Revalidate();
}