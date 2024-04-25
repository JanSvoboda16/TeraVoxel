#pragma once
#include "MeshNode.h"
#include "VolumeSegment.h"
#include "../TeraVoxel.Client.Core/ProjectInfo.h"

class SurfaceExtractorBase
{
public:
	virtual ~SurfaceExtractorBase() {};
	virtual std::shared_ptr<MeshNode> ExtractSurface(const std::shared_ptr<VolumeSegment<bool>> &binMap, const ProjectInfo &projectInfo,  bool interpolate, const Eigen::Vector2f &interpolationBoundary) = 0;
};

