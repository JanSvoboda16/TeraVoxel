#pragma once
#include <TeraVoxel.Client.Core/ProjectInfo.h>

class VolumeLoaderGenericBase
{
public:
	VolumeLoaderGenericBase(const BlockBasedDatasetInfo& datasetInfo) : _datasetInfo(datasetInfo)
	{ }
	virtual ~VolumeLoaderGenericBase() {}
	virtual BlockBasedDatasetInfo& GetDatasetInfo() { return _datasetInfo; };
	virtual void BindOnBlockLoaded(std::function<bool(void)> function) = 0;

protected:
	BlockBasedDatasetInfo _datasetInfo;
};

