/*
 * Author: Jan Svoboda
 * University: BRNO UNIVERSITY OF TECHNOLOGY, FACULTY OF INFORMATION TECHNOLOGY
 */

#include "ToolWindow.h"
#include "imgui.h"
#include "imgui_stdlib.h"
#include "SurfaceExtractionView.h"

void ToolWindow::Update()
{
	ImGui::Begin("Tools");

	const char* tools[] = { "None", "Surface Extraction"};	

	if (ImGui::Combo("Tool", &_selectedToolId, tools, IM_ARRAYSIZE(tools)))
	{
		ChangeView();
	}

	if (_view != nullptr)
	{
		_view->Update();
	}

	ImGui::End();
}

void ToolWindow::ChangeView()
{
	switch (_selectedToolId)
	{
	case 1:
		_view = std::make_shared<SurfaceExtractionView>(_volumeViewContext); break;
	default:
		_view = nullptr; break;
	}
}
