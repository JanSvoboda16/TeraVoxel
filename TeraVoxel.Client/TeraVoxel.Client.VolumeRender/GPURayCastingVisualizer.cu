#undef EIGEN_USE_GPU
#include <fstream>
#include "GPURayCastingVolumeVisualizer.h"
#include "../TeraVoxel.Client.Core/TemplatedFunctionCaller.h"
#include <cuda_runtime.h>
#include <device_launch_parameters.h>
#include <iostream>
#include "VolumeLoaderBase.h"
#include "../TeraVoxel.Client.Core/ProjectManager.h"
#include "RayCastingUtilities.cuh"

__device__ float getTextureValue2(Vector3i position, GPURayCastingVolumeTexture<float>* texture) {
    cudaTextureObject_t tex = texture->GetTexture_D(position / 256, 0);

    if (tex == 0)
    {
        return 0;
    }

    float texPosX = position[0] % 256;
    float texPosY = position[1] % 256;
    float texPosZ = position[2] % 256;

    return tex3D<float>(tex, texPosX + 0.5f, texPosY + 0.5f, texPosZ + 0.5f);
}

__device__ float getTextureCornerValue(Vector3f position, GPURayCastingVolumeTexture<float>* texture)
{
    Vector3i pos000 = position.cast<int>();
    Vector3i pos001 = pos000 + Vector3i(0, 0, 1);
    Vector3i pos010 = pos000 + Vector3i(0, 1, 0);
    Vector3i pos011 = pos000 + Vector3i(0, 1, 1);

    Vector3i pos100 = pos000 + Vector3i(1, 0, 0);
    Vector3i pos101 = pos000 + Vector3i(1, 0, 1);
    Vector3i pos110 = pos000 + Vector3i(1, 1, 0);
    Vector3i pos111 = pos000 + Vector3i(1, 1, 1);

    float val000 = getTextureValue2(pos000, texture);
    float val001 = getTextureValue2(pos001, texture);
    float val010 = getTextureValue2(pos010, texture);
    float val011 = getTextureValue2(pos011, texture);
    float val100 = getTextureValue2(pos100, texture);
    float val101 = getTextureValue2(pos101, texture);
    float val110 = getTextureValue2(pos110, texture);
    float val111 = getTextureValue2(pos111, texture);

    position = position - pos000.cast<float>();
    float c00 = val000 * (1.f - position[0]) + val100 * position[0];
    float c01 = val001 * (1.f - position[0]) + val101 * position[0];
    float c10 = val010 * (1.f - position[0]) + val110 * position[0];
    float c11 = val011 * (1.f - position[0]) + val111 * position[0];

    float c0 = c00 * (1.f - position[1]) + c10 * position[1];
    float c1 = c01 * (1.f - position[1]) + c11 * position[1];
    return c0 * (1.f - position[2]) + c1 * position[2];
}

__device__ float getTextureValue(Vector3f position, GPURayCastingVolumeTexture<float>* texture) 
{   

    float texPosX = fmodf(position[0], 256);
    float texPosY = fmodf(position[1], 256);
    float texPosZ = fmodf(position[2], 256);

    if (texPosX > 255.f || texPosY > 255.f || texPosZ > 255.f)
    {
        return getTextureCornerValue(position, texture);
    }
    
    cudaTextureObject_t tex = texture->GetTexture_D(position.cast<int>() / 256, 0);
    if (tex == 0)
    {
        return 0;
    }

    return tex3D<float>(tex, texPosX + 0.5f, texPosY + 0.5f, texPosZ + 0.5f);
}


