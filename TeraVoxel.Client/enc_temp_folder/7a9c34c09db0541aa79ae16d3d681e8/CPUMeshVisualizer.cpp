#include "CPUMeshVisualizer.h"

std::vector<Vector3f> CPUMeshVisualizer::ComputeViewFrustumIntersecions(const Vector3f& A, const Vector3f& B)
{
	// intersections of lines with planes:
	// 
	// position (any point on the line) (x1,y1,z1)
	// direction (a,b,c)
	// x = x1+a*k
	// y = y1+b*k
	// z = z1+c*k
	// 
	// the intersection of a line with a plane perpendicular to the Z-axis -> 'z' is constant
	// k=(z-z1)/c k>=0
	// pz=position + k*direction
	//
	// the intersection of a line with a plane perpendicular to the X-axis -> 'x' is constant
	// k=(x-x1)/b k>=0
	// px=position + k*direction
	//
	// the intersection of a line with a plane perpendicular to the Y-axis -> 'y' is constant
	// k=(y-y1)/a k>=0
	// py=position + k*direction

	Vector3f maxIndexes(1.f,1.f,1.f);
	Vector3f minIndexes(-1.f, -1.f, -1.f);
	Vector3f direction = (B - A);
	Vector3f k = (minIndexes-A).array() / direction.array();
	std::vector<Vector3f> intersections;

	for (size_t j = 0; j < 2; j++)
	{
		for (size_t i = 0; i < 3; i++)
		{
			Vector3f intersection = A + direction * k[i];
			if (k[i] > 0 && (intersection - A).norm() < direction.norm())
			{
				if ((intersection[(i + 1) % 3] >= -1.f && intersection[(i + 1) % 3] <= 1.f)
					&& (intersection[(i + 2) % 3] >= -1.f && intersection[(i + 2) % 3] <= 1.f))
				{
					if (j == 0)
					{
						intersection[i] = minIndexes[i];
					}
					else
					{
						intersection[i] = maxIndexes[i];
					}
					intersections.push_back(intersection);
				}
			}
		}
		k = (maxIndexes - A).array() / direction.array();
	}


	int intersectionsCount = intersections.size();
	
	// Two walls
	if (intersectionsCount == 2)
	{
		if ((intersections[0] - A).norm() > (intersections[1] - A).norm())
		{
			std::swap(intersections[0], intersections[1]);			
		}
	}

	// Different -> we use the closest and the farest
	else if (intersectionsCount >= 3)
	{

		float closest = (intersections[0] - A).norm();
		int closestIndex = 0;
		float farest = (intersections[0] - A).norm();
		int farestIndex = 0;

		for (int i = 1; i < intersectionsCount; i++)
		{
			float distance = (intersections[i] -A).norm();
			if (distance > farest)
			{
				farest = distance;
				farestIndex = i;
			}
			if (distance < closest)
			{
				closest = distance;
				closestIndex = i;
			}
		}


		auto closestPosition = intersections[closestIndex];
		auto farestPosition = intersections[farestIndex];


		intersections.resize(2);
		intersections[0] = closestPosition;
		intersections[1] = farestPosition;		
	}

	return intersections;
}

