#pragma once
#include "ColorMappingTable.h"
#include "VolumeVisualizerSettingsBase.h"

class CPURCVolumeVisualizerSettings : public VolumeVisualizerSettingsBase
{
public:
	ColorMappingTable mappingTable;

	float ambientIntensity = 0.5;
	float difustionIntensity = 1;
	float reflectionIntensity = 1;
	float reflectionSharpness = 5;

	bool shading = false;
};

