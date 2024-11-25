#undef EIGEN_USE_GPU
#include <fstream>
#include <cuda_runtime.h>
#include <device_launch_parameters.h>
#include <cuda_fp16.h>
#include "GPURayCastingVolumeVisualizer.h"
#include <iostream>
#include "../TeraVoxel.Client.Core/TemplatedFunctionCaller.h"
#include "VolumeLoaderBase.h"
#include "../TeraVoxel.Client.Core/ProjectManager.h"
#include "RayCastingUtilities.cuh"
#include "GPURayCastingVolumeMemory.cuh"

struct Light
{
    Vector3f positionOdDirection;
    bool directional;
    float intensity;
};

__inline__ __device__ Vector3f ComputeShadowRay(Camera* camera, Vector3f position, const Vector3f& lightPos, GPURayCastingVolumeTexture* texture, const Vector3f& volumeDimensions, const MaterialTableItem* materialTable, uint16_t materialTableSize)
{
    const float stepSize = 1.f;
    const Vector3f step = (lightPos - position).normalized() * stepSize;
    position += step;
    Vector3f start, end;

    const float alphaCoef = camera->DeshrinkVector(step).norm();
  
    bool intersected = RayCastingUtilities::ComputeRayIntersection(step, position, volumeDimensions, start, end);
    const uint32_t stepCount = (start - end).norm() / stepSize;

    Vector3f translucency(1.f, 1.f, 1.f);

    if (intersected)
    {
        for (uint32_t i = 0; i < stepCount; i++)
        {
            float value = texture->GetTextureValue(position);

            bool itemFound = false;
            uint16_t index = 0;
            for (size_t j = 0; j < materialTableSize; j++)
            {
                const MaterialTableItem& item = materialTable[j];
                if (item.range[0] <= value && item.range[1] >= value)
                {
                    itemFound = true;
                    index = j;
                    break;
                }
            }

            if (itemFound)
            {
                const MaterialTableItem& item = materialTable[index];
                float valminran0 = value - item.range[0];

                float tr = (item.RedTranslucencyDivRange() * valminran0 + item.translucencyColorFrom[0]);
                float tg = (item.GreenTranslucencyDivRange() * valminran0 + item.translucencyColorFrom[1]);
                float tb = (item.BlueTranslucencyDivRange() * valminran0 + item.translucencyColorFrom[2]);

                translucency[0] = translucency[0] * powf(tr, alphaCoef);
                translucency[1] = translucency[1] * powf(tg, alphaCoef);
                translucency[2] = translucency[2] * powf(tb, alphaCoef);

                if ((translucency.array() < 0.02f).all())
                {
                    break;
                }
            }

            position += step;
        }
    }

    return translucency;
}

__global__ void ComputeShadowsKernel(GPURayCastingVolumeTexture* volume, cudaSurfaceObject_t shadowTexture, Camera* camera, Vector3f lightPos, Vector3i sizes, int subsamplingFactor, MaterialTableItem* materialTable, uint16_t materialTableSize)
{
    const int x = blockIdx.x * blockDim.x + threadIdx.x;
    const int y = blockIdx.y * blockDim.y + threadIdx.y;
    const int z = blockIdx.z * blockDim.z + threadIdx.z;

    if (x >= sizes.x() || y >= sizes.y() || z >= sizes.z()) 
    {
        return;
    }

    float4 value;
    Vector3f shadow = ComputeShadowRay(camera, Vector3f(x, y, z) * (float)subsamplingFactor, lightPos, volume, (sizes * subsamplingFactor).cast<float>(), materialTable, materialTableSize);
    value.x = shadow.x(); // Převod z float na __half
    value.y = shadow.y();
    value.z = shadow.z();

    surf3Dwrite(value, shadowTexture, x*sizeof(float4), y, z); // Zápis do surface objectu   

}

