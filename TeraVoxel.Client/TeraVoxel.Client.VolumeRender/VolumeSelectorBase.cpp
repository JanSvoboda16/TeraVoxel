/*
 * Author: Jan Svoboda
 * University: BRNO UNIVERSITY OF TECHNOLOGY, FACULTY OF INFORMATION TECHNOLOGY
 */

#include "VolumeSelectorBase.h"

VolumeSelectorBase::VolumeSelectorBase(const std::shared_ptr<VolumeCacheGenericBase>& volumeCache) :
	_volumeCache(volumeCache),
	_projectInfo(volumeCache->GetProjectInfo())
{
	auto countOfItems = _projectInfo.dataSizeX * _projectInfo.dataSizeY * _projectInfo.dataSizeZ;
	_mask = std::make_shared<VolumeSegment<bool>>(_projectInfo.dataSizeX, _projectInfo.dataSizeY, _projectInfo.dataSizeZ, new bool[countOfItems]);

	std::fill(_mask->data, _mask->data + countOfItems, false);
}

void VolumeSelectorBase::Reset()
{
	std::fill(_mask->data, _mask->data + (_mask->x * _mask->y * _mask->z), false);
}
