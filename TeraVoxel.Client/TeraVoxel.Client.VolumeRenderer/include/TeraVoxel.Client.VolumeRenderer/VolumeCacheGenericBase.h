/*
 * Author: Jan Svoboda
 * University: BRNO UNIVERSITY OF TECHNOLOGY, FACULTY OF INFORMATION TECHNOLOGY
 */

#pragma once
#include <memory>
#include <TeraVoxel.Client.Core\ProjectInfo.h>
#include "TeraVoxel.Client.VolumeRenderer/VolumeLoaderGenericBase.h"
#include "TeraVoxel.Client.VolumeRenderer/VolumeLoaderFactory.h"

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

