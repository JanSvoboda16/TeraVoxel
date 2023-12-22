#pragma once
#include "MultiLayeredFramebuffer.h"
#include "MeshObject.h"
#include "Camera.h"

using Eigen::Vector2d;
using Eigen::Vector3d;

class CPUMeshVisualizer
{
	std::shared_ptr<MultiLayeredFramebuffer> _framebuffer;
	std::shared_ptr<MeshObject> _rootObject;
	std::shared_ptr<Camera> _camera;
	bool Clipping(const std::array<Vertex, 3>& triangle, std::vector<std::array<Vertex, 3>>& outTriangles);
	void RenderNode(const std::shared_ptr<MeshObject>& node, const Matrix4f& baseTransform);
	std::vector<Vector3f> ComputeViewFrustumIntersecions(const Vector3f& A, const Vector3f& B);
	Vector4b InterpolateColour(const Vector4b& color1, const Vector4b& color2, const Vector4b& color3, float alpha, float beta, float gamma);
	Vector4b InterpolateColour(const Vector4b& color1, const Vector4b& color2, float alpha, float beta);
public:
	CPUMeshVisualizer(const std::shared_ptr<MeshObject>& rootObject, const std::shared_ptr<Camera>& camera);
	void ComputeFrame();

	std::shared_ptr<MultiLayeredFramebuffer> GetFrameBuffer() { return _framebuffer; }
	void RasterizeLine(const Vertex& A, const Vertex& B);
	void RasterizeTriangle(std::array<Vertex, 3> &triangle);
};

