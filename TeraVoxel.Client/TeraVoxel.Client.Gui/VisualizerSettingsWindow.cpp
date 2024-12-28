/*
 * Author: Jan Svoboda
 * University: BRNO UNIVERSITY OF TECHNOLOGY, FACULTY OF INFORMATION TECHNOLOGY
 */
#include "VisualizerSettingsWindow.h"
#include "../TeraVoxel.Client.VolumeRender/EmptyVolumeVisualizerFactory.h"
#include "../TeraVoxel.Client.VolumeRender/CPURCVolumeVisualizerFactory.h"
#include "../TeraVoxel.Client.VolumeRender/GPURCVolumeVisualizerFactory.h"

void VisualizerSettingsWindow::SetVisualizer(int visualizerId)
{   
	// TODO  -> do only when scene is editable. Should not cause any problems now. 
	if (_volumeViewContext->scene != nullptr)
	{
		switch (visualizerId)
		{
		case 0:
			_volumeViewContext->scene->ChangeVisualizer(std::make_shared<EmptyVolumeVisualizerFactory>(std::make_shared<EmptyVolumeVisualizerSettings>()));
			break;
		case 1:
			_volumeViewContext->scene->ChangeVisualizer(std::make_shared<CPURCVolumeVisualizerFactory>(_fastRayCastingVisualizerSettings));
			break;
		case 2:
			_volumeViewContext->scene->ChangeVisualizer(std::make_shared<GPURCVolumeVisualizerFactory>(_gpuRayCastingVisualizerSettings));
			break;
		}
		_volumeViewContext->sceneUpdated.Notify();
	}
}

void VisualizerSettingsWindow::ChangeView(int visualizerId)
{
	switch (visualizerId)
	{
	case 0:
		_view = nullptr;
		break;
	case 1:
		_view = std::shared_ptr<IView>((IView*) new CPURayCastingView(_volumeViewContext, _fastRayCastingVisualizerSettings));
		break;
	case 2:
		_view = std::shared_ptr<IView>((IView*) new GPURayCastingView(_volumeViewContext, _gpuRayCastingVisualizerSettings));
		break;
	}
}

void VisualizerSettingsWindow::Update()
{
	ImGui::Begin("Visualizer Settings");

	const char* visualizerNames[] = { "None", "CPU Ray casting", "GPU Ray casting" };

	if (ImGui::Combo("Visualizer:", &_selectedVisualizerId, visualizerNames, IM_ARRAYSIZE(visualizerNames)))
	{
		SetVisualizer(_selectedVisualizerId);
		ChangeView(_selectedVisualizerId);
	}

	if (_view != nullptr)
	{
		_view->Update();
	}

	ImGui::End();
}
