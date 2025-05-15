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
#include "TeraVoxel.Client.VolumeRenderer/VolumeBlock.h"
#include <TeraVoxel.Client.Core/ProjectManager.h>
#include <TeraVoxel.Client.Core/ProjectInfo.h>
#include <TeraVoxel.Client.Core/SettingsContext.h>
#include "TeraVoxel.Client.VolumeRenderer/VolumeLoaderFactory.h"
#include "TeraVoxel.Client.VolumeRenderer/VolumeLoaderGenericBase.h"
#include "TeraVoxel.Client.VolumeRenderer/GPUEntity.h"
#include <NeuroVoxel/Common/Indexing.h>
#include "GPURayCastingVolumeTexture.cuh"

/// <summary>
/// Class designed for managing textures on the GPU and automatically loading data into these textures.
/// </summary>
class GPURayCastingVolumeMemory : public GPUEntity
{
public:

	/// <summary>
	/// Constructor
	/// </summary>
	/// <param name="camera">Scene camera</param>
	/// <param name="volumeLoaderFactory">VolumeLaoder</param>
	GPURayCastingVolumeMemory(const std::shared_ptr<Camera>&camera, const std::shared_ptr<VolumeLoaderFactory>& volumeLoaderFactory);
	
	~GPURayCastingVolumeMemory();

	/// <summary>
	/// Returns pointer to texture on GPU. 
	/// </summary>
	GPURayCastingVolumeTexture* GetTextureDevicePtr() { return _composedTexture_d; };

	/// <summary>
	/// Returns true if any data changed. Second way is to use VersionId().
	/// </summary>
	bool MemoryChanged() { bool val = _memoryChanged.load(); _memoryChanged.store(false); return val; };

	/// <summary>
	/// Prepares textures to be rendered
	/// </summary>
	void Prepare();

	/// <summary>
	/// Revalidates textures based on the desired objectQuality.
	/// If some textures are not of the desired quality, it asks the loader to load it. 
	/// </summary>
	/// <param name="objectQuality">Higher numver means lower quality</param>
	void Revalidate(float objectQuality);

	/// <summary>
	/// Cleans usage flags.
	/// </summary>
	void CleanUsage();

private:
	template<typename T>
	TextureBlock CreateTexture(const Eigen::Vector3i& segment, T* data, uint8_t downscale, bool& success);

	template<typename T>
	float GetValueMultiplicator();
	void DeleteTexture(TextureBlock texture);
	float GetPriority(int xIndex, int yIndex, int zIndex);
	int GetRequiredDownscale(int xIndex, int yIndex, int zIndex, float qualityDivider);
	// Preloads data 
	template<typename T>
	void Preload(int threadCount);


	template<typename T>
	bool OnDataLoaded();

	template<typename T>
	void RevalidateTemplated(float objectQquality);

	template<typename T>
	void DownscaleWithHigherQuality(int maxCount, float objectQuality);

	template <typename T>
	TextureBlock CreateDownscaledTexture(TextureBlock texture_d_orig);

	std::atomic<bool> _memoryChanged = false;

	BlockBasedDatasetInfo _datasetInfo;
	Eigen::Vector3i _segmentCount;
	uint16_t _segmentSize;

	std::vector<std::shared_ptr<VolumeBlockRequestTicket>> _tickets;

	std::stack<TextureBlock> _loadedTextures;

	std::mutex _loadedTexturesMutex;

	std::vector<TextureBlock> _textures_h;

	TextureBlock* _textures_d;

	uint8_t _preloadLevel = 3;

	std::mutex _gpuMemoryLock;

	GPURayCastingVolumeTexture* _composedTexture_d;
	VolumeLoaderBase* _volumeLoader; // unique_ptr sets nullptr and then call destuctor
	std::shared_ptr<Camera> _camera;

	float _valueMultiplier;
};

