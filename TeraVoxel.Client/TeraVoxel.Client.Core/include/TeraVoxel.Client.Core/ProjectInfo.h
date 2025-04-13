/*
 * Author: Jan Svoboda
 * University: BRNO UNIVERSITY OF TECHNOLOGY, FACULTY OF INFORMATION TECHNOLOGY
 */
#pragma once
#include <string>
#include <json.hpp>
#include <vector>
#include <Eigen/Dense>
#include <NeuroVoxel/Common/DataType.h>

enum ProjectState
{
	ProjectCreated,
	SourceFileUploading,
	SourceFileUploaded,
	ProjectConverting,
	ProjectConverted
};

struct DatasetInfo
{
	DataCommon::StorableType dataType;
	int sizeX = 0;
	int sizeY = 0;
	int sizeZ = 0;
	int dataSizeX = 0, dataSizeY = 0, dataSizeZ = 0;
	Eigen::Vector3f voxelDimensions = { 1.f, 1.f, 1.f };
};

struct BlockBasedDatasetInfo : public DatasetInfo
{
	int segmentSize = 0;
};

// TODO rozdělit na project info a DatasetInfo
class ProjectInfo
{
	public:
		std::string name;
		int sizeX = 0;
		int sizeY = 0;
		int sizeZ = 0;
		std::string dataType;
		int segmentSize = 0;
		float voxelDimensions[3] = {1.f, 1.f, 1.f};
		int dataSizeX= 0, dataSizeY = 0 , dataSizeZ = 0;
		bool isLittleEndian = false;
		bool zTransformed = false;
		bool compressed = true;
		ProjectState state;

	public:
		BlockBasedDatasetInfo ToBlockBasedDatasetInfo() const
		{
			BlockBasedDatasetInfo info;
			info.dataType = DataCommon::StorableTypeFromDotNetString(dataType);
			info.dataSizeX = dataSizeX;
			info.dataSizeY = dataSizeY;
			info.dataSizeZ = dataSizeZ;
			info.voxelDimensions = Eigen::Vector3f(voxelDimensions);
			info.sizeX = sizeX;
			info.sizeY = sizeY;
			info.sizeZ = sizeZ;
			info.segmentSize = segmentSize;
			return info;
		}

		NLOHMANN_DEFINE_TYPE_INTRUSIVE(ProjectInfo, name, sizeX, sizeY, sizeZ, dataType, segmentSize, voxelDimensions, dataSizeX, dataSizeY, dataSizeZ, state, isLittleEndian, zTransformed, compressed)

};



