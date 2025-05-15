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
/// Comunicates with the server.
/// </summary>
class NeuroVoxelServerService : HttpServerService
{
public:
	/// <summary>
	/// Constructor
	/// </summary>
	/// <param name="url">Server url</param>
	NeuroVoxelServerService(const std::string& url) : HttpServerService(url) { }
	NeuroVoxelServerService() : HttpServerService("") { }

    /// <summary>
    /// Gets names of all datasets available on the server.
    /// </summary>
    /// <returns>Names of datasets</returns>
    std::vector<std::string> GetDatasets() const;

    /// <summary>
    /// Gets dataset metadata.
    /// </summary>
    /// <param name="datasetName">Name of the daataset</param>
    /// <returns>Metadata</returns>
    NeuroVoxel::DatasetMetadata GetDatasetMetadata(std::string datasetName) const;

	/// <summary>
	/// Gets node data (compressed)
	/// </summary>
	/// <param name="datasetName">Name of the dataset</param>
	/// <param name="index">Node index (XYZ serialized)</param>
	/// <param name="level">Compression quality level</param>
	/// <returns></returns>
	std::unique_ptr<std::vector<uint8_t>> GetNode(string datasetName, int index, int level);
};


