#include "MarchingCubesSurfaceExtractor.h"
#include "Transformations.h"


__forceinline bool MarchingCubesSurfaceExtractor::GetValue(const std::shared_ptr<VolumeSegment<bool>>& binMap, int x, int y, int z, const ProjectInfo& projectInfo)
{
	bool* data = binMap->data;
	if (x >= 0 && x < projectInfo.dataSizeX && y >= 0 && y < projectInfo.dataSizeY && z >= 0 && z < projectInfo.dataSizeZ)
	{
		return data[x + y * projectInfo.dataSizeX + z * projectInfo.dataSizeX * projectInfo.dataSizeY];
	}
	else
	{
		return false;
	}
}

std::shared_ptr<MeshNode> MarchingCubesSurfaceExtractor::ExtractSurface(const std::shared_ptr<VolumeSegment<bool>>& binMap, const ProjectInfo& projectInfo)
{
	Mesh mesh;
	mesh.SetMode(MeshMode::List);

	//Data True = inside False = outside. Aligned as sequence of x rows firstly in y and then z
	bool* data = binMap->data;
	char colorer = 0;
	
	for (int z = -1; z < projectInfo.dataSizeZ; z++)
	{
		for (int y = -1; y < projectInfo.dataSizeY; y++)
		{
			for (int x = -1; x < projectInfo.dataSizeX; x++)
			{
				int v000 = GetValue(binMap, x, y, z, projectInfo);
				int v001 = GetValue(binMap, x + 1, y, z, projectInfo);
				int v010 = GetValue(binMap, x + 1, y + 1, z, projectInfo);
				int v011 = GetValue(binMap, x , y + 1, z, projectInfo);

				int v100 = GetValue(binMap, x, y, z + 1, projectInfo);
				int v101 = GetValue(binMap, x + 1, y, z + 1, projectInfo);
				int v110 = GetValue(binMap, x +1 , y + 1, z + 1, projectInfo);
				int v111 = GetValue(binMap, x, y + 1, z + 1, projectInfo);

				int index = v000 | (v001 << 1) | (v010 << 2) | (v011 << 3) | (v100 << 4) | (v101 << 5) | (v110 << 6) | (v111 << 7);

				std::vector<int> triangleVertIndexes = TriangleTable[index];

				uint8_t light = 125 + ((0b00000001 & index) + ((0b00001000 & index) >> 3) + ((0b00010000 & index)>>4) + ((0b10000000 & index)>>7)) * 31;
				
				Vector3f position(x,y,z);
				for (size_t i = 0; i < triangleVertIndexes.size() / 3; i++)
				{
					for (size_t j = 0; j < 3; j++)
					{
						mesh.Data().push_back(IndexToVertex(triangleVertIndexes[i * 3 + j], Vector4b(125 + index/2, 125 + index / 2, 125 + index / 2, 255), position));
					}
				}
			}
		}
	}

	auto node = std::make_shared<MeshNode>();
	node->transformation = Transformations::GetShrinkMatrix(projectInfo.voxelDimensions[0], projectInfo.voxelDimensions[1], projectInfo.voxelDimensions[2]);
	node->meshes.push_back(mesh);
	return node;
}

Vertex MarchingCubesSurfaceExtractor::IndexToVertex(int index, const Vector4b &color, const Vector3f &position)
{
	switch (index)
	{
	case 0: return Vertex{ Vector3f(0.5,0.0,0.0) + position, color };
	case 1: return Vertex{ Vector3f(1.0,0.5,0.0) + position, color };
	case 2: return Vertex{ Vector3f(0.5,1.0,0.0) + position, color };
	case 3: return Vertex{ Vector3f(0.0,0.5,0.0) + position, color };

	case 4: return Vertex{ Vector3f(0.5,0.0,1.0) + position, color };
	case 5: return Vertex{ Vector3f(1.0,0.5,1.0) + position, color };
	case 6: return Vertex{ Vector3f(0.5,1.0,1.0) + position, color };
	case 7: return Vertex{ Vector3f(0.0,0.5,1.0) + position, color };

	case 8:  return Vertex{ Vector3f(0.0,0.0,0.5) + position, color };
	case 9:  return Vertex{ Vector3f(1.0,0.0,0.5) + position, color };
	case 10: return Vertex{ Vector3f(1.0,1.0,0.5) + position, color };
	case 11: return Vertex{ Vector3f(0.0,1.0,0.5) + position, color };
	}
}