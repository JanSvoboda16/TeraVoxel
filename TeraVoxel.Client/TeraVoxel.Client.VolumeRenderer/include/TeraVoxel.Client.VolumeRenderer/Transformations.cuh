/*
 * Author: Jan Svoboda
 * University: BRNO UNIVERSITY OF TECHNOLOGY, FACULTY OF INFORMATION TECHNOLOGY
 */
#pragma once
#include <Eigen/Dense>
#include <device_launch_parameters.h>

using Eigen::Matrix4f;

class Transformations
{
public:
	/// <summary>
	/// Computes a rotation matrix for rotation around an axis.
	/// </summary>
	/// <param name="axis">Axis ('x', 'y', 'z')</param>
	/// <param name="alfa">Angle in rads</param>
	/// <returns>Rotation matrix</returns>
	__host__ __device__ static Matrix4f GetRotationMatrix(char axis, float angle);
	/// <summary>
	/// Computes a shink matrix
	/// </summary>
	/// <param name="x"></param>
	/// <param name="y"></param>
	/// <param name="z"></param>
	/// <returns></returns>
	__host__ __device__ static Matrix4f GetShrinkMatrix(float x, float y, float z);
	/// <summary>
	/// Computes a translation matrix
	/// </summary>
	/// <param name="x"></param>
	/// <param name="y"></param>
	/// <param name="z"></param>
	/// <returns></returns>
	__host__ __device__ static Matrix4f GetTranslationMatrix(float x, float y, float z);

};
