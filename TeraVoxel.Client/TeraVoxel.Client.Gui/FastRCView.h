#pragma once
#include "IView.h"
#include "imgui.h"
#include "../TeraVoxel.Client.VolumeRender/ColorMappingTable.h"
#include "imgui_stdlib.h"
#include <iostream>
#include <filesystem>
#include <fstream>
#include "../TeraVoxel.Client.Core/nlohman/json.hpp"
#include "VolumeViewContext.h"
#include "../TeraVoxel.Client.VolumeRender/FastRayCastingVolumeVisualizer.h"

using nlohmann::json;
namespace fs = std::filesystem;

class FastRCView : IView
{
public:
	FastRCView(std::shared_ptr<VolumeViewContext> volumeViewContext, std::shared_ptr<FastRCVolumeVisualizerSettings> visualizerSettings) :
		_volumeViewContext(volumeViewContext), _visualizerSettings(visualizerSettings) 
	{
		LoadTables();
	};

	void Update() override;

private:
	std::vector<std::string> _mappingTables; // Names of all mapping tables
	std::shared_ptr<FastRCVolumeVisualizerSettings> _visualizerSettings;
	std::shared_ptr<VolumeViewContext> _volumeViewContext;

	// Saves the current mapping table to a file
	void SaveToFile(std::string fileName);
	// Loads all table's names
	void LoadTables();
	// Loads the selected table
	void LoadTable(std::string fileName);
};

