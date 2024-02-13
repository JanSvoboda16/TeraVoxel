#pragma once
#include "VolumeViewContext.h"
class SceneObjectsWindow
{
public:

	SceneObjectsWindow(const std::shared_ptr<VolumeViewContext>& volumeViewContext): _volumeViewContext(volumeViewContext)
	{
		_volumeViewContext->sceneEditable.Register([this]() { this->UpdateScene(); });
		_volumeViewContext->sceneReplaced.Register([this]() { _sceneUpdateNeeded = true; });
	}

	void UpdateScene();
	void Update();

private:
	std::shared_ptr<VolumeViewContext> _volumeViewContext;

	bool _sceneUpdateNeeded = false;
	
	bool _crossVisible = true;
	bool _boundingBoxVisible = false;
};

