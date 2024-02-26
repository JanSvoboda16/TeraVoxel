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
	static std::unique_ptr<VolumeScene> Create(const ProjectInfo& projectInfo, const std::string& serverUrl)
	{
		Vector3f voxelDimensions = Vector3f(projectInfo.voxelDimensions);
		Vector3f size = Vector3f(projectInfo.dataSizeX, projectInfo.dataSizeY, projectInfo.dataSizeZ).array() * voxelDimensions.array();
		Vector3f initialPosition = size / 2;

		std::shared_ptr<Camera> camera = std::make_shared<Camera>(initialPosition, initialPosition[2] * 4, voxelDimensions, 0, 0, 1.2, 20.f, size.maxCoeff() * 5);

		// Volume loader
		ProjectManager projectManager(serverUrl);
		std::shared_ptr<VolumeLoaderFactory> loaderFactory = std::make_shared<NetVolumeLoaderFactory>(projectManager, projectInfo);

		auto rootMeshNode = std::make_shared<MeshNode>();

		// Scene
		auto emptyVisualizerFactory = std::make_shared<EmptyVolumeVisualizerFactory>(std::make_shared<EmptyVolumeVisualizerSettings>());
		return std::make_unique<VolumeScene>(camera, loaderFactory, emptyVisualizerFactory, rootMeshNode);
	}
};

