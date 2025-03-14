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
#include <NeuroVoxel/Common/Indexing.h>

/// <summary>
/// Class used for storing volumetric data in textures by blocks on the GPU.
/// </summary>
class GPURayCastingVolumeTexture
{
public:
	__host__ GPURayCastingVolumeTexture(cudaTextureObject_t* textures_d, uint8_t* downscales, const Eigen::Vector3i& segmentCount, uint16_t segmentSize, float valueMultiplier);
	__device__ __inline__ uint8_t GetSegmentDownscale(const Eigen::Vector3i& segment);
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

		return tex3D<float>(tex, texPosX+0.5, texPosY + 0.5, texPosZ + 0.5) * _valueMultiplier;
	}

	// Using software interpolation
	__device__ __inline__ float GetTextureCornerValue(Vector3f position)
	{
		Eigen::Vector3i segmentCoords = position.cast<int>() / _segmentSize;

		auto downscale = GetSegmentDownscale(segmentCoords);
		uint16_t downscaleVoxels = 1 << downscale;

		Vector3i pos000 = (position/downscaleVoxels).cast<int>() * downscaleVoxels;
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

	uint8_t* _downscales_d;
	cudaTextureObject_t* _textures_d;
	Eigen::Vector3i _segmentCount;
	uint16_t _segmentSize;
	float _valueMultiplier;
};

/// <summary>
/// Class designed for managing textures on the GPU and automatically loading data into these textures.
/// </summary>
class GPURayCastingVolumeMemory : public GPUEntity
{
public:
	GPURayCastingVolumeMemory(const std::shared_ptr<Camera>&camera, const std::shared_ptr<VolumeLoaderFactory>& volumeLoaderFactory, uint64_t vramLimit = 4000000000);
	
	~GPURayCastingVolumeMemory();

	GPURayCastingVolumeTexture* GetTextureDevicePtr() { return _composedTexture_d; };
	void Update();

	bool MemoryChanged() { bool val = _memoryChanged.load(); _memoryChanged.store(false); return val; };

private:
	template<typename T>
	void CreateTexture(const Eigen::Vector3i& segment, T* data, uint8_t downscale);

	template<typename T>
	float GetValueMultiplicator();
	void DeleteTexture(const Eigen::Vector3i& segment);
	float GetPriority(int xIndex, int yIndex, int zIndex);
	int GetRequiredDownscale(int xIndex, int yIndex, int zIndex);
	// Preloads data 
	template<typename T>
	void Preload();


	template<typename T>
	void StoreLoadedData();

	template<typename T>
	void RevalidateTemplated();

	template<typename T>
	void DownscaleWithHigherQuality(int maxCount);

	std::atomic<bool> _memoryChanged = false;

	BlockBasedDatasetInfo _datasetInfo;
	Eigen::Vector3i _segmentCount;
	uint16_t _segmentSize;

	std::vector<std::shared_ptr<VolumeSegmentRequestTicket>> _tickets;

	cudaArray_t* _arrays_h;
	cudaTextureObject_t* _textures_h;

	cudaArray_t* _arrays_d;
	cudaTextureObject_t* _textures_d;

	uint8_t* _downscales_h;
	uint8_t* _downscales_d;
	
	uint64_t _vramLimit;
	uint64_t _vramUsed = 0;

	uint8_t _preloadLevel = 3;

	GPURayCastingVolumeTexture* _composedTexture_d;
	std::shared_ptr<VolumeLoaderGenericBase> _volumeLoader;
	std::shared_ptr<Camera> _camera;
};

