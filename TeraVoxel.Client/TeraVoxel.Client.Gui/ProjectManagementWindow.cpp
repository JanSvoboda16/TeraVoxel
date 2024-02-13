/*
 * Author: Jan Svoboda
 * University: BRNO UNIVERSITY OF TECHNOLOGY, FACULTY OF INFORMATION TECHNOLOGY
 */
#include "ProjectManagementWindow.h"
#include "../TeraVoxel.Client.VolumeRender/NetMemoryVolumeSceneFactory.h"
#include "imgui.h"
#include "imgui_stdlib.h"
#include "../TeraVoxel.Client.Core/ProjectManager.h"

ProjectManagementWindow::ProjectManagementWindow(std::shared_ptr<VolumeViewContext> volumeViewContext)
{
	_volumeViewContext = volumeViewContext;
	ProjectManager manager(_serverUrl);
}

void ProjectManagementWindow::Update()
{
	ImGui::Begin("Projects");
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
			ProjectManager manager(_serverUrl);
			_projects = manager.GetAllProjectsInfo();
			_connectedServerUrl = _serverUrl;
		}
		catch (const std::exception& ex)
		{
			_errorMessage = ex.what();
			_connectedServerUrl = "";
		}
	}

	// CREATING A NEW PROJECT OR UPLOADING A FILE
	if (!_connectedServerUrl.empty())
	{
		ImGui::Text("Create project");
		ImGui::InputText("Project Name", &_createProjectName);

		try
		{
			if (ImGui::Button("Create") && !_createProjectName.empty())
			{
				ProjectManager manager(_serverUrl);
				manager.CreateProject(_createProjectName);
				_refreshContext = true;
			};
		}
		catch (const std::exception& ex)
		{
			_errorMessage = ex.what();
		}

		ImGui::InputText("File to upload", &_fileToUploadPath);
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
						ImGui::Text(project.name.c_str());
						ImGui::Text(project.dataType.c_str());

						ImGui::TableSetColumnIndex(1);
						auto loadLable = "Load##" + std::to_string(row);
						auto unloadLable = "Unload##" + std::to_string(row);
						auto deleteLabel = "Delete##" + std::to_string(row);
						auto buildLabel = "Build##" + std::to_string(row);
						auto uploadLabel = "Upload##" + std::to_string(row);

						// FIRST COLUMN
						// LOAD/UNLOAD project - only when converted
						if (project.name != _selectedProjectName)
						{

							if (project.state == ProjectConverted && ImGui::Button(loadLable.c_str()))
							{
								_selectedProjectIndex = row;
								_selectedProjectName = project.name;

								_volumeViewContext->scene = NetMemoryVolumeSceneFactory::Create(project, _connectedServerUrl);
								_volumeViewContext->sceneReplaced.Notify();
							}
						}
						else
						{
							if (project.state == ProjectConverted && ImGui::Button(unloadLable.c_str()))
							{
								_selectedProjectIndex = -1;
								_selectedProjectName = "";

								_volumeViewContext->scene = nullptr;
								_volumeViewContext->sceneReplaced.Notify();
							}
						}

						auto state = project.state;

						// SECOND COLUMN
						// DELETE project - only when the project is not selected and no process is running
						ImGui::TableSetColumnIndex(2);
						if (project.name != _selectedProjectName && ImGui::Button(deleteLabel.c_str()))
						{
							if (state != ProjectConverting && state != SourceFileUploading)
							{
								ProjectManager manager(_connectedServerUrl);
								manager.DeleteProject(project.name.c_str());
								_refreshContext = true;
							}
						}

						// THIRD COLUMN
						// BUILD/UPLOAD 
						ImGui::TableSetColumnIndex(3);
						switch (state)
						{
						case ProjectCreated:
							if (ImGui::Button(uploadLabel.c_str()) && !_fileToUploadPath.empty())
							{
								std::thread([](std::string serverUrl, std::string projectName, bool& _refreshContext, std::string fileUploadPath)
									{
										try
										{
											ProjectManager manager(serverUrl);
											manager.UploadFile(projectName, fileUploadPath);
											_refreshContext = true;
										}
										catch (const exception& ex)
										{

										}
									}, _connectedServerUrl, project.name, std::ref(_refreshContext), _fileToUploadPath
										).detach();
									project.state = SourceFileUploading;
							}
							break;
						case SourceFileUploaded:
							if (ImGui::Button(buildLabel.c_str()))
							{
								ProjectManager manager(_connectedServerUrl);
								manager.ConvertProject(project.name);
								_refreshContext = true;
								project.state = ProjectConverting;
							}
							break;

						case SourceFileUploading:
							ImGui::Text("Uploading...");
							break;
						case ProjectConverting:
							ImGui::Text("Processing...");
							break;
						default:break;
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
					ProjectManager manager(_serverUrl);
					_projects = manager.GetAllProjectsInfo();
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

	ImGui::End();
}
