/*
 * Author: Jan Svoboda
 * University: BRNO UNIVERSITY OF TECHNOLOGY, FACULTY OF INFORMATION TECHNOLOGY
 */

#pragma once
#include "TeraVoxel.Client.VolumeRenderer/VolumeCache.h"
#include "TeraVoxel.Client.VolumeRenderer/VolumeBlock.h"

class VolumeSelectorBase
{
public:
	virtual ~VolumeSelectorBase() {};
	VolumeSelectorBase(const std::shared_ptr<VolumeCacheBase>& volumeCache);
	void Reset();
	std::shared_ptr<VolumeBlock<bool>> GetMask(){	return _mask;}

protected:
	DatasetInfo _datasetInfo;
	std::shared_ptr<VolumeCacheBase> _volumeCache;
	std::shared_ptr<VolumeBlock<bool>> _mask;
};

