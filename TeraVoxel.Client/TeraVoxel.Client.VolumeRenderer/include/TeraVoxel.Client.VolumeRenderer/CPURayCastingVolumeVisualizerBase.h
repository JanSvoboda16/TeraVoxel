#pragma once
#include "TeraVoxel.Client.VolumeRenderer/VolumeVisualizerBase.h"
#include <any>
#include <TeraVoxel.Client.Core/TemplatedFunctionCaller.h>
#include "TeraVoxel.Client.VolumeRenderer/RayCastingUtilities.cuh"

class RayCastingVolumeVisualizerBase : public VolumeVisualizerBase
{
public:
	RayCastingVolumeVisualizerBase(const std::shared_ptr<Camera>& camera, const std::shared_ptr<VolumeLoaderFactory>& volumeLoaderFactory);
	bool ComputeRayIntersection(const Vector3f& rayDireciton, Vector3f& start, Vector3f& stop);
};


inline RayCastingVolumeVisualizerBase::RayCastingVolumeVisualizerBase(const std::shared_ptr<Camera>& camera,  const std::shared_ptr<VolumeLoaderFactory>& volumeLoaderFactory)
	: VolumeVisualizerBase(camera, volumeLoaderFactory)
{ }


inline bool RayCastingVolumeVisualizerBase::ComputeRayIntersection(const Vector3f& rayDireciton, Vector3f& start, Vector3f& stop)
{	
	auto projectInfo = _volumeLoaderFactory->GetDatasetInfo();
	auto dataSizes = Vector3f(projectInfo.dataSizeX, projectInfo.dataSizeY, projectInfo.dataSizeZ);

	return RayCastingUtilities::ComputeRayIntersection(rayDireciton, this->_camera->GetShrankPosition(), dataSizes, start, stop);
}