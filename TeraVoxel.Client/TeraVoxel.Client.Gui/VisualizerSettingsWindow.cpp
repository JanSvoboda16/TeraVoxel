/*
 * Author: Jan Svoboda
 * University: BRNO UNIVERSITY OF TECHNOLOGY, FACULTY OF INFORMATION TECHNOLOGY
 */
#include "VisualizerSettingsWindow.h"

void VisualizerSettingsWindow::SetVisualizer(int visualizerId)
{
    if (_volumeViewContext->scene != nullptr) {
        switch (visualizerId) {
        case 0:
            VolumeVisualizerSetter<EmptyVolumeVisualizerSetter>::Set(_volumeViewContext->scene, std::static_pointer_cast<VolumeVisualizerSettingsBase>(std::make_shared<EmptyVolumeVisualizerSettings>()));
            break;
        case 1:            
            VolumeVisualizerSetter<FastRcVolumeVisualizerSetter>::Set(_volumeViewContext->scene, std::static_pointer_cast<VolumeVisualizerSettingsBase>(_fastRayCastingVisualizerSettings));
            break;
        case 2:
            // Add your visualizer here
            break;
        }
        _volumeViewContext->sceneUpdated.Notify();
    }
}

void VisualizerSettingsWindow::ChangeView(int visualizerId) {
    switch (visualizerId)
    {
    case 0:
        _view = nullptr;
        break;
    case 1:
        _view = std::shared_ptr<IView>((IView*) new FastRCView(_volumeViewContext, _fastRayCastingVisualizerSettings));    
        break;
    default:
        // Add your visualizer view here
        break;
    }
}

void VisualizerSettingsWindow::Update()
{
    ImGui::Begin("Visualizer Settings");    
    
    const char* visualizerNames[] = { "None", "Fast RC", "Your visualizer" };

    if (ImGui::Combo("combo", &_selectedVisualizerId, visualizerNames, IM_ARRAYSIZE(visualizerNames))) {
        SetVisualizer(_selectedVisualizerId);
        ChangeView(_selectedVisualizerId);
    }
        
    if (_view != nullptr) {
        _view->Update();
    }

    ImGui::End();
}
