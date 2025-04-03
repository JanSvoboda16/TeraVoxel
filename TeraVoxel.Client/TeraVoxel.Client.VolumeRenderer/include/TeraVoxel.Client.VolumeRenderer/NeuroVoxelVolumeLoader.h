#pragma once
#include "TeraVoxel.Client.VolumeRenderer/VolumeLoaderBase.h"
//#include "TeraVoxel.Client.Core/TypeToString.h"
#include <NeuroVoxel/CompressedDataset.h>
#include <NeuroVoxel/MultiHashDataReader.h>
#include <TeraVoxel.Client.Core/NeuroVoxelServerService.h>
#include <filesystem>

template <typename VoxelT>
class NeuroVoxelVolumeLoader : public VolumeLoaderBase<VoxelT>
{
public:
	NeuroVoxelVolumeLoader(const std::shared_ptr<NeuroVoxel::IReadableCompressedDataset>& dataset, int threadCount, const BlockBasedDatasetInfo& datasetInfo) :
		VolumeLoaderBase<VoxelT>(datasetInfo, threadCount),
		_dataset(dataset)
	{
		_reader = std::make_shared<NeuroVoxel::MultiHashDataReader<VoxelT>>(_dataset);
	}

	~NeuroVoxelVolumeLoader()
	{
		this->_endLoopingThreads = true;
		this->_loadingTreads.clear();
	}

	VoxelT* LoadSegmentData(int x, int y, int z, int downscale) override
	{
		int downscaleLevels = 4;

		float levelsPerModel = float(downscaleLevels) / _dataset->GetMetadata().levels;

		auto readedData = _reader->ReadData(
			Eigen::Vector3i(x * this->_datasetInfo.segmentSize, y * this->_datasetInfo.segmentSize, z * this->_datasetInfo.segmentSize),
			Eigen::Vector3i((x + 1) * this->_datasetInfo.segmentSize, (y + 1) * this->_datasetInfo.segmentSize, (z + 1) * this->_datasetInfo.segmentSize),
			downscale, int(downscale / levelsPerModel));

		uint32_t segmentSize = this->_datasetInfo.segmentSize;

		segmentSize = segmentSize >> downscale;
		VoxelT* data = new VoxelT[segmentSize * segmentSize * segmentSize];

		for (int i = 0; i < segmentSize * segmentSize * segmentSize; i++)
		{
			uint_fast16_t zpos = i / (segmentSize * segmentSize);
			uint_fast32_t mod = i % (segmentSize * segmentSize);
			uint_fast16_t ypos = mod / segmentSize;
			uint_fast16_t xpos = mod % segmentSize;

			auto index = Serialization::GetZCurveIndex(xpos, ypos, zpos);
			data[index] = readedData[i];
		}

		return data;
	}

private:
	std::shared_ptr<NeuroVoxel::MultiHashDataReader<VoxelT>> _reader;
	std::shared_ptr<NeuroVoxel::IReadableCompressedDataset> _dataset;
	NeuroVoxelServerService _service;
	std::string _name;

};