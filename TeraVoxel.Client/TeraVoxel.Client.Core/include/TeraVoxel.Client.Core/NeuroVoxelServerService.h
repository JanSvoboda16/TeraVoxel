/*
 * Author: Jan Svoboda
 * University: BRNO UNIVERSITY OF TECHNOLOGY, FACULTY OF INFORMATION TECHNOLOGY
 */
#pragma once
#include <list>
#include <memory>
#include <format>
#include <json.hpp>
#include <NeuroVoxel/CompressedDataset.h>
#include "TeraVoxel.Client.Core/HttpServerService.h"
#include "TeraVoxel.Client.Core/Logger.h"
#include "TeraVoxel.Client.Core/ServerException.h"
#include "TeraVoxel.Client.Core/ProjectInfo.h"

/// <summary>
/// Comunicates with the server
/// </summary>
class NeuroVoxelServerService : HttpServerService
{
public:
	NeuroVoxelServerService(const std::string& url) : HttpServerService(url) { }
	NeuroVoxelServerService() : HttpServerService("") { }

    std::vector<std::string> GetDatasets() const;

    NeuroVoxel::DatasetMetadata GetDatasetMetadata(std::string datasetName) const;

	std::unique_ptr<std::vector<uint8_t>> GetNode(string datasetName, int index, int level);

};