bool CPUMeshVisualizer::Clipping(const std::array<Vertex, 3> &triangle, std::vector<std::array<Vertex, 3>> &outTriangles)
{

	std::vector<Vertex> vertices;
	bool anyOut = false;

	for (uint8_t i = 0; i < 3; i++)
	{
		auto& vertexa = triangle[i];
		auto& vertexb = triangle[(i + 1) % 3];
		auto& posa = vertexa.position;
		auto& posb = vertexb.position;
		
		bool aOut = (posa.array() < -1.f).any() || (posa.array() > 1.f).any();
		bool bOut = (posb.array() < -1.f).any() || (posb.array() > 1.f).any();

		anyOut |= aOut || bOut;

		if (aOut || bOut)
		{
			auto intersections = ComputeViewFrustumIntersecions(posa, posb);

			if (!aOut)
			{
				vertices.push_back(vertexa);
			}

			float lineLength = (vertexa.position - vertexb.position).norm();
			for (Vector3f intersection: intersections)
			{
				auto bMultiplier = lineLength > 0.00001 ? (vertexa.position - intersection).norm() / lineLength : 0.5f;
				Vector4b colour = InterpolateColour(vertexa.colour, vertexb.colour, 1.f - bMultiplier, bMultiplier);
				vertices.push_back({ intersection, colour });
			}
		}
		else
		{
			vertices.push_back(vertexa);
		}
	}

	if (anyOut == false)
	{
		return false;
	}

	outTriangles.clear();
	int triangleCount = vertices.size() - 2;
	int vertexPos = 1;
	for (int i = 0; i < triangleCount; i++)
	{	
		outTriangles.push_back({vertices[0], vertices[vertexPos], vertices[vertexPos+1]});
		vertexPos += 1;
	}
	return true;

}

void CPUMeshVisualizer::RenderNode(const std::shared_ptr<MeshObject>& node, const Matrix4f &baseTransform)
{
	Matrix4f transform = node->transformation * baseTransform;
	Matrix4f transformWithProjection = _camera->GetProjectionMatrix() * transform;
	Matrix4f viewportTransformation = _camera->GetViewPortTransformationMatrix();
	std::vector<std::array<Vertex, 3>> clippedTriangles;

	if (node == nullptr)
	{
		return;
	}
	
	for (size_t i = 0; i < node->meshes.size(); i++)
	{
		auto mesh = node->meshes[i];

		for (size_t j = 0; j < mesh.GetTriangleCount(); j++)
		{
			auto triangle = mesh.GetTriangle(j);

			// Projection and object tranformation
			for (uint8_t i = 0; i < 3; i++)
			{
				Vertex& vertex = triangle[i];
				Vector4f projectedPosition = transformWithProjection * vertex.position.homogeneous();
				projectedPosition /= projectedPosition[3];
				vertex.position = projectedPosition.head(3);
			}



			if (Clipping(triangle, clippedTriangles))
			{
				for (auto& triangleCl : clippedTriangles)
				{
					for (uint8_t i = 0; i < 3; i++)
					{
						Vertex& vertex = triangleCl[i];
						Vector4f projectedPosition = viewportTransformation * vertex.position.homogeneous();
						vertex.position = projectedPosition.head(3);
					}

					RasterizeTriangle(triangleCl);
				}
			}
			else
			{
				for (uint8_t i = 0; i < 3; i++)
				{
					Vertex& vertex = triangle[i];
					Vector4f projectedPosition = viewportTransformation * vertex.position.homogeneous();
					vertex.position = projectedPosition.head(3);
				}

				RasterizeTriangle(triangle);
			}
		}
	}

	for (size_t i = 0; i < node->subobjects.size(); i++)
	{
		RenderNode(node->subobjects[i], transform);
	}	
}

CPUMeshVisualizer::CPUMeshVisualizer(const std::shared_ptr<MeshObject>& rootObject, const std::shared_ptr<Camera>& camera): _rootObject(rootObject), _camera(camera)
{
	auto screenSize = _camera->GetScreenSize();
	_framebuffer = std::make_shared<MultiLayeredFramebuffer>(screenSize[1], screenSize[0]);
}

void CPUMeshVisualizer::ComputeFrame()
{
	auto screenSize = _camera->GetScreenSize();
	if (screenSize[1] != _framebuffer->GetWidth() || screenSize[0] != _framebuffer->GetHeight())
	{
		_framebuffer->Resize(screenSize[1], screenSize[0]);
	}
	RenderNode(_rootObject, Matrix4f::Identity());
}

