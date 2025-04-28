/*
 * Author: Jan Svoboda
 * University: BRNO UNIVERSITY OF TECHNOLOGY, FACULTY OF INFORMATION TECHNOLOGY
 */

#include "TeraVoxel.Client.VolumeRenderer/VolumeSelectorBase.h"

VolumeSelectorBase::VolumeSelectorBase(const std::shared_ptr<VolumeCacheBase>& volumeCache) :
	_volumeCache(volumeCache),
	_datasetInfo(volumeCache->GetDatasetInfo())
{
	auto countOfItems = _datasetInfo.dataSizeX * _datasetInfo.dataSizeY * _datasetInfo.dataSizeZ;
	_mask = std::make_shared<VolumeBlock<bool>>(_datasetInfo.dataSizeX, _datasetInfo.dataSizeY, _datasetInfo.dataSizeZ, new bool[countOfItems]);

	std::fill(_mask->data, _mask->data + countOfItems, false);
}

void VolumeSelectorBase::Reset()
{
	std::fill(_mask->data, _mask->data + (_mask->x * _mask->y * _mask->z), false);
}
