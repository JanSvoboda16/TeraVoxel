#pragma once
#include "TeraVoxel.Client.VolumeRenderer/NeuroVoxelServerDataset.h"

NeuroVoxelServerDataset::NeuroVoxelServerDataset(const std::string& name, const NeuroVoxelServerService& service) :
    _name(name),
    _service(service),
    _metadata(service.GetDatasetMetadata(name))
{
    auto totalNodes = _metadata.gridDimensions.prod() * _metadata.levels;
    _nodeMutexes = std::vector<std::mutex>(totalNodes);
    _localDataset = NeuroVoxel::CompressedDataset::Create("dataset.temp", _metadata.dataDimensions, _metadata.blockDimensions, _metadata.gridDimensions, _metadata.dataType, true, _metadata.levels);
}

std::shared_ptr<NeuroVoxel::CompressionModel> NeuroVoxelServerDataset::GetNode(const Eigen::Vector3i& coordinates, int level)
{
    auto localIndex = DataCommon::Indexing::XYZToIdx(coordinates, _metadata.gridDimensions);
    auto globalIndex = localIndex + _metadata.gridDimensions.prod() * level;
    _nodeMutexes[globalIndex].lock();
    try {

        if (!_localDataset->NodeValid(coordinates, level))
        {
            _localDataset->SetNodeBinary(_service.GetNode(_name, localIndex, level), coordinates, level, _metadata.nodeTypes[globalIndex]);
        }
    }
    catch (...) 
    {
        _nodeMutexes[globalIndex].unlock();
        throw;
    }
    _nodeMutexes[globalIndex].unlock();
    return _localDataset->GetNode(coordinates, level);
}

bool NeuroVoxelServerDataset::NodeValid(const Eigen::Vector3i& coordinates, int level)
{
    return true;
}

NeuroVoxel::DatasetMetadata& NeuroVoxelServerDataset::GetMetadata()
{
    return _metadata;
}
