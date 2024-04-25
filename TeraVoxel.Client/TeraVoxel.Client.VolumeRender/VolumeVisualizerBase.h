#pragma once
#include <memory>
#include "Camera.h"
#include "../TeraVoxel.Client.Core/ProjectInfo.h"
#include "VolumeLoaderFactory.h"
#include "MeshNode.h"
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
	std::shared_ptr<unsigned char[]> _framebuffer;
	std::shared_ptr<VolumeLoaderFactory> _volumeLoaderFactory;
	ProjectInfo _projectInfo;

	virtual void ComputeFrameInternal(int downscale) = 0;

public:
	VolumeVisualizerBase(const std::shared_ptr<Camera>& camera, const std::shared_ptr<VolumeLoaderFactory>& volumeLoaderFactory, const std::shared_ptr<MeshNode>& meshNode);
	virtual ~VolumeVisualizerBase() {};
	void ComputeFrame(std::shared_ptr<unsigned char[]> _framebuffer, int width, int height, int downscale);
	virtual bool DataChanged() = 0;
};

inline VolumeVisualizerBase::VolumeVisualizerBase(const std::shared_ptr<Camera>& camera, const std::shared_ptr<VolumeLoaderFactory>& volumeLoaderFactory, const std::shared_ptr<MeshNode>& meshNode)
{
	_camera = camera;
	_volumeLoaderFactory = volumeLoaderFactory;
	_projectInfo = volumeLoaderFactory->GetProjectInfo();
	_meshNode = meshNode;
}

inline void VolumeVisualizerBase::ComputeFrame(std::shared_ptr<unsigned char[]> framebuffer, int width, int height, int downscale)
{
	_framebuffer = framebuffer;
	_camera->ChangeScreenSize(width, height);

	ComputeFrameInternal(downscale);
}