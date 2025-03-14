/*
 * Author: Jan Svoboda
 * University: BRNO UNIVERSITY OF TECHNOLOGY, FACULTY OF INFORMATION TECHNOLOGY
 */
#undef EIGEN_USE_GPU
#include <fstream>
#include <cuda_runtime.h>
#include <device_launch_parameters.h>
#include <cuda_fp16.h>
#include <iostream>
#include <TeraVoxel.Client.Core/TemplatedFunctionCaller.h>
#include "TeraVoxel.Client.VolumeRenderer/GPURayCastingVolumeVisualizer.h"
#include "TeraVoxel.Client.VolumeRenderer/VolumeLoaderBase.h"
#include <TeraVoxel.Client.Core/ProjectManager.h>
#include "TeraVoxel.Client.VolumeRenderer/RayCastingUtilities.cuh"
#include "TeraVoxel.Client.VolumeRenderer/GPURayCastingVolumeMemory.cuh"

#define MAX_LIGHTS 5

/// <summary>
/// Computes shadows on the given position for the given lightsource.
/// </summary>
/// <param name="camera">Camera</param>
/// <param name="position">Position in space</param>
/// <param name="lightPos">Position of the light</param>
/// <param name="texture">Volumetric data texture</param>
/// <param name="volumeDimensions">Volume dimensions</param>
/// <param name="skipDistance">Unsampled distance from the origin position</param>
/// <param name="materialTable">Material table</param>
/// <param name="materialTableSize">Size of the mateiral table</param>
/// <returns>% of shadows on the given position for each chanel</returns>
__inline__ __device__ Vector3f ComputeShadowRay(Camera* camera, Vector3f position, const Vector3f& lightPos, GPURayCastingVolumeTexture* texture, const Vector3f& volumeDimensions, float skipDistance, const MaterialTableItem* materialTable, uint16_t materialTableSize)
{
    const float stepSize = 1.f;
    const Vector3f direction = (lightPos - position).normalized();
    const Vector3f step = direction * stepSize;
    position += direction * skipDistance;
    Vector3f start, end;

    const float alphaCoef = camera->DeshrinkVector(step).norm(); // Coefient for transparecny normalization 
    bool intersected = RayCastingUtilities::ComputeRayIntersection(step, position, volumeDimensions, start, end);
    const uint32_t stepCount = (start - end).norm() / stepSize; // Count of steps to be sampled

    Vector3f translucency(1.f, 1.f, 1.f);

    if (intersected)
    {
        // Sampling
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

/// <summary>
/// Computes shadows in the scene for the given lightsource.
/// </summary>
/// <param name="volume">Volumtric data texture</param>
/// <param name="shadowTexture">Shadow texture</param>
/// <param name="camera">Camera</param>
/// <param name="lightPos">Position of the lightsource</param>
/// <param name="sizes">Sizes of the shadow texture</param>
/// <param name="subsamplingFactor">Subsampling factor</param>
/// <param name="materialTable">Material table</param>
/// <param name="materialTableSize">Material table size</param>
/// <returns></returns>
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
    Vector3f shadow = ComputeShadowRay(camera, Vector3f(x, y, z) * (float)subsamplingFactor, lightPos, volume, (sizes * subsamplingFactor).cast<float>(), subsamplingFactor, materialTable, materialTableSize);
    value.x = shadow.x();
    value.y = shadow.y();
    value.z = shadow.z();

    surf3Dwrite(value, shadowTexture, x*sizeof(float4), y, z);
}

/// <summary>
/// Computes frame
/// </summary>
/// <param name="image">Image array</param>
/// <param name="width">Width of the output image</param>
/// <param name="height">Height of the output image</param>
/// <param name="camera">Camera</param>
/// <param name="texture">Volumetric data texture</param>
/// <param name="volumeDimensions">Volumetric data dimensions</param>
/// <param name="materialTable">Material table</param>
/// <param name="materialTableSize">Size of the material table</param>
/// <param name="shaddowTextures">Shadow textures</param>
/// <param name="shadowSubsampling">Shadow subsampling</param>
/// <param name="lightSettings">Lightsources + settings</param>
/// <returns></returns>
__global__ void ComputeFrameKernel(unsigned char* image, int width, int height, Camera* camera, GPURayCastingVolumeTexture* texture, Vector3f volumeDimensions, MaterialTableItem* materialTable, uint16_t materialTableSize, cudaTextureObject_t* shaddowTextures, int shadowSubsampling, LightSettings* lightSettings)
{
    const int x = blockIdx.x * blockDim.x + threadIdx.x;
    const int y = blockIdx.y * blockDim.y + threadIdx.y;

    if (x >= width || y >= height) {
        return;
    }

    int framebufferIndex = (y * width + x) * 4;
    float stepSize = 0.5; // size of step 

    Vector3f direction = camera->GetShrankRayDirection(x, y).normalized();
    Vector3f step = direction * stepSize;
    Vector3f position = camera->GetShrankPosition();
    Vector3f start, end;

    float aplhaCoeficient = camera->DeshrinkVector(step).norm(); // coefient for transparecny normalization 

    bool intersected = RayCastingUtilities::ComputeRayIntersection(step, position, volumeDimensions, start, end);

    position = start;

    uint32_t stepCount = (start - end).norm() / stepSize; // Count of steps to be sampled

    float stepWordSpaceLength = aplhaCoeficient * stepSize;

    Vector3f colorAcumulator(0.f, 0.f, 0.f); // Final colur acumulator
    Vector3f translucency(1.f, 1.f, 1.f); // Translucency acumulator

    if (intersected)
    {
        // Sampling
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
                float normalizedGradSize = fminf(grad.norm() / texture->GetMaxValue(), 1.f);              
                bool frontGrad = direction.dot(grad) > 0;
                
                const MaterialTableItem& item = materialTable[index];
                float valminran0 = value - item.range[0];

                Vector3f totalReflectedIntensity(0.f, 0.f, 0.f);
                for (size_t l = 0; l < lightSettings->numLights; l++)
                {
                    Light& light = lightSettings->lights[l];
                    Vector3f lightPos(light.position[0], light.position[1], light.position[2]);
                    Vector3f lightDirection = (lightPos - position).normalized();

                    float4 shTex = tex3D<float4>(shaddowTextures[l], position.x() / shadowSubsampling + 0.5f, position.y() / shadowSubsampling + 0.5f, position.z() / shadowSubsampling + 0.5f);
                    Vector3f shadow(shTex.x, shTex.y, shTex.z);
                    Vector3f lightIntensity = shadow * light.intensity; // Light intensity will be parameter

                    float specularCoeficient = (item.SpecularReflectionDivRange() * valminran0 + item.specularReflectionFrom);
                    float specularSharpness = (item.SpecularSharpnessDivRange() * valminran0 + item.specularSharpnessFrom);

                    // Split available "energy" to specular and diffused partition.
                    float specularEnergy = specularCoeficient * normalizedGradSize; // only on front sides
                    float diffusedEnergy = 1.f - specularEnergy;
                    float diffusedEnergy1 = diffusedEnergy * normalizedGradSize;
                    float diffusedEnergy2 = diffusedEnergy * (1.f - normalizedGradSize);

                    // Intensities for 100% white material and not translucent material
                    Vector3f R = 2 * (normal.dot(lightDirection)) * normal - lightDirection;
                    Vector3f specularIntensity = specularEnergy * lightIntensity * powf(fmaxf(0.f, R.dot(-direction)), specularSharpness) * ((9.f * specularSharpness + 3.f) / 6.24f) * frontGrad; // Normalizace
                    Vector3f diffusedIntensity1 = diffusedEnergy1 * lightIntensity * max(normal.dot(lightDirection), 0.f) * 0.477f * frontGrad;
                    Vector3f diffusedIntensity2 = diffusedEnergy2 * lightIntensity * 0.239; 
                    totalReflectedIntensity += diffusedIntensity1 + diffusedIntensity2 + specularIntensity;
                }         

                totalReflectedIntensity += Vector3f(lightSettings->ambientIntensity, lightSettings->ambientIntensity, lightSettings->ambientIntensity);

                // Compute translucency
                float tr = (item.RedTranslucencyDivRange() * valminran0 + item.translucencyColorFrom[0]);
                float tg = (item.GreenTranslucencyDivRange() * valminran0 + item.translucencyColorFrom[1]);
                float tb = (item.BlueTranslucencyDivRange() * valminran0 + item.translucencyColorFrom[2]);

                // Apply material
                float reflectness = 1.f - max(max(tr, tg), tb); // if material is transparent, light interact less with it
                float r = (item.RedReflectionDivRange() * valminran0 + item.reflectionColorFrom[0]) * totalReflectedIntensity[0];
                float g = (item.GreenReflectionDivRange() * valminran0 + item.reflectionColorFrom[1]) * totalReflectedIntensity[1];
                float b = (item.BlueReflectionDivRange() * valminran0 + item.reflectionColorFrom[2]) * totalReflectedIntensity[2];

                r *= reflectness;
                g *= reflectness;
                b *= reflectness;

                // Acumulation
                colorAcumulator[0] = colorAcumulator[0] + r * translucency[0];
                colorAcumulator[1] = colorAcumulator[1] + g * translucency[1];
                colorAcumulator[2] = colorAcumulator[2] + b * translucency[2];

                translucency[0] = translucency[0] * powf(tr, stepWordSpaceLength);
                translucency[1] = translucency[1] * powf(tg, stepWordSpaceLength);
                translucency[2] = translucency[2] * powf(tb, stepWordSpaceLength);

                if ((translucency.array() < 0.02f).all())
                {
                    break;
                }
            }

            position += step;
        }
    }

    image[framebufferIndex] = min((int)(colorAcumulator[0] * 255), 255);        // Red
    image[framebufferIndex + 1] = min((int)(colorAcumulator[1] * 255), 255);    // Green
    image[framebufferIndex + 2] = min((int)(colorAcumulator[2] * 255), 255);    // Blue
    image[framebufferIndex + 3] = 255;                                          // Alpha  
}

