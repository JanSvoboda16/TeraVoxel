/*
 * Author: Jan Svoboda
 * University: BRNO UNIVERSITY OF TECHNOLOGY, FACULTY OF INFORMATION TECHNOLOGY
 */
#pragma once
#include "ProjectInfo.h"
#include <list>
#include <memory>
#include <format>
#include "HttpManagerBase.h"
#include "nlohman/json.hpp"
#include "Logger.h"
#include "../TeraVoxel.Client.Core/ServerException.h"

/// <summary>
/// Comunicates with the server
/// </summary>
class ProjectManager : HttpManagerBase
{
public:
	ProjectManager(const std::string& url) : HttpManagerBase(url) { }
	ProjectManager() : HttpManagerBase("") { }
	std::vector<ProjectInfo> GetAllProjectsInfo();
	std::vector<unsigned char> GetSegment(const std::string& projectName, int x, int y, int z, int downscale, int bytesToRead);
	void CreateProject(const std::string &projectName);
	void DeleteProject(const std::string &projectName);
	void ConvertProject(const std::string& projectName);
	void UploadFile(const std::string& projectName, const std::string& filePath);	

private:
	void DecompressData(const unsigned char* abSrc, unsigned char* abDst, int inputLength, int outputLength);
};


