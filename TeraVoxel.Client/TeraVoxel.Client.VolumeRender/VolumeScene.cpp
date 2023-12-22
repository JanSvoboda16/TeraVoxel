/*
 * Author: Jan Svoboda
 * University: BRNO UNIVERSITY OF TECHNOLOGY, FACULTY OF INFORMATION TECHNOLOGY
 */
#include "pch.h"
#include "VolumeScene.h"

template <typename T>
VolumeScene<T>::VolumeScene(const std::shared_ptr<Camera>& camera, const ProjectInfo& projectInfo, const std::shared_ptr<VolumeLoaderFactory<T>>& volumeLoaderFactory, const std::shared_ptr<IVolumeVisualizerFactory<T>>& visualizerFac) //TODO new constructor
{
	_projectInfo = projectInfo;
	_camera = camera;
	_visualizer = visualizerFac->Create(camera, projectInfo, volumeLoaderFactory);
	_volumeLoaderFactory = volumeLoaderFactory;
}

template<typename T>
VolumeScene<T>::~VolumeScene()
{
	if (_renderingThread.valid())
	{
		_renderingThread.wait();
	}
}

template <typename T>
void VolumeScene<T>::ComputeFrame(int width, int height, bool _fast)
{
	_framebufferIndex = !_framebufferIndex;

	int* frameWidth;
	int* frameHeight;
	if (_framebufferIndex)
	{
		frameWidth = &frameWidth1;
		frameHeight = &frameHeight1;
	}
	else
	{
		frameWidth = &frameWidth2;
		frameHeight = &frameHeight2;
	}

	// Creating a framebuffer with sizes of a frame if actual sizes doesn't match
	if (*frameWidth != width || *frameHeight != height)
	{
		*frameWidth = width;
		*frameHeight = height;
		if (_framebufferIndex)
		{
			_framebuffer1 = std::shared_ptr<unsigned char[]>(new unsigned char[width * height * 4]);
		}
		else
		{
			_framebuffer2 = std::shared_ptr<unsigned char[]>(new unsigned char[width * height * 4]);
		}
		_camera->ChangeScreenSize(width, height);
	}

	_frameReady.store(false, std::memory_order::relaxed);
	_renderingInProgress.store(true, std::memory_order::relaxed);

	if (_visualizerChanged)
	{
		_visualizer = _visualizerFactory->Create(_camera, _projectInfo, _volumeLoaderFactory);
		_visualizerChanged = false;
	}

	_renderingThread = std::async(std::launch::async, &VolumeScene<T>::ComputeFrameTask, this, width, height, _fast);
}

template <typename T>
int VolumeScene<T>::GetFrameWidth()
{
	return _framebufferIndex ? frameWidth1 : frameWidth2;
}

template <typename T>
int VolumeScene<T>::GetFrameHeight()
{
	return _framebufferIndex ? frameHeight1 : frameHeight2;
}

template <typename T>
bool VolumeScene<T>::DataChanged()
{
	return _visualizer->DataChanged();
}

template <typename T>
std::shared_ptr<unsigned char[]> VolumeScene<T>::GetFrame()
{
	if (!_frameReady)
	{
		//TODO
	}
	return  _framebufferIndex ? _framebuffer1 : _framebuffer2;
}

template<typename T>
inline std::shared_ptr<Camera> VolumeScene<T>::GetCamera()
{
	return _camera;
}

template<typename T>
bool VolumeScene<T>::FrameReady()
{
	return _frameReady.load(std::memory_order::acquire);
}

template<typename T>
bool VolumeScene<T>::RenderingInProgress()
{
	return _renderingInProgress;
}

template<typename T>
void VolumeScene<T>::ChangeVisualizer(std::shared_ptr<IVolumeVisualizerFactory<T>> visualizerFac)
{
	_visualizerFactory = visualizerFac;
	_visualizerChanged = true;
}

template<typename T>
const char* VolumeScene<T>::GetDataTypeName()
{
	return TypeToString::ToString<T>();
}

template<typename T>
void VolumeScene<T>::ComputeFrameTask(int width, int height, bool _fast)
{
	Logger::GetInstance()->LogEvent("VolumeScene", "Rendering:Started", "", _fast ? "fast" : "full");
	
	auto meshObject = std::make_shared<MeshObject>();
	
	Mesh mesh;
	mesh.Data().push_back({Vector3f(0,0,0),Vector4b(255,0,0,255)});
	mesh.Data().push_back({Vector3f(0,255,0),Vector4b(0,255,0,255)});
	mesh.Data().push_back({Vector3f(255,0,0),Vector4b(0,0,255,255)});
	mesh.Data().push_back({Vector3f(255,255,0),Vector4b(0,0,255,255) });
	mesh.SetMode(MeshMode::Strip);
	meshObject->meshes.push_back(mesh);
	meshObject->transformation = Matrix4f::Identity();

	
	_visualizer->ComputeFrame(_framebufferIndex ? _framebuffer1 : _framebuffer2, width, height, _fast ? 2 : 1);

	CPUMeshVisualizer visualizer(meshObject, _camera);
	visualizer.ComputeFrame();
	auto meshBuffer = visualizer.GetFrameBuffer();

	for (size_t x = 0; x < width; x++)
	{
		for (size_t y = 0; y < height; y++)
		{
			auto framebuffer = _framebufferIndex ? _framebuffer1 : _framebuffer2;
			auto value = meshBuffer->GetFragmentsOrdered(x, y)[0];
			if (value.a != 0)
			{
				framebuffer[(x + y * width) * 4] = value.r;
				framebuffer[(x + y * width) * 4 + 1] = value.g;
				framebuffer[(x + y * width) * 4 + 2] = value.b;
				framebuffer[(x + y * width) * 4 + 3] = value.a;
			}
		}
	}

	_frameReady.store(true, std::memory_order::release);
	_renderingInProgress.store(false, std::memory_order::release);
	Logger::GetInstance()->LogEvent("VolumeScene", "Rendering:Ended", "", _fast ? "fast" : "full");
}

template VolumeScene<uint8_t>;
template VolumeScene<uint16_t>;
template VolumeScene<uint32_t>;
template VolumeScene<uint64_t>;
template VolumeScene<float>;
template VolumeScene<double>;
template VolumeScene<int8_t>;
template VolumeScene<int16_t>;
template VolumeScene<int32_t>;
template VolumeScene<int64_t>;