__global__ void ComputeFrameKernel(unsigned char* image, int width, int height, Camera* camera, GPURayCastingVolumeTexture* texture, Vector3f volumeDimensions, MaterialTableItem* materialTable, uint16_t materialTableSize, cudaTextureObject_t shaddowTexture, int shadowSubsampling)
{
    const Vector3f lightPos(200000, 0, 0);
    const int x = blockIdx.x * blockDim.x + threadIdx.x;
    const int y = blockIdx.y * blockDim.y + threadIdx.y;

    if (x >= width || y >= height) {
        return;
    }

    int framebufferIndex = (y * width + x) * 4;
    float stepSize = 0.5;

    

    Vector3f step = camera->GetShrankRayDirection(x, y).normalized()*stepSize;
    Vector3f position = camera->GetShrankPosition();
    Vector3f start, end;

    float aplhaCoeficient = camera->DeshrinkVector(step).norm();

    bool intersected = RayCastingUtilities::ComputeRayIntersection(step, position, volumeDimensions, start, end);

    position = start;


    uint32_t stepCount = (start - end).norm() / stepSize;

    Vector3f colorAcumulator(0.f, 0.f, 0.f);
    Vector3f translucency(1.f, 1.f, 1.f);

    if (intersected)
    {
        for (uint32_t i = 0; i < stepCount; i++)
        {
            float value = texture->GetTextureValue(position);

            bool itemFound = false;
            uint16_t index = 0;
            for (size_t j = 0; j < materialTableSize; j++)
            {
                const MaterialTableItem& item = materialTable[j];
                if (item.range[0] <= value && item.range[1] >= value)
                {
                    itemFound = true;
                    index = j;
                    break;
                }
            }

            if (itemFound)
            {
                Vector3f grad = texture->GetTextureGrad(position);
                Vector3f normal = -grad.normalized();
                Vector3f light = (lightPos - position).normalized();

                float normalizedGradSize = fminf(grad.norm() / texture->GetMaxValue(), 1.f);
                bool frontGrad = grad.dot(-normal) > 0;
                float4 shTex = tex3D<float4>(shaddowTexture, position.x() / shadowSubsampling, position.y() / shadowSubsampling, position.z() / shadowSubsampling);
                Vector3f shadow(shTex.x, shTex.y, shTex.z);
                Vector3f lightIntensity = shadow * 0.7f;
                Vector3f diffusedIntensity1 = lightIntensity * frontGrad * max(normal.dot(light), 0.f) * normalizedGradSize;
                Vector3f diffusedIntensity2 = lightIntensity * (1.f - normalizedGradSize);
                Vector3f totalLight = diffusedIntensity1 + diffusedIntensity2 + Vector3f(0.3f, 0.3f, 0.3f);

                const MaterialTableItem& item = materialTable[index];
                float valminran0 = value - item.range[0];

                float r = (item.RedReflectionDivRange() * valminran0 + item.reflectionColorFrom[0]) * totalLight[0];
                float g = (item.GreenReflectionDivRange() * valminran0 + item.reflectionColorFrom[1]) * totalLight[1];
                float b = (item.BlueReflectionDivRange() * valminran0 + item.reflectionColorFrom[2]) * totalLight[2];

                float tr = (item.RedTranslucencyDivRange() * valminran0 + item.translucencyColorFrom[0]);
                float tg = (item.GreenTranslucencyDivRange() * valminran0 + item.translucencyColorFrom[1]);
                float tb = (item.BlueTranslucencyDivRange() * valminran0 + item.translucencyColorFrom[2]);

                float reflectness = 1.f - max(max(tr, tg), tb); // if material is transparent, light probably goes more into material
                colorAcumulator[0] = colorAcumulator[0] + r * reflectness * translucency[0];
                colorAcumulator[1] = colorAcumulator[1] + g * reflectness * translucency[1];
                colorAcumulator[2] = colorAcumulator[2] + b * reflectness * translucency[2];

                translucency[0] = translucency[0] * powf(tr, aplhaCoeficient * stepSize);
                translucency[1] = translucency[1] * powf(tg, aplhaCoeficient * stepSize);
                translucency[2] = translucency[2] * powf(tb, aplhaCoeficient * stepSize);

                if ((translucency.array() < 0.02f).all())
                {
                    break;
                }
            }

            position += step;
        }
    }

    image[framebufferIndex] = min((int)(colorAcumulator[0] * 255), 255);     // Red
    image[framebufferIndex + 1] = min((int)(colorAcumulator[1] * 255), 255);   // Green
    image[framebufferIndex + 2] = min((int)(colorAcumulator[2] * 255), 255);  // Blue
    image[framebufferIndex + 3] = 255;   // Alfa   
}

GPURayCastingVolumeVisualizer::GPURayCastingVolumeVisualizer(const std::shared_ptr<Camera>& camera, const std::shared_ptr<VolumeLoaderFactory>& volumeLoaderFactory, const std::shared_ptr<GPURCVolumeVisualizerSettings>& settings)
    : VolumeVisualizerBase(camera, volumeLoaderFactory),
    _settings(settings),
    _memory(std::make_unique<GPURayCastingVolumeMemory>(camera, volumeLoaderFactory))
{
    
}

