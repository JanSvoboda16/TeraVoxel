#pragma once
#include <memory>
#include "TeraVoxel.Client.VolumeRenderer/VolumeLoaderGenericBase.h"
#include <TeraVoxel.Client.Core/SettingsContext.h>

class VolumeLoaderFactory
{
public:
	virtual ~VolumeLoaderFactory() {}
	virtual std::unique_ptr<VolumeLoaderBase> Create(int threadCount = SettingsContext::GetInstance().loadingThreadCount) = 0;
	BlockBasedDatasetInfo GetDatasetInfo() { return _datasetInfo; }
	VolumeLoaderFactory(const BlockBasedDatasetInfo& datasetInfo) : _datasetInfo(datasetInfo) { }

protected:
	BlockBasedDatasetInfo _datasetInfo;
};

