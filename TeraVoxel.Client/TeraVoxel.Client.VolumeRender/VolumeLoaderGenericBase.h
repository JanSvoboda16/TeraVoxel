#pragma once
#include "../TeraVoxel.Client.Core/ProjectInfo.h"

class VolumeLoaderGenericBase
{
public:
	virtual ~VolumeLoaderGenericBase() {}
	ProjectInfo GetProjectInfo() { return _projectInfo; }

protected:
	ProjectInfo _projectInfo;
};

