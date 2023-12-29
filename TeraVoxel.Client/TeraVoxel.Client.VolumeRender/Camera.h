/*
 * Author: Jan Svoboda
 * University: BRNO UNIVERSITY OF TECHNOLOGY, FACULTY OF INFORMATION TECHNOLOGY
 */
#pragma once
#include <vector>
#include <Eigen/Dense>
#include <algorithm>
#include <string>
#include "Transformations.h"
#include "../TeraVoxel.Client.Core/Logger.h"
#include "MeshNode.h"

using Eigen::Vector2i;
using Eigen::Vector2f;
using Eigen::Vector3f;
using Eigen::Vector4f;
using Eigen::Vector3i;
using Eigen::Vector3d;
using Eigen::Matrix4f;

class Camera
{
public:
	Camera(const Vector3f& observerCenter, int observerDistance, const Vector3f& voxelDimensions, int width, int height, float viewAngle, float nearPlaneDistance = 20, float farPlaneDiscance = 10000, std::shared_ptr<MeshNode> orbiterMeshNode = nullptr);
	
	/// <summary>
	/// Changes the position of the camera
	/// </summary>
	/// <param name="position">positon</param>
	void ChangePosition(const Vector3f& position);	

	/// <summary>
	/// Changes an axis of the rotation
	/// </summary>
	/// <param name="axis">'x' or 'y' or 'z'</param>
	/// <param name="rotate">changes rotation direction</param>
	void ChangeObserverAxis(char axis, bool rotate);

	/// <summary>
	/// Move the observer depend on parameters
	/// </summary>
	/// <param name="deltaXAngle">x angle change (rad)</param>
	/// <param name="deltaYAngle">y angle change (rad)</param>
	/// <param name="deltaDistance">changes the distance from the center (does not correspond to the actual length)</param>
	/// <param name="deltaXCenter">changes the distance from the center (does not correspond to the actual length)</param>
	/// <param name="deltaYCenter">changes the distance from the center (does not correspond to the actual length)</param>
	/// <param name="deltaZCenter">changes the distance from the center (does not correspond to the actual length)</param>
	void Observe(float deltaXAngle, float deltaYAngle, float deltaDistance, float deltaXCenter, float deltaYCenter, float deltaZCenter);
	
	/// <summary>
	/// Rotates the camera
	/// </summary>
	/// <param name="rotation">rotation matrix</param>
	void Rotate(const Vector3f& rotation);

	/// <summary>
	/// Returns ray direction for the pixel in a shrank space.
	/// </summary>
	/// <param name="xPixel">x pixel position</param>
	/// <param name="yPixel">y pixel position</param>
	/// <returns>ray direction</returns>
	Vector3f GetShrankRayDirection(int xPixel, int yPixel);

	/// <summary>
	/// Returns ray direction for the pixel space.
	/// </summary>
	/// <param name="xPixel">x pixel position</param>
	/// <param name="yPixel">y pixel position</param>
	/// <returns>ray direction</returns>
	Vector3f GetRayDirection(int xPixel, int yPixel);

	/// <summary>
	/// Returns the projection matrix for the current camera position and rotation.
	/// </summary>
	/// <returns>Projection matrix</returns>
	Matrix4f GetProjectionMatrix();

	/// <summary>
	/// Returns the viewPort matrix based on screen sizes. 
	/// </summary>
	/// <returns>view port matrix</returns>
	Matrix4f GetViewPortTransformationMatrix();

	/// <summary>
	/// Returns the matrix which transforms objects before projection can be aplied.
	/// </summary>
	/// <returns></returns>
	Matrix4f GetPositionMatrix();
	
	/// <summary>
	/// Returns max size of the voxel dimensions
	/// </summary>
	/// <returns>max dimension</returns>
	float GetVoxelSizeMean();

	/// <summary>
	/// Returns the camera position in a shrank space
	/// </summary>
	/// <returns></returns>
	Vector3f GetShrankPosition() { return _shrankSpacePosition; }
	
	/// <summary>
	/// Shrinks a vector
	/// </summary>
	/// <param name="vector">Vector</param>
	/// <returns>shrank vector</returns>
	Vector3f ShrinkVector(Vector3f vector) { return vector.array() * _correction.array(); }

