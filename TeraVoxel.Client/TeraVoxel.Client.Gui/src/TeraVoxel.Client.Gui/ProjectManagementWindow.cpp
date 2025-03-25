/*
 * Author: Jan Svoboda
 * University: BRNO UNIVERSITY OF TECHNOLOGY, FACULTY OF INFORMATION TECHNOLOGY
 */
#include "TeraVoxel.Client.Gui/ProjectManagementWindow.h"
#include "imgui.h"
#include "imgui_stdlib.h"
#include <TeraVoxel.Client.Gui/NetProjectView.h>
#include <TeraVoxel.Client.Gui/NeuroVoxelView.h>

ProjectManagementWindow::ProjectManagementWindow(const std::shared_ptr<VolumeViewContext>& volumeViewContext)
{
	_volumeViewContext = volumeViewContext;
	SetDatasetSourceView(_selectedDataSourceId);
}

void ProjectManagementWindow::Update()
{
	ImGui::Begin("Projects");

	const char* datasetSources[] = { "NetProject", "NeuroVoxel"};

	if (ImGui::Combo("Dataset sources:", &_selectedDataSourceId,  datasetSources, IM_ARRAYSIZE(datasetSources)))
	{
		SetDatasetSourceView(_selectedDataSourceId);
	}

	if (_view != nullptr)
	{
		_view->Update();
	}

	ImGui::End();
}

void ProjectManagementWindow::SetDatasetSourceView(int id)
{
	switch (id)
	{
	case 0:
		_view = std::move(std::unique_ptr<IView>(new NetProjectView(_volumeViewContext)));
		break;
	case 1:
		_view = std::move(std::unique_ptr<IView>(new NeuroVoxelView(_volumeViewContext)));
		break;
	default:
		break;
	}
}
