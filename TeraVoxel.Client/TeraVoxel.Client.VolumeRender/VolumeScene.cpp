/*
 * Author: Jan Svoboda
 * University: BRNO UNIVERSITY OF TECHNOLOGY, FACULTY OF INFORMATION TECHNOLOGY
 */
#include "pch.h"
#include "VolumeScene.h"
#include "../TeraVoxel.Client.Core/TypeToString.h"

VolumeScene::VolumeScene(const std::shared_ptr<Camera>& camera, const std::shared_ptr<VolumeLoaderFactory>& volumeLoaderFactory, const std::shared_ptr<IVolumeVisualizerFactory>& visualizerFactory, const std::shared_ptr<MeshNode>& meshNode) //TODO new constructor
{
	_camera = camera;
	_volumeVisualizer = visualizerFactory->Create(camera, volumeLoaderFactory, meshNode);
	_volumeLoaderFactory = volumeLoaderFactory;
	_meshNode = meshNode;
}

VolumeScene::~VolumeScene()
{
	if (_renderingThread.valid())
	{
		_renderingThread.wait();
	}
}

void VolumeScene::ComputeFrame(int width, int height, bool _fast)
{
	_framebufferIndex = !_framebufferIndex;

	int* frameWidth;
	int* frameHeight;
	if (_framebufferIndex)
	{
		frameWidth = &frameWidth1;
		frameHeight = &frameHeight1;
	}
	else
	{
		frameWidth = &frameWidth2;
		frameHeight = &frameHeight2;
	}

	// Creating a framebuffer with sizes of a frame if actual sizes doesn't match
	if (*frameWidth != width || *frameHeight != height)
	{
		*frameWidth = width;
		*frameHeight = height;
		if (_framebufferIndex)
		{
			_framebuffer1 = std::shared_ptr<unsigned char[]>(new unsigned char[width * height * 4]);
		}
		else
		{
			_framebuffer2 = std::shared_ptr<unsigned char[]>(new unsigned char[width * height * 4]);
		}
		_camera->ChangeScreenSize(width, height);
	}

	_frameReady.store(false, std::memory_order::relaxed);
	_renderingInProgress.store(true, std::memory_order::relaxed);

	if (_visualizerChanged)
	{
		_volumeVisualizer = _visualizerFactory->Create(_camera, _volumeLoaderFactory, _meshNode);
		_visualizerChanged = false;
	}

	_renderingThread = std::async(std::launch::async, &VolumeScene::ComputeFrameTask, this, width, height, _fast);
}

int VolumeScene::GetFrameWidth()
{
	return _framebufferIndex ? frameWidth1 : frameWidth2;
}

int VolumeScene::GetFrameHeight()
{
	return _framebufferIndex ? frameHeight1 : frameHeight2;
}

bool VolumeScene::DataChanged()
{
	return _volumeVisualizer->DataChanged();
}

std::shared_ptr<unsigned char[]> VolumeScene::GetFrame()
{
	if (!_frameReady)
	{
		//TODO
	}
	return  _framebufferIndex ? _framebuffer1 : _framebuffer2;
}

std::shared_ptr<Camera> VolumeScene::GetCamera()
{
	return _camera;
}

std::shared_ptr<MeshNode> VolumeScene::GetMeshNode()
{
	return _meshNode;
}

bool VolumeScene::FrameReady()
{
	return _frameReady.load(std::memory_order::acquire);
}

bool VolumeScene::RenderingInProgress()
{
	return _renderingInProgress;
}

void VolumeScene::ChangeVisualizer(std::shared_ptr<IVolumeVisualizerFactory> visualizerFac)
{
	_visualizerFactory = visualizerFac;
	_visualizerChanged = true;
}

ProjectInfo VolumeScene::GetProjectInfo() 
{ 
	return _volumeLoaderFactory->GetProjectInfo();
}

void VolumeScene::ComputeFrameTask(int width, int height, bool _fast)
{
	Logger::GetInstance()->LogEvent("VolumeScene", "Rendering:Started", "", _fast ? "fast" : "full");
	
	auto meshObject = std::make_shared<MeshNode>();
	
	_volumeVisualizer->ComputeFrame(_framebufferIndex ? _framebuffer1 : _framebuffer2, width, height, _fast ? 2 : 1);

	_frameReady.store(true, std::memory_order::release);
	_renderingInProgress.store(false, std::memory_order::release);
	Logger::GetInstance()->LogEvent("VolumeScene", "Rendering:Ended", "", _fast ? "fast" : "full");
}