GPURayCastingVolumeVisualizer::GPURayCastingVolumeVisualizer(const std::shared_ptr<Camera>& camera, const std::shared_ptr<VolumeLoaderFactory>& volumeLoaderFactory, const std::shared_ptr<GPURCVolumeVisualizerSettings>& settings)
    : VolumeVisualizerBase(camera, volumeLoaderFactory),
    _settings(settings),
    _memory(std::make_unique<GPURayCastingVolumeMemory>(camera, volumeLoaderFactory))
{

    int sizeX = _datasetInfo.dataSizeX / _shadowSubsampling;
    int sizeY = _datasetInfo.dataSizeY / _shadowSubsampling;
    int sizeZ = _datasetInfo.dataSizeZ / _shadowSubsampling;

    /* Shadow textures creation */

    cudaExtent extent = make_cudaExtent(sizeX, sizeY, sizeZ);
    cudaChannelFormatDesc channelDesc = cudaCreateChannelDesc<float4>(); 

    _shadowArrays_h = new cudaArray_t[MAX_LIGHTS];
    _shadowTextures_h = new cudaTextureObject_t[MAX_LIGHTS];

    for (size_t i = 0; i < MAX_LIGHTS; i++)
    {
        _shadowArrays_h[i] = nullptr;
        _shadowTextures_h[i] = NULL;
    }

    cudaMalloc(&_shadowTextures_d, sizeof(cudaTextureObject_t) * MAX_LIGHTS);
}

