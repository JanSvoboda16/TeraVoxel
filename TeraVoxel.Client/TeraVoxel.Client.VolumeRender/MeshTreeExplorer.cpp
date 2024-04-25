#include "MeshTreeExplorer.h"
#include <iostream>
#include <string>
#include <sstream>

std::shared_ptr<MeshNode> MeshTreeExplorer::Find(const std::shared_ptr<MeshNode>& node, const std::string& path)
{
	std::shared_ptr<MeshNode> nodeCpy = node;
	std::istringstream iss(path);
	std::string token;
	while (std::getline(iss, token, '.'))
	{
		if (nodeCpy == nullptr)
		{
			return nullptr;
		}

		bool found = false;
		for (auto& subNode : nodeCpy->subNodes)
		{
			if (subNode->name == token)
			{
				found = true;
				nodeCpy = node;
			}
		}

		if (!found)
		{
			return nullptr;
		}
	}

	return nodeCpy;
}

bool MeshTreeExplorer::Delete(const std::shared_ptr<MeshNode>& node, const std::string& path)
{
	std::shared_ptr<MeshNode> nodeCpy = node;
	std::shared_ptr<MeshNode> parent;
	std::istringstream iss(path);
	std::string token;
	while (std::getline(iss, token, '.'))
	{
		if (nodeCpy == nullptr)
		{
			return false;
		}

		bool found = false;
		for (auto& subNode : nodeCpy->subNodes)
		{
			if (subNode->name == token)
			{
				parent = nodeCpy;
				found = true;
				nodeCpy = subNode;
			}
		}

		if (!found)
		{
			return false;
		}
	}

	if (parent != nullptr)
	{
		parent->subNodes.remove(nodeCpy);
	}

	return true;
}