#pragma once
#include "IView.h"
#include <memory>
#include "VolumeViewContext.h"

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

