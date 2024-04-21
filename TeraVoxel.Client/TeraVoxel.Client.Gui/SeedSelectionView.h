#pragma once
#include "ISelectionView.h"
#include "../TeraVoxel.Client.VolumeRender/SeedVolumeSelector.h"
#include "VolumeViewContext.h"

class SeedSelectionView : public ISelectionView
{
public:
	SeedSelectionView(const std::shared_ptr<VolumeViewContext>& volumeViewContext, const std::shared_ptr<VolumeCacheGenericBase>& cache);
	~SeedSelectionView();
	std::shared_ptr<VolumeSegment<bool>> GetSelection() override;

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

