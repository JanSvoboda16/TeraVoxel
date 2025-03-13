/*
 * Author: Jan Svoboda
 * University: BRNO UNIVERSITY OF TECHNOLOGY, FACULTY OF INFORMATION TECHNOLOGY
 */

#pragma once
#include <memory>
#include <TeraVoxel.Client.Core\ProjectInfo.h>
#include "TeraVoxel.Client.VolumeRenderer/VolumeLoaderGenericBase.h"
#include "TeraVoxel.Client.VolumeRenderer/VolumeLoaderFactory.h"

class VolumeCacheGenericBase
{
public:
	VolumeCacheGenericBase(const DatasetInfo &info) : _datasetInfo(info){ }
	DatasetInfo GetDatasetInfo() { return _datasetInfo; }
	virtual ~VolumeCacheGenericBase() {};
	virtual void Flush() = 0;;
protected:
	DatasetInfo _datasetInfo;
};

