#pragma once
#include "MaterialTable.cuh"
#include "VolumeVisualizerSettingsBase.h"
#include "../TeraVoxel.Client.VolumeRender/GPUEntity.h"
#include "../TeraVoxel.Client.Core/nlohman/json.hpp"

struct Light
{
	float position[3];
	float intensity;

	NLOHMANN_DEFINE_TYPE_INTRUSIVE(Light, position, intensity);

	Light() : position{ 0, 0, 0 }, intensity(0) { }
};

struct LightSettings 
{
	float ambientIntensity = 0.02f;
	Light lights[5];
	uint8_t numLights = 0;

	NLOHMANN_DEFINE_TYPE_INTRUSIVE(LightSettings, ambientIntensity, lights, numLights);
};

class GPURCVolumeVisualizerSettings: public GPUEntity, public VolumeVisualizerSettingsBase
{
public:
	MaterialTable materialTable;
	LightSettings lightSettings;
};

