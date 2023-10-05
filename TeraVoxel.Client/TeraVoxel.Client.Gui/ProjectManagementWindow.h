/*
 * Author: Jan Svoboda
 * University: BRNO UNIVERSITY OF TECHNOLOGY, FACULTY OF INFORMATION TECHNOLOGY
 */
#pragma once
#include <string>
#include "imgui.h"
#include "time.h"
#include "imgui_stdlib.h"
#include "../TeraVoxel.Client.Core/ProjectManager.h"
#include "VolumeViewContext.h"
#include "../TeraVoxel.Client.VolumeRender/NetMemoryVolumeSceneFactory.h"
#include <thread>
#include <vector>
#define CONTEXT_REFRESH_RATE 10000

class ProjectManagementWindow
{
public:
	ProjectManagementWindow(std::shared_ptr<VolumeViewContext> volumeViewContext)
	{
		_volumeViewContext = volumeViewContext;
		ProjectManager manager(_serverUrl);
	}
	void Update();

private:
	std::shared_ptr<VolumeViewContext> _volumeViewContext;

	// Variables of the textboxes
	std::string _serverUrl = "localhost:5000";
	std::string _createProjectName;
	std::string _fileToUploadPath;

	std::string _errorMessage;			// Error message
	std::string _showedErrorMessage;	// Error message in the error textbox

	std::vector<ProjectInfo> _projects;	// All project's metadata
	std::string _connectedServerUrl;	// Url of the current server
	int _selectedProjectIndex = -1;		// Index of the selected project
	std::string _selectedProjectName;	// Name of the selected project

	bool _refreshContext = false;		// Data should be reloaded from the server
	clock_t _lastRefresth = 0;			// Last refresh timestamp
	int _errorMessageDurationCounter = 0;
};

