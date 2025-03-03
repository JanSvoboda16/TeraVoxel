/*
 * Author: Jan Svoboda
 * University: BRNO UNIVERSITY OF TECHNOLOGY, FACULTY OF INFORMATION TECHNOLOGY
 */
#pragma once
#pragma once
#include "TeraVoxel.Client.Gui/IView.h"
#include "imgui.h"
#include <TeraVoxel.Client.VolumeRenderer/ColorMappingTable.h>
#include <json.hpp>
#include "TeraVoxel.Client.Gui/VolumeViewContext.h"
#include <TeraVoxel.Client.VolumeRenderer/GPURCVolumeVisualizerSettings.h>

using nlohmann::json;
namespace fs = std::filesystem;

class GPURayCastingView : public IView
{
public:
	GPURayCastingView(std::shared_ptr<VolumeViewContext> volumeViewContext, std::shared_ptr<GPURCVolumeVisualizerSettings> visualizerSettings);;
	~GPURayCastingView();

	void Update() override;

private:
	std::vector<std::string> _materialTables; // Names of all mapping tables
	std::vector<std::string> _lightings;

	std::shared_ptr<GPURCVolumeVisualizerSettings> _visualizerSettings;
	std::shared_ptr<VolumeViewContext> _volumeViewContext;

	GPURCVolumeVisualizerSettings _visualizerSettingsPrivate;

	// Saves the current mapping table to a file
	void SaveTable(std::string fileName);
	// Loads all table's names
	void LoadTables();
	// Loads the selected table
	void LoadTable(std::string fileName);

	void SaveLighting(std::string fileName);
	void LoadLightings();
	void LoadLighting(std::string fileName);
	
	void UpdateSettings();
};

