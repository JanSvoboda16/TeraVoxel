/*
 * Author: Jan Svoboda
 * University: BRNO UNIVERSITY OF TECHNOLOGY, FACULTY OF INFORMATION TECHNOLOGY
 */

#pragma once
#include "TeraVoxel.Client.Gui/IView.h"
#include <TeraVoxel.Client.VolumeRenderer/VolumeSelectorBase.h>

class ISelectionView : public IView
{
public:
	ISelectionView(std::shared_ptr<VolumeSelectorBase> volumeSelector)
	{
		_volumeSelector = volumeSelector;
	}

	virtual std::shared_ptr<VolumeBlock<bool>> GetSelection()
	{
		return _volumeSelector->GetMask();
	}

	std::shared_ptr<VolumeSelectorBase> _volumeSelector;
};

