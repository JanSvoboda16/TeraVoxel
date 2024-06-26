/*
 * Author: Jan Svoboda
 * University: BRNO UNIVERSITY OF TECHNOLOGY, FACULTY OF INFORMATION TECHNOLOGY
 */
#include "pch.h"
#include "Camera.h"

Camera::Camera(const Vector3f& observerCenter, int observerDistance, const  Vector3f& voxelDimensions, int width, int height, float viewAngle, float nearPlaneDistance, float farPlaneDistance, const std::shared_ptr<MeshNode> orbiterMeshNode)
	: _observerCenterMeshNode(orbiterMeshNode)
{
	_correction = 1 / voxelDimensions.array();
	_voxelDimensions = voxelDimensions;
	_observerCenter = observerCenter;
	_totalObsDistance = observerDistance;
	_position = _observerCenter - Vector3f(0, 0, -observerDistance);
	_screenWidth = width;
	_screenHeight = height;
	_viewAngle = viewAngle;
	_nearPlaneDistance = nearPlaneDistance;
	_farPlaneDistance = farPlaneDistance;

	RecomputeParams();
}

void Camera::ChangePosition(const Vector3f& position)
{
	_position = position;
	RecomputeParams();
}

void Camera::ChangeObserverAxis(char axis, bool rotate)
{
	//y -> default
	//x -> rotate -90 around z
	//z -> rotate 90 around -> 90 around z
	//rotate true -> rotate around z first

	_axisSwapper = rotate ? Transformations::GetRotationMatrix('z', EIGEN_PI) : Transformations::GetRotationMatrix('z', 0);

	if (axis == 'x')
	{
		_axisSwapper = Transformations::GetRotationMatrix('z', -EIGEN_PI / 2) * _axisSwapper;
	}
	else if (axis == 'z')
	{
		_axisSwapper = Transformations::GetRotationMatrix('z', EIGEN_PI / 2) * Transformations::GetRotationMatrix('x', EIGEN_PI / 2) * _axisSwapper;
	}
}

void Camera::Observe(float deltaXAngle, float deltaYAngle, float deltaDistance, float deltaXCenter, float deltaYCenter, float deltaZCenter)
{
	_totalObsYAngle += deltaYAngle;

	auto deltaDistanceLogaritmic = deltaDistance / 500 * _totalObsDistance;
	if (_totalObsDistance + deltaDistanceLogaritmic > 0)
	{
		_totalObsDistance += deltaDistanceLogaritmic;
	}
	if (abs(_totalObsXAngle + deltaXAngle) < EIGEN_PI / 2)
	{
		_totalObsXAngle += deltaXAngle;
	}

	// add rot here
	_rotation = _axisSwapper * Transformations::GetRotationMatrix('y', _totalObsYAngle) * Transformations::GetRotationMatrix('x', _totalObsXAngle);
	Vector4f centerMovement(deltaXCenter * _totalObsDistance / 500, deltaYCenter * _totalObsDistance / 500, deltaZCenter * _totalObsDistance / 500, 1);
	_observerCenter += (_rotation * centerMovement).head<3>();

	_position = Vector3f(0, 0, -_totalObsDistance);
	_position = (_rotation * Vector4f(_position[0], _position[1], _position[2], 1)).head<3>();
	_position += _observerCenter;

	RecomputeParams();
}

void Camera::Rotate(const Vector3f& rotation)
{
	auto x = Transformations::GetRotationMatrix('x', rotation[0]);
	auto y = Transformations::GetRotationMatrix('y', rotation[1]);
	auto z = Transformations::GetRotationMatrix('z', rotation[2]);
	_rotation = x * y * z;
	RecomputeParams();
}

Matrix4f Camera::GetRotationMatrix()
{
	return _rotation;
}

void Camera::SetRotationMatrix(const Matrix4f &matrix)
{;
	_rotation = matrix;
	RecomputeParams();

}

void Camera::ChangeScreenSize(int width, int height)
{
	_screenHeight = height;
	_screenWidth = width;
	RecomputeParams();
}

Vector3f Camera::GetShrankRayDirection(int xPixel, int yPixel)
{
	float x = xPixel - _screenWidth / 2.0;
	float y = yPixel - _screenHeight / 2.0;
	Vector4f vector(x, y, _depth, 1);
	return (_rayShrankRotation * vector).head<3>();
}

Vector3f Camera::GetRayDirection(int xPixel, int yPixel)
{
	float x = xPixel - _screenWidth / 2.0;
	float y = yPixel - _screenHeight / 2.0;
	Vector4f vector(x, y, _depth, 1);
	return (_rotation * vector).head<3>();
}

Matrix4f Camera::GetProjectionMatrix()
{
	return _projectionMatrix;
}

Matrix4f Camera::GetViewPortTransformationMatrix()
{
	return Transformations::GetShrinkMatrix((_screenWidth-1) / 2.f, -(_screenHeight-1) / 2.f, 1) * Transformations::GetTranslationMatrix(1,-1,0);
}

Matrix4f Camera::GetPositionMatrix()
{
	return _positionMatrix;
}

float Camera::GetVoxelSizeMean()
{
	return _voxelDimensions.array().mean();
}


Vector3f Camera::GedDistanceFromProjected(float zValue, int xPixel, int yPixel)
{
	float realZValue = _zValueCoef1 / (_zValueCoef2 - zValue);
	float x = xPixel - _screenWidth / 2.0;
	float y = yPixel - _screenHeight / 2.0;

	float depthRation = 1/_depth * realZValue;

	return (_rotation * Vector4f(x*depthRation, y*depthRation, realZValue,1)).head(3);
}

void Camera::RecomputeParams()
{
	_depth = _screenWidth / (2 * tanf(_viewAngle / 2.0));
	_rayShrankRotation = Transformations::GetShrinkMatrix(_correction[0], _correction[1], _correction[2]) * _rotation;
	_shrankSpacePosition = _position.array() * _correction.array();

	float r = tanf(_viewAngle / 2)*_nearPlaneDistance;
	float t = r * _screenHeight / _screenWidth;	

	// camera is looking into positive x, not negative
	_projectionMatrix << 
		_nearPlaneDistance / r, 0, 0, 0,
		0, -_nearPlaneDistance / t, 0, 0,
		0, 0, (_farPlaneDistance+_nearPlaneDistance) / (_farPlaneDistance- _nearPlaneDistance), -2.f * (_nearPlaneDistance * _farPlaneDistance) / (_farPlaneDistance- _nearPlaneDistance),
		0, 0, 1, 0;

	_zValueCoef1 = (2 * _nearPlaneDistance * _farPlaneDistance) / (_farPlaneDistance - _nearPlaneDistance);
	_zValueCoef2 = (_farPlaneDistance+_nearPlaneDistance) / (_farPlaneDistance - _nearPlaneDistance);

	_positionMatrix = _rotation.inverse() * Transformations::GetTranslationMatrix(-_position[0], -_position[1], -_position[2]);

	if (_observerCenterMeshNode != nullptr)
	{
		_observerCenterMeshNode->transformation = Transformations::GetTranslationMatrix(_observerCenter[0], _observerCenter[1], _observerCenter[2]) ;
	}	
}

void Camera::BindObserverCenterMeshNode(const std::shared_ptr<MeshNode>& meshNode)
{
	_observerCenterMeshNode = meshNode;
}