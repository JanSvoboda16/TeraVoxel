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

std::shared_ptr<MeshNode> MarchingCubesSurfaceExtractor::ExtractSurface(const std::shared_ptr<VolumeSegment<bool>>& binMap, const ProjectInfo& projectInfo, bool interpolate, const Eigen::Vector2f &interpolationBoundary)
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
						mesh.Data().push_back(IndexToVertex(triangleVertIndexes[i * 3 + j], Vector4b(125 + index/2, 125 + index / 2, 125 + index / 2, 255), position, interpolate, interpolationBoundary));
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

bool IsBetween(float value, const Eigen::Vector2f& boundary) {
	return (value >= boundary.x()) && (value <= boundary.y());
}

float ComputeDistance(float value1, float value2, float edgeValue) {
	return (edgeValue - value1) / (value2 - value1);
}


Vertex MarchingCubesSurfaceExtractor::IndexToVertex(int index, const Vector4b& color, const Vector3f& position, bool interpolate, const Eigen::Vector2f& interpolationBoundary) {
	return CALL_TEMPLATED_FUNCTION(IndexToVertexTemplated, _volumeCache->GetProjectInfo().dataType.c_str(), index, color,  position, interpolate, interpolationBoundary);
}

template <typename T>
Vector3f MarchingCubesSurfaceExtractor::InterpolateVectorTemplated(Vector3f vector, Vector3f position, const Eigen::Vector2f& interpolationBoundary) {
	auto cache = std::dynamic_pointer_cast<VolumeCache<T>>(_volumeCache);
	Vector3f point1, point2;

	if (vector.x() == 0.5) {
		point1 = position + Vector3f(0, vector.y(), vector.z());
		point2 = point1 + Vector3f(1, 0, 0);
	}
	else if (vector.y() == 0.5) {
		point1 = position + Vector3f(vector.x(), 0, vector.z());
		point2 = point1 + Vector3f(0, 1, 0);
	}
	else {
		point1 = position + Vector3f(vector.x(), vector.y(), 0);
		point2 = point1 + Vector3f(0, 0, 1);
	}

	if ((point1.x() < _projectInfo.dataSizeX && point1.y() < _projectInfo.dataSizeY && point1.z() < _projectInfo.dataSizeZ && point1.x() >= 0 && point1.y() >= 0 && point1.z() >= 0)
		&& (point2.x() < _projectInfo.dataSizeX && point2.y() < _projectInfo.dataSizeY && point2.z() < _projectInfo.dataSizeZ && point2.x() >= 0 && point2.y() >= 0 && point2.z() >= 0))
	{
		T value1 = cache->GetValue(point1.x(), point1.y(), point1.z());
		T value2 = cache->GetValue(point2.x(), point2.y(), point2.z());


		float distance = 0.5;
		if (IsBetween(value1, interpolationBoundary))
		{
			if (value2 < interpolationBoundary.x())
			{
				float edgeValue = interpolationBoundary.x();
				distance = ComputeDistance(value1, value2, edgeValue);
			}
			else if (value2 > interpolationBoundary.y())
			{
				float edgeValue = interpolationBoundary.y();
				distance = ComputeDistance(value1, value2, edgeValue);
			}
		}
		else if (IsBetween(value2, interpolationBoundary))
		{
			if (value1 < interpolationBoundary.x())
			{
				float edgeValue = interpolationBoundary.x();
				distance = ComputeDistance(value1, value2, edgeValue);
			}
			else if (value1 > interpolationBoundary.y())
			{
				float edgeValue = interpolationBoundary.y();
				distance = ComputeDistance(value1, value2, edgeValue);
			}
		}

		if (vector.x() == 0.5) {
			vector.x() = distance;
		}
		else if (vector.y() == 0.5) {
			vector.y() = distance;
		}
		else {
			vector.z() = distance;
		}
	}


	return vector;
}
template <typename T>
Vertex MarchingCubesSurfaceExtractor::IndexToVertexTemplated(int index, const Vector4b& color, const Vector3f& position, bool interpolate, const Eigen::Vector2f& interpolationBoundary)
{
	Vector3f vector;
	switch (index)
	{
	case 0: vector = Vector3f(0.5, 0.0, 0.0); break;
	case 1: vector = Vector3f(1.0, 0.5, 0.0); break;
	case 2: vector = Vector3f(0.5, 1.0, 0.0); break;
	case 3: vector = Vector3f(0.0, 0.5, 0.0); break;

	case 4: vector = Vector3f(0.5, 0.0, 1.0); break;
	case 5: vector = Vector3f(1.0, 0.5, 1.0); break;
	case 6: vector = Vector3f(0.5, 1.0, 1.0); break;
	case 7: vector = Vector3f(0.0, 0.5, 1.0); break;

	case 8:  vector = Vector3f(0.0, 0.0, 0.5); break;
	case 9:  vector = Vector3f(1.0, 0.0, 0.5); break;
	case 10: vector = Vector3f(1.0, 1.0, 0.5); break;
	case 11: vector = Vector3f(0.0, 1.0, 0.5); break;
	}

	if (interpolate) {
		vector = InterpolateVectorTemplated<T>(vector, position, interpolationBoundary);
	}
	return Vertex{ vector + position, color };

}
