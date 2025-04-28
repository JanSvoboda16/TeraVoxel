#pragma once
#include <NeuroVoxel/ICompressedDataset.h>
#include <TeraVoxel.Client.Core/NeuroVoxelServerService.h>
#include <NeuroVoxel/CompressedDataset.h>

class NeuroVoxelServerDataset : public NeuroVoxel::IReadableCompressedDataset
{
public:
	NeuroVoxelServerDataset(const std::string& name, const NeuroVoxelServerService& service);

	std::shared_ptr<NeuroVoxel::CompressionModel> GetNode(const Eigen::Vector3i& coordinates, int level) override;
	bool NodeValid(const Eigen::Vector3i& coordinates, int level) override;
	NeuroVoxel::DatasetMetadata& GetMetadata() override;
private:
	std::string _name;
	NeuroVoxelServerService _service;
	std::vector<std::mutex> _nodeMutexes;
	NeuroVoxel::DatasetMetadata _metadata;
	std::unique_ptr<NeuroVoxel::CompressedDataset> _localDataset;
	
};