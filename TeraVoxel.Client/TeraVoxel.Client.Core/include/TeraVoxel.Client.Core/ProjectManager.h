/*
 * Author: Jan Svoboda
 * University: BRNO UNIVERSITY OF TECHNOLOGY, FACULTY OF INFORMATION TECHNOLOGY
 */
#pragma once
#include <list>
#include <memory>
#include <format>
#include <json.hpp>
#include "TeraVoxel.Client.Core/HttpServerService.h"
#include "TeraVoxel.Client.Core/Logger.h"
#include "TeraVoxel.Client.Core/ServerException.h"
#include "TeraVoxel.Client.Core/ProjectInfo.h"

/// <summary>
/// Comunicates with the server
/// </summary>
class ProjectManager : HttpServerService
{
public:
	ProjectManager(const std::string& url) : HttpServerService(url) { }
	ProjectManager() : HttpServerService("") { }
	std::vector<ProjectInfo> GetAllProjectsInfo();
	std::vector<unsigned char> GetSegment(const std::string& projectName, int x, int y, int z, int downscale, int bytesToRead, bool compressed);
	void CreateProject(const std::string &projectName);
	void DeleteProject(const std::string &projectName);
	void ConvertProject(const std::string& projectName);
	void UploadFile(const std::string& projectName, const std::string& filePath);	

private:
	void DecompressData(const unsigned char* abSrc, unsigned char* abDst, int inputLength, int outputLength);
};


