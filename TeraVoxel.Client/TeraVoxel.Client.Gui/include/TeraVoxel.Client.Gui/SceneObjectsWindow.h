/*
 * Author: Jan Svoboda
 * University: BRNO UNIVERSITY OF TECHNOLOGY, FACULTY OF INFORMATION TECHNOLOGY
 */
#pragma once
#include "TeraVoxel.Client.Gui/VolumeViewContext.h"
#include "TeraVoxel.Client.Gui/IView.h"

/// <summary>
/// This windows enables to turn on and off rasterized objects in the scene.
/// </summary>
class SceneObjectsWindow : public IView
{
public:
	SceneObjectsWindow(const std::shared_ptr<VolumeViewContext>& volumeViewContext): _volumeViewContext(volumeViewContext)
	{
		_volumeViewContext->sceneEditable.Register([this]() { this->UpdateScene(); });
		_volumeViewContext->sceneReplaced.Register([this]() { _sceneUpdateNeeded = true; });
	}

	void UpdateScene();
	void Update() override;

private:
	std::shared_ptr<VolumeViewContext> _volumeViewContext;

	bool _sceneUpdateNeeded = false;
	
	bool _crossVisible = true;
	bool _boundingBoxVisible = false;
};

