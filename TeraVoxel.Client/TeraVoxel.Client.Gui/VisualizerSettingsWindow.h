/*
 * Author: Jan Svoboda
 * University: BRNO UNIVERSITY OF TECHNOLOGY, FACULTY OF INFORMATION TECHNOLOGY
 */
#pragma once
#include "imgui.h"
#include "../TeraVoxel.Client.VolumeRender/ColorMappingTable.h"
#include "imgui_stdlib.h"
#include <iostream>
#include <filesystem>
#include <fstream>
#include "../TeraVoxel.Client.Core/nlohman/json.hpp"
#include "FastRCView.h"
#include "../TeraVoxel.Client.VolumeRender/VolumeVisualizerSetter.h"
#include "../TeraVoxel.Client.VolumeRender/FastRcVolumeVisualizerSetter.h"
#include "../TeraVoxel.Client.VolumeRender/EmptyVolumeVisualizerSetter.h"

// Used for changing and controlling visualizers
class VisualizerSettingsWindow : IView
{
public:
	VisualizerSettingsWindow(std::shared_ptr<VolumeViewContext> volumeViewContext) : _volumeViewContext(volumeViewContext) 
	{
		_fastRayCastingVisualizerSettings = std::make_shared<FastRCVolumeVisualizerSettings>();
		_volumeViewContext->sceneReplaced.Register([this]() { SetVisualizer(_selectedVisualizerId); });

		SetVisualizer(_selectedVisualizerId);
		ChangeView(_selectedVisualizerId);
	};
	void Update() override;

private:
	std::shared_ptr<VolumeViewContext> _volumeViewContext;
	std::shared_ptr<FastRCVolumeVisualizerSettings> _fastRayCastingVisualizerSettings;
	std::shared_ptr<IView> _view;

	int _selectedVisualizerId = 1;

	/// <summary>
	/// Set a selected visualizer to the scene
	/// </summary>
	/// <param name="visualizerID">Id of the selected visualizer</param>
	void SetVisualizer(int visualizerID);

	/// <summary>
	/// Changes a visualizer view depend on the current visualizer
	/// </summary>
	/// <param name="visualizerId">Id of the current visualizer</param>
	void ChangeView(int visualizerId);
};

