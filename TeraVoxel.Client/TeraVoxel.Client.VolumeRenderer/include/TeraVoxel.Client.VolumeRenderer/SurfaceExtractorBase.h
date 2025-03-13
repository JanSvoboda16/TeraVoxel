#pragma once
#include "TeraVoxel.Client.VolumeRenderer/MeshNode.h"
#include "TeraVoxel.Client.VolumeRenderer/VolumeSegment.h"
#include <TeraVoxel.Client.Core/ProjectInfo.h>

class SurfaceExtractorBase
{
public:
	virtual ~SurfaceExtractorBase() {};
	virtual std::shared_ptr<MeshNode> ExtractSurface(const std::shared_ptr<VolumeSegment<bool>> &binMap, bool interpolate, const Eigen::Vector2f &interpolationBoundary) = 0;
};

