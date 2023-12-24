#include "CPUMeshVisualizer.h"

bool CPUMeshVisualizer::ComputeViewFrustumIntersecion(const Vector3f& A, const Vector3f& B, uint8_t plane, int16_t distance, Vector3f& outIntersection)
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

bool CPUMeshVisualizer::NpFpClipping(const std::array<Vertex, 3>& triangle, std::vector<std::array<Vertex, 3>>& outTriangles)
{

	std::vector<std::array<Vertex, 3>> inTriangles;
	inTriangles.push_back(triangle);
	std::array<float, 2> distances = { _camera->GetNearPlaneDistance(), _camera->GetFarPlaneDistance() };
	for (size_t j = 0; j < 2; j++)
	{
		float dist;
		if (j == 0)
		{
			dist = _camera->GetNearPlaneDistance();
		}
		else
		{
			dist = _camera->GetFarPlaneDistance();
		}
		outTriangles.clear();
		for (std::array<Vertex, 3>&triangle : inTriangles)
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

			int triangleCount = vertices.size() - 2;
			int vertexPos = 1;
			for (int i = 0; i < triangleCount; i++)
			{
				outTriangles.push_back({ vertices[0], vertices[vertexPos], vertices[vertexPos + 1] });
				vertexPos += 1;
			}
		}

		inTriangles = outTriangles;

	}

	return true;

}

bool CPUMeshVisualizer::SidesClipping(const std::array<Vertex, 3> &triangle, std::vector<std::array<Vertex, 3>> &outTriangles)
{
	std::vector<std::array<Vertex, 3>> inTriangles;
	inTriangles.push_back(triangle);
	for (int dist = -1; dist < 3; dist +=2)
	{
		for (size_t plane = 0; plane < 2; plane++)
		{
			outTriangles.clear();
			for (std::array<Vertex, 3>& triangle: inTriangles)
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
						aOut = (posa[plane] < -1.f);
						bOut = (posb[plane] < -1.f);
					}
					else
					{
						aOut = (posa[plane] > 1.f);
						bOut = (posb[plane] > 1.f);
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
							Vector4b colour = InterpolateColour(vertexa.colour, vertexb.colour, 1.f - bMultiplier, bMultiplier);
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
					outTriangles.push_back({ vertices[0], vertices[vertexPos], vertices[vertexPos + 1] });
					vertexPos += 1;
				}
			}

			inTriangles = outTriangles;
		}
	}

	return true;
}

void CPUMeshVisualizer::RenderNode(const std::shared_ptr<MeshObject>& node, const Matrix4f &baseTransform)
{
	Matrix4f transform = node->transformation * baseTransform;
	Matrix4f projection = _camera->GetProjectionMatrix();
	Matrix4f transformWithCameraPosition = _camera->GetPositionMatrix() * transform;
	Matrix4f viewportTransformation = _camera->GetViewPortTransformationMatrix();
	std::vector<std::array<Vertex, 3>> clippedTriangles;
	std::vector<std::array<Vertex, 3>> NfFpClippedTriangles;

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

			for (uint8_t i = 0; i < 3; i++)
			{
				Vertex& vertex = triangle[i];
				Vector4f projectedPosition = transformWithCameraPosition * vertex.position.homogeneous();
				vertex.position = projectedPosition.head(3);
			}

			if (!NpFpClipping(triangle, NfFpClippedTriangles))
			{
				NfFpClippedTriangles.push_back(triangle);
			}

			for (auto& triangle: NfFpClippedTriangles)
			{
				for (uint8_t i = 0; i < 3; i++)
				{
					Vertex& vertex = triangle[i];
					Vector4f projectedPosition = projection * vertex.position.homogeneous();
					projectedPosition /= projectedPosition[3];
					vertex.position = projectedPosition.head(3);
				}		
				if (SidesClipping(triangle, clippedTriangles))
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

float CPUMeshVisualizer::InterpolateValue(const float A, const float B, const float C, float alpha, float beta, float gamma)
{
	return A * alpha + B * beta + C * gamma;
}

float CPUMeshVisualizer::InterpolateValue(const float A, const float B, float alpha, float beta)
{
	return A * alpha + B * beta;
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


	Vector2f lastPos = pos;
	for (int i = 0; i <= steps; ++i)
	{	
		float bMultiplyier = lineLength > 0.00001 ? (pos - A).norm() / lineLength : 0.5f ;
		Vector4b colour = InterpolateColour(vertexA.colour, vertexB.colour, 1.f - bMultiplyier, bMultiplyier);
		float depth = InterpolateValue(vertexA.position[2], vertexB.position[2], 1.f - bMultiplyier, bMultiplyier);
		_framebuffer->SetValue(pos[0], pos[1], 255,0,0, colour[3], depth);
		lastPos = pos;
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
	
	for (size_t i = 0; i < 2; i++)
	{		
		for (size_t y = 0; y < countOfSteps; y++)
		{
			uint16_t xpos = (int)ddaPos1[0];
			uint16_t ypos = (int)ddaPos1[1];

			int countOfXSteps = abs((int)ddaPos1[0] - (int)ddaPos2[0]);		

			int xPosMover = xpos > ddaPos2[0] ? -1 : 1;
			
			for (size_t x = 0; x <= countOfXSteps; x++)
			{
				float alpha;
				float beta;
				float gamma;

				ComputeBarycentricCoordinates(trianglePos1, trianglePos2,trianglePos3, Vector2f(xpos, ypos), alpha, beta, gamma);
				Vector4b colour = InterpolateColour(triangle[0].colour, triangle[1].colour, triangle[2].colour, alpha, beta, gamma);
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

	for (size_t i = 0; i < 3; i++)
	{
		RasterizeLine(triangle[i], triangle[(i + 1) % 3]);
	}
}
