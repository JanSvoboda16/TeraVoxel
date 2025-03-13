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
#include "TeraVoxel.Client.VolumeRenderer/Camera.cuh"
#include "TeraVoxel.Client.VolumeRenderer/VolumeSegment.h"
#include <TeraVoxel.Client.Core/ProjectManager.h>
#include <TeraVoxel.Client.Core/ProjectInfo.h>
#include <TeraVoxel.Client.Core/SettingsContext.h>
#include "TeraVoxel.Client.VolumeRenderer/VolumeLoaderFactory.h"
#include "TeraVoxel.Client.VolumeRenderer/VolumeLoaderBase.h"
#include "TeraVoxel.Client.Core/TemplatedFunctionCaller.h"

/// <summary>
/// Class used for storing volumetric data in textures by blocks on the GPU.
/// </summary>
class GPURayCastingVolumeTexture
{
public:
	__host__ GPURayCastingVolumeTexture(cudaTextureObject_t* textures_d, const Eigen::Vector3i& segmentCount, uint16_t segmentSize, float valueMultiplier);
	/// <summary>
	/// Gets block (texture) on coordinates.
	/// </summary>
	/// <param name="segment">coordinates</param>
	/// <returns></returns>
	__device__ cudaTextureObject_t GetTextureSegment(const Eigen::Vector3i& segment);

	/// <summary>
	/// Gets value on the given position.
	/// </summary>
	/// <param name="position">coordinates</param>
	/// <returns>value</returns>
	__device__ float GetTextureValue(const Vector3f& position);

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


private:

	// Similar to GetTextureValue. IMPORTANT -> NO RECURSION ON GPU
	__device__ __inline__ float GetTextureValue2(Vector3i position) 
	{
		cudaTextureObject_t tex = GetTextureSegment(position / _segmentSize);

		if (tex == 0)
		{
			return 0;
		}

		float texPosX = position[0] % _segmentSize;
		float texPosY = position[1] % _segmentSize;
		float texPosZ = position[2] % _segmentSize;

		return tex3D<float>(tex, texPosX + 0.5f, texPosY + 0.5f, texPosZ + 0.5f);
	}

	// Using software interpolation
	__device__ __inline__ float GetTextureCornerValue(Vector3f position)
	{
		Vector3i pos000 = position.cast<int>();
		Vector3i pos001 = pos000 + Vector3i(0, 0, 1);
		Vector3i pos010 = pos000 + Vector3i(0, 1, 0);
		Vector3i pos011 = pos000 + Vector3i(0, 1, 1);

		Vector3i pos100 = pos000 + Vector3i(1, 0, 0);
		Vector3i pos101 = pos000 + Vector3i(1, 0, 1);
		Vector3i pos110 = pos000 + Vector3i(1, 1, 0);
		Vector3i pos111 = pos000 + Vector3i(1, 1, 1);

		float val000 = GetTextureValue2(pos000);
		float val001 = GetTextureValue2(pos001);
		float val010 = GetTextureValue2(pos010);
		float val011 = GetTextureValue2(pos011);
		float val100 = GetTextureValue2(pos100);
		float val101 = GetTextureValue2(pos101);
		float val110 = GetTextureValue2(pos110);
		float val111 = GetTextureValue2(pos111);

		position = position - pos000.cast<float>();
		float c00 = val000 * (1.f - position[0]) + val100 * position[0];
		float c01 = val001 * (1.f - position[0]) + val101 * position[0];
		float c10 = val010 * (1.f - position[0]) + val110 * position[0];
		float c11 = val011 * (1.f - position[0]) + val111 * position[0];

		float c0 = c00 * (1.f - position[1]) + c10 * position[1];
		float c1 = c01 * (1.f - position[1]) + c11 * position[1];
		return c0 * (1.f - position[2]) + c1 * position[2];
	}

	cudaTextureObject_t* _textures_d;
	Eigen::Vector3i _segmentCount;
	uint16_t _segmentSize;
	float _valueMultiplier;
};

/// <summary>
/// Class designed for managing textures on the GPU and automatically loading data into these textures.
/// </summary>
class GPURayCastingVolumeMemory 
{
public:
	GPURayCastingVolumeMemory(const std::shared_ptr<Camera>&camera, const std::shared_ptr<VolumeLoaderFactory>& volumeLoaderFactory);
	~GPURayCastingVolumeMemory();
	GPURayCastingVolumeTexture* GetTextureDevicePtr() { return _composedTexture_d; };

private:
	template<typename T>
	void CreateTexture(const Eigen::Vector3i& segment, T* data);

	template<typename T>
	float GetValueMultiplicator();

	// Preloads data 
	template<typename T>
	void Preload();

