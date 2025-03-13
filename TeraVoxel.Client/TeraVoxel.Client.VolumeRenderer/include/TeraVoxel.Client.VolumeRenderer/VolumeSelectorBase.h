/*
 * Author: Jan Svoboda
 * University: BRNO UNIVERSITY OF TECHNOLOGY, FACULTY OF INFORMATION TECHNOLOGY
 */

#pragma once
#include "TeraVoxel.Client.VolumeRenderer/VolumeCache.h"
#include "TeraVoxel.Client.VolumeRenderer/VolumeSegment.h"

class VolumeSelectorBase
{
public:
	virtual ~VolumeSelectorBase() {};
	VolumeSelectorBase(const std::shared_ptr<VolumeCacheGenericBase>& volumeCache);
	void Reset();
	std::shared_ptr<VolumeSegment<bool>> GetMask(){	return _mask;}

protected:
	DatasetInfo _datasetInfo;
	std::shared_ptr<VolumeCacheGenericBase> _volumeCache;
	std::shared_ptr<VolumeSegment<bool>> _mask;
};