Vector4b CPUMeshVisualizer::InterpolateColour(const Vector4b& color1, const Vector4b& color2, const Vector4b& color3, float alpha, float beta, float gamma)
{
	return (color1.cast<float>() * alpha + color2.cast<float>() * beta + color3.cast<float>() * gamma).cast<uint8_t>();
}

Vector4b CPUMeshVisualizer::InterpolateColour(const Vector4b& color1, const Vector4b& color2, float alpha, float beta)
{
	return (color1.cast<float>() * alpha + color2.cast<float>() * beta).cast<uint8_t>();
}

void ComputeBarycentricCoordinates(const Vector2f& A, const Vector2f& B, const Vector2f& C, const Vector2f& P, float& alpha, float& beta, float& gamma)
{
	float denominator = ((B.y() - C.y()) * (A.x() - C.x())) + ((C.x() - B.x()) * (A.y() - C.y()));
	if (fabs(denominator) < 0.0000001)
	{
		alpha = 1.f/3.f;
		beta = 1.f / 3.f;
		gamma = 1.f / 3.f;
		return;
	}

	alpha = ((B.y() - C.y()) * (P.x() - C.x()) + (C.x() - B.x()) * (P.y() - C.y())) / denominator;
	beta = ((C.y() - A.y()) * (P.x() - C.x()) + (A.x() - C.x()) * (P.y() - C.y())) / denominator;
	gamma = 1.0f - alpha - beta;
}

void CPUMeshVisualizer::RasterizeLine(const Vertex& vertexA, const Vertex& vertexB)
{
	Vector2f A = vertexA.position.head(2).cast<int>().cast<float>();
	Vector2f B = vertexB.position.head(2).cast<int>().cast<float>();
	Vector2f diff = B - A;

	Vector2f pos = A;
	
	float maxLength = std::max(fabs(diff[0]), fabs(diff[1]));

	Vector2f step = diff / maxLength;

	int steps = (int)maxLength;

	float lineLength = diff.norm();

	for (int i = 0; i <= steps; ++i)
	{	
		float bMultiplyier = lineLength > 0.00001 ? (pos - A).norm() / lineLength : 0.5f ;
		Vector4b colour = InterpolateColour(vertexA.colour, vertexB.colour, 1.f - bMultiplyier, bMultiplyier);
		_framebuffer->SetValue(pos[0], pos[1], colour[0], colour[1], colour[2], colour[3], 0);
		pos += step;
	}
}