	BlockBasedDatasetInfo _datasetInfo;
	Eigen::Vector3i _segmentCount;
	uint16_t _segmentSize;

	cudaArray_t* _arrays_h;
	cudaTextureObject_t* _textures_h;

	cudaArray_t* _arrays_d;
	cudaTextureObject_t* _textures_d;

	GPURayCastingVolumeTexture* _composedTexture_d;

	std::shared_ptr<VolumeLoaderGenericBase> _volumeLoader;
};

template<typename T>
void GPURayCastingVolumeMemory::CreateTexture(const Eigen::Vector3i& segment, T* data)
{
	int index = segment[0] + segment[1] * _segmentCount[0] + segment[2] * _segmentCount[0] * _segmentCount[1];

	cudaExtent extent = make_cudaExtent(_segmentSize, _segmentSize, _segmentSize);
	cudaChannelFormatDesc channelDesc = cudaCreateChannelDesc<T>();
	cudaMalloc3DArray(&_arrays_h[index], &channelDesc, extent);

	cudaMemcpy3DParms copyParams = { 0 };
	copyParams.srcPtr = make_cudaPitchedPtr(data, _segmentSize * sizeof(T), _segmentSize, _segmentSize);
	copyParams.dstArray = _arrays_h[index];
	copyParams.extent = extent;
	copyParams.kind = cudaMemcpyHostToDevice;

	cudaMemcpy3D(&copyParams);

	cudaResourceDesc resDesc = {};
	resDesc.resType = cudaResourceTypeArray;
	resDesc.res.array.array = _arrays_h[index];

	cudaTextureDesc texDesc = {};
	texDesc.addressMode[0] = cudaAddressModeClamp; // Wrap, Clamp, Border
	texDesc.addressMode[1] = cudaAddressModeClamp;
	texDesc.addressMode[2] = cudaAddressModeClamp;
	texDesc.filterMode = cudaFilterModeLinear;      // Linear nebo Point
	texDesc.readMode = cudaReadModeElementType;
	texDesc.normalizedCoords = 0;                   // Použít norm. souřadnice (0 až 1).

	cudaCreateTextureObject(&_textures_h[index], &resDesc, &texDesc, nullptr);

	uint32_t totalSegments = _segmentCount[0] * _segmentCount[1] * _segmentCount[2];

	// TODO
	cudaMemcpy(_arrays_d, _arrays_h, sizeof(cudaArray_t) * totalSegments, cudaMemcpyHostToDevice);
	cudaMemcpy(_textures_d, _textures_h, sizeof(cudaTextureObject_t) * totalSegments, cudaMemcpyHostToDevice);
}

template<typename T>
float GPURayCastingVolumeMemory::GetValueMultiplicator()
{
	if constexpr (std::is_floating_point_v<T>) 
	{
		return 1.f;
	}
	
	return std::numeric_limits<T>::max();
}

template<typename T>
void GPURayCastingVolumeMemory::Preload()
{
	auto loader = std::dynamic_pointer_cast<VolumeLoaderBase<T>>(_volumeLoader);
	const auto segmentSize = _datasetInfo.segmentSize;

	const auto sizeX = _datasetInfo.sizeX;
	const auto sizeY = _datasetInfo.sizeY;
	const auto sizeZ = _datasetInfo.sizeZ;

	const auto sCountX = sizeX / segmentSize;
	const auto sCountY = sizeY / segmentSize;
	const auto sCountZ = sizeZ / segmentSize;

	uint32_t voxelsInSegment = segmentSize * segmentSize * segmentSize;

	std::vector<float> h_array;
	h_array.resize(voxelsInSegment);

	for (size_t bz = 0; bz < sCountZ; bz++)
	{
		for (size_t by = 0; by < sCountY; by++)
		{
			for (size_t bx = 0; bx < sCountX; bx++)
			{

				auto data = loader->LoadSync(bx, by, bz, 0);

				for (size_t i = 0; i < voxelsInSegment; ++i)
				{
					uint16_t z = i / (segmentSize * segmentSize);
					uint16_t y = (i % (segmentSize * segmentSize)) / segmentSize;
					uint16_t x = (i % (segmentSize * segmentSize)) % segmentSize;
					uint32_t mortonIndex = Serialization::GetZCurveIndex(x, y, z);
					h_array[i] = data->data[mortonIndex];
				}


				MemoryContext::GetInstance().memoryInfoWriteMutex.lock();
				MemoryContext::GetInstance().usedMemory -= loader->GetBlockRequiredMemory(0);
				MemoryContext::GetInstance().memoryInfoWriteMutex.unlock();

				CreateTexture<float>(Vector3i(bx, by, bz), h_array.data());
			}
		}
	}
}

