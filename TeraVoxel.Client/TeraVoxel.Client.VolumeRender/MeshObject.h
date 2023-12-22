#pragma once
#include "Mesh.h"

using Eigen::Matrix4f;

struct MeshObject
{
	std::vector<Mesh> meshes;
	Matrix4f transformation;
	std::vector<std::shared_ptr<MeshObject>> subobjects;
};

