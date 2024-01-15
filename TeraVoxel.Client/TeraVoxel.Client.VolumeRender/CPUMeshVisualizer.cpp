#include "CPUMeshVisualizer.h"

__forceinline bool CPUMeshVisualizer::ComputeViewFrustumIntersecion(const Vector3f& A, const Vector3f& B, uint8_t plane, int16_t distance, Vector3f& outIntersection)
{
	Vector3f direction = (B - A);
	float k = (distance - A[plane]) / direction[plane];
	Vector3f intersection = A + direction * k;
	if (k > 0 && (intersection - A).norm() < direction.norm())
	{
		outIntersection = intersection;
		return true;
	}
	return false;
}

__forceinline std::vector<std::array<Vertex, 3>> CPUMeshVisualizer::NpFpClipping(const std::array<Vertex, 3>& triangle)
{
	std::vector<std::array<Vertex, 3>> outTriangles;
	outTriangles.push_back(triangle);

	auto nearplane = _camera->GetNearPlaneDistance();
	auto farplane = _camera->GetFarPlaneDistance();

	bool needed = false;
	for (size_t i = 0; i < 3; i++)
	{
		needed |= triangle[i].position[2] > farplane;
		needed |= triangle[i].position[2] < nearplane;
	}

	if (!needed)
	{
		return outTriangles;
	}

	for (size_t j = 0; j < 2; j++)
	{
		float dist;
		if (j == 0)
		{
			dist = nearplane;
		}
		else
		{
			dist = farplane;
		}
		
		std::vector<std::array<Vertex, 3>> triangleStash;
		for (std::array<Vertex, 3>&triangle : outTriangles)
		{
			std::vector<Vertex> vertices;
			for (uint8_t i = 0; i < 3; i++)
			{
				auto& vertexa = triangle[i];
				auto& vertexb = triangle[(i + 1) % 3];
				auto& posa = vertexa.position;
				auto& posb = vertexb.position;

				bool aOut;
				bool bOut;
				if (j==0)
				{
					aOut = (posa[2] < dist);
					bOut = (posb[2] < dist);
				}
				else
				{
					aOut = (posa[2] > dist);
					bOut = (posb[2] > dist);
				}
				if (aOut || bOut)
				{
					if (!aOut)
					{
						vertices.push_back(vertexa);
					}

					Vector3f intersection;
					if (ComputeViewFrustumIntersecion(posa, posb, 2, dist, intersection))
					{
						float lineLength = (vertexa.position - vertexb.position).norm();

						auto bMultiplier = lineLength > 0.00001f ? (vertexa.position - intersection).norm() / lineLength : 0.5f;
						Vector4b colour = InterpolateColor(vertexa.colour, vertexb.colour, 1.f - bMultiplier, bMultiplier);
						vertices.push_back({ intersection, colour });
					}
				}
				else
				{
					vertices.push_back(vertexa);
				}

			}

			int triangleCount = vertices.size() - 2;
			int vertexPos = 1;
			for (int i = 0; i < triangleCount; i++)
			{
				triangleStash.push_back({ vertices[0], vertices[vertexPos], vertices[vertexPos + 1] });
				vertexPos += 1;
			}
		}

		outTriangles = std::move(triangleStash);
	}

	return outTriangles;
}

__forceinline std::vector<std::array<Vertex, 3>> CPUMeshVisualizer::SidesClipping(const std::array<Vertex, 3> &triangle)
{
	std::vector<std::array<Vertex, 3>> outTriangles;
	outTriangles.push_back(triangle);

	bool needed = false;
	for (size_t i = 0; i < 3; i++)
	{
		needed |= (triangle[i].position.array() > 1.f).any();
		needed |= (triangle[i].position.array() < -1.f).any();
	}

	if (!needed)
	{
		return outTriangles;
	}

	for (int dist = -1; dist < 3; dist +=2)
	{
		for (size_t plane = 0; plane < 2; plane++)
		{		
			std::vector<std::array<Vertex, 3>> triangleStash;

			for (std::array<Vertex, 3>& triangle: outTriangles)
			{			
				std::vector<Vertex> vertices;
				for (uint8_t i = 0; i < 3; i++)
				{
					auto& vertexa = triangle[i];
					auto& vertexb = triangle[(i + 1) % 3];
					auto& posa = vertexa.position;
					auto& posb = vertexb.position;

					bool aOut;
					bool bOut;
					if (dist == -1)
					{
						aOut = (posa[plane] < dist);
						bOut = (posb[plane] < dist);
					}
					else
					{
						aOut = (posa[plane] > dist);
						bOut = (posb[plane] > dist);
					}
					if (aOut || bOut)
					{
						if (!aOut)
						{
							vertices.push_back(vertexa);
						}

						Vector3f intersection;
						if (ComputeViewFrustumIntersecion(posa, posb, plane, dist, intersection))
						{
							float lineLength = (vertexa.position - vertexb.position).norm();
						
							auto bMultiplier = lineLength > 0.00001 ? (vertexa.position - intersection).norm() / lineLength : 0.5f;
							Vector4b colour = InterpolateColor(vertexa.colour, vertexb.colour, 1.f - bMultiplier, bMultiplier);
							vertices.push_back({ intersection, colour });						
						}
					}
					else
					{
						vertices.push_back(vertexa);
					}
				
				}
				
				int triangleCount = vertices.size() - 2;
				int vertexPos = 1;
				for (int i = 0; i < triangleCount; i++)
				{
					triangleStash.push_back({ vertices[0], vertices[vertexPos], vertices[vertexPos + 1] });
					vertexPos += 1;
				}
			}

			outTriangles = std::move(triangleStash);
		}
	}

	return outTriangles;
}