GPURayCastingVolumeMemory::~GPURayCastingVolumeMemory()
{
	uint32_t totalSegments = _segmentCount[0] * _segmentCount[1] * _segmentCount[2];

	for (size_t i = 0; i < totalSegments; i++)
	{
		if (_arrays_h[i] != NULL)
		{
			cudaFreeArray(_arrays_h[i]);
			cudaDestroyTextureObject(_textures_h[i]);
		}
	}

	cudaFree(_arrays_d);
	cudaFree(_textures_d);
	cudaFree(_composedTexture_d);

	delete[] _arrays_h;
	delete[] _textures_h;
}

GPURayCastingVolumeMemory::GPURayCastingVolumeMemory(const std::shared_ptr<Camera>& camera, const std::shared_ptr<VolumeLoaderFactory>& volumeLoaderFactory):
	_datasetInfo(volumeLoaderFactory->GetDatasetInfo()),
	_volumeLoader(volumeLoaderFactory->Create())
{	

	_segmentSize = _datasetInfo.segmentSize;
	_segmentCount = Vector3i(_datasetInfo.sizeX / _segmentSize, _datasetInfo.sizeY / _segmentSize, _datasetInfo.sizeZ / _segmentSize);

	uint32_t totalSegments = _segmentCount[0] * _segmentCount[1] * _segmentCount[2];	

	_arrays_h = new cudaArray_t[totalSegments];
	_textures_h = new cudaTextureObject_t[totalSegments];

	cudaMalloc(&_arrays_d, sizeof(cudaArray_t) * totalSegments);
	cudaMalloc(&_textures_d, sizeof(cudaTextureObject_t) * totalSegments);

	for (size_t i = 0; i < totalSegments; i++)
	{
		_arrays_h[i] = NULL;
		_textures_h[i] = NULL;
	}

	cudaMemcpy(_arrays_d, _arrays_h, sizeof(cudaArray_t) * totalSegments, cudaMemcpyHostToDevice);
	cudaMemcpy(_textures_d, _textures_h, sizeof(cudaTextureObject_t) * totalSegments, cudaMemcpyHostToDevice);


	GPURayCastingVolumeTexture composedTexture(_textures_d, _segmentCount, _segmentSize, CALL_TEMPLATED_FUNCTION(GetValueMultiplicator, _datasetInfo.dataType.c_str()));

	cudaMalloc(&_composedTexture_d, sizeof(GPURayCastingVolumeTexture));
	cudaMemcpy(_composedTexture_d, &composedTexture, sizeof(GPURayCastingVolumeTexture), cudaMemcpyHostToDevice);

	CALL_TEMPLATED_FUNCTION(Preload, _datasetInfo.dataType.c_str());
}

GPURayCastingVolumeTexture::GPURayCastingVolumeTexture(cudaTextureObject_t* textures_d, const Eigen::Vector3i& segmentCount, uint16_t segmentSize, float valueMutliplier) :
	_textures_d(textures_d),
	_segmentCount(segmentCount),
	_segmentSize(segmentSize),
	_valueMultiplier(valueMutliplier)
{}

 __device__ __inline__ cudaTextureObject_t GPURayCastingVolumeTexture::GetTextureSegment(const Eigen::Vector3i& segment)
{
	if ((segment.array() >= _segmentCount.array()).any())
	{
		return NULL;
	}

	int index = segment[0] + segment[1] * _segmentCount[0] + segment[2] * _segmentCount[0] * _segmentCount[1];

	return _textures_d[index];
}

__device__ __inline__ float GPURayCastingVolumeTexture::GetTextureValue(const Vector3f& position)
{
	 float texPosX = fmodf(position[0], _segmentSize);
	 float texPosY = fmodf(position[1], _segmentSize);
	 float texPosZ = fmodf(position[2], _segmentSize);

	 if (texPosX > (_segmentSize - 1) || texPosY > (_segmentSize - 1) || texPosZ > (_segmentSize - 1))
	 {
		 return GetTextureCornerValue(position);
	 }

	 cudaTextureObject_t tex = GetTextureSegment(position.cast<int>() / _segmentSize);
	 if (tex == 0)
	 {
		 return 0;
	 }

	 return tex3D<float>(tex, texPosX + 0.5f, texPosY + 0.5f, texPosZ + 0.5f);
 }

__device__ __inline__ Vector3f GPURayCastingVolumeTexture::GetTextureGrad(const Vector3f& position)
{
	float center = GetTextureValue(position);
	float cx = GetTextureValue(position + Vector3f(1.f, 0.f, 0.f));
	float cy = GetTextureValue(position + Vector3f(0.f, 1.f, 0.f));
	float cz = GetTextureValue(position + Vector3f(0.f, 0.f, 1.f));

	return Vector3f(cx - center, cy - center, cz - center);
}