template<typename T>
void GPURayCastingVolumeMemory::CreateTexture(const Eigen::Vector3i& segment, T* data, uint8_t downscale)
{
	int index = segment[0] + segment[1] * _segmentCount[0] + segment[2] * _segmentCount[0] * _segmentCount[1];

	cudaExtent extent = make_cudaExtent(_segmentSize >> downscale, _segmentSize >> downscale, _segmentSize >> downscale);
	cudaChannelFormatDesc channelDesc = cudaCreateChannelDesc<T>();
	cudaMalloc3DArray(&_arrays_h[index], &channelDesc, extent);

	cudaMemcpy3DParms copyParams = { 0 };
	copyParams.srcPtr = make_cudaPitchedPtr(data, (_segmentSize >> downscale) * sizeof(T), _segmentSize >> downscale, _segmentSize >> downscale);
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
	texDesc.readMode = cudaReadModeNormalizedFloat;
	texDesc.normalizedCoords = 0;                   // Set corrds to <0, 1>.

	cudaCreateTextureObject(&_textures_h[index], &resDesc, &texDesc, nullptr);

	uint32_t totalSegments = _segmentCount[0] * _segmentCount[1] * _segmentCount[2];

	_downscales_h[index] = downscale;

	cudaMemcpy(_downscales_d + index, _downscales_h + index, sizeof(uint8_t), cudaMemcpyHostToDevice);
	cudaMemcpy(_arrays_d + index, _arrays_h + index, sizeof(cudaArray_t), cudaMemcpyHostToDevice);
	cudaMemcpy(_textures_d + index, _textures_h + index, sizeof(cudaTextureObject_t), cudaMemcpyHostToDevice);
}

void GPURayCastingVolumeMemory::DeleteTexture(const Eigen::Vector3i& segment)
{
	int index = segment[0] + segment[1] * _segmentCount[0] + segment[2] * _segmentCount[0] * _segmentCount[1];

	if (_textures_h[index]) {
		cudaDestroyTextureObject(_textures_h[index]);
		_textures_h[index] = 0;
	}

	if (_arrays_h[index]) {
		cudaFreeArray(_arrays_h[index]);
		_arrays_h[index] = nullptr;
	}

	// GPU pointers  UPDATE 

	uint8_t zeroDownscale = 0;
	cudaMemcpy(_downscales_d + index, &zeroDownscale, sizeof(uint8_t), cudaMemcpyHostToDevice);

	cudaArray_t nullArray = nullptr;
	cudaMemcpy(_arrays_d + index, &nullArray, sizeof(cudaArray_t), cudaMemcpyHostToDevice);

	cudaTextureObject_t nullTexture = 0;
	cudaMemcpy(_textures_d + index, &nullTexture, sizeof(cudaTextureObject_t), cudaMemcpyHostToDevice);
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

	const auto sizeX = _datasetInfo.sizeX;
	const auto sizeY = _datasetInfo.sizeY;
	const auto sizeZ = _datasetInfo.sizeZ;

	const auto sCountX = sizeX / _segmentSize;
	const auto sCountY = sizeY / _segmentSize;
	const auto sCountZ = sizeZ / _segmentSize;

	int segmentSize = _segmentSize >> _preloadLevel;

	uint32_t voxelsInSegment =  segmentSize *  segmentSize * segmentSize;

	std::vector<T> h_array;
	h_array.resize(voxelsInSegment);

	for (size_t bz = 0; bz < sCountZ; bz++)
	{
		for (size_t by = 0; by < sCountY; by++)
		{
			for (size_t bx = 0; bx < sCountX; bx++)
			{

				auto data = loader->LoadSync(bx, by, bz, _preloadLevel);

				for (size_t i = 0; i < voxelsInSegment; ++i)
				{
					uint16_t z = i / (segmentSize * segmentSize);
					uint16_t y = (i % (segmentSize * segmentSize)) / segmentSize;
					uint16_t x = (i % (segmentSize * segmentSize)) % segmentSize;
					uint32_t mortonIndex = Serialization::GetZCurveIndex(x, y, z);
					h_array[i] = data->data[mortonIndex];
				}

				MemoryContext::GetInstance().memoryInfoWriteMutex.lock();
				MemoryContext::GetInstance().usedMemory -= loader->GetBlockRequiredMemory(_preloadLevel);
				MemoryContext::GetInstance().memoryInfoWriteMutex.unlock();

				CreateTexture<T>(Vector3i(bx, by, bz), h_array.data(), _preloadLevel);
			}
		}
	}
}

