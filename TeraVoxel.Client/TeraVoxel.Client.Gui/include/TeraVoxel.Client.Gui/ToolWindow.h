/*
 * Author: Jan Svoboda
 * University: BRNO UNIVERSITY OF TECHNOLOGY, FACULTY OF INFORMATION TECHNOLOGY
 */

#pragma once
#include "TeraVoxel.Client.Gui/IView.h"
#include <memory>
#include "TeraVoxel.Client.Gui/VolumeViewContext.h"

class ToolWindow : public IView
{
public: 
	ToolWindow(const std::shared_ptr<VolumeViewContext>& volumeViewContext) : _volumeViewContext(volumeViewContext) { }
	void Update();

private: 
	void ChangeView();

	int _selectedToolId = 0;
	std::shared_ptr<IView> _view;
	std::shared_ptr<VolumeViewContext> _volumeViewContext;
};

