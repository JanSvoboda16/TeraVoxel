/*
 * Author: Jan Svoboda
 * University: BRNO UNIVERSITY OF TECHNOLOGY, FACULTY OF INFORMATION TECHNOLOGY
 */
#pragma once
#include<memory>
#include"Camera.h"
#include <mutex>
#include "CPURayCastingVolumeObjectMemory.h"
#include <future>
#include <memory>
#include "ColorMappingTable.h"
#include "MeshNode.h"

class IVolumeScene
{
public:
	virtual ~IVolumeScene() {};
	/// <summary>
	/// Generates an image to the internal framebuffer
	/// </summary>
	/// <param name="width">width of the image</param>
	/// <param name="height">heigh of the image</param>
	/// <param name="fast">if true, low quality rendering is used</param>
	virtual void ComputeFrame(int width, int height, bool fast) = 0;

	/// <summary>
	/// Returns a pointer to the one of two internal framebuffers
	/// </summary>
	/// <returns>Pointer to the framebuffer</returns>
	virtual std::shared_ptr<unsigned char[]> GetFrame() = 0;
	
	/// <summary>
	/// Gets frame width
	/// </summary>
	/// <returns></returns>
	virtual int GetFrameWidth() = 0;

	/// <summary>
	/// Gets frame height
	/// </summary>
	/// <returns></returns>
	virtual int GetFrameHeight() = 0;

	/// <summary>
	/// Signalize that data in data memmory has changed
	/// </summary>
	/// <returns>True if data has changed</returns>
	virtual bool DataChanged() = 0;

	/// <summary>
	/// Gets name of the data type used in scene template
	/// </summary>
	/// <returns>Name of the data type</returns>
	virtual const char* GetDataTypeName() = 0;

	/// <summary>
	/// Gets pointer to the internal camera
	/// </summary>
	/// <returns></returns>
	virtual shared_ptr<Camera> GetCamera() = 0;

	/// <summary>
	/// Gets pointer to the root of all displayed meshes.
	/// </summary>
	/// <returns></returns>
	virtual std::shared_ptr<MeshNode> GetMeshNode() = 0;

	/// <summary>
	/// Infors if rendering successfully ended
	/// </summary>
	/// <returns>True if frame is ready</returns>
	virtual bool FrameReady() = 0;

	/// <summary>
	/// Informs if rendering is in progress
	/// </summary>
	/// <returns>True if rendering is in progress</returns>
	virtual bool RenderingInProgress() = 0;	

	/// <summary>
	/// Gets project info
	/// </summary>
	/// <returns></returns>
	virtual ProjectInfo GetProjectInfo() = 0;
};

