#pragma once
#include "VolumeVisualizerBase.h"
#include "CPURayCastingVolumeObjectMemory.h"

template <typename T>
class CPURayCastingVolumeVisualizerBase : public VolumeVisualizerBase<T>
{
protected:
	CPURayCastingVolumeObjectMemory<T> _memory;
public:
	CPURayCastingVolumeVisualizerBase(const std::shared_ptr<Camera>& camera, const ProjectInfo& projectInfo, const std::shared_ptr<VolumeLoaderFactory<T>>& volumeLoaderFactory, const std::shared_ptr<MeshNode>& meshNode);
	virtual void ComputeFrameInternal(int downscale) override = 0;
	bool ComputeRayIntersection(const Vector3f& rayDireciton, Vector3f& start, Vector3f& stop);
};

template<typename T>
inline CPURayCastingVolumeVisualizerBase<T>::CPURayCastingVolumeVisualizerBase(const std::shared_ptr<Camera>& camera, const ProjectInfo& projectInfo, const std::shared_ptr<VolumeLoaderFactory<T>>& volumeLoaderFactory, const std::shared_ptr<MeshNode>& meshNode)
	: VolumeVisualizerBase<T>(camera, projectInfo, volumeLoaderFactory, meshNode),
	_memory(camera, projectInfo, volumeLoaderFactory) { }

template <typename T>
bool CPURayCastingVolumeVisualizerBase<T>::ComputeRayIntersection(const Vector3f& rayDireciton, Vector3f& start, Vector3f& stop)
{
	// intersections of lines with planes:
	// 
	// position (any point on the line) (x1,y1,z1)
	// direction (a,b,c)
	// x = x1+a*k
	// y = y1+b*k
	// z = z1+c*k
	// 
	// the intersection of a line with a plane perpendicular to the Z-axis -> 'z' is constant
	// k=(z-z1)/c k>=0
	// pz=position + k*direction
	//
	// the intersection of a line with a plane perpendicular to the X-axis -> 'x' is constant
	// k=(x-x1)/b k>=0
	// px=position + k*direction
	//
	// the intersection of a line with a plane perpendicular to the Y-axis -> 'y' is constant
	// k=(y-y1)/a k>=0
	// py=position + k*direction

	auto dataSizes = _memory.GetDataSizes();
	Vector3f maxIndexes(dataSizes[0] - 1, dataSizes[1] - 1, dataSizes[2] - 1);
	Vector3f position = this->_camera->GetShrankPosition();
	Vector3f k = (-position).array() / rayDireciton.array();
	Vector3f intersections[6];
	short intersectionsCount = 0;


	for (size_t j = 0; j < 2; j++)
	{
		for (size_t i = 0; i < 3; i++)
		{
			// The intersection is in front of the camera
			if (k[i] >= 0)
			{
				Vector3f intersection = position + rayDireciton * k[i];
				if ((intersection[(i + 1) % 3] >= 0 && intersection[(i + 1) % 3] <= (maxIndexes[(i + 1) % 3]))
					&& (intersection[(i + 2) % 3] >= 0 && intersection[(i + 2) % 3] <= (maxIndexes[(i + 2) % 3])))
				{
					if (j == 0) {
						intersection[i] = 0;
					}
					else {
						intersection[i] = (maxIndexes[i]);
					}
					intersections[intersectionsCount++] = intersection;
				}
			}
		}
		k = (maxIndexes - position).array() / rayDireciton.array();
	}

	if (intersectionsCount == 0) {
		return false;
	}

	// The camera is inside of the volume
	if (position[0] >= 0 && position[1] >= 0 && position[2] >= 0 &&
		position[0] <= maxIndexes[0] && position[1] <= maxIndexes[1] && position[2] <= maxIndexes[2])
	{
		// Wall
		if (intersectionsCount == 1)
		{
			start = position;
			stop = intersections[0];
			return true;
		}
		// Edge, corner etc...
		// We use the farest
		else
		{
			float farest = (intersections[0] - position).norm();
			int farestIndex = 0;

			for (int i = 1; i < intersectionsCount; i++)
			{
				float distance = (intersections[i] - position).norm();
				if (distance > farest) {
					farest = distance;
					farestIndex = i;
				}
			}
			start = position;
			stop = intersections[farestIndex];
			return true;
		}
	}
	// The camera is outside of the volume
	else
	{
		// Two walls
		if (intersectionsCount == 2)
		{
			if ((intersections[0] - position).norm() > (intersections[1] - position).norm())
			{
				start = intersections[1];
				stop = intersections[0];
				return true;
			}
			else
			{
				start = intersections[0];
				stop = intersections[1];
				return true;
			}
		}
		// Different -> we use the closest and the farest
		else if (intersectionsCount >= 3) {

			float closest = (intersections[0] - position).norm();
			int closestIndex = 0;
			float farest = (intersections[0] - position).norm();
			int farestIndex = 0;

			for (int i = 1; i < intersectionsCount; i++)
			{
				float distance = (intersections[i] - position).norm();
				if (distance > farest) {
					farest = distance;
					farestIndex = i;
				}
				if (distance < closest) {
					closest = distance;
					closestIndex = i;
				}
			}

			if (closestIndex != farestIndex) {
				start = intersections[closestIndex];
				stop = intersections[farestIndex];
				return true;
			}
		}
	}

	return false;
}