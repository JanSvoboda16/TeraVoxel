/*
 * Author: Jan Svoboda
 * University: BRNO UNIVERSITY OF TECHNOLOGY, FACULTY OF INFORMATION TECHNOLOGY
 */
#pragma once
#include<memory>
#include"Camera.h"
#include <mutex>
#include "CPURayCastingVolumeObjectMemory.h"
#include <future>
#include "ColorMappingTable.h"
#include "IVolumeScene.h"
#include "IVolumeVisualizerFactory.h"
#include "../TeraVoxel.Client.Core/TypeToString.h"
#include "CPUMeshVisualizer.h"

using Eigen::Vector3d;
using Eigen::Vector3i;
using Eigen::Vector4f;

class VolumeScene
{
public:
	VolumeScene(const std::shared_ptr<Camera>& camera, const std::shared_ptr<VolumeLoaderFactory>& volumeLoaderF, const std::shared_ptr<IVolumeVisualizerFactory>& visualizerFac, const std::shared_ptr<MeshNode>& meshNode);
	~VolumeScene();
	void ComputeFrame(int width, int height, bool fast) ;
	int GetFrameWidth();
	int GetFrameHeight();
	bool DataChanged();
	std::shared_ptr<unsigned char[]> GetFrame();
	std::shared_ptr<Camera> GetCamera();
	std::shared_ptr<MeshNode> GetMeshNode();
	std::shared_ptr<VolumeLoaderFactory> GetVolumeLoaderFactory();
	bool FrameReady();
	bool RenderingInProgress();
	void ChangeVisualizer(std::shared_ptr<IVolumeVisualizerFactory> visualizerFac);
	ProjectInfo GetProjectInfo();

private:
	int frameWidth1;				// Width of framebuffer1
	int frameHeight1;				// Height of framebuffer1
	int frameWidth2;				// Width of framebuffer2
	int frameHeight2;				// Height of framebuffer2
	bool _framebufferIndex = false; // Index of the currently used framebuffer

	// Framebuffers
	std::shared_ptr<unsigned char[]> _framebuffer1;
	std::shared_ptr<unsigned char[]> _framebuffer2;

	// Scene parts
	std::shared_ptr<Camera> _camera;
	std::shared_ptr<MeshNode> _meshNode;
	std::shared_ptr<VolumeLoaderFactory> _volumeLoaderFactory;
	std::shared_ptr<VolumeVisualizerBase> _volumeVisualizer;
	std::shared_ptr<IVolumeVisualizerFactory> _visualizerFactory;

	bool _visualizerChanged = false;
	std::atomic<bool> _frameReady = false;
	std::mutex _renderingMutex;
	std::future<void> _renderingThread;
	std::atomic<bool> _renderingInProgress = false;

	void ComputeFrameTask(int width, int height, bool _fast);
};