template<typename T>
TextureBlock GPURayCastingVolumeMemory::CreateTexture(const Eigen::Vector3i& segment, T* data, uint8_t downscale, bool& success)
{
	if ((!Common::Data::SupportsNormalizedFloat<T>()) && (!std::is_floating_point<T>::value))
	{
		uint64_t voxelCount = uint64_t(_segmentSize >> downscale) * uint64_t(_segmentSize >> downscale) * uint64_t(_segmentSize >> downscale);
		std::vector<float> floatData;
		floatData.reserve(voxelCount);
		for (size_t i = 0; i < voxelCount; i++)
		{
			floatData.push_back(float(data[i]) / std::numeric_limits<T>::max());
		}

		return CreateTexture<float>(segment, floatData.data(), downscale, success);
	}

	int index = segment[0] + segment[1] * _segmentCount[0] + segment[2] * _segmentCount[0] * _segmentCount[1];

	TextureBlock handler;

	cudaStream_t stream;
	cudaStreamCreate(&stream);

	cudaExtent extent = make_cudaExtent(_segmentSize >> downscale, _segmentSize >> downscale, _segmentSize >> downscale);
	cudaChannelFormatDesc channelDesc = cudaCreateChannelDesc<T>();

	success = false;

	_gpuMemoryLock.lock();
	size_t freeMemory, totalMemory;
	cudaMemGetInfo(&freeMemory, &totalMemory);
	
	if (freeMemory > 2000000000) 
	{		
		cudaMalloc3DArray(&handler.array, &channelDesc, extent);
		success = true;
	}

	_gpuMemoryLock.unlock();

	if (!success) {
		
		cudaStreamSynchronize(stream);
		cudaStreamDestroy(stream);
		return handler;
	}

	cudaMemcpy3DParms copyParams = { 0 };
	copyParams.srcPtr = make_cudaPitchedPtr(data, (_segmentSize >> downscale) * sizeof(T), _segmentSize >> downscale, _segmentSize >> downscale);
	copyParams.dstArray = handler.array;
	copyParams.extent = extent;
	copyParams.kind = cudaMemcpyHostToDevice;

	cudaMemcpy3DAsync(&copyParams, stream);

	cudaResourceDesc resDesc = {};
	resDesc.resType = cudaResourceTypeArray;
	resDesc.res.array.array = handler.array;

	cudaTextureDesc texDesc = {};
	texDesc.addressMode[0] = cudaAddressModeClamp; // Wrap, Clamp, Border
	texDesc.addressMode[1] = cudaAddressModeClamp;
	texDesc.addressMode[2] = cudaAddressModeClamp;
	texDesc.filterMode = cudaFilterModeLinear;      // Linear nebo Point
	texDesc.readMode = Common::Data::SupportsNormalizedFloat<T>() ? cudaReadModeNormalizedFloat : cudaReadModeElementType;
	texDesc.normalizedCoords = 0;                   // Set corrds to <0, 1>.

	cudaCreateTextureObject(&handler.texture, &resDesc, &texDesc, nullptr);

	cudaStreamSynchronize(stream);
	cudaStreamDestroy(stream);

	handler.downscale = downscale;
	handler.coordinates = segment;

	return handler;
}

