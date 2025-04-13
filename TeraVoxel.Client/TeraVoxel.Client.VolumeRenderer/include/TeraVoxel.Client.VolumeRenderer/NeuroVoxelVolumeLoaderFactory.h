#pragma once
#include "TeraVoxel.Client.VolumeRenderer/VolumeLoaderFactory.h"
#include "TeraVoxel.Client.VolumeRenderer/NeuroVoxelVolumeLoader.h"
#include <TeraVoxel.Client.Core/TemplatedFunctionCaller.h>

static BlockBasedDatasetInfo BBDFromMetadata(const NeuroVoxel::DatasetMetadata& metadata, int segmentSize) {
	BlockBasedDatasetInfo info;
	info.dataSizeX = metadata.dataDimensions[0];
	info.dataSizeY = metadata.dataDimensions[1];
	info.dataSizeZ = metadata.dataDimensions[2];
	info.dataType = metadata.dataType;
	info.segmentSize = segmentSize;
	info.sizeX = std::ceil(info.dataSizeX / (float)info.segmentSize) * info.segmentSize;
	info.sizeY = std::ceil(info.dataSizeY / (float)info.segmentSize) * info.segmentSize;
	info.sizeZ = std::ceil(info.dataSizeZ / (float)info.segmentSize) * info.segmentSize;
	info.voxelDimensions = metadata.voxelSize;
	return info;
}

class NeuroVoxelVolumeLoaderFactory : public VolumeLoaderFactory
{
public:

	NeuroVoxelVolumeLoaderFactory(const std::shared_ptr<NeuroVoxel::IReadableCompressedDataset>& dataset) :
		_dataset(dataset),
		VolumeLoaderFactory(BBDFromMetadata(dataset->GetMetadata(), 256))
	{ }

	std::unique_ptr<VolumeLoaderGenericBase> Create(int threadCount) override
	{
		return CALL_TEMPLATED_FUNCTION2(CreateInternal, _datasetInfo.dataType, threadCount);
	}

private:

	std::shared_ptr<NeuroVoxel::IReadableCompressedDataset> _dataset;
	template <typename T>
	std::unique_ptr<VolumeLoaderGenericBase> CreateInternal(int threadCount)
	{
		return std::make_unique<NeuroVoxelVolumeLoader<T>>(_dataset, threadCount, _datasetInfo);
	}
};

