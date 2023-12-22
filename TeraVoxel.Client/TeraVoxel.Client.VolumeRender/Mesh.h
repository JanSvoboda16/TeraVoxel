#pragma once
#include <cstdint>
#include <vector>
#include <array>
#include <Eigen/Dense>

using Eigen::Vector3f;
using Eigen::Vector4f;

typedef Eigen::Matrix<uint8_t, 4, 1 > Vector4b;

struct Vertex
{
	Vector3f position;
	Vector4b colour;
};

enum MeshMode
{
	Strip,
	Fan,
	List
};

class Mesh
{

public:
	Mesh() {};
	Mesh(std::vector<Vertex>&& data, MeshMode mode) : _vertices(std::move(data)), _mode(mode) {};
	Mesh(const std::vector<Vertex>& data, MeshMode mode) : _vertices(data), _mode(mode) {};	
	std::vector<Vertex>& Data(){ return _vertices;}
	MeshMode GetMode() { return _mode; }
	void  SetMode(MeshMode mode) { _mode = mode; }

	std::array<Vertex, 3> GetTriangle(uint32_t position);
	uint32_t GetTriangleCount();
private:
	std::vector<Vertex> _vertices;
	MeshMode _mode = MeshMode::List;
};

