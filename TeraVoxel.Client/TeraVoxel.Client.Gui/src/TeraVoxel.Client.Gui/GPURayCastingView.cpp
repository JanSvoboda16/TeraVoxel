/*
 * Author: Jan Svoboda
 * University: BRNO UNIVERSITY OF TECHNOLOGY, FACULTY OF INFORMATION TECHNOLOGY
 */

#include "TeraVoxel.Client.Gui/GPURayCastingView.h"
#include "imgui_stdlib.h"

#define CLAMP_FLOAT(x, mn, mx) ((x) = fmaxf(fminf((x), (mx)), (mn)))
#define CLAMP_FLOAT_MIN(x, mn) ((x) = fmaxf((x), (mn)))
#define CLAMP_FLOAT_MAX(x, mX) ((x) = fminf((x), (mx)))


GPURayCastingView::GPURayCastingView(std::shared_ptr<VolumeViewContext> volumeViewContext, std::shared_ptr<GPURCVolumeVisualizerSettings> visualizerSettings) :
	_volumeViewContext(volumeViewContext), _visualizerSettings(visualizerSettings)
{
	_volumeViewContext->sceneEditable.Register(this, "UpdateSettings", [this]() { UpdateSettings(); });
	LoadTables();
    LoadLightings();
}

GPURayCastingView::~GPURayCastingView()
{
    _volumeViewContext->sceneEditable.Unregister(this, "UpdateSettings");
}

