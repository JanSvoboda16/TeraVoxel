/*
 * Author: Jan Svoboda
 * University: BRNO UNIVERSITY OF TECHNOLOGY, FACULTY OF INFORMATION TECHNOLOGY
 */
#include "ProjectManagementWindow.h"


 // Konverze barevného prostoru HSL na RGB pro vytvoření duhových barev
float hueToRGB(float p, float q, float t)
{
	if (t < 0.0f) t += 1.0f;
	if (t > 1.0f) t -= 1.0f;
	if (t < 1.0f / 6.0f) return p + (q - p) * 6.0f * t;
	if (t < 1.0f / 2.0f) return q;
	if (t < 2.0f / 3.0f) return p + (q - p) * (2.0f / 3.0f - t) * 6.0f;
	return p;
}

Vector4b CalculateRainbowColor(float theta, float phi)
{

	// Mapování úhlu na barvu duhy
	float hue = phi / (2.0f * 3.1419); // Využití úhlu phi pro vyjádření barvy duhy
	float saturation = 1.0f;
	float lightness = 0.5f;

	// Pro změnu barvy v obou osách můžeme využít i úhel theta
	hue += theta / 3.1419;
	hue = fmod(hue, 1.0f); // Upravení hodnoty barvy do rozmezí 0 až 1

	float r, g, b;
	float q = lightness < 0.5f ? lightness * (1.0f + saturation) : lightness + saturation - lightness * saturation;
	float p = 2.0f * lightness - q;
	r = hueToRGB(p, q, hue + 1.0f / 3.0f);
	g = hueToRGB(p, q, hue);
	b = hueToRGB(p, q, hue - 1.0f / 3.0f);

	// Konverze hodnot RGB na formát pro vrchol meshu (Vector4b)
	int red = static_cast<int>(r * 255.0f);
	int green = static_cast<int>(g * 255.0f);
	int blue = static_cast<int>(b * 255.0f);

	return { static_cast<uint8_t>(red), static_cast<uint8_t>(green), static_cast<uint8_t>(blue), 255 }; // Nastavení alfa kanálu na 255
}

void CreateSphere(Mesh& mesh,int numSegments, float radius)
{
	const float PI = 3.1419;

	for (int i = 0; i < numSegments; ++i)
	{
		for (int j = 0; j <= numSegments; ++j)
		{
			float theta1 = static_cast<float>(i) / static_cast<float>(numSegments) * PI;
			float theta2 = static_cast<float>(i + 1) / static_cast<float>(numSegments) * PI;
			float phi1 = static_cast<float>(j) / static_cast<float>(numSegments) * 2.0f * PI;
			float phi2 = static_cast<float>(j + 1) / static_cast<float>(numSegments) * 2.0f * PI;

			float x1 = radius * sin(theta1) * cos(phi1); // Poloměr koule = 25 (poloměr = průměr / 2)
			float y1 = radius * sin(theta1) * sin(phi1);
			float z1 = radius * cos(theta1);

			float x2 = radius * sin(theta2) * cos(phi1);
			float y2 = radius * sin(theta2) * sin(phi1);
			float z2 = radius * cos(theta2);

			float x3 = radius * sin(theta1) * cos(phi2);
			float y3 = radius * sin(theta1) * sin(phi2);
			float z3 = radius * cos(theta1);

			float x4 = radius * sin(theta2) * cos(phi2);
			float y4 = radius * sin(theta2) * sin(phi2);
			float z4 = radius * cos(theta2);

			Vector4b color1 = CalculateRainbowColor(theta1, phi1);
			Vector4b color2 = CalculateRainbowColor(theta2, phi1);
			Vector4b color3 = CalculateRainbowColor(theta1, phi2);
			Vector4b color4 = CalculateRainbowColor(theta2, phi2);

			auto& meshData = mesh.Data();

			// Pushing vertex and color data into meshData
			meshData.push_back({ Vector3f(x1, y1, z1), color1 });
			meshData.push_back({ Vector3f(x2, y2, z2), color2 });
			meshData.push_back({ Vector3f(x3, y3, z3), color3 });

			meshData.push_back({ Vector3f(x2, y2, z2), color2 });
			meshData.push_back({ Vector3f(x4, y4, z4), color4 });
			meshData.push_back({ Vector3f(x3, y3, z3), color3 });
		}
	}
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

								Vector3f voxelDimensions = Vector3f(project.voxelDimensions);
								Vector3f initialPosition = Vector3f(project.dataSizeX, project.dataSizeY, project.dataSizeZ).array() / 2 * voxelDimensions.array();
								std::shared_ptr<Camera> camera = std::make_shared<Camera>(initialPosition, initialPosition[2] * 4, voxelDimensions, 0, 0, 1.2);


								auto meshNode = std::make_shared<MeshNode>();
								meshNode->transformation = Matrix4f::Identity();

								
								Mesh mesh;
								CreateSphere(mesh, 50,60);
								auto eye = std::make_shared<MeshNode>();
								eye->meshes.push_back(mesh);
								eye->transformation = Transformations::GetTranslationMatrix(130, 570, 170)*Transformations::GetRotationMatrix('y', EIGEN_PI / 2);

								auto eye2 = std::make_shared<MeshNode>();
								eye2->meshes.push_back(mesh);
								eye2->transformation = Transformations::GetTranslationMatrix(280, 560, 180) * Transformations::GetRotationMatrix('y', EIGEN_PI / 2);


								meshNode->subNodes.push_back(eye);
								meshNode->subNodes.push_back(eye2);

								_volumeViewContext->scene = NetMemoryVolumeSceneFactory::Create(camera, project, _connectedServerUrl, meshNode);
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
