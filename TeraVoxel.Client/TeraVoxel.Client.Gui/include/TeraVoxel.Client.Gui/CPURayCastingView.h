/*
 * Author: Jan Svoboda
 * University: BRNO UNIVERSITY OF TECHNOLOGY, FACULTY OF INFORMATION TECHNOLOGY
 */
#pragma once
#include "TeraVoxel.Client.Gui/IView.h"
#include "imgui.h"
#include <TeraVoxel.Client.VolumeRenderer/ColorMappingTable.h>
#include <json.hpp>
#include "VolumeViewContext.h"
#include <TeraVoxel.Client.VolumeRenderer/CPURayCastingVolumeVisualizer.h>

using nlohmann::json;
namespace fs = std::filesystem;

class CPURayCastingView : IView
{
public:
	CPURayCastingView(std::shared_ptr<VolumeViewContext> volumeViewContext, std::shared_ptr<CPURCVolumeVisualizerSettings> visualizerSettings) :
		_volumeViewContext(volumeViewContext), _visualizerSettings(visualizerSettings) 
	{
		LoadTables();
	};

	void Update() override;

private:
	std::vector<std::string> _mappingTables; // Names of all mapping tables
	std::shared_ptr<CPURCVolumeVisualizerSettings> _visualizerSettings;
	std::shared_ptr<VolumeViewContext> _volumeViewContext;

	// Saves the current mapping table to a file
	void SaveToFile(std::string fileName);
	// Loads all table's names
	void LoadTables();
	// Loads the selected table
	void LoadTable(std::string fileName);
};

