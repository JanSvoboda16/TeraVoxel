#pragma once
#include <memory>
#include "Camera.cuh"
#include "../TeraVoxel.Client.Core/ProjectInfo.h"
#include "VolumeLoaderFactory.h"
#include "MeshNode.h"
#include "MultiLayeredFramebufferBase.h"
#include <thread>

struct color
{
	uint8_t r, g, b, a;
};

class VolumeVisualizerBase
{
protected:
	std::shared_ptr<Camera> _camera;
	std::shared_ptr<MeshNode> _meshNode;
	std::shared_ptr<VolumeLoaderFactory> _volumeLoaderFactory;
	ProjectInfo _projectInfo;

	virtual void ComputeFrameInternal(std::shared_ptr<unsigned char[]>& framebuffer, int downscale, const std::shared_ptr<MultiLayeredFramebufferBase>& multiLayeredFramebuffer) = 0;

public:
	VolumeVisualizerBase(const std::shared_ptr<Camera>& camera, const std::shared_ptr<VolumeLoaderFactory>& volumeLoaderFactory);
	virtual ~VolumeVisualizerBase() {};
	void ComputeFrame(std::shared_ptr<unsigned char[]>& _framebuffer, int width, int height, int downscale, const std::shared_ptr<MultiLayeredFramebufferBase>& multiLayeredFramebuffer = nullptr);
	virtual bool DataChanged() = 0;
};

inline VolumeVisualizerBase::VolumeVisualizerBase(const std::shared_ptr<Camera>& camera, const std::shared_ptr<VolumeLoaderFactory>& volumeLoaderFactory)
{
	_camera = camera;
	_volumeLoaderFactory = volumeLoaderFactory;
	_projectInfo = volumeLoaderFactory->GetProjectInfo();
}

inline void VolumeVisualizerBase::ComputeFrame(std::shared_ptr<unsigned char[]>& framebuffer, int width, int height, int downscale, const std::shared_ptr<MultiLayeredFramebufferBase>& multiLayeredFramebuffer)
{
	_camera->ChangeScreenSize(width, height);

	ComputeFrameInternal(framebuffer, downscale, multiLayeredFramebuffer);
}