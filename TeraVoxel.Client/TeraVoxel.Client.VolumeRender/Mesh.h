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
	Mesh(std::vector<Vertex>&& data, MeshMode mode, bool outlining = false) : _vertices(std::move(data)), _mode(mode), _outlining(outlining) {};
	Mesh(const std::vector<Vertex>& data, MeshMode mode, bool outlining = false) : _vertices(data), _mode(mode), _outlining(outlining){};	
	
	/// <summary>
	/// Returns reference to the internal vertices memmory
	/// </summary>
	/// <returns></returns>
	std::vector<Vertex>& Data(){ return _vertices;}

	/// <summary>
	/// Gets mode of the mesh (strip|fan|list)
	/// </summary>
	/// <returns></returns>
	MeshMode GetMode() { return _mode; }

	/// <summary>
	/// Sets mode of the mesh (strip|fan|list)
	/// Default mode is list
	/// </summary>
	/// <returns></returns>
	void  SetMode(MeshMode mode) { _mode = mode; }	

	/// <summary>
	/// Gets information about outlining.
	/// </summary>
	/// <returns></returns>
	bool OutliningEnabled() { return _outlining; }

	/// <summary>
	/// Enables or disables outlining
	/// If outlining is enabled, rasterizer can rasterize thin triangles.
	/// Causes problems with transparent triangles.
	/// </summary>
	/// <param name="enabled"></param>
	void SetOutlining(bool enabled) { _outlining = enabled; }

	/// <summary>
	/// Returns N-th triangle of the mesh;
	/// </summary>
	/// <param name="position"></param>
	/// <returns></returns>
	std::array<Vertex, 3> GetTriangle(uint32_t position);

	/// <summary>
	/// Returns count of all triangles.
	/// </summary>
	/// <returns></returns>
	uint32_t GetTriangleCount();
private:
	std::vector<Vertex> _vertices;
	MeshMode _mode = MeshMode::List;
	bool _outlining = false;
};

