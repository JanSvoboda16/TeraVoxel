/*
 * Author: Jan Svoboda
 * University: BRNO UNIVERSITY OF TECHNOLOGY, FACULTY OF INFORMATION TECHNOLOGY
 */
#include "TeraVoxel.Client.Core/pch.h"
#include "TeraVoxel.Client.Core/NeuroVoxelServerService.h"
#include <httplib/httplib.h>

 // fix byte colision in ZLIB
typedef unsigned char byteRedefiner;
#define byte byteRedefiner
#define ZLIB_WINAPI
#include "zlib.h"
#include "zconf.h"
using nlohmann::json;

std::vector<std::string> NeuroVoxelServerService::GetDatasets() const
{
	httplib::Client cli = httplib::Client(Url);
	cli.set_read_timeout(10);
	httplib::ContentReceiver rec = httplib::ContentReceiver();
	httplib::Result result = cli.Get("/NeuroVoxel/GetDatasets");

	if (result.error() != httplib::Error::Success)
	{
		throw ServerException("Unable to connect");
	}
	if (result->status != 200)
	{
		throw ServerException("Server responded with code: " + std::to_string(result->status));
	}

	json data = json::parse(result->body);

	return data.get<std::vector<string>>();
}

NeuroVoxel::DatasetMetadata NeuroVoxelServerService::GetDatasetMetadata(std::string datasetName) const
{
	httplib::Client cli = httplib::Client(Url);
	cli.set_read_timeout(10);
	httplib::ContentReceiver rec = httplib::ContentReceiver();
	httplib::Result result = cli.Get("/NeuroVoxel/GetDatasetMetadata?datasetName=" + datasetName);

	if (result.error() != httplib::Error::Success)
	{
		throw ServerException("Unable to connect");
	}
	if (result->status != 200)
	{
		throw ServerException("Server responded with code: " + std::to_string(result->status));
	}

	json data = json::parse(result->body);

	return data.get<NeuroVoxel::DatasetMetadata>();
}

std::unique_ptr<std::vector<uint8_t>> NeuroVoxelServerService::GetNode(string datasetName, int index, int level)
{
	httplib::Client cli = httplib::Client(Url);
	cli.set_read_timeout(10);
	httplib::ContentReceiver rec = httplib::ContentReceiver();
	httplib::Result result = cli.Get("/NeuroVoxel/GetNode?datasetName=" + datasetName +"&index=" + std::to_string(index) +  "&level=" + std::to_string(level));

	if (result.error() != httplib::Error::Success)
	{
		throw ServerException("Unable to connect");
	}
	if (result->status != 200)
	{
		throw ServerException("Server responded with code: " + std::to_string(result->status));
	}
	
	auto resultData = std::make_unique<std::vector<uint8_t>>(result->body.begin(), result->body.end());
	return resultData;
}
