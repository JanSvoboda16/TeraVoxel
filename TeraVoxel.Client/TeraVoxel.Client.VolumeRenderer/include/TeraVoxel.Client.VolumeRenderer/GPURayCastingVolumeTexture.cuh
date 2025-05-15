/*
 * Author: Jan Svoboda
 * University: BRNO UNIVERSITY OF TECHNOLOGY, FACULTY OF INFORMATION TECHNOLOGY
 */
#pragma once
#include <fstream>
#include <cuda_runtime.h>
#include <device_launch_parameters.h>
#include <limits>
#include <type_traits>
#include <concepts>
#include "TeraVoxel.Client.VolumeRenderer/VolumeBlock.h"
#include <NeuroVoxel/Common/Indexing.h>

struct TextureBlock
{
	cudaArray_t array;
	cudaTextureObject_t texture;
	uint8_t downscale;
	Eigen::Vector3i coordinates;

	int used = false;
};

/// <summary>
/// Class used for storing volumetric data in textures by blocks on the GPU.
/// </summary>
class GPURayCastingVolumeTexture
{
public:
	__host__ GPURayCastingVolumeTexture(TextureBlock* textures_d, const Eigen::Vector3i& segmentCount, uint16_t segmentSize, float valueMultiplier);

	/// <summary>
	/// Gets value on the given position.
	/// </summary>
	/// <param name="position">coordinates</param>
	/// <returns>value</returns>
	__device__ float GetTextureValue(const Vector3f& position, int& downscale);

	/// <summary>
	/// Computes gradient on the given position.
	/// </summary>
	/// <param name="position">coordinates</param>
	/// <returns>gradietn</returns>
	__device__ Vector3f GetTextureGrad(const Vector3f& position);

	/// <summary>
	/// Gets max value in the texture (used for normalization)
	/// </summary>
	/// <returns>max value</returns>
	__device__ float GetMaxValue() { return _valueMultiplier; }


protected: 

	__device__ __inline__ uint8_t GetSegmentDownscale(const Eigen::Vector3i& segment);
	/// <summary>
	/// Gets block (texture) on coordinates.
	/// </summary>
	/// <param name="segment">coordinates</param>
	/// <returns></returns>
	__device__ cudaTextureObject_t GetTextureSegment(const Eigen::Vector3i& segment);

private:

	// Similar to GetTextureValue. IMPORTANT -> NO RECURSION ON GPU
	__device__ __inline__ float GetTextureValue2(Vector3i position);

	// Using software interpolation
	__device__ __inline__ float GetTextureCornerValue(Vector3f position);


	TextureBlock* _textures_d;
	Eigen::Vector3i _segmentCount;
	uint16_t _segmentSize;
	float _valueMultiplier;
};

__device__ __inline__ uint8_t GPURayCastingVolumeTexture::GetSegmentDownscale(const Eigen::Vector3i& segment)
{
	if ((segment.array() >= _segmentCount.array()).any())
	{
		return 0;
	}

	int index = segment[0] + segment[1] * _segmentCount[0] + segment[2] * _segmentCount[0] * _segmentCount[1];

	return _textures_d[index].downscale;
}

__device__ __inline__ cudaTextureObject_t GPURayCastingVolumeTexture::GetTextureSegment(const Eigen::Vector3i& segment)
{
	if ((segment.array() >= _segmentCount.array()).any())
	{
		return NULL;
	}

	int index = segment[0] + segment[1] * _segmentCount[0] + segment[2] * _segmentCount[0] * _segmentCount[1];

	if (!_textures_d[index].used)
	{
		atomicOr(&_textures_d[index].used, 1);
	}

	return _textures_d[index].texture;
}

__device__ __inline__ float GPURayCastingVolumeTexture::GetTextureValue(const Vector3f& position, int& downscale)
{
	float texPosX = fmodf(position[0], _segmentSize);
	float texPosY = fmodf(position[1], _segmentSize);
	float texPosZ = fmodf(position[2], _segmentSize);

	downscale = GetSegmentDownscale(position.cast<int>() / _segmentSize);
	auto downscaleVoxels = (1 << downscale);
	if (texPosX > (_segmentSize - downscaleVoxels) || texPosY > (_segmentSize - downscaleVoxels) || texPosZ > (_segmentSize - downscaleVoxels))
	{
		return GetTextureCornerValue(position);
	}

	cudaTextureObject_t tex = GetTextureSegment(position.cast<int>() / _segmentSize);

	texPosX = (texPosX) / downscaleVoxels;
	texPosY = (texPosY) / downscaleVoxels;
	texPosZ = (texPosZ) / downscaleVoxels;

	if (tex == 0)
	{
		return 0;
	}

	return tex3D<float>(tex, texPosX + 0.5, texPosY + 0.5, texPosZ + 0.5) * _valueMultiplier;
}

