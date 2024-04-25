/*
 * Author: Jan Svoboda
 * University: BRNO UNIVERSITY OF TECHNOLOGY, FACULTY OF INFORMATION TECHNOLOGY
 */

#pragma once
#include "VolumeSelectorBase.h"
#include <map>
#include <Eigen/Dense>

using Eigen::Vector3f;
using Eigen::Vector3i;

class SeedVolumeSelector : public VolumeSelectorBase
{
public:
	SeedVolumeSelector(const std::shared_ptr<VolumeCacheGenericBase>& volumeCache) : VolumeSelectorBase(volumeCache) { }
	void Select(const Vector3f &seed, float lowerBound, float upperBound, float maxDifference, bool erase = false);

private:	
	template<typename T>
	void SelectTemplated(const Vector3f seed, float lowerBound, float upperBound, float maxDifference, bool erase);
	template<typename T>
	bool GetData(const Vector3i& position, T& value, const std::shared_ptr<VolumeCache<T>>& cache);
};

