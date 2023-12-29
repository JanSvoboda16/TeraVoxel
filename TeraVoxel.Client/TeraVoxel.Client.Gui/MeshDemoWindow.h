#pragma once
#include "VolumeViewContext.h"
class MeshDemoWindow
{
public:

	MeshDemoWindow(const std::shared_ptr<VolumeViewContext>& volumeViewContext): _volumeViewContext(volumeViewContext)
	{
		PrepareMeshes();
		_volumeViewContext->sceneEditable.Register([this]() { this->UpdateScene(); });
		_volumeViewContext->sceneReplaced.Register([this]() { _sceneUpdateNeeded = true; });
	}

	void PrepareMeshes();
	void UpdateScene();
	void Update();

private:
	std::shared_ptr<VolumeViewContext> _volumeViewContext;
	std::shared_ptr<MeshNode> _sphere;
	std::shared_ptr<MeshNode> _cube;
	std::shared_ptr<MeshNode> _plane;
	float _meshPosition[3] = { 0,0,0 };
	float _meshScale[3] = { 1,1,1 };
	int _alpha = 150;
	int _alphaChanged = false;
	int _demoId = 0;
	bool _sceneUpdateNeeded = false;
};

