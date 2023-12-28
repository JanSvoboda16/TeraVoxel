/*
 * Author: Jan Svoboda
 * University: BRNO UNIVERSITY OF TECHNOLOGY, FACULTY OF INFORMATION TECHNOLOGY
 */
#pragma once
#include <memory>
#include "Camera.h"
#include "MeshGenerator.h"
#include "CPURayCastingVolumeObjectMemory.h"
#include "NetVolumeLoader.h"
#include "ColorMappingTable.h"
#include "IVolumeScene.h"
#include "VolumeScene.h"
#include "NetVolumeLoaderFactory.h"
#include "CPURCVolumeVisualizerFactory.h"
#include "EmptyVolumeVisualizerFactory.h"

 /// <summary>
 /// Used for creating a typed instance of the VolumeScene class. 
 /// </summary>
class NetMemoryVolumeSceneFactory
{

public:
	static std::unique_ptr<IVolumeScene> Create(const ProjectInfo& projectInfo, const std::string& serverUrl)
	{
		if (projectInfo.dataType == "System.Sbyte")
		{
			return CreateInstance<int8_t>(projectInfo, serverUrl);
		}
		else if (projectInfo.dataType == "System.Byte")
		{
			return CreateInstance<uint8_t>(projectInfo, serverUrl);
		}
		else if (projectInfo.dataType == "System.Int16")
		{
			return CreateInstance<int16_t>(projectInfo, serverUrl);
		}
		else if (projectInfo.dataType == "System.UInt16")
		{
			return CreateInstance<uint16_t>(projectInfo, serverUrl);
		}
		else if (projectInfo.dataType == "System.Int32")
		{
			return CreateInstance<int32_t>(projectInfo, serverUrl);
		}
		else if (projectInfo.dataType == "System.UInt32")
		{
			return CreateInstance<uint32_t>(projectInfo, serverUrl);
		}
		else if (projectInfo.dataType == "System.Single")
		{
			return CreateInstance<float>(projectInfo, serverUrl);
		}
		else if (projectInfo.dataType == "System.Double")
		{
			return CreateInstance<double>(projectInfo, serverUrl);
		}
		else if (projectInfo.dataType == "System.Int64")
		{
			return CreateInstance<int64_t>(projectInfo, serverUrl);
		}
		else if (projectInfo.dataType == "System.UInt64")
		{
			return CreateInstance<uint64_t>(projectInfo, serverUrl);
		}
		else
		{
			throw std::exception("Unknown data type");
		}
	}

private:
	template <typename T>
	static std::unique_ptr<IVolumeScene> CreateInstance(const ProjectInfo& projectInfo, const std::string& serverUrl)
	{
		// Camera
		Vector3f voxelDimensions = Vector3f(projectInfo.voxelDimensions);
		Vector3f size = Vector3f(projectInfo.dataSizeX, projectInfo.dataSizeY, projectInfo.dataSizeZ).array() * voxelDimensions.array();
		Vector3f initialPosition = size / 2;

		std::shared_ptr<Camera> camera = std::make_shared<Camera>(initialPosition, initialPosition[2] * 4, voxelDimensions, 0, 0, 1.2, 20.f, size.maxCoeff() * 5);

		// Volume loader
		ProjectManager projectManager(serverUrl);
		std::shared_ptr<VolumeLoaderFactory<T>> loaderFactory = std::make_shared<NetVolumeLoaderFactory<T>>(projectManager);

		// Scene controlling meshes
		auto axisCross = MeshGenerator::AxisCross(size.maxCoeff(), Vector4b(255,0,0,255));

		auto rootMeshNode = std::make_shared<MeshNode>();
		rootMeshNode->subNodes.push_back(axisCross);

		camera->BindOrbiterMeshNode(axisCross);

		// Scene
		auto emptyVisualizerFactory = std::make_shared<EmptyVolumeVisualizerFactory<T>>(std::make_shared<EmptyVolumeVisualizerSettings>());
		return std::make_unique<VolumeScene<T>>(camera, projectInfo, loaderFactory, emptyVisualizerFactory,rootMeshNode);
	}
};

