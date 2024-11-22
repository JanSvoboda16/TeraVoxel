#pragma once
#include <cstdint>
#include <cuda_runtime.h>
#include <device_launch_parameters.h>
#include <Eigen/Dense>

template <typename T>
class GPURayCastingVolumeTexture 
{
	
public:
	__host__ GPURayCastingVolumeTexture(const Eigen::Vector3i& segmentCount, uint16_t levels,  uint16_t segmentSize):
		_segmentCount(segmentCount),
		_segmentSize(segmentSize),
		_levels(levels)
	{
		uint32_t totalSegments = segmentCount[0] * segmentCount[1] * segmentCount[2] * levels;

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
	}

	__device__ cudaTextureObject_t GetTexture_D(const Eigen::Vector3i& segment, uint16_t level) 
	{
		if ((segment.array() >= _segmentCount.array()).any())
		{
			return NULL;
		}

		int index = segment[0] + segment[1] * _segmentCount[0] + segment[2] * _segmentCount[0] * _segmentCount[1] + level * _segmentCount[0] * _segmentCount[1] * _segmentCount[2];
		
		return _textures_d[index];
	}

	__host__ void CreateTexture(const Eigen::Vector3i& segment, uint16_t level, T* data) 
	{
		int index = segment[0] + segment[1] * _segmentCount[0] + segment[2] * _segmentCount[0] * _segmentCount[1] + level * _segmentCount[0] * _segmentCount[1] * _segmentCount[2];

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

		uint32_t totalSegments = _segmentCount[0] * _segmentCount[1] * _segmentCount[2] * _levels;

		// TODO
		cudaMemcpy(_arrays_d, _arrays_h, sizeof(cudaArray_t) * totalSegments, cudaMemcpyHostToDevice);
		cudaMemcpy(_textures_d, _textures_h, sizeof(cudaTextureObject_t) * totalSegments, cudaMemcpyHostToDevice);
	}

	__host__ ~GPURayCastingVolumeTexture()
	{
		uint32_t totalSegments = _segmentCount[0] * _segmentCount[1] * _segmentCount[2] * _levels;

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

		delete[] _arrays_h;
		delete[] _textures_h;
	}
	
public:
	Eigen::Vector3i _segmentCount;
	uint16_t _levels, _segmentSize;
	// obsahuje pole polí/textur, samotné textury a pole json na GPU
	cudaArray_t* _arrays_h;
	cudaTextureObject_t* _textures_h;

	cudaArray_t* _arrays_d;
	cudaTextureObject_t* _textures_d;
};