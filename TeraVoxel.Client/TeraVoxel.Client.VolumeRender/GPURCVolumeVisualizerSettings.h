#pragma once
#include "MaterialTable.cuh"
#include "VolumeVisualizerSettingsBase.h"
#include "../TeraVoxel.Client.VolumeRender/GPUEntity.h"

class GPURCVolumeVisualizerSettings: public GPUEntity, public VolumeVisualizerSettingsBase
{
public:
	MaterialTable materialTable;
	float ambientIntensity = 0.5f;
};

