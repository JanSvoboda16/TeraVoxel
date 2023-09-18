#pragma once
#include "ColorMappingTable.h"
#include "VolumeVisualizerSettingsBase.h"

class FastRCVolumeVisualizerSettings : public VolumeVisualizerSettingsBase
{
public:
	ColorMappingTable mappingTable;

	float ampbientIntensity = 0.5;
	float difustionIntensity = 0.5;
	float reflectionIntensity = 5;
	float reflectionSharpness = 20;

	bool shading = false;
};

