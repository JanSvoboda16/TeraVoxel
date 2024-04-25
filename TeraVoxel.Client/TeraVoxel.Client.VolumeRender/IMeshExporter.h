#pragma once
#include <memory>
#include "MeshNode.h"

class IMeshExporter
{
public:
	virtual ~IMeshExporter() {};
	virtual void Export(std::shared_ptr<MeshNode> mesh, std::string path) = 0;
};

