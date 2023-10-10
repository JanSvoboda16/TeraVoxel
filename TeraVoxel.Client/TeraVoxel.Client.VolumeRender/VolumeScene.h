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

using Eigen::Vector3d;
using Eigen::Vector3i;
using Eigen::Vector4f;

template <typename T>
class VolumeScene : public IVolumeScene
{
public:
	VolumeScene(const std::shared_ptr<Camera>& camera, const ProjectInfo& projectInfo, const std::shared_ptr<VolumeLoaderFactory<T>>& volumeLoaderFactory, const std::shared_ptr<IVolumeVisualizerFactory<T>>& visualizerFac);
	~VolumeScene() override;
	void ComputeFrame(int width, int height, bool fast) override;
	int GetFrameWidth() override;
	int GetFrameHeight() override;
	bool DataChanged() override;
	std::shared_ptr<unsigned char[]> GetFrame() override;
	std::shared_ptr<Camera> GetCamera() override;
	bool FrameReady() override;
	bool RenderingInProgress() override;
	void ChangeVisualizer(std::shared_ptr<IVolumeVisualizerFactory<T>> visualizerFac);
	const char* GetDataTypeName() override;

private:
	int frameWidth1;				// Width of framebuffer1
	int frameHeight1;				// Height of framebuffer1
	int frameWidth2;				// Width of framebuffer2
	int frameHeight2;				// Height of framebuffer2
	bool _framebufferIndex = false; // Index of the currently used framebuffer

	ProjectInfo _projectInfo;

	// Framebuffers
	std::shared_ptr<unsigned char[]> _framebuffer1;
	std::shared_ptr<unsigned char[]> _framebuffer2;

	// Scene parts
	std::shared_ptr<Camera> _camera;
	std::shared_ptr<VolumeLoaderFactory<T>> _volumeLoaderFactory;
	std::shared_ptr<VolumeVisualizerBase<T>> _visualizer;
	std::shared_ptr<IVolumeVisualizerFactory<T>> _visualizerFactory;

	bool _visualizerChanged = false;
	std::atomic<bool> _frameReady = false;
	std::mutex _renderingMutex;
	std::future<void> _renderingThread;
	std::atomic<bool> _renderingInProgress = false;

	void ComputeFrameTask(int width, int height, bool _fast);
};