void CPUMeshVisualizer::RenderNode(const std::shared_ptr<MeshNode>& node, const Matrix4f &baseTransform)
{
	Matrix4f transform = baseTransform * node->transformation;
	Matrix4f projection = _camera->GetProjectionMatrix();
	Matrix4f transformWithCameraPosition = _camera->GetPositionMatrix() * transform;
	Matrix4f viewportTransformation = _camera->GetViewPortTransformationMatrix();
	std::vector<std::array<Vertex, 3>> clippedTriangles;
	std::vector<std::array<Vertex, 3>> NfFpClippedTriangles;

	if (node == nullptr)
	{
		return;
	}
	
	for (auto& mesh: node->meshes)
	{
		
		for (size_t j = 0; j < mesh.GetTriangleCount(); j++)
		{
			auto triangle = mesh.GetTriangle(j);

			for (uint8_t i = 0; i < 3; i++)
			{
				Vertex& vertex = triangle[i];
				Vector4f projectedPosition = transformWithCameraPosition * vertex.position.homogeneous();
				vertex.position = projectedPosition.head(3);
			}
			
			auto NfFpClippedTriangles = NpFpClipping(triangle);

			
			for (auto& triangle: NfFpClippedTriangles)
			{
				{
					for (uint8_t i = 0; i < 3; i++)
					{
						Vertex& vertex = triangle[i];
						Vector4f projectedPosition = projection * vertex.position.homogeneous();
						projectedPosition /= projectedPosition[3];
						vertex.position = projectedPosition.head(3);
					}

					auto clippedTriangles = SidesClipping(triangle);

					for (auto& triangleCl : clippedTriangles)
					{
						for (uint8_t i = 0; i < 3; i++)
						{
							Vertex& vertex = triangleCl[i];
							Vector4f projectedPosition = viewportTransformation * vertex.position.homogeneous();
							vertex.position = projectedPosition.head(3);
						}

						RasterizeTriangle(triangleCl, mesh.OutliningEnabled());
					}
				}
			}
			
		}
	}

	for (auto& subnode: node->subNodes)
	{
		RenderNode(subnode, transform);
	}	
}

CPUMeshVisualizer::CPUMeshVisualizer(const std::shared_ptr<MeshNode>& rootObject, const std::shared_ptr<Camera>& camera): _rootObject(rootObject), _camera(camera)
{
	auto screenSize = _camera->GetScreenSize();
	_framebuffer = std::make_shared<MultiLayeredFramebuffer>(screenSize[0], screenSize[1]);
}

void CPUMeshVisualizer::ComputeFrame()
{
	auto screenSize = _camera->GetScreenSize();

	if (screenSize[0] != _framebuffer->GetWidth() || screenSize[1] != _framebuffer->GetHeight())
	{
		_framebuffer->Resize(screenSize[0], screenSize[1]);
	}
	else
	{
		_framebuffer->Clear();
	}
	
	RenderNode(_rootObject, Matrix4f::Identity());
}

__forceinline Vector4b CPUMeshVisualizer::InterpolateColor(const Vector4b& color1, const Vector4b& color2, const Vector4b& color3, float alpha, float beta, float gamma)
{
	Vector4f color = (color1.cast<float>() * alpha + color2.cast<float>() * beta + color3.cast<float>() * gamma);
	color[0] = std::max(0.0f, std::min(color[0], 255.f));
	color[1] = std::max(0.0f, std::min(color[1], 255.f));
	color[2] = std::max(0.0f, std::min(color[2], 255.f));
	color[3] = std::max(0.0f, std::min(color[3], 255.f));
	return color.cast<uint8_t>();
}

