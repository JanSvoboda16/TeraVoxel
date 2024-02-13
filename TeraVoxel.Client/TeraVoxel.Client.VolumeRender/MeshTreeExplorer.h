#pragma once
#include "MeshNode.h"

class MeshTreeExplorer
{
public:
	/// <summary>
	/// Tries to find a subnode with given path. (path: subnode.subsubnode.subsubsubnode...)
	/// If not found -> nullptr
	/// </summary>
	/// <param name="node"></param>
	/// <param name="path"></param>
	/// <returns></returns>
	static std::shared_ptr<MeshNode> Find(const std::shared_ptr<MeshNode>& node, const std::string& path);

	/// <summary>
	/// Deletes subnode with given path if exists.
	/// </summary>
	/// <param name="node"></param>
	/// <param name="path"></param>
	/// <returns>true if existed</returns>
	static bool Delete(const std::shared_ptr<MeshNode>& node, const std::string& path);
};

