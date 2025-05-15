/*
 * Author: Jan Svoboda
 * University: BRNO UNIVERSITY OF TECHNOLOGY, FACULTY OF INFORMATION TECHNOLOGY
 */
#pragma once
#include <TeraVoxel.Client.Core/ProjectInfo.h>

class VolumeLoaderBase
{
public:
	VolumeLoaderBase(const BlockBasedDatasetInfo& datasetInfo) : _datasetInfo(datasetInfo)
	{ }
	virtual ~VolumeLoaderBase() {}
	virtual BlockBasedDatasetInfo& GetDatasetInfo() { return _datasetInfo; };
	virtual void BindOnBlockLoaded(std::function<bool(void)> function) = 0;

protected:
	BlockBasedDatasetInfo _datasetInfo;
};