inline void GPURayCastingVolumeMemory::DeleteTexture(TextureBlock texture)
{
	cudaDestroyTextureObject(texture.texture);
	cudaFreeArray(texture.array);
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
void GPURayCastingVolumeMemory::Preload(int threadCount)
{
	auto loader = dynamic_cast<VolumeLoaderBaseGenericBase<T>*>(_volumeLoader);
	const auto sizeX = _datasetInfo.sizeX;
	const auto sizeY = _datasetInfo.sizeY;
	const auto sizeZ = _datasetInfo.sizeZ;

	const auto sCountX = sizeX / _segmentSize;
	const auto sCountY = sizeY / _segmentSize;
	const auto sCountZ = sizeZ / _segmentSize;

	int segmentSize = _segmentSize >> _preloadLevel;
	uint32_t voxelsInSegment = segmentSize * segmentSize * segmentSize;

	loader->Preload(_preloadLevel, threadCount);

	std::vector<T> h_array;
	h_array.resize(voxelsInSegment);

	int count;
	while (true)
	{
		auto data = loader->TakeFirstLoaded(count);
		if (count == 0)
		{
			break;
		}

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

		bool success;
		auto blockCoords = Vector3i(data->x, data->y, data->z);
		auto textureHandler = CreateTexture<T>(blockCoords, h_array.data(), _preloadLevel, success);

		auto index = Common::Data::Indexing::XYZToIdx(blockCoords, Vector3i(sCountX, sCountY, sCountZ));
		_textures_h[index] = textureHandler;
	}

	cudaMemcpy(_textures_d, _textures_h.data(), sizeof(TextureBlock) * sCountX * sCountY * sCountZ, cudaMemcpyHostToDevice);

	_memoryChanged.store(true, std::memory_order_release);
}

template<typename T>
bool GPURayCastingVolumeMemory::OnDataLoaded()
{
	auto loader = dynamic_cast<VolumeLoaderBaseGenericBase<T>*>(_volumeLoader);

	const auto sizeX = _datasetInfo.sizeX;
	const auto sizeY = _datasetInfo.sizeY;
	const auto sizeZ = _datasetInfo.sizeZ;

	const auto sCountX = sizeX / _segmentSize;
	const auto sCountY = sizeY / _segmentSize;
	const auto sCountZ = sizeZ / _segmentSize;

	int count;
	auto volume = loader->TakeFirstLoaded(count);

	auto index = Common::Data::Indexing::XYZToIdx(Eigen::Vector3i(volume->x, volume->y, volume->z ), Eigen::Vector3i(sCountX, sCountY, sCountZ));

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

	bool success;
	auto texture = CreateTexture<T>(Eigen::Vector3i(volume->x, volume->y, volume->z), h_array.data(), volume->downscale, success);
	
	if (success) 
	{
		_loadedTexturesMutex.lock();
		_loadedTextures.push(texture);
		_loadedTexturesMutex.unlock();
	}

	MemoryContext::GetInstance().memoryInfoWriteMutex.lock();
	MemoryContext::GetInstance().usedMemory -= loader->GetBlockRequiredMemory(volume->downscale);
	MemoryContext::GetInstance().memoryInfoWriteMutex.unlock();

	return success;
}

inline GPURayCastingVolumeMemory::~GPURayCastingVolumeMemory()
{
	delete _volumeLoader;

	uint32_t totalSegments = _segmentCount[0] * _segmentCount[1] * _segmentCount[2];

	for (size_t i = 0; i < totalSegments; i++)
	{
		DeleteTexture(_textures_h[i]);
	}

	cudaFree(_textures_d);
	cudaFree(_composedTexture_d);
}

__forceinline float GPURayCastingVolumeMemory::GetPriority(int xIndex, int yIndex, int zIndex)
{
	auto vecPos = _camera->DeshrinkVector(Vector3f(xIndex, yIndex, zIndex) + Vector3f(0.5f, 0.5f, 0.5f));
	return (vecPos - _camera->GetShrankPosition()).norm();
}

__forceinline int GPURayCastingVolumeMemory::GetRequiredDownscale(int xIndex, int yIndex, int zIndex, float qualityDivider)
{
	auto vecPos = _camera->DeshrinkVector(Vector3f(xIndex, yIndex, zIndex) + Vector3f(0.5f, 0.5f, 0.5f));
	auto dist = (vecPos - _camera->GetPosition()).norm();
	float quality = _camera->GetScreenSize().x() / (2 * tanf(_camera->GetViewAngle() * 0.5) * dist) * _camera->GetVoxelSizeMean() / qualityDivider;
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
void GPURayCastingVolumeMemory::RevalidateTemplated(float objectQuality)
{
	auto loader = dynamic_cast<VolumeLoaderBaseGenericBase<T>*>(_volumeLoader);
	int segmentSizeShifter = (int)(log2(_segmentSize) + 0.5);

	const auto sizeX = _datasetInfo.sizeX;
	const auto sizeY = _datasetInfo.sizeY;
	const auto sizeZ = _datasetInfo.sizeZ;

	const auto sCountX = sizeX / _segmentSize;
	const auto sCountY = sizeY / _segmentSize;
	const auto sCountZ = sizeZ / _segmentSize;

	const auto segmentCount = sCountX * sCountY * sCountZ;

	cudaMemcpy(_textures_h.data(), _textures_d, sizeof(TextureBlock) * segmentCount, cudaMemcpyDeviceToHost);

	size_t freeMemory, totalMemory;
	cudaMemGetInfo(&freeMemory, &totalMemory);

	bool vramFull = freeMemory < 2000000000; // 2GB difference Another check in texture creating

	for (size_t z = 0; z < sCountZ; z++)
	{
		for (size_t y = 0; y < sCountY; y++)
		{
			for (size_t x = 0; x < sCountX; x++)
			{
				uint64_t index = Common::Data::Indexing::XYZToIdx(Eigen::Vector3i(x,y,z), { sCountX, sCountY, sCountZ });
				int requiredDownscale = GetRequiredDownscale((x << segmentSizeShifter) + _segmentSize / 2, (y << segmentSizeShifter) + _segmentSize / 2, (z << segmentSizeShifter) + _segmentSize / 2, objectQuality);

				auto ticket = _tickets[index];

				// Volume not updated yet
				if (ticket != nullptr && ticket->state == RequestState::Loaded)
				{
					continue;
				}

				if (ticket != nullptr && (_tickets[index]->state == RequestState::UnableToLoadOrSkipped))
				{
					_tickets[index] = nullptr;
					ticket = nullptr;
				}

				
				if (ticket == nullptr)
				{
					if (!vramFull)
					{
						if (_textures_h[index].used && _textures_h[index].downscale > requiredDownscale)
						{
							float priority = GetPriority((x << segmentSizeShifter) + _segmentSize / 2, (y << segmentSizeShifter) + _segmentSize / 2, (z << segmentSizeShifter) + _segmentSize / 2);

							_tickets[index] = loader->LoadAsync(x, y, z, requiredDownscale, priority);
						}
					}
				}
				else
				{
					float priority = GetPriority((x << segmentSizeShifter) + _segmentSize / 2, (y << segmentSizeShifter) + _segmentSize / 2, (z << segmentSizeShifter) + _segmentSize / 2);

					ticket->mutex.lock();

					ticket->needed = _textures_h[index].used && _textures_h[index].downscale > requiredDownscale;
					
					ticket->downscale = requiredDownscale;
					ticket->priority = priority;

					ticket->mutex.unlock();					
				}

			}
		}
	}

	// Downscale N of segments with higher quality
	if (vramFull)
	{
		DownscaleWithHigherQuality<T>(5, objectQuality);
	}

	cudaMemcpy(_textures_d, _textures_h.data(), sizeof(TextureBlock) * segmentCount, cudaMemcpyHostToDevice);
}

template <typename T>
void GPURayCastingVolumeMemory::DownscaleWithHigherQuality(int maxCount, float objectQuality)
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
		Eigen::Vector3i XYZ = Common::Data::Indexing::IdxToXYZ(i, Eigen::Vector3i(sCountX, sCountY, sCountZ));
		auto requiredDownscale = GetRequiredDownscale((XYZ.x() << segmentSizeShifter) + _segmentSize / 2, (XYZ.y() << segmentSizeShifter) + _segmentSize / 2, (XYZ.z() << segmentSizeShifter) + _segmentSize / 2, objectQuality);

		if (_textures_h[i].downscale < requiredDownscale || (_textures_h[i].used == false && _textures_h[i].downscale < _preloadLevel))
		{			
			TextureBlock orig = _textures_h[i];
			
			_textures_h[i] =  CreateDownscaledTexture<T>(orig);

			DeleteTexture(orig);

			_memoryChanged.store(true, std::memory_order_release);
			IncrementVersionId();
			count++;
			if (++count >= maxCount) 
			{
				break;
			}			
		}		
	}
}