__forceinline Vector4b CPUMeshVisualizer::InterpolateColor(const Vector4b& color1, const Vector4b& color2, float alpha, float beta)
{
	Vector4f color(color1.cast<float>() * alpha + color2.cast<float>() * beta);
	color[0] = std::max(0.0f, std::min(color[0], 255.f));
	color[1] = std::max(0.0f, std::min(color[1], 255.f));
	color[2] = std::max(0.0f, std::min(color[2], 255.f));
	color[3] = std::max(0.0f, std::min(color[3], 255.f));
	return color.cast<uint8_t>();
}

__forceinline float CPUMeshVisualizer::InterpolateValue(const float A, const float B, const float C, float alpha, float beta, float gamma)
{
	return A * alpha + B * beta + C * gamma;
}

__forceinline float CPUMeshVisualizer::InterpolateValue(const float A, const float B, float alpha, float beta)
{
	return A * alpha + B * beta;
}

__forceinline void ComputeBarycentricCoordinates(const Vector2f& A, const Vector2f& B, const Vector2f& C, const Vector2f& P, float& alpha, float& beta, float& gamma)
{
	// Inspired by solutions here: https://gamedev.stackexchange.com/questions/23743/whats-the-most-efficient-way-to-find-barycentric-coordinates
	float denominator = ((B.y() - C.y()) * (A.x() - C.x())) + ((C.x() - B.x()) * (A.y() - C.y()));
	
	// if denominator is small linear interpolation is used
	if (fabs(denominator) < 0.000001)
	{
		float abDist = (A - B).norm();
		float acDist = (A - C).norm();
		
		if (abDist > acDist)
		{
			beta = (A - P).norm() / abDist;
			alpha = (1.f - beta);
			gamma = 0;
		}
		else
		{
			gamma = (A - P).norm() / acDist;
			alpha = (1.f - gamma);
			beta = 0;
		}
		return;
	}

	float denomReverse = 1.0 / denominator;
	alpha = ((B.y() - C.y()) * (P.x() - C.x()) + (C.x() - B.x()) * (P.y() - C.y())) *denomReverse;
	beta = ((C.y() - A.y()) * (P.x() - C.x()) + (A.x() - C.x()) * (P.y() - C.y())) *denomReverse;
	gamma = 1.0f - alpha - beta;
}

__forceinline void CPUMeshVisualizer::RasterizeLine(const Vertex& vertexA, const Vertex& vertexB)
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
		float bMultiplyier = (pos - A).norm() / lineLength;
		Vector4b colour = InterpolateColor(vertexA.colour, vertexB.colour, 1.f - bMultiplyier, bMultiplyier);
		float depth = InterpolateValue(vertexA.position[2], vertexB.position[2], 1.f - bMultiplyier, bMultiplyier);
		_framebuffer->SetValue(pos[0], pos[1],colour[0], colour[1], colour[2], colour[3], depth);
		pos += step;
	}
}

__forceinline void CPUMeshVisualizer::RasterizeTriangle(std::array<Vertex, 3>& triangle, bool outlining)
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
	
	for (size_t i = 0; i < 2; i++)
	{	
		for (size_t y = 0; y < countOfSteps; y++)
		{
			uint16_t xpos = (int)ddaPos1[0];
			uint16_t ypos = (int)ddaPos1[1];

			int countOfXSteps = abs((int)ddaPos1[0] - (int)ddaPos2[0]);		

			int xPosMover = xpos > ddaPos2[0] ? -1 : 1;
			
			// rightest pixel must be empty -> another triangle can be rasterized there
			if (xPosMover < 0)
			{
				xpos += xPosMover;
			}

			for (size_t x = 0; x < countOfXSteps; x++)
			{
				float alpha;
				float beta;
				float gamma;

				ComputeBarycentricCoordinates(trianglePos1, trianglePos2,trianglePos3, Vector2f(xpos, ypos), alpha, beta, gamma);
				Vector4b colour = InterpolateColor(triangle[0].colour, triangle[1].colour, triangle[2].colour, alpha, beta, gamma);
				float depth = InterpolateValue(triangle[0].position[2], triangle[1].position[2], triangle[2].position[2], alpha, beta, gamma);
				_framebuffer->SetValue(xpos, ypos, colour[0], colour[1], colour[2], colour[3], depth);
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

	if (outlining)
	{
		for (size_t i = 0; i < 3; i++)
		{
			RasterizeLine(triangle[i], triangle[(i + 1) % 3]);
		}
	}
}
