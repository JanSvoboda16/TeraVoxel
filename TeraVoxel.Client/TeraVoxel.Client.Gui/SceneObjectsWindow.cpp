#include "SceneObjectsWindow.h"
#include "imgui.h"
#include "../TeraVoxel.Client.VolumeRender/MeshGenerator.h"
#include "../TeraVoxel.Client.VolumeRender/MeshTreeExplorer.h"
#include "../TeraVoxel.Client.VolumeRender/Transformations.h"

void SceneObjectsWindow::UpdateScene()
{
	if (_sceneUpdateNeeded)
	{
		auto meshNode = _volumeViewContext->scene->GetMeshNode();
		auto projectInfo = _volumeViewContext->scene->GetProjectInfo();
		MeshTreeExplorer::Delete(meshNode, "CenterCross");
		MeshTreeExplorer::Delete(meshNode, "BoundingBox");

		Vector3f volumeSize(projectInfo.dataSizeX * projectInfo.voxelDimensions[0], projectInfo.dataSizeY * projectInfo.voxelDimensions[1], projectInfo.dataSizeZ * projectInfo.voxelDimensions[2]);

		if (_crossVisible)
		{			
			auto axisCross = MeshGenerator::AxisCross(volumeSize.maxCoeff(), Vector4b(100, 0, 100, 255));
			axisCross->name = "CenterCross";

			meshNode->subNodes.push_back(axisCross);

			_volumeViewContext->scene->GetCamera()->BindObserverCenterMeshNode(axisCross);
		}

		if (_boundingBoxVisible)
		{
			auto boundingBox = MeshGenerator::Cube(volumeSize.x(), volumeSize.y(), volumeSize.z(), [](bool a, bool b, bool c) { return  Vector4b(150, 100, 0, 70); });
			boundingBox->name = "BoundingBox";

			meshNode->subNodes.push_back(boundingBox);
		}

		_sceneUpdateNeeded = false;
		_volumeViewContext->sceneUpdated.Notify();
	}
}

void SceneObjectsWindow::Update()
{
	ImGui::Begin("Scene objects");

	if (ImGui::Checkbox("Center cross", &_crossVisible))
	{
		_sceneUpdateNeeded = true;
	}

	if (ImGui::Checkbox("Bonding box", &_boundingBoxVisible))
	{
		_sceneUpdateNeeded = true;
	}

	ImGui::End();
}
