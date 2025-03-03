#pragma once
#include "TeraVoxel.Client.VolumeRenderer/Mesh.h"
#include <list>

using Eigen::Matrix4f;

struct MeshNode
{
	std::string name;
	std::vector<Mesh> meshes;
	Matrix4f transformation = Matrix4f::Identity();
	std::list<std::shared_ptr<MeshNode>> subNodes;
};

