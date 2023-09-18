/*
 * Author: Jan Svoboda
 * University: BRNO UNIVERSITY OF TECHNOLOGY, FACULTY OF INFORMATION TECHNOLOGY
 */
#include "Transformations.h"

Matrix4f Transformations::GetRotationMatrix(char axis, float angle) 
{
	float sinAngle = sinf(angle);
	float cosAngle = cosf(angle);
	Matrix4f rotation;
	switch (axis)
	{
	case 'x':
		rotation << 1, 0, 0, 0,
			0, cosAngle, -sinAngle, 0,
			0, sinAngle, cosAngle, 0,
			0, 0, 0, 1;
		break;
	case 'y':
		rotation << cosAngle, 0, -sinAngle, 0,
			0, 1, 0, 0,
			sinAngle, 0, cosAngle, 0,
			0, 0, 0, 1;
		break;
	case 'z':
		rotation << cosAngle, -sinAngle, 0, 0,
			sinAngle, cosAngle, 0, 0,
			0, 0, 1, 0,
			0, 0, 0, 1;
		break;
	default:
		break;
	}
	return rotation;
}

Matrix4f Transformations::GetShrinkMatrix(float x, float y, float z) 
{
	Matrix4f matrix;
	matrix << x, 0, 0, 0,
		0, y, 0, 0,
		0, 0, z, 0,
		0, 0, 0, 1;
	return matrix;
}

Matrix4f Transformations::GetTranslationMatrix(float x, float y, float z)
{
	 Matrix4f matrix;
	 matrix << 1, 0, 0, x,
		 0, 1, 0, y,
		 0, 0, 1, z,
		 0, 0, 0, 1;
	 return matrix;
}
