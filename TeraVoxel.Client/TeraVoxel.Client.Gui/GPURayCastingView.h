#pragma once
#pragma once
#include "IView.h"
#include "imgui.h"
#include "../TeraVoxel.Client.VolumeRender/ColorMappingTable.h"
#include "../TeraVoxel.Client.Core/nlohman/json.hpp"
#include "VolumeViewContext.h"
#include "../TeraVoxel.Client.VolumeRender/GPURCVolumeVisualizerSettings.h"

using nlohmann::json;
namespace fs = std::filesystem;

class GPURayCastingView : public IView
{
public:
	GPURayCastingView(std::shared_ptr<VolumeViewContext> volumeViewContext, std::shared_ptr<GPURCVolumeVisualizerSettings> visualizerSettings);;

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
	// 
	void UpdateSettings();
};