GPURayCastingVolumeVisualizer::~GPURayCastingVolumeVisualizer()
{
    if (_materialTable_d != nullptr)
    {
        cudaFree(_materialTable_d);
    }

    if (_lightSettings_d != nullptr)
    {
        cudaFree(_lightSettings_d);
    }

    for (size_t i = 0; i < _numShadows; i++)
    {
        cudaDestroyTextureObject(_shadowTextures_h[i]);
        cudaFreeArray(_shadowArrays_h[i]);
    }

    delete[] _shadowArrays_h;
    delete[] _shadowTextures_h;

    cudaFree(_shadowTextures_d);
}

bool GPURayCastingVolumeVisualizer::DataChanged()
{
    return _memory->MemoryChanged();
}

void GPURayCastingVolumeVisualizer::UpdateShadowTexture(Camera* camera_d) 
{
    int sizeX = _datasetInfo.dataSizeX / _shadowSubsampling;
    int sizeY = _datasetInfo.dataSizeY / _shadowSubsampling;
    int sizeZ = _datasetInfo.dataSizeZ / _shadowSubsampling;

    // Create more arrays if light/s was/were created
    for (size_t i = _numShadows; i < _settings->lightSettings.numLights; i++)
    {
        cudaExtent extent = make_cudaExtent(sizeX, sizeY, sizeZ);
        cudaChannelFormatDesc channelDesc = cudaCreateChannelDesc<float4>();
        cudaMalloc3DArray(&_shadowArrays_h[i], &channelDesc, extent);
    }

    // Remove arrays if light/s was/were removed
    for (size_t i = _settings->lightSettings.numLights; i < _numShadows; i++)
    {
        cudaFreeArray(_shadowArrays_h[i]);
        
        if (_shadowTextures_h[i] != NULL) 
        {
            cudaDestroyTextureObject(_shadowTextures_h[i]);
            _shadowTextures_h[i] = NULL;
        }
    }

    _numShadows = _settings->lightSettings.numLights;

    for (size_t i = 0; i < _settings->lightSettings.numLights; i++)
    {
        // Shadows computation, texture is recreated if existed

        cudaTextureObject_t texture = _shadowTextures_h[i];
        cudaArray_t shadowArray = _shadowArrays_h[i];

        if (_shadowTextures_h[i] != NULL)
        {
            cudaDestroyTextureObject(_shadowTextures_h[i]);
            _shadowTextures_h[i] = NULL;
        }     

        dim3 blockSize(4, 4, 4);
        dim3 gridSize((sizeX + blockSize.x - 1) / blockSize.x, (sizeY + blockSize.y - 1) / blockSize.y, (sizeZ + blockSize.z - 1) / blockSize.z);

        cudaResourceDesc resDesc = {};
        resDesc.resType = cudaResourceTypeArray;
        resDesc.res.array.array = shadowArray;

        cudaSurfaceObject_t surfObj;
        cudaCreateSurfaceObject(&surfObj, &resDesc);
        auto& light = _settings->lightSettings.lights[i];
        ComputeShadowsKernel << <gridSize, blockSize >> > (_memory->GetTextureDevicePtr(), surfObj, camera_d, Vector3f(light.position[0], light.position[1], light.position[2]), Vector3i(sizeX, sizeY, sizeZ), _shadowSubsampling, _materialTable_d, _settings->materialTable.table.size());
        cudaDeviceSynchronize();

        cudaTextureDesc texDesc = {};
        texDesc.addressMode[0] = cudaAddressModeClamp;
        texDesc.addressMode[1] = cudaAddressModeClamp;
        texDesc.addressMode[2] = cudaAddressModeClamp;
        texDesc.filterMode = cudaFilterModeLinear;
        texDesc.readMode = cudaReadModeElementType;
        texDesc.normalizedCoords = 0;

        cudaCreateTextureObject(&_shadowTextures_h[i], &resDesc, &texDesc, nullptr);
        cudaDestroySurfaceObject(surfObj);
    }   

    cudaMemcpy(_shadowTextures_d, _shadowTextures_h, sizeof(cudaTextureObject_t) * MAX_LIGHTS, cudaMemcpyHostToDevice);
}

