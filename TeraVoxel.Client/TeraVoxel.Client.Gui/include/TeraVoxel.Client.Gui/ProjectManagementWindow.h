/*
 * Author: Jan Svoboda
 * University: BRNO UNIVERSITY OF TECHNOLOGY, FACULTY OF INFORMATION TECHNOLOGY
 */
#pragma once
#include <string>
#include <time.h>
#include "TeraVoxel.Client.Gui/VolumeViewContext.h"
#include <thread>
#include <vector>
#include "TeraVoxel.Client.Gui/IView.h"

#define CONTEXT_REFRESH_RATE 10000

class ProjectManagementWindow : public IView
{
public:
	ProjectManagementWindow(const std::shared_ptr<VolumeViewContext> &volumeViewContext);
	void Update();

private:

	void SetDatasetSourceView(int id);

	std::unique_ptr<IView> _view; // NeuroVoxel OR NetProject
	std::shared_ptr<VolumeViewContext> _volumeViewContext; // Shared
	int _selectedDataSourceId = 1;
};

