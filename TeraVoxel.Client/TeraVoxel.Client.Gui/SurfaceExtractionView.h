/*
 * Author: Jan Svoboda
 * University: BRNO UNIVERSITY OF TECHNOLOGY, FACULTY OF INFORMATION TECHNOLOGY
 */

#pragma once
#include <memory>
#include "ISelectionView.h"
#include "VolumeViewContext.h"
#include "../TeraVoxel.Client.VolumeRender/SurfaceExtractorBase.h"

class SurfaceExtractionView : public IView
{
public:
	SurfaceExtractionView(const std::shared_ptr<VolumeViewContext>& volumeViewContext);
	~SurfaceExtractionView();
	void EditMeshes();
	void Update() override;
	
private:
	void ChangeSelector();
	void ChangeExtractor();
	
	std::shared_ptr<VolumeViewContext> _volumeViewContext;
	std::shared_ptr<ISelectionView> _selectorView;
	std::shared_ptr<SurfaceExtractorBase> _extractor;
	std::shared_ptr<MeshNode> _surface;
	std::shared_ptr<VolumeCacheGenericBase> _cache;

	std::string _exportFilePath = "untitled.stl";
	int _selectedSelectorId = 0;;
	int _selectedExtractorId = 0;
	bool _sceneUpdateNeeded = false;

	bool _interpolate = false;
	Vector2f _interpolationBoundaries = Vector2f(0,0);
};

