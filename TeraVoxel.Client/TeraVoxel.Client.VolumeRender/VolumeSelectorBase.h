#pragma once
#include "VolumeLoaderFactory.h"
#include "VolumeSegment.h"

class VolumeSelectorBase
{
public:
	virtual ~VolumeSelectorBase() {};
	VolumeSelectorBase(const std::shared_ptr<VolumeLoaderFactory>& volumeLoaderFactory);
	void Reset();
	std::shared_ptr<VolumeSegment<bool>> GetMask(){	return _mask;}

protected:
	std::shared_ptr<VolumeLoaderGenericBase> _volumeLoader;
	ProjectInfo _projectInfo;
	std::shared_ptr<VolumeSegment<bool>> _mask;
};

