#pragma once
#include <TeraVoxel.Client.VolumeRenderer/VolumeScene.h>
#include <TeraVoxel.Client.VolumeRenderer/Camera.cuh>
#include <TeraVoxel.Client.Core/WindowNotification.h>

/// <summary>
/// This structure enables to edit scene across all windows. 
/// </summary>
struct VolumeViewContext
{
	// Contains a loaded scene
	// This scene is edited by other windows
	std::shared_ptr<VolumeScene> scene;

	// If the scene is edited, some windows must be notified about that change
	// This objects are used for that
	WindowNotification sceneReplaced;
	WindowNotification sceneUpdated;

	// Scene should be edited only when object get's this notification.
	WindowNotification sceneEditable;
};