__device__ __inline__ Vector3f GPURayCastingVolumeTexture::GetTextureGrad(const Vector3f& position)
{
	int downscale;
	float center = GetTextureValue(position, downscale);
	float cx = GetTextureValue(position + Vector3f(1.f, 0.f, 0.f), downscale);
	float cy = GetTextureValue(position + Vector3f(0.f, 1.f, 0.f), downscale);
	float cz = GetTextureValue(position + Vector3f(0.f, 0.f, 1.f), downscale);

	return Vector3f(cx - center, cy - center, cz - center);
}

__device__ __inline__ float GPURayCastingVolumeTexture::GetTextureValue2(Vector3i position)
{
	uint8_t downscale = GetSegmentDownscale(position / _segmentSize);
	cudaTextureObject_t tex = GetTextureSegment(position / _segmentSize);

	if (tex == 0)
	{
		return 0;
	}

	float texPosX = position[0] % _segmentSize;
	float texPosY = position[1] % _segmentSize;
	float texPosZ = position[2] % _segmentSize;

	texPosX = (texPosX) / (1 << downscale);
	texPosY = (texPosY) / (1 << downscale);
	texPosZ = (texPosZ) / (1 << downscale);

	return tex3D<float>(tex, texPosX + 0.5, texPosY + 0.5, texPosZ + 0.5) * _valueMultiplier;
}

__device__ __inline__ float GPURayCastingVolumeTexture::GetTextureCornerValue(Vector3f position)
{
	Eigen::Vector3i segmentCoords = position.cast<int>() / _segmentSize;

	auto downscale = GetSegmentDownscale(segmentCoords);
	uint16_t downscaleVoxels = 1 << downscale;

	Vector3i pos000 = (position / downscaleVoxels).cast<int>() * downscaleVoxels;
	Vector3i pos001 = pos000 + Vector3i(0, 0, downscaleVoxels);
	Vector3i pos010 = pos000 + Vector3i(0, downscaleVoxels, 0);
	Vector3i pos011 = pos000 + Vector3i(0, downscaleVoxels, downscaleVoxels);

	Vector3i pos100 = pos000 + Vector3i(downscaleVoxels, 0, 0);
	Vector3i pos101 = pos000 + Vector3i(downscaleVoxels, 0, downscaleVoxels);
	Vector3i pos110 = pos000 + Vector3i(downscaleVoxels, downscaleVoxels, 0);
	Vector3i pos111 = pos000 + Vector3i(downscaleVoxels, downscaleVoxels, downscaleVoxels);

	float val000 = GetTextureValue2(pos000);
	float val001 = GetTextureValue2(pos001);
	float val010 = GetTextureValue2(pos010);
	float val011 = GetTextureValue2(pos011);
	float val100 = GetTextureValue2(pos100);
	float val101 = GetTextureValue2(pos101);
	float val110 = GetTextureValue2(pos110);
	float val111 = GetTextureValue2(pos111);

	position = (position - pos000.cast<float>()) / downscaleVoxels;
	float c00 = val000 * (1.f - position[0]) + val100 * position[0];
	float c01 = val001 * (1.f - position[0]) + val101 * position[0];
	float c10 = val010 * (1.f - position[0]) + val110 * position[0];
	float c11 = val011 * (1.f - position[0]) + val111 * position[0];

	float c0 = c00 * (1.f - position[1]) + c10 * position[1];
	float c1 = c01 * (1.f - position[1]) + c11 * position[1];
	return c0 * (1.f - position[2]) + c1 * position[2];
}

inline GPURayCastingVolumeTexture::GPURayCastingVolumeTexture(TextureBlock* textures_d, const Eigen::Vector3i& segmentCount, uint16_t segmentSize, float valueMutliplier) :
	_textures_d(textures_d),
	_segmentCount(segmentCount),
	_segmentSize(segmentSize),
	_valueMultiplier(valueMutliplier)
{}