template<typename T>
void GPURayCastingVolumeMemory::StoreLoadedData()
{
	auto loader = std::dynamic_pointer_cast<VolumeLoaderBase<T>>(_volumeLoader);

	const auto sizeX = _datasetInfo.sizeX;
	const auto sizeY = _datasetInfo.sizeY;
	const auto sizeZ = _datasetInfo.sizeZ;

	const auto sCountX = sizeX / _segmentSize;
	const auto sCountY = sizeY / _segmentSize;
	const auto sCountZ = sizeZ / _segmentSize;

	int count;
	while (true)
	{
		auto volume = loader->TakeFirstLoaded(count);
		if (count == 0)
		{
			break;
		}
	
		auto index = DataCommon::Indexing::XYZToIdx(Eigen::Vector3i(volume->x, volume->y, volume->z ), Eigen::Vector3i(sCountX, sCountY, sCountZ));

		uint32_t segmentSize = _segmentSize >> volume->downscale;
		uint32_t voxelsInSegment = segmentSize * segmentSize * segmentSize;
		std::vector<T> h_array;
		h_array.resize(voxelsInSegment);
		for (size_t i = 0; i < voxelsInSegment; ++i)
		{
			uint16_t z = i / (segmentSize * segmentSize);
			uint16_t y = (i % (segmentSize * segmentSize)) / segmentSize;
			uint16_t x = (i % (segmentSize * segmentSize)) % segmentSize;
			uint32_t mortonIndex = Serialization::GetZCurveIndex(x, y, z);
			h_array[i] = volume->data[mortonIndex];
		}

		DeleteTexture(Eigen::Vector3i( volume->x, volume->y, volume->z));
		CreateTexture<T>(Eigen::Vector3i(volume->x, volume->y, volume->z), h_array.data(), volume->downscale);

		MemoryContext::GetInstance().memoryInfoWriteMutex.lock();
		MemoryContext::GetInstance().usedMemory -= loader->GetBlockRequiredMemory(volume->downscale);
		MemoryContext::GetInstance().memoryInfoWriteMutex.unlock();

		_memoryChanged.store(true, std::memory_order_release);
		IncrementVersionId();
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

	cudaFree(_downscales_d);
	cudaFree(_arrays_d);
	cudaFree(_textures_d);
	cudaFree(_composedTexture_d);

	delete[] _downscales_h;
	delete[] _arrays_h;
	delete[] _textures_h;
}

__forceinline float GPURayCastingVolumeMemory::GetPriority(int xIndex, int yIndex, int zIndex)
{
	auto vecPos = _camera->DeshrinkVector(Vector3f(xIndex, yIndex, zIndex));
	return (vecPos - _camera->GetShrankPosition()).norm();
}

__forceinline int GPURayCastingVolumeMemory::GetRequiredDownscale(int xIndex, int yIndex, int zIndex)
{
	auto vecPos = _camera->DeshrinkVector(Vector3f(xIndex, yIndex, zIndex));
	auto dist = (vecPos - _camera->GetPosition()).norm();
	float quality = _camera->GetScreenSize().x() / (2 * tanf(_camera->GetViewAngle() * 0.5) * dist) * _camera->GetVoxelSizeMean();
	auto actualDownscale = 0;

	if (quality <= 0.125)
	{
		actualDownscale = 3;
	}
	else if (quality <= 0.25)
	{
		actualDownscale = 2;
	}
	else if (quality <= 0.5)
	{
		actualDownscale = 1;
	}
	return actualDownscale;
}

template <typename T>
void GPURayCastingVolumeMemory::RevalidateTemplated()
{
	auto loader = std::dynamic_pointer_cast<VolumeLoaderBase<T>>(_volumeLoader);
	int segmentSizeShifter = (int)(log2(_segmentSize) + 0.5);

	const auto sizeX = _datasetInfo.sizeX;
	const auto sizeY = _datasetInfo.sizeY;
	const auto sizeZ = _datasetInfo.sizeZ;

	const auto sCountX = sizeX / _segmentSize;
	const auto sCountY = sizeY / _segmentSize;
	const auto sCountZ = sizeZ / _segmentSize;

	bool vramFull = _vramUsed >= _vramLimit - 100000000; //100MB difference

	for (size_t z = 0; z < sCountZ; z++)
	{
		for (size_t y = 0; y < sCountY; y++)
		{
			for (size_t x = 0; x < sCountX; x++)
			{
				uint64_t index = DataCommon::Indexing::XYZToIdx(Eigen::Vector3i(x,y,z), { sCountX, sCountY, sCountZ });
				int requiredDownscale = GetRequiredDownscale((x << segmentSizeShifter) + _segmentSize / 2, (y << segmentSizeShifter) + _segmentSize / 2, (z << segmentSizeShifter) + _segmentSize / 2);

				auto ticket = _tickets[index];


				if (ticket != nullptr && (ticket->state == RequestState::Loaded || _tickets[index]->state == RequestState::UnableToLoad))
				{
					_tickets[index] = nullptr;
					ticket = nullptr;
				}

				if (!vramFull)
				{
					if (ticket == nullptr)
					{
						//used
						if (true && _downscales_h[index] > requiredDownscale)
						{
							float priority = GetPriority((x << segmentSizeShifter) + _segmentSize / 2, (y << segmentSizeShifter) + _segmentSize / 2, (z << segmentSizeShifter) + _segmentSize / 2);

							_tickets[index] = loader->LoadAsync(x, y, z, requiredDownscale, priority);
						}
					}
					else
					{
						float priority = GetPriority((x << segmentSizeShifter) + _segmentSize / 2, (y << segmentSizeShifter) + _segmentSize / 2, (z << segmentSizeShifter) + _segmentSize / 2);

						ticket->mutex.lock();

						ticket->needed = true && _downscales_h[index] > requiredDownscale; // used
						ticket->downscale = requiredDownscale;
						ticket->priority = priority;

						ticket->mutex.unlock();
					}
				}

			}
		}
	}

	int segmentCount = sCountX * sCountY * sCountZ;

	// Downscale N of segments with higher quality
	if (vramFull)
	{
		DownscaleWithHigherQuality<T>(10);
	}
}

template <typename T>
void GPURayCastingVolumeMemory::DownscaleWithHigherQuality(int maxCount)
{
	const auto sizeX = _datasetInfo.sizeX;
	const auto sizeY = _datasetInfo.sizeY;
	const auto sizeZ = _datasetInfo.sizeZ;

	const auto sCountX = sizeX / _segmentSize;
	const auto sCountY = sizeY / _segmentSize;
	const auto sCountZ = sizeZ / _segmentSize;

	int segmentCount = sCountX * sCountY * sCountZ;
	int segmentSizeShifter = (int)(log2(_segmentSize) + 0.5);

	int count = 0;
	for (size_t i = 0; i < segmentCount; i++)
	{
		Eigen::Vector3i XYZ = DataCommon::Indexing::IdxToXYZ(i, Eigen::Vector3i(sCountX, sCountY, sCountZ));
		auto requiredDownscale = GetRequiredDownscale((XYZ.x() << segmentSizeShifter) + _segmentSize / 2, (XYZ.y() << segmentSizeShifter) + _segmentSize / 2, (XYZ.z() << segmentSizeShifter) + _segmentSize / 2);
		// The quality is better than we need 
		if (_downscales_h[i] < requiredDownscale)
		{
			// NOT BEING RELOADED -> only this thread has access to this block

			// Downscale a block to the required quality
			int downscaledSegmentSize = _segmentSize >> requiredDownscale;
			T* downscaledData = new T[downscaledSegmentSize * downscaledSegmentSize * downscaledSegmentSize];
			int actualSegmentSize = _segmentSize >> _downscales_h[i];
			int downscaledVoxelCount = downscaledSegmentSize * downscaledSegmentSize * downscaledSegmentSize;
			int downscale = requiredDownscale - _downscales_h[i];
			int downscaleVoxCount = (size_t)1 << downscale; // 1 << downscale == 2 ^ downscale
			downscaleVoxCount = downscaleVoxCount * downscaleVoxCount * downscaleVoxCount;

			// Downscaling
			// RECOMPUTE VRAM;
			

			_memoryChanged.store(true, std::memory_order_release);
			IncrementVersionId();
		}		
	}
}

void GPURayCastingVolumeMemory::Update()
{
	CALL_TEMPLATED_FUNCTION2(StoreLoadedData, _datasetInfo.dataType);
	CALL_TEMPLATED_FUNCTION2(RevalidateTemplated, _datasetInfo.dataType);
}

GPURayCastingVolumeMemory::GPURayCastingVolumeMemory(const std::shared_ptr<Camera>& camera, const std::shared_ptr<VolumeLoaderFactory>& volumeLoaderFactory, uint64_t vramLimit):
	_datasetInfo(volumeLoaderFactory->GetDatasetInfo()),
	_volumeLoader(volumeLoaderFactory->Create()),
	_vramLimit(vramLimit),
	_camera(camera)
{	

	_segmentSize = _datasetInfo.segmentSize;
	_segmentCount = Vector3i(_datasetInfo.sizeX / _segmentSize, _datasetInfo.sizeY / _segmentSize, _datasetInfo.sizeZ / _segmentSize);

	uint32_t totalSegments = _segmentCount[0] * _segmentCount[1] * _segmentCount[2];	

	_downscales_h = new uint8_t[totalSegments];
	_arrays_h = new cudaArray_t[totalSegments];
	_textures_h = new cudaTextureObject_t[totalSegments];

	_tickets.resize(totalSegments);

	cudaMalloc(&_arrays_d, sizeof(cudaArray_t) * totalSegments);
	cudaMalloc(&_textures_d, sizeof(cudaTextureObject_t) * totalSegments);

	for (size_t i = 0; i < totalSegments; i++)
	{
		_arrays_h[i] = NULL;
		_textures_h[i] = NULL;
	}

	cudaMemcpy(_arrays_d, _arrays_h, sizeof(cudaArray_t) * totalSegments, cudaMemcpyHostToDevice);
	cudaMemcpy(_textures_d, _textures_h, sizeof(cudaTextureObject_t) * totalSegments, cudaMemcpyHostToDevice);

	cudaMalloc(&_downscales_d, sizeof(uint8_t) * totalSegments);


	GPURayCastingVolumeTexture composedTexture(_textures_d, _downscales_d, _segmentCount, _segmentSize, CALL_TEMPLATED_FUNCTION2(GetValueMultiplicator, _datasetInfo.dataType));

	cudaMalloc(&_composedTexture_d, sizeof(GPURayCastingVolumeTexture));
	cudaMemcpy(_composedTexture_d, &composedTexture, sizeof(GPURayCastingVolumeTexture), cudaMemcpyHostToDevice);

	CALL_TEMPLATED_FUNCTION2(Preload, _datasetInfo.dataType);
	_volumeLoader->BindOnSegmentLoaded([this]() { _memoryChanged.store(true); });
}

GPURayCastingVolumeTexture::GPURayCastingVolumeTexture(cudaTextureObject_t* textures_d, uint8_t* downscales, const Eigen::Vector3i& segmentCount, uint16_t segmentSize, float valueMutliplier) :
	_textures_d(textures_d),
	_downscales_d(downscales),
	_segmentCount(segmentCount),
	_segmentSize(segmentSize),
	_valueMultiplier(valueMutliplier)
{}

__device__ __inline__ uint8_t GPURayCastingVolumeTexture::GetSegmentDownscale(const Eigen::Vector3i& segment)
{
	if ((segment.array() >= _segmentCount.array()).any())
	{
		return 0;
	}

	int index = segment[0] + segment[1] * _segmentCount[0] + segment[2] * _segmentCount[0] * _segmentCount[1]; 

	return _downscales_d[index];
}

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

	 auto downscale = GetSegmentDownscale(position.cast<int>() / _segmentSize);
	 auto downscaleVoxels = (1 << downscale);
	 if (texPosX > (_segmentSize - downscaleVoxels) || texPosY > (_segmentSize - downscaleVoxels) || texPosZ > (_segmentSize - downscaleVoxels))
	 {
		 return GetTextureCornerValue(position);
	 }

	 cudaTextureObject_t tex = GetTextureSegment(position.cast<int>() / _segmentSize);
	 
	 texPosX = (texPosX ) / downscaleVoxels;
	 texPosY = (texPosY ) / downscaleVoxels;
	 texPosZ = (texPosZ ) / downscaleVoxels;

	 if (tex == 0)
	 {
		 return 0;
	 }

	 return tex3D<float>(tex, texPosX + 0.5, texPosY + 0.5, texPosZ + 0.5) * _valueMultiplier;
 }

__device__ __inline__ Vector3f GPURayCastingVolumeTexture::GetTextureGrad(const Vector3f& position)
{
	float center = GetTextureValue(position);
	float cx = GetTextureValue(position + Vector3f(1.f, 0.f, 0.f));
	float cy = GetTextureValue(position + Vector3f(0.f, 1.f, 0.f));
	float cz = GetTextureValue(position + Vector3f(0.f, 0.f, 1.f));

	return Vector3f(cx - center, cy - center, cz - center);
}
