#include "FastRCView.h"

void FastRCView::SaveToFile(std::string fileName)
{
    std::ofstream file;
    file.open("ColorMappingTables/" + fileName);
    json data = _visualizerSettings->mappingTable;
    file << data;
    file.close();
}

void FastRCView::LoadTables()
{
    std::string path = "ColorMappingTables";
    fs::create_directory(path);
    _mappingTables.clear();

    for (const auto& entry : fs::directory_iterator(path))
        _mappingTables.push_back(entry.path().filename().string());
}

void FastRCView::LoadTable(std::string fileName)
{
    std::fstream file;
    file.open("ColorMappingTables/" + fileName);
    std::stringstream strStream;
    strStream << file.rdbuf();

    json data = json::parse(strStream);
    _volumeViewContext->sceneUpdated.Notify();
    _visualizerSettings->mappingTable = data.get<ColorMappingTable>();
}

void FastRCView::Update()
{
    static ImGuiTableFlags flags = ImGuiTableFlags_ScrollY | ImGuiTableFlags_RowBg | ImGuiTableFlags_BordersOuter | ImGuiTableFlags_BordersV | ImGuiTableFlags_Resizable | ImGuiTableFlags_Reorderable | ImGuiTableFlags_Hideable;
    ImGui::Text("Color Mapping Tables");
    if (ImGui::Button("Load"))
    {
        LoadTables();
    }
    ImGui::Text("Existing tables");
    if (ImGui::BeginTable("MappingSelect", 3, flags, ImVec2(0,300)))
    {
        ImGui::TableSetupScrollFreeze(0, 1);
        ImGui::TableSetupColumn("Name", ImGuiTableColumnFlags_None);

        ImGuiListClipper clipper;
        clipper.Begin(_mappingTables.size());

        while (clipper.Step())
        {
            for (int row = clipper.DisplayStart; row < clipper.DisplayEnd; row++)
            {
                ImGui::TableNextRow();

                ImGui::TableSetColumnIndex(0);
                ImGui::Text(_mappingTables[row].c_str());

                auto loadLabel = "Load##" + std::to_string(row);
                ImGui::TableSetColumnIndex(1);
                if (ImGui::Button(loadLabel.c_str()))
                {
                    LoadTable(_mappingTables[row]);
                }

                auto deleteLabel = "Delete##" + std::to_string(row);
                ImGui::TableSetColumnIndex(2);
                if (ImGui::Button(deleteLabel.c_str()))
                {
                    fs::remove("ColorMappingTables/" + _mappingTables[row]);
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
        _visualizerSettings->mappingTable.Table.push_back(ColorMappingItem());
        _volumeViewContext->sceneUpdated.Notify();
    }

    ImGui::Text("Mapping table");
    if (ImGui::BeginTable("MappingTable", 4, flags, ImVec2(0, 200)))
    {

        ImGui::TableSetupScrollFreeze(0, 1);
        ImGui::TableSetupColumn("Range", ImGuiTableColumnFlags_None);
        ImGui::TableSetupColumn("Color from", ImGuiTableColumnFlags_None);
        ImGui::TableSetupColumn("Color to", ImGuiTableColumnFlags_None);
        ImGui::TableHeadersRow();

        ImGuiListClipper clipper;
        clipper.Begin(_visualizerSettings->mappingTable.Table.size());

        while (clipper.Step())
        {
            for (int row = clipper.DisplayStart; row < clipper.DisplayEnd; row++)
            {
                ImGui::TableNextRow();

                ImGui::TableSetColumnIndex(0);
                ImGui::PushItemWidth(-1);
                auto rangeLabel = "##Range" + std::to_string(row);
                if (ImGui::InputFloat2(rangeLabel.c_str(), (_visualizerSettings->mappingTable.Table[row].Range)))
                    _volumeViewContext->sceneUpdated.Notify();
                ImGui::PopItemWidth();


                auto colorFromLabel = "##ColorFrom" + std::to_string(row);
                ImGui::TableSetColumnIndex(1);
                ImGui::PushItemWidth(-1);
                if (ImGui::ColorEdit4(colorFromLabel.c_str(), (_visualizerSettings->mappingTable.Table[row].ColorFrom), ImGuiColorEditFlags_AlphaBar | ImGuiColorEditFlags_Float))
                    _volumeViewContext->sceneUpdated.Notify();
                ImGui::PopItemWidth();


                auto colorToLabel = "##ColorTo" + std::to_string(row);
                ImGui::TableSetColumnIndex(2);
                ImGui::PushItemWidth(-1);
                if (ImGui::ColorEdit4(colorToLabel.c_str(), (_visualizerSettings->mappingTable.Table[row].ColorTo), ImGuiColorEditFlags_AlphaBar | ImGuiColorEditFlags_Float))
                    _volumeViewContext->sceneUpdated.Notify();
                ImGui::PopItemWidth();

                auto deleteLabel = "Delete##" + std::to_string(row);
                ImGui::TableSetColumnIndex(3);
                if (ImGui::Button(deleteLabel.c_str())) {
                    _visualizerSettings->mappingTable.Table.erase(_visualizerSettings->mappingTable.Table.begin() + row);
                    _volumeViewContext->sceneUpdated.Notify();
                }
            }
            
        }
        
        ImGui::EndTable();

        if (ImGui::Checkbox("Interpolate + shading", &_visualizerSettings->shading)) {
            _volumeViewContext->sceneUpdated.Notify();
        }
        if (_visualizerSettings->shading) {
            if (ImGui::DragFloat("Ambient intensity", &_visualizerSettings->ampbientIntensity)) {
                _volumeViewContext->sceneUpdated.Notify();
            }
            if (ImGui::DragFloat("Difusion intensity", &_visualizerSettings->difustionIntensity)) {
                _volumeViewContext->sceneUpdated.Notify();
            }
            if (ImGui::DragFloat("Reflection intensity", &_visualizerSettings->reflectionIntensity)) {
                _volumeViewContext->sceneUpdated.Notify();
            }
            if (ImGui::DragFloat("Reflection sharpness", &_visualizerSettings->reflectionSharpness)) {
                _volumeViewContext->sceneUpdated.Notify();

            }
        }

        
    }
}