void CPUMeshVisualizer::RasterizeTriangle(std::array<Vertex, 3>& triangle)
{
	int minY = triangle[0].position[1];
	int minYIndex = 0;
	for (size_t i = 1; i < 3; i++)
	{
		if (minY > triangle[i].position[1])
		{
			minY = triangle[i].position[1];
			minYIndex = i;
		}
	}

	std::rotate(triangle.begin(), triangle.begin() + minYIndex, triangle.end());
	
	if (triangle[1].position[1] < triangle[2].position[1])
	{
		std::swap(triangle[1], triangle[2]);
	}
	

	Vector2f trianglePos1 = triangle[0].position.head(2).cast<int>().cast<float>();
	Vector2f trianglePos2 = triangle[1].position.head(2).cast<int>().cast<float>();
	Vector2f trianglePos3 = triangle[2].position.head(2).cast<int>().cast<float>();

	Vector2f dda1 = trianglePos2 - trianglePos1;
	Vector2f dda2 = trianglePos3 - trianglePos1;
	

	Vector2f ddaPos1 = trianglePos1;
	Vector2f ddaPos2 = trianglePos1;

	int countOfSteps= abs((int)dda2[1]);

	if (dda1[1] != 0.f) dda1 /= fabs(dda1[1]);
	if (dda2[1] != 0.f) dda2 /= fabs(dda2[1]);

	ddaPos1 += dda1;
	ddaPos2 += dda2;
	
	for (size_t i = 0; i < 2; i++)
	{		
		for (size_t y = 1; y <= countOfSteps; y++)
		{
			uint16_t xpos = (int)ddaPos1[0];
			uint16_t ypos = (int)ddaPos1[1];

			int countOfXSteps = abs((int)ddaPos1[0] - (int)ddaPos2[0]);		

			int xPosMover = xpos > ddaPos2[0] ? -1 : 1;
			
			for (size_t x = 0; x < countOfXSteps; x++)
			{
				float alpha;
				float beta;
				float gamma;

				ComputeBarycentricCoordinates(trianglePos1, trianglePos2,trianglePos3, Vector2f(xpos, ypos), alpha, beta, gamma);
				Vector4b colour = InterpolateColour(triangle[0].colour, triangle[1].colour, triangle[2].colour, alpha, beta, gamma);
				_framebuffer->SetValue(xpos, ypos, colour[0], colour[1], colour[2], colour[3], 0);
				xpos += xPosMover;
			}
			
			ddaPos1 += dda1;
			ddaPos2 += dda2;
		}

		dda2 = trianglePos2 - trianglePos3;
		ddaPos2 = trianglePos3;
		countOfSteps = abs((int)dda2[1]);
		if (dda2[1] != 0.f) dda2 /= fabs(dda2[1]);		
	}

	for (size_t i = 0; i < 3; i++)
	{
		RasterizeLine(triangle[i], triangle[(i + 1) % 3]);
	}


	/*
	Vector3f dda1 = triangle[1].position - triangle[0].position;
	Vector3f dda2 = triangle[2].position - triangle[0].position;

	uint16_t countOfPos1Steps = fabs(dda1[1]);
	uint16_t countOfPos2Steps = fabs(dda2[1]);

	uint16_t countOfYSteps = fmax(countOfPos1Steps, countOfPos2Steps) + 1;

	if (dda1[1] != 0.f) dda1 /= fabs(dda1[1]);
	if (dda2[1] != 0.f) dda2 /= fabs(dda2[1]);

	Vector3f pos1 = triangle[0].position;
	Vector3f pos2 = triangle[0].position;
	auto screenSize = _camera->GetScreenSize();

	uint16_t yStep = 0;
	while (yStep < countOfYSteps)
	{
		uint16_t countOfXSteps = abs(pos2[0] - pos1[0]) + 1;

		Vector3f pixelPos = pos1;	

		Vector3f pixelDda = pos2 - pos1;

		if (pixelDda[0] != 0.f) pixelDda /= fabs(pixelDda[0]);

		if (pos1[1] >= screenSize[0]) break;

		if (pos1[1] > 0.f)
		{
			for (uint16_t i = 0; i < countOfXSteps; i++)
			{
				if (pixelPos[0] > 0.f && pixelPos[0] < screenSize[1])
				{
					float alpha;
					float beta;
					float gamma;

					ComputeBarycentricCoordinates(triangle[0].position.head(2), triangle[1].position.head(2), triangle[2].position.head(2), pixelPos.head(2), alpha, beta, gamma);
					Vector4b colour = InterpolateColour(triangle[0].colour, triangle[1].colour, triangle[2].colour, alpha, beta, gamma);
					_framebuffer->SetValue(pixelPos[0], pixelPos[1], colour[0], colour[1], colour[2], colour[3], pixelPos[2]);
					pixelPos += pixelDda;
				}
			}
		}
		
		if (yStep == countOfPos1Steps)
		{
			dda1 = triangle[2].position - triangle[1].position;
			pos1 = triangle[1].position;
			if (dda1[1] != 0.f) dda1 /= fabs(dda1[1]);
		}

		if (yStep == countOfPos2Steps)
		{
			dda2 = triangle[1].position - triangle[2].position;
			pos2 = triangle[2].position;
			if (dda2[1] != 0.f) dda2 /= fabs(dda2[1]);
		}

		pos1 += dda1;
		pos2 += dda2;
		yStep++;
	}*/
}
