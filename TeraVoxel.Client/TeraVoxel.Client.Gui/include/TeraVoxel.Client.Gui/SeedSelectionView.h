/*
 * Author: Jan Svoboda
 * University: BRNO UNIVERSITY OF TECHNOLOGY, FACULTY OF INFORMATION TECHNOLOGY
 */

#pragma once
#include "TeraVoxel.Client.Gui/ISelectionView.h"
#include <TeraVoxel.Client.VolumeRenderer/SeedVolumeSelector.h>
#include "TeraVoxel.Client.Gui/VolumeViewContext.h"

class SeedSelectionView : public ISelectionView
{
public:
	SeedSelectionView(const std::shared_ptr<VolumeViewContext>& volumeViewContext, const std::shared_ptr<VolumeCacheBase>& cache);
	~SeedSelectionView();
	std::shared_ptr<VolumeBlock<bool>> GetSelection() override;

	void Update();

private:
	void UpdateMeshes();
	
	std::vector<Vector3f> _points;
	std::vector<Vector3f> _settings;

	std::shared_ptr<VolumeViewContext> _volumeViewContext;
	float _lowerBoundary = 0;
	float _upperBoundary = 0;
	float _maxDifference = 0;
	bool _meshUpdateNeeded = false;
};

