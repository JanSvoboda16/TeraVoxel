#pragma once
#include "MultiLayeredFramebuffer.h"
#include "MeshNode.h"
#include "Camera.h"

using Eigen::Vector2d;
using Eigen::Vector3d;

class CPUMeshVisualizer
{
	std::shared_ptr<MultiLayeredFramebuffer> _framebuffer;
	std::shared_ptr<MeshNode> _rootObject;
	std::shared_ptr<Camera> _camera;
	bool ComputeViewFrustumIntersecion(const Vector3f& A, const Vector3f& B, uint8_t plane, int16_t distance, Vector3f& outIntersection);
	std::vector<std::array<Vertex, 3>> NpFpClipping(const std::array<Vertex, 3>& triangle);
	std::vector<std::array<Vertex, 3>> SidesClipping(const std::array<Vertex, 3>& triangle);
	void RenderNode(const std::shared_ptr<MeshNode>& node, const Matrix4f& baseTransform);
	Vector4b InterpolateColor(const Vector4b& color1, const Vector4b& color2, const Vector4b& color3, float alpha, float beta, float gamma);
	Vector4b InterpolateColor(const Vector4b& color1, const Vector4b& color2, float alpha, float beta);
	float InterpolateValue(const float A, const float B, const float C, float alpha, float beta, float gamma);
	float InterpolateValue(const float A, const float B, float alpha, float beta);

public:
	CPUMeshVisualizer(const std::shared_ptr<MeshNode>& rootObject, const std::shared_ptr<Camera>& camera);
	void ComputeFrame();

	std::shared_ptr<MultiLayeredFramebuffer> GetFrameBuffer() { return _framebuffer; }
	void RasterizeLine(const Vertex& A, const Vertex& B);
	void RasterizeTriangle(std::array<Vertex, 3> &triangle);
};