inline void GPURayCastingVolumeMemory::Prepare()
{
	const auto sizeX = _datasetInfo.sizeX;
	const auto sizeY = _datasetInfo.sizeY;
	const auto sizeZ = _datasetInfo.sizeZ;

	const auto sCountX = sizeX / _segmentSize;
	const auto sCountY = sizeY / _segmentSize;
	const auto sCountZ = sizeZ / _segmentSize;
	
	_loadedTexturesMutex.lock();
	while (!_loadedTextures.empty())
	{
		auto& texture = _loadedTextures.top(); _loadedTextures.pop();
		auto index = Common::Data::Indexing::XYZToIdx(texture.coordinates, { sCountX, sCountY, sCountZ });

		DeleteTexture(_textures_h[index]);
		_textures_h[index] = texture;
		_tickets[index] = nullptr;
	}

	cudaMemcpy(_textures_d, _textures_h.data(), sizeof(TextureBlock) * sCountX * sCountY * sCountZ, cudaMemcpyHostToDevice);
	_loadedTexturesMutex.unlock();
}

inline void GPURayCastingVolumeMemory::Revalidate(float objectQuality)
{
	CALL_TEMPLATED_FUNCTION2(RevalidateTemplated, _datasetInfo.dataType, objectQuality);
}

inline void GPURayCastingVolumeMemory::CleanUsage()
{
	const auto sizeX = _datasetInfo.sizeX;
	const auto sizeY = _datasetInfo.sizeY;
	const auto sizeZ = _datasetInfo.sizeZ;

	const auto sCountX = sizeX / _segmentSize;
	const auto sCountY = sizeY / _segmentSize;
	const auto sCountZ = sizeZ / _segmentSize;

	for (size_t i = 0; i < sCountX * sCountY * sCountZ; i++)
	{
		_textures_h[i].used = false;
	}

	cudaMemcpy(_textures_d, _textures_h.data(), sizeof(TextureBlock) * sCountX * sCountY * sCountZ, cudaMemcpyHostToDevice);
}

