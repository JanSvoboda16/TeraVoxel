/*
 * Author: Jan Svoboda
 * University: BRNO UNIVERSITY OF TECHNOLOGY, FACULTY OF INFORMATION TECHNOLOGY
 */
#pragma once
#include <d3d11.h>
#include <d3dcompiler.h>
#include "../TeraVoxel.Client.VolumeRender/Camera.h"
#include "../TeraVoxel.Client.VolumeRender/CPURayCastingVolumeObjectMemory.h"
#include "../TeraVoxel.Client.VolumeRender/VolumeScene.h"
#include "../TeraVoxel.Client.VolumeRender/ColorMappingTable.h"
#include "../TeraVoxel.Client.VolumeRender/NetMemoryVolumeSceneFactory.h"
#include "../VolumeViewContext.h"
#include <future>

class VolumeViewWindow
{
public:
	VolumeViewWindow(ID3D11Device* g_pd3dDevice, std::shared_ptr<VolumeViewContext> volumeViewContext)
	{
		this->g_pd3dDevice = g_pd3dDevice;
		_volumeViewContext = volumeViewContext;

		// if the scene has changed, the view must be rerendered
		_volumeViewContext->sceneUpdated.Register([this]() { _rerender = true; });
		_volumeViewContext->sceneReplaced.Register([this]() { _rerender = true; });

	}
	void RGBAToTexture(const unsigned char* _renderingFramebuffer, ID3D11ShaderResourceView** out_srv, int width, int height);
	void Update();

private:
	ID3D11Device* g_pd3dDevice;
	ID3D11ShaderResourceView* _view = NULL;

	std::shared_ptr<CPURCVolumeVisualizerSettings> _visualizerSettings = std::make_shared<CPURCVolumeVisualizerSettings>();

	std::shared_ptr<VolumeViewContext> _volumeViewContext;

	std::atomic<unsigned char*> _renderingFramebuffer = nullptr; // Constains a pointer to the rendered image
	std::future<void> _renderingAwaiter; // Awaiter for rendering thread
	std::atomic<bool> _frameGenerated = false;
	std::shared_ptr<unsigned char[]> _framebuffer = nullptr; // Constains a pointer to the showed image

	// State variables
	int _lastFrameWidth = 0;
	int _lastFrameHeight = 0;
	bool _fast = false;
	char _observerAxis = 'y';
	bool _axisRotate = false;
	bool _rerender = false;

	// Deltas for orbiter controll
	float _xAngleDelta = 0,
		_yAngleDelta = 0,
		_wheelDelta = 0,
		_xCenterDelta = 0,
		_yCenterDelta = 0,
		_zCenterDelta = 0;

	clock_t _fps_start = 0;
	int _framesCount;
	float _fps = 0;
};

