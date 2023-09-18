/*
 * Author: Jan Svoboda
 * University: BRNO UNIVERSITY OF TECHNOLOGY, FACULTY OF INFORMATION TECHNOLOGY
 */
#pragma once
#include <string>
#include "nlohman/json.hpp"
#include <vector>

enum ProjectState
{
	ProjectCreated,
	SourceFileUploading,
	SourceFileUploaded,
	ProjectConverting,
	ProjectConverted
};

class ProjectInfo
{
	public:
		std::string name;
		int sizeX = 0;
		int sizeY = 0;
		int sizeZ = 0;
		std::string dataType;
		int segmentSize = 0;
		float voxelDimensions[3];
		int dataSizeX= 0, dataSizeY = 0 , dataSizeZ = 0;
		bool isLittleEndian = false;
		ProjectState state;

	public:
		NLOHMANN_DEFINE_TYPE_INTRUSIVE(ProjectInfo, name, sizeX, sizeY, sizeZ, dataType, segmentSize, voxelDimensions, dataSizeX, dataSizeY, dataSizeZ, state, isLittleEndian)

};