void GPURayCastingView::Update()
{
    // TODO FIX, DONT EDIT SETTINGS WHEN RENDERING
    static ImGuiTableFlags flags = ImGuiTableFlags_ScrollY | ImGuiTableFlags_RowBg | ImGuiTableFlags_BordersOuter | ImGuiTableFlags_BordersV | ImGuiTableFlags_Resizable | ImGuiTableFlags_Reorderable | ImGuiTableFlags_Hideable;
    if (ImGui::CollapsingHeader("Material Tables")) 
    {
        ImGui::Text("Existing tables");
        if (ImGui::BeginTable("MaterialSelect", 3, flags, ImVec2(0, 200)))
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
                        fs::remove("MaterialTables/" + _materialTables[row]);
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
            SaveTable(fileName);
            LoadTables();
        }

        if (ImGui::Button("Add row"))
        {
            _visualizerSettingsPrivate.materialTable.table.push_back(MaterialTableItem());
            _visualizerSettingsPrivate.IncrementVersionId();
        }

        ImGui::Text("MaterialTable");
        if (ImGui::BeginTable("MappingTable", 4, flags, ImVec2(0, 200)))
        {
            ImGui::TableSetupScrollFreeze(0, 1);
            ImGui::TableSetupColumn("Range", ImGuiTableColumnFlags_None);
            ImGui::TableSetupColumn("Material From", ImGuiTableColumnFlags_None);
            ImGui::TableSetupColumn("Material To", ImGuiTableColumnFlags_None);
            ImGui::TableHeadersRow();

   
            for (int row = 0; row < _visualizerSettingsPrivate.materialTable.table.size(); row++)
            {
                ImGui::TableNextRow();

                ImGui::TableSetColumnIndex(0);
                ImGui::PushItemWidth(-1);
                auto rangeLabel = "##Range" + std::to_string(row);
                if (ImGui::InputFloat2(rangeLabel.c_str(), _visualizerSettingsPrivate.materialTable.table[row].range))
                    _visualizerSettingsPrivate.IncrementVersionId();
                ImGui::PopItemWidth();

                auto deleteLabel = "Delete##" + std::to_string(row);
                ImGui::TableSetColumnIndex(3);
                if (ImGui::Button(deleteLabel.c_str())) {
                    _visualizerSettingsPrivate.materialTable.table.erase(_visualizerSettingsPrivate.materialTable.table.begin() + row);
                    _visualizerSettingsPrivate.IncrementVersionId();
                }

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

                ImGui::TableNextRow();
                auto transFromLabel = "##TransFrom" + std::to_string(row);
                ImGui::TableSetColumnIndex(1);
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

                ImGui::TableNextRow();
                auto specRefFromLabel = "##SpecRefFrom" + std::to_string(row);
                ImGui::TableSetColumnIndex(1);
                ImGui::PushItemWidth(-1);
                if (ImGui::InputFloat(specRefFromLabel.c_str(), &_visualizerSettingsPrivate.materialTable.table[row].specularReflectionFrom, 0.05f))
                {
                    CLAMP_FLOAT(_visualizerSettingsPrivate.materialTable.table[row].specularReflectionFrom, 0.f, 1.f);
                    _visualizerSettingsPrivate.IncrementVersionId();
                }
                ImGui::PopItemWidth();

                auto specRefToLabel = "##SpecRefTo" + std::to_string(row);
                ImGui::TableSetColumnIndex(2);
                ImGui::PushItemWidth(-1);
                if (ImGui::InputFloat(specRefToLabel.c_str(), &_visualizerSettingsPrivate.materialTable.table[row].specularReflectionTo, 0.05f))
                {
                    CLAMP_FLOAT(_visualizerSettingsPrivate.materialTable.table[row].specularReflectionTo, 0.f, 1.f);
                    _visualizerSettingsPrivate.IncrementVersionId();
                }
                ImGui::PopItemWidth();

                ImGui::TableNextRow();
                auto specSharpFromLabel = "##SpecSharpFrom" + std::to_string(row);
                ImGui::TableSetColumnIndex(1);
                ImGui::PushItemWidth(-1);
                if (ImGui::InputFloat(specSharpFromLabel.c_str(), &_visualizerSettingsPrivate.materialTable.table[row].specularSharpnessFrom, 0.05f))
                {
                    CLAMP_FLOAT_MIN(_visualizerSettingsPrivate.materialTable.table[row].specularSharpnessFrom, 0);
                    _visualizerSettingsPrivate.IncrementVersionId();
                }
                ImGui::PopItemWidth();

                auto specSharpToLabel = "##SpecSharpTo" + std::to_string(row);
                ImGui::TableSetColumnIndex(2);
                ImGui::PushItemWidth(-1);
                if (ImGui::InputFloat(specSharpToLabel.c_str(), &_visualizerSettingsPrivate.materialTable.table[row].specularSharpnessTo, 0.05f))
                {
                    CLAMP_FLOAT_MIN(_visualizerSettingsPrivate.materialTable.table[row].specularSharpnessTo, 0);
                    _visualizerSettingsPrivate.IncrementVersionId();
                }
                ImGui::PopItemWidth();
            }            

            ImGui::EndTable();
        }

    }
    if (ImGui::CollapsingHeader("Lightings")) 
    {
        ImGui::Text("Existing lightings");
        if (ImGui::BeginTable("LightingSellect", 3, flags, ImVec2(0, 200)))
        {
            ImGui::TableSetupScrollFreeze(0, 1);
            ImGui::TableSetupColumn("Name", ImGuiTableColumnFlags_None);

            ImGuiListClipper clipper;
            clipper.Begin(_lightings.size());

            while (clipper.Step())
            {
                for (int row = clipper.DisplayStart; row < clipper.DisplayEnd; row++)
                {
                    ImGui::TableNextRow();

                    ImGui::TableSetColumnIndex(0);
                    ImGui::Text(_lightings[row].c_str());

                    auto loadLabel = "Load##" + std::to_string(row);
                    ImGui::TableSetColumnIndex(1);
                    if (ImGui::Button(loadLabel.c_str()))
                    {
                        LoadLighting(_lightings[row]);
                    }

                    auto deleteLabel = "Delete##" + std::to_string(row);
                    ImGui::TableSetColumnIndex(2);
                    if (ImGui::Button(deleteLabel.c_str()))
                    {
                        fs::remove("Lightings/" + _lightings[row]);
                        LoadLightings();
                    }
                }
            }

            ImGui::EndTable();
        }

        static std::string fileNameLighting;
        ImGui::Text("Save this lighting");
        ImGui::InputText("File name#Lighting", &fileNameLighting);

        if (ImGui::Button("Save Lighting"))
        {
            SaveLighting(fileNameLighting);
            LoadLightings();
        }

        if (ImGui::Button("Add light"))
        {
            if (_visualizerSettingsPrivate.lightSettings.numLights < 5)
            {
                _visualizerSettingsPrivate.lightSettings.lights[_visualizerSettingsPrivate.lightSettings.numLights] = Light();
                _visualizerSettingsPrivate.lightSettings.numLights++;
            }
        }

        ImGui::Text("Lights");
        if (ImGui::BeginTable("LightsTable", 3, flags, ImVec2(0, 200)))
        {
            ImGui::TableSetupScrollFreeze(0, 1);
            ImGui::TableSetupColumn("Position", ImGuiTableColumnFlags_None);
            ImGui::TableSetupColumn("Intensity", ImGuiTableColumnFlags_None);
            ImGui::TableHeadersRow();

            ImGuiListClipper clipper;
            clipper.Begin(_visualizerSettingsPrivate.lightSettings.numLights);

            while (clipper.Step())
            {
                for (int row = clipper.DisplayStart; row < clipper.DisplayEnd; row++)
                {
                    ImGui::TableNextRow();

                    ImGui::TableSetColumnIndex(0);
                    ImGui::PushItemWidth(-1);
                    auto posLabel = "##LightPosition" + std::to_string(row);
                    if (ImGui::InputFloat3(posLabel.c_str(), _visualizerSettingsPrivate.lightSettings.lights[row].position))
                        _visualizerSettingsPrivate.IncrementVersionId();
                    ImGui::PopItemWidth();

                    ImGui::TableSetColumnIndex(1);
                    ImGui::PushItemWidth(-1);
                    auto intensityLabel = "##LightIntensity" + std::to_string(row);
                    if (ImGui::InputFloat(intensityLabel.c_str(), &_visualizerSettingsPrivate.lightSettings.lights[row].intensity, 0.1f))
                    {
                        CLAMP_FLOAT(_visualizerSettingsPrivate.lightSettings.lights[row].intensity, 0.f, 299792458.f);
                        _visualizerSettingsPrivate.IncrementVersionId();
                    }
                    ImGui::PopItemWidth();

                    auto deleteLabel = "DeleteLight##" + std::to_string(row);
                    ImGui::TableSetColumnIndex(2);
                    if (ImGui::Button(deleteLabel.c_str()))
                    {
                        for (size_t i = row; i < _visualizerSettingsPrivate.lightSettings.numLights - 1; ++i)
                        {
                            _visualizerSettingsPrivate.lightSettings.lights[i] = _visualizerSettingsPrivate.lightSettings.lights[i + 1];
                        }

                        _visualizerSettingsPrivate.lightSettings.numLights--;
                        _visualizerSettingsPrivate.IncrementVersionId();
                    }
                }
            }

            ImGui::EndTable();
        }

        if (ImGui::InputFloat("Ambient intensity", &_visualizerSettingsPrivate.lightSettings.ambientIntensity, 0.02f))
        {
            CLAMP_FLOAT(_visualizerSettingsPrivate.lightSettings.ambientIntensity, 0.f, 1.f);
            _visualizerSettingsPrivate.IncrementVersionId();
        }
    }
}