GPURayCastingVolumeVisualizer::~GPURayCastingVolumeVisualizer()
{
    if (_materialTable_d != nullptr)
    {
        cudaFree(_materialTable_d);
    }

    if (_shadowTexture != NULL)
    {
        cudaDestroyTextureObject(_shadowTexture);
    }

    if (_shadowArray != nullptr) 
    {
        cudaFreeArray(_shadowArray);
    }
}

bool GPURayCastingVolumeVisualizer::DataChanged()
{
    return false;
}

void GPURayCastingVolumeVisualizer::UpdateShadowTexture(Camera* camera_d) 
{
    if (_shadowArray != nullptr)
    {
        cudaFreeArray(_shadowArray);
        _shadowArray = nullptr;
    }

    if (_shadowTexture != NULL)
    {
        cudaDestroyTextureObject(_shadowTexture);
        _shadowTexture = NULL;
    }

    uint8_t subsamplingFactor = 2;
    int sizeX = _projectInfo.dataSizeX / subsamplingFactor;
    int sizeY = _projectInfo.dataSizeY / subsamplingFactor;
    int sizeZ = _projectInfo.dataSizeZ / subsamplingFactor;

    cudaExtent extent = make_cudaExtent(sizeX, sizeY, sizeZ);
    cudaChannelFormatDesc channelDesc = cudaCreateChannelDesc<float4>();
    cudaMalloc3DArray(&_shadowArray, & channelDesc, extent);

    // Definice dimenzí bloků a mřížky
    dim3 blockSize(4,4,4);
    dim3 gridSize((sizeX + blockSize.x - 1) / blockSize.x, (sizeY + blockSize.y - 1) / blockSize.y, (sizeZ + blockSize.z - 1) / blockSize.z);

  

    cudaResourceDesc resDesc = {};
    resDesc.resType = cudaResourceTypeArray;
    resDesc.res.array.array = _shadowArray;

    cudaSurfaceObject_t surfObj;
    cudaCreateSurfaceObject(&surfObj, &resDesc);

    ComputeShadowsKernel << <gridSize, blockSize >> > (_memory->GetTextureDevicePtr(), surfObj,camera_d, Vector3f(20000, 0, 0), Vector3i(sizeX, sizeY, sizeZ), 2, _materialTable_d, _settings->materialTable.table.size());
    cudaDeviceSynchronize();

    cudaTextureDesc texDesc = {};
    texDesc.addressMode[0] = cudaAddressModeClamp;
    texDesc.addressMode[1] = cudaAddressModeClamp;
    texDesc.addressMode[2] = cudaAddressModeClamp;
    texDesc.filterMode = cudaFilterModeLinear; 
    texDesc.readMode = cudaReadModeElementType;
    texDesc.normalizedCoords = 0; 

    cudaCreateTextureObject(&_shadowTexture, &resDesc, &texDesc, nullptr);

    cudaDestroySurfaceObject(surfObj);

}

void GPURayCastingVolumeVisualizer::UpdateEntities(Camera* camera_d)
{
    bool recomputeShadows = false;
    if (_settings->VersionId() != _settingsDeviceVersion)
    {
        _settings->materialTable.RecomputeDeltas();
        if (_materialTable_d != nullptr) 
        {
            cudaFree(_materialTable_d);
        }
        _materialTable_d = _settings->materialTable.CreateTableOnDevice();
        _settingsDeviceVersion = _settings->VersionId();

        recomputeShadows = true;
    }

    if (recomputeShadows)
    {
        UpdateShadowTexture(camera_d);
    }
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

    UpdateEntities(d_camera);

    // Definice dimenzí bloků a mřížky
    dim3 blockSize(16, 16);
    dim3 gridSize((width + blockSize.x - 1) / blockSize.x, (height + blockSize.y - 1) / blockSize.y);

    // Spuštění kernelu
    ComputeFrameKernel<<<gridSize, blockSize>>>(d_image, width, height, d_camera, _memory->GetTextureDevicePtr(), Vector3f(_projectInfo.dataSizeX, _projectInfo.dataSizeY, _projectInfo.dataSizeZ), _materialTable_d, _settings->materialTable.table.size(), _shadowTexture, 2);

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
