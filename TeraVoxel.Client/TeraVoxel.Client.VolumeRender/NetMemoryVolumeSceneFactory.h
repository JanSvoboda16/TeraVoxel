/*
 * Author: Jan Svoboda
 * University: BRNO UNIVERSITY OF TECHNOLOGY, FACULTY OF INFORMATION TECHNOLOGY
 */
#pragma once
#include<memory>
#include"Camera.h"
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
	static std::unique_ptr<IVolumeScene> Create(std::shared_ptr<Camera> camera, const ProjectInfo& projectInfo, const std::string& serverUrl)
	{
		if (projectInfo.dataType == "System.Sbyte")
		{
			return CreateInstance<int8_t>(camera, projectInfo, serverUrl);
		}
		else if (projectInfo.dataType == "System.Byte")
		{
			return CreateInstance<uint8_t>(camera, projectInfo, serverUrl);
		}
		else if (projectInfo.dataType == "System.Int16")
		{
			return CreateInstance<int16_t>(camera, projectInfo, serverUrl);
		}
		else if (projectInfo.dataType == "System.UInt16")
		{
			return CreateInstance<uint16_t>(camera, projectInfo, serverUrl);
		}
		else if (projectInfo.dataType == "System.Int32")
		{
			return CreateInstance<int32_t>(camera, projectInfo, serverUrl);
		}
		else if (projectInfo.dataType == "System.UInt32")
		{
			return CreateInstance<uint32_t>(camera, projectInfo, serverUrl);
		}
		else if (projectInfo.dataType == "System.Single")
		{
			return CreateInstance<float>(camera, projectInfo, serverUrl);
		}
		else if (projectInfo.dataType == "System.Double")
		{
			return CreateInstance<double>(camera, projectInfo, serverUrl);
		}
		else if (projectInfo.dataType == "System.Int64")
		{
			return CreateInstance<int64_t>(camera, projectInfo, serverUrl);
		}
		else if (projectInfo.dataType == "System.UInt64")
		{
			return CreateInstance<uint64_t>(camera, projectInfo, serverUrl);
		}
		else
		{
			throw std::exception("Unknown data type");
		}
	}

private:
	template <typename T>
	static std::unique_ptr<IVolumeScene> CreateInstance(const std::shared_ptr<Camera> &camera, const ProjectInfo& projectInfo, const std::string& serverUrl)
	{
		ProjectManager projectManager(serverUrl);
		std::shared_ptr<VolumeLoaderFactory<T>> loaderFactory = std::make_shared<NetVolumeLoaderFactory<T>>(projectManager);

		// TODO rozdelit projectInfo -> projectInfo, sourceInfo
		auto emptyVisualizerFactory = std::shared_ptr<IVolumeVisualizerFactory<T>>((IVolumeVisualizerFactory<T> *) new EmptyVolumeVisualizerFactory<T>(std::make_shared<EmptyVolumeVisualizerSettings>()));
		return std::unique_ptr<IVolumeScene>((IVolumeScene*) new VolumeScene<T>(camera, projectInfo, loaderFactory, emptyVisualizerFactory));
	}
};

