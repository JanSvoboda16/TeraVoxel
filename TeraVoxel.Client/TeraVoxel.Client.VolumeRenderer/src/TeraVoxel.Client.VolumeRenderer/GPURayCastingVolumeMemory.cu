#include "TeraVoxel.Client.VolumeRenderer/GPURayCastingVolumeMemory.cuh"

template <typename T>
__global__ void downsampleTexture3D(cudaTextureObject_t inputTex, cudaSurfaceObject_t outputSurf, 
    int newWidth, int newHeight, int newDepth, float valueMultiplier) {
    int x = blockIdx.x * blockDim.x + threadIdx.x;
    int y = blockIdx.y * blockDim.y + threadIdx.y;
    int z = blockIdx.z * blockDim.z + threadIdx.z;


    if (x >= newWidth || y >= newHeight || z >= newDepth) return; 

    float c000 = tex3D<float>(inputTex, 2 * x, 2 * y, 2 * z);
    float c100 = tex3D<float>(inputTex, 2 * x + 1, 2 * y, 2 * z);
    float c010 = tex3D<float>(inputTex, 2 * x, 2 * y + 1, 2 * z);
    float c110 = tex3D<float>(inputTex, 2 * x + 1, 2 * y + 1, 2 * z);

    float c001 = tex3D<float>(inputTex, 2 * x, 2 * y, 2 * z + 1);
    float c101 = tex3D<float>(inputTex, 2 * x + 1, 2 * y, 2 * z + 1);
    float c011 = tex3D<float>(inputTex, 2 * x, 2 * y + 1, 2 * z + 1);
    float c111 = tex3D<float>(inputTex, 2 * x + 1, 2 * y + 1, 2 * z + 1);

    T avg =(c000 + c100 + c010 + c110 + c001 + c101 + c011 + c111) * 0.125f * valueMultiplier;

    surf3Dwrite(avg, outputSurf, x * sizeof(T), y, z);
}

template <typename T>
TextureBlockHandler GPURayCastingVolumeMemory::CreateDownscaledTexture(TextureBlockHandler texture_h_orig)
{
    if (!DataCommon::SupportNormalizedFloat<T>() && (!std::is_floating_point<T>::value))
    {
        return CreateDownscaledTexture<float>(texture_h_orig);
    }

    TextureBlockHandler outputTexture;

    cudaArray_t originalArray = texture_h_orig.array;


    int newWidth = _segmentSize >> (texture_h_orig.downscale + 1);
    int newHeight = _segmentSize >> (texture_h_orig.downscale + 1);
    int newDepth = _segmentSize >> (texture_h_orig.downscale + 1);

    cudaChannelFormatDesc newChannelDesc = cudaCreateChannelDesc<T>();
    cudaArray_t newArray;
    cudaMalloc3DArray(&newArray, &newChannelDesc, make_cudaExtent(newWidth, newHeight, newDepth), cudaArraySurfaceLoadStore);

    cudaResourceDesc resDesc = {};
    resDesc.resType = cudaResourceTypeArray;
    resDesc.res.array.array = newArray;

    cudaTextureDesc texDesc = {};
    texDesc.addressMode[0] = cudaAddressModeClamp;
    texDesc.addressMode[1] = cudaAddressModeClamp;
    texDesc.addressMode[2] = cudaAddressModeClamp;
    texDesc.filterMode = cudaFilterModeLinear;
    texDesc.readMode = !DataCommon::SupportNormalizedFloat<T>() ? cudaReadModeElementType : cudaReadModeNormalizedFloat;
    texDesc.normalizedCoords = 0;

    cudaTextureObject_t newTexture;
    cudaCreateTextureObject(&newTexture, &resDesc, &texDesc, nullptr);

    outputTexture.array = newArray;
    outputTexture.texture = newTexture;
    outputTexture.downscale = texture_h_orig.downscale + 1;
    outputTexture.coordinates = texture_h_orig.coordinates; // Pozice zůstává nezměněná

    // Vytvoření výstupní surface
    cudaSurfaceObject_t outputSurface;
    cudaCreateSurfaceObject(&outputSurface, &resDesc);

    // Definice bloků a mřížky pro downsampling (přizpůsobte velikost bloků podle potřeby)
    dim3 blockSize(8, 8, 8);
    dim3 gridSize((newWidth + blockSize.x - 1) / blockSize.x,
        (newHeight + blockSize.y - 1) / blockSize.y,
        (newDepth + blockSize.z - 1) / blockSize.z);

    // Spuštění downsamplingového jádra
    downsampleTexture3D<T> << <gridSize, blockSize >> > (texture_h_orig.texture, outputSurface,
        newWidth, newHeight, newDepth, _valueMultiplier);

    cudaDestroySurfaceObject(outputSurface);

    return outputTexture;
}


template TextureBlockHandler GPURayCastingVolumeMemory::CreateDownscaledTexture<uint8_t>(TextureBlockHandler);
template TextureBlockHandler GPURayCastingVolumeMemory::CreateDownscaledTexture<uint16_t>(TextureBlockHandler);
template TextureBlockHandler GPURayCastingVolumeMemory::CreateDownscaledTexture<uint32_t>(TextureBlockHandler);
template TextureBlockHandler GPURayCastingVolumeMemory::CreateDownscaledTexture<uint64_t>(TextureBlockHandler);
template TextureBlockHandler GPURayCastingVolumeMemory::CreateDownscaledTexture<float>(TextureBlockHandler);
template TextureBlockHandler GPURayCastingVolumeMemory::CreateDownscaledTexture<int8_t>(TextureBlockHandler);
template TextureBlockHandler GPURayCastingVolumeMemory::CreateDownscaledTexture<int16_t>(TextureBlockHandler);
template TextureBlockHandler GPURayCastingVolumeMemory::CreateDownscaledTexture<int32_t>(TextureBlockHandler);
template TextureBlockHandler GPURayCastingVolumeMemory::CreateDownscaledTexture<int64_t>(TextureBlockHandler);
