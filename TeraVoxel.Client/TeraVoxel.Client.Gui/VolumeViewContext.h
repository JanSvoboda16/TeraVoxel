#pragma once
#include "../../TeraVoxel.Client.VolumeRender/IVolumeScene.h"
#include "../../TeraVoxel.Client.VolumeRender/Camera.h"
#include "../../TeraVoxel.Client.Core/WindowNotification.h"


struct VolumeViewContext
{
	// Contains a loaded scene
	// This scene is edited by other windows
	shared_ptr<IVolumeScene> scene;

	// If the scene is edited, some windows must be notified about that change
	// This objects are used for that
	WindowNotification sceneReplaced;
	WindowNotification sceneUpdated;
};

