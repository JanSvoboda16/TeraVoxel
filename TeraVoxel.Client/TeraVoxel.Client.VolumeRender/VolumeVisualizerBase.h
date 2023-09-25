#pragma once
#include <memory>
#include "Camera.h"
#include "../TeraVoxel.Client.Core/ProjectInfo.h"
#include "VolumeLoaderFactory.h"
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
	std::shared_ptr<unsigned char[]> _framebuffer;
	std::shared_ptr<VolumeLoaderFactory<T>> _volumeLoaderFactory;
	ProjectInfo _projectInfo;

	virtual void ComputeFrameInternal(int downscale) = 0;

public:
	VolumeVisualizerBase(const std::shared_ptr<Camera>& camera, const ProjectInfo& projectInfo, const std::shared_ptr<VolumeLoaderFactory<T>>& volumeLoaderFactory);
	virtual ~VolumeVisualizerBase() {};
	void ComputeFrame(std::shared_ptr<unsigned char[]> _framebuffer, int width, int height, int downscale);
	virtual bool DataChanged() = 0;
};

template<typename T>
inline VolumeVisualizerBase<T>::VolumeVisualizerBase(const std::shared_ptr<Camera>& camera, const ProjectInfo& projectInfo, const std::shared_ptr<VolumeLoaderFactory<T>>& volumeLoaderFactory)
{
	_camera = camera;
	_volumeLoaderFactory = _volumeLoaderFactory;
	_projectInfo = projectInfo;
}

template<typename T>
void VolumeVisualizerBase<T>::ComputeFrame(std::shared_ptr<unsigned char[]> framebuffer, int width, int height, int downscale)
{
	_framebuffer = framebuffer;
	_camera->ChangeScreenSize(width, height);
	ComputeFrameInternal(downscale);
}