inline GPURayCastingVolumeMemory::GPURayCastingVolumeMemory(const std::shared_ptr<Camera>& camera, const std::shared_ptr<VolumeLoaderFactory>& volumeLoaderFactory):
	_datasetInfo(volumeLoaderFactory->GetDatasetInfo()),
	_volumeLoader(volumeLoaderFactory->Create().release()),
	_camera(camera),
	_valueMultiplier(CALL_TEMPLATED_FUNCTION2(GetValueMultiplicator, _datasetInfo.dataType))
{	

	_segmentSize = _datasetInfo.segmentSize;
	_segmentCount = Vector3i(_datasetInfo.sizeX / _segmentSize, _datasetInfo.sizeY / _segmentSize, _datasetInfo.sizeZ / _segmentSize);

	uint32_t totalSegments = _segmentCount[0] * _segmentCount[1] * _segmentCount[2];	

	_textures_h.resize(totalSegments);

	_tickets.resize(totalSegments);

	cudaMalloc(&_textures_d, sizeof(TextureBlock) * totalSegments);

	GPURayCastingVolumeTexture composedTexture(_textures_d, _segmentCount, _segmentSize, _valueMultiplier);

	cudaMalloc(&_composedTexture_d, sizeof(GPURayCastingVolumeTexture));
	cudaMemcpy(_composedTexture_d, &composedTexture, sizeof(GPURayCastingVolumeTexture), cudaMemcpyHostToDevice);

	CALL_TEMPLATED_FUNCTION2(Preload, _datasetInfo.dataType, SettingsContext::GetInstance().preloadingThreadCount.load());
	_volumeLoader->BindOnBlockLoaded(
		[this]() 
		{ 
			bool success = CALL_TEMPLATED_FUNCTION2(OnDataLoaded, _datasetInfo.dataType);
			_memoryChanged.store(true); 
			IncrementVersionId();
			return success;
		});
}