	/// <summary>
	/// Deshrink a vector
	/// </summary>
	/// <param name="vector">Vector</param>
	/// <returns>Deshrank vector</returns>
	Vector3f DeshrinkVector(Vector3f vector) { return vector.array() * _voxelDimensions.array(); }

	/// <summary>
	/// Changes size of the screen in pixels
	/// </summary>
	/// <param name="width">width in pixels</param>
	/// <param name="height">height in pixels</param>
	void ChangeScreenSize(int width, int height);

	/// <summary>
	/// Gets size of the screen in pixels
	/// </summary>
	/// <returns>size of the screen</returns>
	Vector2i GetScreenSize() { return Vector2i(_screenWidth, _screenHeight); }

	/// <summary>
	/// Gets the camera view angle
	/// </summary>
	/// <returns></returns>
	float GetViewAngle() { return _viewAngle; }

	/// <summary>
	/// Sets the camera view angle
	/// </summary>
	/// <param name="viewAngle"></param>
	void SetViewAngle(float viewAngle)	{ _viewAngle = viewAngle; RecomputeParams(); }

	/// <summary>
	/// Returns real distance from the camera based on zValue and screen coordinates.
	/// </summary>
	/// <param name="zValue">zValue</param>
	/// <param name="xPixel">x coordinate</param>
	/// <param name="yPixel">y coordinate</param>
	/// <returns></returns>
	Vector3f GedDistanceFromProjected(float zValue, int xPixel, int yPixel);

	/// <summary>
	/// Gets the camera position
	/// </summary>
	/// <returns>position</returns>
	Vector3f GetPosition() { return _position; }

	/// <summary>
	/// Sets the farplane distance
	/// </summary>
	/// <param name="farPlaneDistance"></param>
	void SetFarPlaneDistance(float farPlaneDistance) { _farPlaneDistance = farPlaneDistance; RecomputeParams(); }

	/// <summary>
	/// Sets the nearplane distance
	/// </summary>
	/// <param name="nearPlaneDistance"></param>
	void SetNearPlaneDistance(float nearPlaneDistance) { _nearPlaneDistance = nearPlaneDistance; RecomputeParams(); }

	/// <summary>
	/// Gets the farplane distance
	/// </summary>
	/// <returns>distance</returns>
	float GetFarPlaneDistance() { return _farPlaneDistance; }

	/// <summary>
	/// Gets the nearplane distance
	/// </summary>
	/// <returns>distance</returns>
	float GetNearPlaneDistance() { return _nearPlaneDistance; }

	/// <summary>
	/// Bind observer center mesh to the camera so
	/// it always will be displayed in the center of the camera observer.
	/// </summary>
	/// <param name="meshNode"></param>
	void BindObserverCenterMeshNode(const std::shared_ptr<MeshNode>& meshNode);

private:
	float _totalObsXAngle = 0, _totalObsYAngle = 0;			// Observer angles
	float _totalObsDistance = 0;							// Distance from the center of rotation to the observer camera
	Vector3f _position = Vector3f(0, 0, 0);					// Camera position
	Vector3f _observerCenter = Vector3f(0, 0, 0); // Center of rotation for the observer
	Matrix4f _rotation = Transformations::GetRotationMatrix('x', 0); // Camera rotation
	Matrix4f _axisSwapper = Transformations::GetRotationMatrix('x', 0);
	Vector3f _shrankSpacePosition;							// Camera position in shrank space
	Vector3f _correction;									// 1 / _voxelDimensions
	Vector3f _voxelDimensions;								// Dimensions of voxels
	Matrix4f _rayShrankRotation;							// The transformation which is applied on each ray. (rotation && shrink)
	Matrix4f _projectionMatrix;								// Projection matrix
	Matrix4f _positionMatrix;

	std::shared_ptr<MeshNode> _observerCenterMeshNode;

	int _screenWidth, _screenHeight;	// screen sizes
	float _viewAngle;					// View angle in RADS
	float _depth;						// Size between camera center and view plane when pixel size is 1
	float _nearPlaneDistance;			// Near plane distance
	float _farPlaneDistance;			// Far plane distance
	
	float _zValueCoef1, _zValueCoef2;

	// Recomputes precomputed parameters
	void RecomputeParams();
};

