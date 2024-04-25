/*
 * Author: Jan Svoboda
 * University: BRNO UNIVERSITY OF TECHNOLOGY, FACULTY OF INFORMATION TECHNOLOGY
 */

#pragma once
#include <memory>
#include "VolumeLoaderGenericBase.h"
#include "VolumeLoaderFactory.h"
#include "..\TeraVoxel.Client.Core\ProjectInfo.h"

class VolumeCacheGenericBase
{
public:
	VolumeCacheGenericBase(const ProjectInfo &info) : _projectInfo(info){ }
	ProjectInfo GetProjectInfo() { return _projectInfo; }
	virtual ~VolumeCacheGenericBase() {};
	virtual void Flush() = 0;;
protected:
	ProjectInfo _projectInfo;
};

