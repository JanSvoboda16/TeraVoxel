/*
 * Author: Jan Svoboda
 * University: BRNO UNIVERSITY OF TECHNOLOGY, FACULTY OF INFORMATION TECHNOLOGY
 */

#include "SeedSelectionView.h"
#include "imgui.h"
#include "imgui_stdlib.h"
#include "../TeraVoxel.Client.VolumeRender/MeshTreeExplorer.h"
#include "../TeraVoxel.Client.VolumeRender/MeshGenerator.h"
#include "../TeraVoxel.Client.VolumeRender/VolumeCache.h"

SeedSelectionView::SeedSelectionView(const std::shared_ptr<VolumeViewContext>& volumeViewContext, const std::shared_ptr<VolumeCacheGenericBase> &cache) :
	ISelectionView(std::make_shared<SeedVolumeSelector>(cache)),
	_volumeViewContext(volumeViewContext)
{
	_volumeViewContext->sceneEditable.Register(this, "Repaint", [this]() { UpdateMeshes(); });
}

SeedSelectionView::~SeedSelectionView()
{
	_volumeViewContext->sceneEditable.Unregister(this, "Repaint");
}

std::shared_ptr<VolumeSegment<bool>> SeedSelectionView::GetSelection()
{
	auto selector = dynamic_pointer_cast<SeedVolumeSelector>(_volumeSelector);
	selector->Reset();
	int index = 0;
	for (auto& position : _points)
	{
		auto& settings = _settings[index++];
		selector->Select(position, settings[0], settings[1], settings[2]);
	}

	return _volumeSelector->GetMask();
}

void SeedSelectionView::Update()
{
	if (ImGui::BeginTable("Points", 4))
	{
		for (int row = 0; row < _points.size(); row++)
		{
			ImGui::TableNextRow();
	
			ImGui::TableSetColumnIndex(0);
			auto& pointVec = _points[row];
			float pointArr[] = { pointVec.x(), pointVec.y(), pointVec.z() };
			auto positionLabel = "Position##" + std::to_string(row);
			if (ImGui::InputFloat3(positionLabel.c_str(), pointArr))
			{
				_meshUpdateNeeded = true;
			}
			pointVec[0] = pointArr[0];
			pointVec[1] = pointArr[1];
			pointVec[2] = pointArr[2];


			ImGui::TableSetColumnIndex(1);
			auto& settingsVec = _settings[row];
			float settingsArr[] = { settingsVec[0], settingsVec[1], settingsVec[2]};
			auto settingsLabel = "Settings##" + std::to_string(row);
			if (ImGui::InputFloat3(settingsLabel.c_str(), settingsArr))
			{
				_meshUpdateNeeded = true;
			}
			settingsVec[0] = settingsArr[0];
			settingsVec[1] = settingsArr[1];
			settingsVec[2] = settingsArr[2];

			ImGui::TableSetColumnIndex(2);
			auto deleteLabel = "Delete##" + std::to_string(row);

			if (ImGui::Button(deleteLabel.c_str()))
			{
				_meshUpdateNeeded = true;
				_points.erase(_points.begin() + row);
				_settings.erase(_settings.begin() + row);
			}

			ImGui::TableSetColumnIndex(3);
			auto centerLabel = "Center##" + std::to_string(row);
			if (ImGui::Button(centerLabel.c_str()))
			{
				_meshUpdateNeeded = true;
				_points[row] = _volumeViewContext->scene->GetCamera()->GetObserverCenter();
			}
		}
		ImGui::EndTable();
	}

	if (ImGui::Button("Add Point"))
	{
		_points.push_back(Vector3f(0, 0, 0));
		_settings.push_back(Vector3f(0, 0, 0));
		_meshUpdateNeeded = true;
	}
}

void SeedSelectionView::UpdateMeshes()
{
	if (_volumeViewContext->scene != nullptr && _meshUpdateNeeded)
	{
		auto rootMesh = _volumeViewContext->scene->GetMeshNode();
		MeshTreeExplorer::Delete(rootMesh, "SelectionPoints");
		auto mesh = std::make_shared<MeshNode>();
		mesh->name = "SelectionPoints";
		
		for (auto &position: _points)
		{
			auto pointMesh = MeshGenerator::AxisCross(5, Vector4b(255, 0, 0, 100));
			pointMesh->transformation = Transformations::GetTranslationMatrix(position.x(), position.y(), position.z());
			mesh->subNodes.push_back(pointMesh);
		}

		_volumeViewContext->sceneUpdated.Notify();
		rootMesh->subNodes.push_back(mesh);

		_meshUpdateNeeded = false;
	}	
}
