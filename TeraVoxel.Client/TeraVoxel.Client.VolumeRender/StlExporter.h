#pragma once
#include "IMeshExporter.h"
#include <iostream>
#include <fstream>
#include "Transformations.h"

class StlExporter : public IMeshExporter
{
public:
	void Export(std::shared_ptr<MeshNode> mesh, std::string path);
private:
	uint32_t ExportRecursive(std::ofstream& fileStream, const std::shared_ptr<MeshNode>& node, const Matrix4f& baseTransform);
};

