#include "VolumeSelectorBase.h"

VolumeSelectorBase::VolumeSelectorBase(const std::shared_ptr<VolumeLoaderFactory> &volumeLoaderFactory) 
{
	_volumeLoader = volumeLoaderFactory->Create(16);

	_projectInfo = _volumeLoader->GetProjectInfo();
	auto countOfItems = _projectInfo.dataSizeX * _projectInfo.dataSizeY * _projectInfo.dataSizeZ;
	_mask = std::make_shared<VolumeSegment<bool>>(_projectInfo.dataSizeX, _projectInfo.dataSizeY, _projectInfo.dataSizeZ, new bool[countOfItems]);

	std::fill(_mask->data, _mask->data + countOfItems, false);
}

void VolumeSelectorBase::Reset()
{
	std::fill(_mask->data, _mask->data + (_mask->x * _mask->y * _mask->z), false);
}
