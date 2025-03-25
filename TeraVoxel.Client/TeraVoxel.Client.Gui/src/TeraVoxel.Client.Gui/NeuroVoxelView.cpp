#include "TeraVoxel.Client.Gui/NeuroVoxelView.h"
#include <TeraVoxel.Client.VolumeRenderer/NetMemoryVolumeSceneFactory.h>
#include "imgui.h"
#include "imgui_stdlib.h"
#include <TeraVoxel.Client.Core/NeuroVoxelServerService.h>

NeuroVoxelView::NeuroVoxelView(const std::shared_ptr<VolumeViewContext>& volumeViewContext)
{
	_volumeViewContext = volumeViewContext;
}

void NeuroVoxelView::Update()
{
	ImGui::Text("Connect to the server");
	ImGui::InputText("Url ", &_serverUrl);

	auto clicked = ImGui::Button("Connect");

	ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(1, 0, 0, 1));
	ImGui::Text(_showedErrorMessage.c_str());
	ImGui::PopStyleColor();

	// SERVER CONNECTION
	if (clicked && !_serverUrl.empty())
	{
		try
		{
			_errorMessage = "";
			NeuroVoxelServerService service(_serverUrl);
			_projects = service.GetDatasets();
			_connectedServerUrl = _serverUrl;
		}
		catch (const std::exception& ex)
		{
			_errorMessage = ex.what();
			_connectedServerUrl = "";
		}
	}

	static ImGuiTableFlags flags = ImGuiTableFlags_ScrollY | ImGuiTableFlags_RowBg | ImGuiTableFlags_BordersOuter | ImGuiTableFlags_BordersV | ImGuiTableFlags_Resizable | ImGuiTableFlags_Reorderable | ImGuiTableFlags_Hideable;

	ImGui::Text("Projects");
	if (ImGui::BeginTable("table_scrolly", 4, flags))
	{
		ImGui::TableSetupScrollFreeze(0, 1);
		ImGui::TableSetupColumn("Name", ImGuiTableColumnFlags_None);
		ImGui::TableSetupColumn("", ImGuiTableColumnFlags_None);
		ImGui::TableHeadersRow();

		if (!_connectedServerUrl.empty())
		{
			ImGuiListClipper clipper;
			clipper.Begin(_projects.size());

			while (clipper.Step())
			{
				try
				{
					for (int row = clipper.DisplayStart; row < clipper.DisplayEnd; row++)
					{
						auto project = _projects[row];
						ImGui::TableNextRow();
						ImGui::TableSetColumnIndex(0);
						ImGui::Text(project.c_str());

						ImGui::TableSetColumnIndex(1);
						auto loadLable = "Load##" + std::to_string(row);
						auto unloadLable = "Unload##" + std::to_string(row);
						auto deleteLabel = "Delete##" + std::to_string(row);
						auto buildLabel = "Build##" + std::to_string(row);
						auto uploadLabel = "Upload##" + std::to_string(row);

						// FIRST COLUMN
						// LOAD/UNLOAD project - only when converted
						if (project != _selectedProjectName)
						{
							if (ImGui::Button(loadLable.c_str()))
							{
								_selectedProjectIndex = row;
								_selectedProjectName = project;

								_volumeViewContext->scene = NeuroVoxelSceneFactory::Create(project, _connectedServerUrl);
								_volumeViewContext->sceneReplaced.Notify();
							}
						}
						else
						{
							if (ImGui::Button(unloadLable.c_str()))
							{
								_selectedProjectIndex = -1;
								_selectedProjectName = "";

								_volumeViewContext->scene = nullptr;
								_volumeViewContext->sceneReplaced.Notify();
							}
						}
					}
				}
				catch (const std::exception& ex)
				{
					_errorMessage = ex.what();
				}
			}
		}

		ImGui::EndTable();

		// Refreshing project info once per CONTEXT_REFRESH_RATE frames or when needed
		if (clock() - _lastRefresth >= CONTEXT_REFRESH_RATE || _refreshContext)
		{
			if (!_connectedServerUrl.empty())
			{
				try
				{
					NeuroVoxelServerService service(_serverUrl);
					_projects = service.GetDatasets();
					_connectedServerUrl = _serverUrl;
				}
				catch (const std::exception& ex)
				{
					_errorMessage = ex.what();
					_connectedServerUrl = "";
				}
			}

			_lastRefresth = clock();
			_refreshContext = false;
		}
	}

	// SHOW ERROR
	if (!_errorMessage.empty())
	{
		_showedErrorMessage = _errorMessage;
		_errorMessage = "";
		_errorMessageDurationCounter = 0;
	}

	// HIDE ERROR AFTER 800 frames
	if (_errorMessageDurationCounter > 800)
	{
		_showedErrorMessage = "";
	}
	else
	{
		_errorMessageDurationCounter++;
	}
}
