#include "SurfaceExtractionView.h"
#include "imgui.h"
#include "imgui_stdlib.h"
#include "SeedSelectionView.h"
#include "../TeraVoxel.Client.VolumeRender/MarchingCubesSurfaceExtractor.h"
#include "../TeraVoxel.Client.VolumeRender/MeshTreeExplorer.h"
#include "../TeraVoxel.Client.VolumeRender/StlExporter.h"

SurfaceExtractionView::SurfaceExtractionView(const std::shared_ptr<VolumeViewContext>& volumeViewContext) : _volumeViewContext(volumeViewContext)
{
	if (_volumeViewContext->scene != nullptr)
	{
		_cache = VolumeCacheFactory::VolumeCacheCreate(_volumeViewContext->scene->GetVolumeLoaderFactory());
		ChangeSelector();
		ChangeExtractor();
	}

	_volumeViewContext->sceneReplaced.Register(this, "Reselect", [this]()
	{
		if (_volumeViewContext->scene != nullptr)
		{
			_cache = VolumeCacheFactory::VolumeCacheCreate(_volumeViewContext->scene->GetVolumeLoaderFactory());
			ChangeSelector();
			ChangeExtractor();
		}
	});

	_volumeViewContext->sceneEditable.Register(this, "Repaint", [this]() { EditMeshes(); });
}

SurfaceExtractionView::~SurfaceExtractionView()
{
	_volumeViewContext->sceneReplaced.Unregister(this, "Reselect");
	_volumeViewContext->sceneEditable.Unregister(this, "Repaint");
}

void SurfaceExtractionView::EditMeshes()
{
	if (_volumeViewContext->scene != nullptr)
	{
		if (_sceneUpdateNeeded)
		{

			MeshTreeExplorer::Delete(_volumeViewContext->scene->GetMeshNode(), "Surface");
			if (_surface != nullptr)
			{
				_volumeViewContext->scene->GetMeshNode()->subNodes.push_back(_surface);
			}

			_volumeViewContext->sceneUpdated.Notify();

			_sceneUpdateNeeded = false;
		}
	}
	else
	{
		_surface = nullptr;
	}
}

void SurfaceExtractionView::Update()
{
	const char* selectors[] = { "Seed Selector" };

	if (_volumeViewContext->scene != nullptr)
	{
		if (ImGui::Combo("Selector", &_selectedSelectorId, selectors, IM_ARRAYSIZE(selectors)))
		{
			ChangeSelector();
		}

		if (_selectorView != nullptr)
		{
			_selectorView->Update();
		}

		const char* extractors[] = { "Marching Cubes" };

		if (ImGui::Combo("Extractor", &_selectedExtractorId, extractors, IM_ARRAYSIZE(extractors)))
		{
			ChangeExtractor();
		}

		ImGui::Checkbox("Interpolate", &_interpolate);
		
		if(_interpolate)
			ImGui::InputFloat2("Interpolation boundaries", _interpolationBoundaries.data());

		if (ImGui::Button("Extract"))
		{
			_surface = _extractor->ExtractSurface(_selectorView->GetSelection(), _volumeViewContext->scene->GetProjectInfo(), _interpolate, _interpolationBoundaries);
			_surface->name = "Surface";
			_sceneUpdateNeeded = true;
			_cache->Flush();
		}
		
		ImGui::InputText("Export path", &_exportFilePath);
		if (ImGui::Button("Export"))
		{
			StlExporter exporter;
			exporter.Export(_surface, _exportFilePath);
		}

		if (ImGui::Button("Reset"))
		{
			_surface = nullptr;
			_sceneUpdateNeeded = true;
		}
	}	
}

void SurfaceExtractionView::ChangeSelector()
{
	switch (_selectedSelectorId)
	{
	case 0:
		_selectorView = std::make_shared<SeedSelectionView>(_volumeViewContext, _cache); break;
	default:
		break;
	}
}

void SurfaceExtractionView::ChangeExtractor()
{
	switch (_selectedExtractorId)
	{
	case 0:
		_extractor = std::make_shared<MarchingCubesSurfaceExtractor>(_cache); break;
	default:
		break;
	}
}