__global__ void fillRedKernel(unsigned char* image, int width, int height, int iteration, Camera* camera, GPURayCastingVolumeTexture<float>* texture, Vector3f volumeDimensions, float minVal, float maxVal)
{    
    int x = blockIdx.x * blockDim.x + threadIdx.x;
    int y = blockIdx.y * blockDim.y + threadIdx.y;

    if (x >= width || y >= height) {
        return;
    }

    int framebufferIndex = (y * width + x) * 4;
    float stepSize = 0.5;

    Vector3f step = camera->GetShrankRayDirection(x, y).normalized()*stepSize;
    Vector3f position = camera->GetShrankPosition();
    Vector3f start, end;
    
    bool intersected = RayCastingUtilities::ComputeRayIntersection(step, position, volumeDimensions, start, end);
    
    position = start;

    
    uint32_t stepCount = (start - end).norm() / stepSize;

    Vector4f colorAcumulator(0.f, 0.f, 0.f, 0.f);

    if (intersected)
    {
        for (uint32_t i = 0; i < stepCount; i++)
        {          
            int value = getTextureValue(position, texture);

            float fvalue = (value - minVal) / (maxVal - minVal);
            fvalue = (fvalue < 0 || fvalue > 1) ? 0 : fvalue;
            float r = fvalue;
            float g = fvalue;
            float b = fvalue;
            float a = fvalue;

            colorAcumulator[0] = colorAcumulator[0] + r * a * (1 - colorAcumulator[3]);
            colorAcumulator[1] = colorAcumulator[1] + g * a * (1 - colorAcumulator[3]);
            colorAcumulator[2] = colorAcumulator[2] + b * a * (1 - colorAcumulator[3]);
            colorAcumulator[3] = colorAcumulator[3] + a * (1 - colorAcumulator[3]);

            position += step;
        }
    }

    
    image[framebufferIndex] = min((int)(colorAcumulator[0]*255), 255);     // Red
    image[framebufferIndex + 1] = min((int)(colorAcumulator[1]*255), 255);   // Green
    image[framebufferIndex + 2] = min((int)(colorAcumulator[2]*255), 255);  // Blue
    image[framebufferIndex + 3] = min((int)(colorAcumulator[3]*255), 255);   // Alfa

   

}

GPURayCastingVolumeVisualizer::GPURayCastingVolumeVisualizer(const std::shared_ptr<Camera>& camera, const std::shared_ptr<VolumeLoaderFactory>& volumeLoaderFactory, const std::shared_ptr<CPURCVolumeVisualizerSettings>& settings)
    :VolumeVisualizerBase(camera, volumeLoaderFactory)
{
    std::shared_ptr<VolumeLoaderGenericBase> loaderBase = _volumeLoaderFactory->Create();
    auto loader = std::dynamic_pointer_cast<VolumeLoaderBase<int16_t>>(loaderBase);
    const auto segmentSize = _projectInfo.segmentSize;

    const auto sizeX = _projectInfo.sizeX;
    const auto sizeY = _projectInfo.sizeY;
    const auto sizeZ = _projectInfo.sizeZ;

    const auto sCountX = sizeX / segmentSize;
    const auto sCountY = sizeY / segmentSize;
    const auto sCountZ = sizeZ / segmentSize;

    GPURayCastingVolumeTexture<float>* texture = new GPURayCastingVolumeTexture<float>(Vector3i(sCountX, sCountY, sCountZ), 1, 256);   
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
                    uint16_t z = i / (segmentSize* segmentSize);
                    uint16_t y = (i % (segmentSize* segmentSize)) / segmentSize;
                    uint16_t x = (i % (segmentSize* segmentSize)) % segmentSize;
                    uint32_t mortonIndex = Serialization::GetZCurveIndex(x, y, z);
                    h_array[i] = data->data[mortonIndex];                    
                }

                texture->CreateTexture(Vector3i(bx, by, bz), 0, h_array.data());
            }
        }
    }
   

    cudaMalloc(&_texture, sizeof(GPURayCastingVolumeTexture<float>));
    cudaMemcpy(_texture, texture, sizeof(GPURayCastingVolumeTexture<float>), cudaMemcpyHostToDevice);  
   


    

    /*std::vector<float> h_array;
    h_array.reserve(256 * 256 * 256);
    
    for (size_t i = 0; i < 256 * 256 * 256; ++i)
    {
        uint16_t z = i / (256 * 256);
        uint16_t y = (i % (256 * 256)) / 256;
        uint16_t x = (i % (256 * 256)) % 256;
        uint32_t mortonIndex = Serialization::GetZCurveIndex(x, y, z);
        h_array[i] = data->data[mortonIndex];
    }

    cudaExtent extent = make_cudaExtent(256, 256, 256);
    cudaChannelFormatDesc channelDesc = cudaCreateChannelDesc<float>();
    cudaMalloc3DArray(&_textureArrayDevice, &channelDesc, extent);

    cudaMemcpy3DParms copyParams = { 0 };
    copyParams.srcPtr = make_cudaPitchedPtr(h_array.data(), 256 * sizeof(float), 256, 256);
    copyParams.dstArray = _textureArrayDevice;
    copyParams.extent = extent;
    copyParams.kind = cudaMemcpyHostToDevice;

    cudaMemcpy3D(&copyParams);


    cudaResourceDesc resDesc = {};
    resDesc.resType = cudaResourceTypeArray;
    resDesc.res.array.array = _textureArrayDevice;

    cudaTextureDesc texDesc = {};
    texDesc.addressMode[0] = cudaAddressModeBorder;  // Wrap, Clamp, Border
    texDesc.addressMode[1] = cudaAddressModeBorder;
    texDesc.addressMode[2] = cudaAddressModeBorder;
    texDesc.filterMode = cudaFilterModeLinear;      // Linear nebo Point
    texDesc.readMode = cudaReadModeElementType;
    texDesc.normalizedCoords = 0;                   // Použít norm. souřadnice (0 až 1).

    cudaCreateTextureObject(&_texObj, &resDesc, &texDesc, nullptr);*/

}