void GPURayCastingVolumeVisualizer::UpdateEntities(Camera* camera_d, bool fast)
{
    _memory->Update();

    bool recomputeShadows = false;
    if (_memory->VersionId() != _memoryVersion && !fast)
    {
        _memoryVersion = _memory->VersionId();
        recomputeShadows = true;
    }
    if (_settings->VersionId() != _settingsDeviceVersion)
    {
        _settings->materialTable.RecomputeDeltas();
        if (_materialTable_d != nullptr) 
        {
            cudaFree(_materialTable_d);
        }

        _materialTable_d = _settings->materialTable.CreateTableOnDevice();
        _settingsDeviceVersion = _settings->VersionId();


        if (_lightSettings_d != nullptr)
        {
            cudaFree(_lightSettings_d);
        }

        cudaMalloc(&_lightSettings_d, sizeof(LightSettings));
        cudaMemcpy(_lightSettings_d, &_settings->lightSettings, sizeof(LightSettings), cudaMemcpyHostToDevice);

        recomputeShadows = true;
    }

    if (recomputeShadows)
    {
        UpdateShadowTexture(camera_d);
    }
}

void GPURayCastingVolumeVisualizer::ComputeFrameInternal(std::shared_ptr<unsigned char[]>& framebuffer, bool fast, const std::shared_ptr<MultiLayeredFramebufferBase>& multiLayeredFramebuffer)
{ 
    Vector2i screenSize = _camera->GetScreenSize();

    long width = screenSize[0];
    long height = screenSize[1];

    unsigned char* d_image = nullptr;

    Camera* d_camera = nullptr;

    // GPU framebuffer
    cudaMalloc((void**)&d_image, width * height * 4 * sizeof(unsigned char));
    cudaMalloc((void**)&d_camera, sizeof(Camera));
    // GPU camera
    cudaMemcpy(d_camera, _camera.get(), sizeof(Camera), cudaMemcpyHostToDevice);

    // Update larger entities (textures etc.) 
    // Updates only when something has changed
    UpdateEntities(d_camera, fast);

    // Blocks/Grid definition
    dim3 blockSize(16, 16);
    dim3 gridSize((width + blockSize.x - 1) / blockSize.x, (height + blockSize.y - 1) / blockSize.y);
    
    // Computation
    ComputeFrameKernel<<<gridSize, blockSize>>>(d_image, width, height, d_camera, _memory->GetTextureDevicePtr(), Vector3f(_datasetInfo.dataSizeX, _datasetInfo.dataSizeY, _datasetInfo.dataSizeZ), _materialTable_d, _settings->materialTable.table.size(), _shadowTextures_d, _shadowSubsampling, _lightSettings_d);
    cudaDeviceSynchronize();

    // Copy framebuffer back to CPU (not great, but needed for CPU renderer compatibility)
    cudaMemcpy(framebuffer.get(), d_image, width * height * 4 * sizeof(unsigned char), cudaMemcpyDeviceToHost);

    // Delete framebufer
    cudaFree(d_image);
    cudaFree(d_camera);
}
