#pragma once
#include "MeshNode.h"

class MeshTreeExplorer
{
public:
	static std::shared_ptr<MeshNode> Find(const std::shared_ptr<MeshNode>& node, const std::string& path);
	static bool Delete(const std::shared_ptr<MeshNode>& node, const std::string& path);
};