GPURayCastingVolumeVisualizer::~GPURayCastingVolumeVisualizer()
{
    cudaDestroyTextureObject(_texObj);
    cudaFreeArray(_textureArrayDevice);
}

bool GPURayCastingVolumeVisualizer::DataChanged()
{
    return false;
}

void GPURayCastingVolumeVisualizer::ComputeFrameInternal(std::shared_ptr<unsigned char[]>& framebuffer, int downscale, const std::shared_ptr<MultiLayeredFramebufferBase>& multiLayeredFramebuffer)
{
    Vector2i screenSize = _camera->GetScreenSize();

    long width = screenSize[0];
    long height = screenSize[1];

    unsigned char* d_image = nullptr;

    Camera* d_camera = nullptr;

    // Alokace paměti na GPU
    cudaMalloc((void**)&d_image, width * height * 4 * sizeof(unsigned char));
    cudaMalloc((void**)&d_camera, sizeof(Camera));
    cudaMemcpy(d_camera, _camera.get(), sizeof(Camera), cudaMemcpyHostToDevice);

    // Definice dimenzí bloků a mřížky
    dim3 blockSize(16, 16);
    dim3 gridSize((width + blockSize.x - 1) / blockSize.x, (height + blockSize.y - 1) / blockSize.y);
    static int iteration = 0;
    iteration += 1;

    // Spuštění kernelu
    fillRedKernel << <gridSize, blockSize >> > (d_image, width, height, iteration, d_camera, _texture, Vector3f(_projectInfo.dataSizeX, _projectInfo.dataSizeY, _projectInfo.dataSizeZ), -20000, 200);

    // Čekání na dokončení všech operací
    cudaDeviceSynchronize();


    // Kopírování výsledků zpět na CPU
    cudaMemcpy(framebuffer.get(), d_image, width * height * 4 * sizeof(unsigned char), cudaMemcpyDeviceToHost);

    // Uvolnění paměti na GPU
    cudaFree(d_image);

    cudaFree(d_camera);
}

template<typename T>
inline void GPURayCastingVolumeVisualizer::CoumputeFrameInternalTemplated(int downscale)
{
    /**/

}
