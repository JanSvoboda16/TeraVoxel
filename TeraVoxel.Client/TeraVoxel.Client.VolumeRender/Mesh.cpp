#include "Mesh.h"

std::array<Vertex, 3> Mesh::GetTriangle(uint32_t position)
{	
	switch (_mode)
	{
	case Strip:		
		return { _vertices[position], _vertices[position+1], _vertices[position+2] };
		position += 1;
	case Fan:		
		return { _vertices[0], _vertices[position + 1], _vertices[position + 2] };
		position += 1;
	case List:
		position *= 3;
		return { _vertices[position], _vertices[position + 1], _vertices[position + 2] };
	default:
		throw std::exception("Unsupported mesh mode");
	}
}

void Mesh::SetTriangle(uint32_t position, const std::array<Vertex, 3>& triangle)
{
	switch (_mode)
	{
	case Strip:
		_vertices[position] = triangle[0];
		_vertices[position + 1] = triangle[1];
		_vertices[position + 2] = triangle[2];
		return;
	case Fan:
		_vertices[position] = triangle[0];
		_vertices[position + 1] = triangle[1];
		_vertices[position + 2] = triangle[2];
		return;
	case List:
		position *= 3;
		_vertices[position] = triangle[0];
		_vertices[position + 1] = triangle[1];
		_vertices[position + 2] = triangle[2];
		return;
	default:
		throw std::exception("Unsupported mesh mode");
	}
}

uint32_t Mesh::GetTriangleCount()
{
	switch (_mode)
	{
		case Strip:
			return _vertices.size() - 2;
		case Fan:
			return _vertices.size() - 2;
		case List: 
			return _vertices.size() / 3;
		default:
			throw std::exception("Unsupported mesh mode");
	}
}
