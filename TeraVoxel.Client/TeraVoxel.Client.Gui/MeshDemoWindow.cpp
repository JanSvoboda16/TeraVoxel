#include "MeshDemoWindow.h"
#include "imgui.h"
#include "../TeraVoxel.Client.VolumeRender/MeshGenerator.h"
#include "../TeraVoxel.Client.VolumeRender/MeshTreeExplorer.h"
#include "../TeraVoxel.Client.VolumeRender/Transformations.h"

void MeshDemoWindow::PrepareMeshes()
{
	_sphere = MeshGenerator::Sphere(200, 50, [this](float a, float b) { return MeshGenerator::SphereRainbowColor(a, b, _alpha); });
	
	_cube = MeshGenerator::Cube(200, 200,200, [this](bool a, bool b, bool c) { return Vector4b(a ? 0 : 255, b ? 0 : 255, c ? 0 : 255, _alpha); });
}

void MeshDemoWindow::UpdateScene()
{
	if (_sceneUpdateNeeded)
	{
		if (_alphaChanged)
		{
			PrepareMeshes();
			_alphaChanged = false;
		}

		std::shared_ptr<MeshNode> demoMesh;
		switch (_demoId)
		{
		case 1: demoMesh = _sphere; break;
		case 2: demoMesh = _cube; break;
		default: demoMesh = nullptr; break;
		}

		if (demoMesh != nullptr)
		{
			demoMesh->transformation = Transformations::GetTranslationMatrix(_meshPosition[0], _meshPosition[1], _meshPosition[2]) * Transformations::GetShrinkMatrix(_meshScale[0], _meshScale[1], _meshScale[2]);
			demoMesh->name = "demo";

			auto meshNode = _volumeViewContext->scene->GetMeshNode();
			MeshTreeExplorer::Delete(meshNode, "demo");
			meshNode->subNodes.push_back(demoMesh);
		}

		_sceneUpdateNeeded = false;
		_volumeViewContext->sceneUpdated.Notify();
	}
}

void MeshDemoWindow::Update()
{
	ImGui::Begin("Mesh demos");

	const char* demos[] = { "None", "Sphere", "Cube"};

	if (ImGui::Combo("Mesh", &_demoId, demos,  IM_ARRAYSIZE(demos)))
	{
		_sceneUpdateNeeded = true;
	}

	if (ImGui::InputFloat3("Position ", _meshPosition))
	{
		_sceneUpdateNeeded = true;
	}

	if (ImGui::InputFloat3("Scale", _meshScale))
	{
		_sceneUpdateNeeded = true;
	}

	if (ImGui::InputInt("Alpha", &_alpha, 1, 100))
	{
		if (_alpha < 0)
		{
			_alpha = 0;
		}
		else if (_alpha > 255)
		{
			_alpha = 255;
		}

		_alphaChanged = true;
		_sceneUpdateNeeded = true;
	}

	ImGui::End();
}