void GPURayCastingView::SaveTable(std::string fileName)
{
    std::ofstream file;
    file.open("MaterialTables/" + fileName);
    json data = _visualizerSettingsPrivate.materialTable;
    file << data;
    file.close();
}

void GPURayCastingView::LoadTables()
{
    std::string path = "MaterialTables";
    fs::create_directory(path);
    _materialTables.clear();

    for (const auto& entry : fs::directory_iterator(path))
        _materialTables.push_back(entry.path().filename().string());
}

void GPURayCastingView::LoadTable(std::string fileName)
{
    std::fstream file;
    file.open("MaterialTables/" + fileName);
    std::stringstream strStream;
    strStream << file.rdbuf();

    json data = json::parse(strStream);
    _visualizerSettingsPrivate.materialTable = data.get<MaterialTable>();
    _visualizerSettingsPrivate.IncrementVersionId();
}

void GPURayCastingView::SaveLighting(std::string fileName)
{
    std::ofstream file;
    file.open("Lightings/ " + fileName);
    json data = _visualizerSettingsPrivate.lightSettings;
    file << data;
    file.close();
}

void GPURayCastingView::LoadLightings()
{
    std::string path = "Lightings";
    fs::create_directory(path);
    _lightings.clear();

    for (const auto& entry : fs::directory_iterator(path))
        _lightings.push_back(entry.path().filename().string());
}

void GPURayCastingView::LoadLighting(std::string fileName)
{
    std::fstream file;
    file.open("Lightings/" + fileName);
    std::stringstream strStream;
    strStream << file.rdbuf();

    json data = json::parse(strStream);
    _visualizerSettingsPrivate.lightSettings = data.get<LightSettings>();
    _visualizerSettingsPrivate.IncrementVersionId();
}

void GPURayCastingView::UpdateSettings()
{
    if (_visualizerSettings->VersionId() != _visualizerSettingsPrivate.VersionId()) {
        (*(_visualizerSettings.get())) = _visualizerSettingsPrivate;
        _volumeViewContext->sceneUpdated.Notify();
    }
}
