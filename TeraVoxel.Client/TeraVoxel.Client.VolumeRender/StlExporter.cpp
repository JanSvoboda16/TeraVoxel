#include "StlExporter.h"
void StlExporter::Export(std::shared_ptr<MeshNode> mesh, std::string path)
{
	std::ofstream stlFile(path, std::ios::binary | std::ios::out | std::ios::trunc);
	if (!stlFile.is_open())
	{
		throw "Unable to cerate stl file.";
	}

	uint32_t zero = 0;
	for (size_t i = 0; i < 20; i++)
	{
		stlFile.write(reinterpret_cast<char*>(&zero), sizeof(uint32_t));
	}

	stlFile.write(reinterpret_cast<char*>(&zero), sizeof(uint32_t));

	auto countOfTriangles = ExportRecursive(stlFile, mesh, Matrix4f::Identity());

	stlFile.seekp(80);

	stlFile.write(reinterpret_cast<char*>(&countOfTriangles), sizeof(uint32_t));

	stlFile.close();
}

uint32_t StlExporter::ExportRecursive(std::ofstream& fileStream, const std::shared_ptr<MeshNode>& node, const Matrix4f& baseTransform)
{
	if (node == nullptr)
	{
		return 0;
	}

	Matrix4f transformation = baseTransform * node->transformation;
	uint32_t triangleCount = 0;

	for (auto& mesh : node->meshes)
	{
		auto meshTriangleCount = mesh.GetTriangleCount();
		triangleCount += meshTriangleCount;

		for (size_t j = 0; j < meshTriangleCount; j++)
		{
			auto triangle = mesh.GetTriangle(j);

			for (int i = 0; i < 3; ++i)
			{
				triangle[i].position = (transformation * triangle[i].position.homogeneous()).head(3);
			}

			Vector3f normal(0, 0, 0);

			for (int i = 0; i < 3; ++i)
			{
				fileStream.write(reinterpret_cast<const char*>(&normal[i]), sizeof(float));
			}

			for (int i = 0; i < 3; ++i)
			{
				fileStream.write(reinterpret_cast<const char*>(&triangle[0].position[i]), sizeof(float));
			}

			for (int i = 0; i < 3; ++i)
			{
				fileStream.write(reinterpret_cast<const char*>(&triangle[1].position[i]), sizeof(float));
			}

			for (int i = 0; i < 3; ++i)
			{
				fileStream.write(reinterpret_cast<const char*>(&triangle[2].position[i]), sizeof(float));
			}

			uint16_t attribute = 0;
			fileStream.write(reinterpret_cast<char*>(&attribute), sizeof(uint16_t));
		}
	}

	for (auto& subnode : node->subNodes)
	{
		triangleCount + ExportRecursive(fileStream, subnode, transformation);
	}

	return triangleCount;
}