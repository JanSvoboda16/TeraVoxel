#pragma once
#include "Mesh.h"

using Eigen::Matrix4f;

struct MeshNode
{
	std::vector<Mesh> meshes;
	Matrix4f transformation;
	std::list<std::shared_ptr<MeshNode>> subNodes;
};

