#include "GPURayCastingView.h"
#include "imgui_stdlib.h"

GPURayCastingView::GPURayCastingView(std::shared_ptr<VolumeViewContext> volumeViewContext, std::shared_ptr<GPURCVolumeVisualizerSettings> visualizerSettings) :
	_volumeViewContext(volumeViewContext), _visualizerSettings(visualizerSettings)
{
	_volumeViewContext->sceneEditable.Register(this, "UpdateSettings", [this]() { UpdateSettings(); });
	LoadTables();
}

void GPURayCastingView::Update()
{
    // TODO FIX, DONT EDIT SETTINGS WHEN RENDERING
    static ImGuiTableFlags flags = ImGuiTableFlags_ScrollY | ImGuiTableFlags_RowBg | ImGuiTableFlags_BordersOuter | ImGuiTableFlags_BordersV | ImGuiTableFlags_Resizable | ImGuiTableFlags_Reorderable | ImGuiTableFlags_Hideable;
    ImGui::Text("Material Tables");
    if (ImGui::Button("Load"))
    {
        LoadTables();
    }
    ImGui::Text("Existing tables");
    if (ImGui::BeginTable("MaterialSelect", 3, flags, ImVec2(0, 300)))
    {
        ImGui::TableSetupScrollFreeze(0, 1);
        ImGui::TableSetupColumn("Name", ImGuiTableColumnFlags_None);

        ImGuiListClipper clipper;
        clipper.Begin(_materialTables.size());

        while (clipper.Step())
        {
            for (int row = clipper.DisplayStart; row < clipper.DisplayEnd; row++)
            {
                ImGui::TableNextRow();

                ImGui::TableSetColumnIndex(0);
                ImGui::Text(_materialTables[row].c_str());

                auto loadLabel = "Load##" + std::to_string(row);
                ImGui::TableSetColumnIndex(1);
                if (ImGui::Button(loadLabel.c_str()))
                {
                    LoadTable(_materialTables[row]);
                }

                auto deleteLabel = "Delete##" + std::to_string(row);
                ImGui::TableSetColumnIndex(2);
                if (ImGui::Button(deleteLabel.c_str()))
                {
                    fs::remove("ColorMappingTables/" + _materialTables[row]);
                    LoadTables();
                }
            }
        }
        ImGui::EndTable();
    }

    ImGui::Text("Color Mapping Editor");
    static std::string fileName;
    ImGui::Text("Save this table");
    ImGui::InputText("File name", &fileName);

    if (ImGui::Button("Save"))
    {
        SaveToFile(fileName);
        LoadTables();
    }

    if (ImGui::Button("Add row"))
    {
        _visualizerSettingsPrivate.materialTable.table.push_back(MaterialTableItem());
        _visualizerSettingsPrivate.IncrementVersionId();
    }

    ImGui::Text("Mapping table");
    if (ImGui::BeginTable("MappingTable", 6, flags, ImVec2(0, 200)))
    {
        ImGui::TableSetupScrollFreeze(0, 1);
        ImGui::TableSetupColumn("Range", ImGuiTableColumnFlags_None);
        ImGui::TableSetupColumn("Color from", ImGuiTableColumnFlags_None);
        ImGui::TableSetupColumn("Color to", ImGuiTableColumnFlags_None);
        ImGui::TableSetupColumn("Translucency from", ImGuiTableColumnFlags_None);
        ImGui::TableSetupColumn("Translucency to", ImGuiTableColumnFlags_None);
        ImGui::TableHeadersRow();

        ImGuiListClipper clipper;
        clipper.Begin(_visualizerSettingsPrivate.materialTable.table.size());

        while (clipper.Step())
        {
            for (int row = clipper.DisplayStart; row < clipper.DisplayEnd; row++)
            {
                ImGui::TableNextRow();

                ImGui::TableSetColumnIndex(0);
                ImGui::PushItemWidth(-1);
                auto rangeLabel = "##Range" + std::to_string(row);
                if (ImGui::InputFloat2(rangeLabel.c_str(), _visualizerSettingsPrivate.materialTable.table[row].range))
                    _visualizerSettingsPrivate.IncrementVersionId();
                ImGui::PopItemWidth();


                auto colorFromLabel = "##ColorFrom" + std::to_string(row);
                ImGui::TableSetColumnIndex(1);
                ImGui::PushItemWidth(-1);
                if (ImGui::ColorEdit3(colorFromLabel.c_str(), _visualizerSettingsPrivate.materialTable.table[row].reflectionColorFrom, ImGuiColorEditFlags_NoInputs | ImGuiColorEditFlags_Float))
                    _visualizerSettingsPrivate.IncrementVersionId();
                ImGui::PopItemWidth();

                auto colorToLabel = "##ColorTo" + std::to_string(row);
                ImGui::TableSetColumnIndex(2);
                ImGui::PushItemWidth(-1);
                if (ImGui::ColorEdit3(colorToLabel.c_str(), _visualizerSettingsPrivate.materialTable.table[row].reflectionColorTo, ImGuiColorEditFlags_NoInputs | ImGuiColorEditFlags_Float))
                    _visualizerSettingsPrivate.IncrementVersionId();                
                ImGui::PopItemWidth();

                auto transFromLabel = "##TransFrom" + std::to_string(row);
                ImGui::TableSetColumnIndex(2);
                ImGui::PushItemWidth(-1);
                if (ImGui::ColorEdit3(transFromLabel.c_str(), _visualizerSettingsPrivate.materialTable.table[row].translucencyColorFrom, ImGuiColorEditFlags_NoInputs | ImGuiColorEditFlags_Float))
                    _visualizerSettingsPrivate.IncrementVersionId();
                ImGui::PopItemWidth();

                auto transToLabel = "##TransTo" + std::to_string(row);
                ImGui::TableSetColumnIndex(2);
                ImGui::PushItemWidth(-1);
                if (ImGui::ColorEdit3(transToLabel.c_str(), _visualizerSettingsPrivate.materialTable.table[row].translucencyColorTo, ImGuiColorEditFlags_NoInputs | ImGuiColorEditFlags_Float))
                    _visualizerSettingsPrivate.IncrementVersionId();
                ImGui::PopItemWidth();

                auto deleteLabel = "Delete##" + std::to_string(row);
                ImGui::TableSetColumnIndex(3);
                if (ImGui::Button(deleteLabel.c_str())) {
                    _visualizerSettingsPrivate.materialTable.table.erase(_visualizerSettingsPrivate.materialTable.table.begin() + row);
                    _visualizerSettingsPrivate.IncrementVersionId();
                }
            }
        }

        ImGui::EndTable();

        if (ImGui::DragFloat("Ambient intensity", &_visualizerSettingsPrivate.ambientIntensity)) {
            _volumeViewContext->sceneUpdated.Notify();
        }
            /*if (ImGui::DragFloat("Difusion intensity", &_visualizerSettings->difustionIntensity)) {
                _volumeViewContext->sceneUpdated.Notify();
            }
            if (ImGui::DragFloat("Reflection intensity", &_visualizerSettings->reflectionIntensity)) {
                _volumeViewContext->sceneUpdated.Notify();
            }
            if (ImGui::DragFloat("Reflection sharpness", &_visualizerSettings->reflectionSharpness)) {
                _volumeViewContext->sceneUpdated.Notify();
            }*/
       
    }
}

void GPURayCastingView::SaveToFile(std::string fileName)
{
}

void GPURayCastingView::LoadTables()
{
}

void GPURayCastingView::LoadTable(std::string fileName)
{
}

void GPURayCastingView::UpdateSettings()
{
    (*(_visualizerSettings.get())) = _visualizerSettingsPrivate;
    _volumeViewContext->sceneUpdated.Notify();
}
