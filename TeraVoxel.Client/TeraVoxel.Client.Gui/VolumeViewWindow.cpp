/*
 * Author: Jan Svoboda
 * University: BRNO UNIVERSITY OF TECHNOLOGY, FACULTY OF INFORMATION TECHNOLOGY
 */
#include "VolumeViewWindow.h"
#include "../TeraVoxel.Client.VolumeRender/MeshTreeExplorer.h"
#include "imgui.h"
#include "time.h"
#include "imgui_stdlib.h"

void VolumeViewWindow::RGBAToTexture(const unsigned char* data, ID3D11ShaderResourceView** out_srv, int  width, int height)
{
	// This function is based on official Dear ImGui example:
	// URL: https://github.com/ocornut/imgui/wiki/Image-Loading-and-Displaying-Examples

	// Create texture
	D3D11_TEXTURE2D_DESC desc;
	ZeroMemory(&desc, sizeof(desc));
	desc.Width = width;
	desc.Height = height;
	desc.MipLevels = 1;
	desc.ArraySize = 1;
	desc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
	desc.SampleDesc.Count = 1;
	desc.Usage = D3D11_USAGE_DEFAULT;
	desc.BindFlags = D3D11_BIND_SHADER_RESOURCE;
	desc.CPUAccessFlags = 0;

	ID3D11Texture2D* pTexture = NULL;
	D3D11_SUBRESOURCE_DATA subResource;
	subResource.pSysMem = data;
	subResource.SysMemPitch = desc.Width * 4;
	subResource.SysMemSlicePitch = 0;
	g_pd3dDevice->CreateTexture2D(&desc, &subResource, &pTexture);

	// Create texture
	D3D11_SHADER_RESOURCE_VIEW_DESC srvDesc;
	ZeroMemory(&srvDesc, sizeof(srvDesc));
	srvDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
	srvDesc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2D;
	srvDesc.Texture2D.MipLevels = desc.MipLevels;
	srvDesc.Texture2D.MostDetailedMip = 0;
	g_pd3dDevice->CreateShaderResourceView(pTexture, &srvDesc, out_srv);
	pTexture->Release();
}

void VolumeViewWindow::Update()
{
	ImGui::Begin("Scene");

	if (_volumeViewContext->scene != nullptr)
	{
		// MOUSE MOVEMENTS
		ImGuiIO& io = ImGui::GetIO();
		if (ImGui::IsWindowHovered())
		{
			if (io.MouseDown[0])
			{
				_yAngleDelta += io.MouseDelta.x;
				_xAngleDelta += io.MouseDelta.y;
				_rerender = true;
				_fast = true;
			}
			if (io.MouseWheel != 0)
			{
				_wheelDelta += io.MouseWheel;
				_rerender = true;
				_fast = true;
			}
			if (io.MouseDown[1])
			{
				_zCenterDelta += io.MouseDelta.y;
				_rerender = true;
				_fast = true;
			}
			if (io.MouseDown[2])
			{
				_xCenterDelta += io.MouseDelta.x;
				_yCenterDelta += io.MouseDelta.y;
				_rerender = true;
				_fast = true;
			}

			// VIEW DIRECTION
			static bool VKeyLasDown = false;
			if (io.KeysDown['V'])
			{
				if (!VKeyLasDown)
				{
					VKeyLasDown = true;
					if (_axisRotate)
					{
						if (_observerAxis == 'y')
						{
							_observerAxis = 'x';
						}
						else if (_observerAxis == 'x')
						{
							_observerAxis = 'z';
						}
						else if (_observerAxis == 'z')
						{
							_observerAxis = 'y';
						}
						_axisRotate = false;
					}
					else
					{
						_axisRotate = true;
					}
					_rerender = true;
				}
			}
			else
			{
				VKeyLasDown = false;
			}
		}

		int frameWidth = ImGui::GetWindowContentRegionWidth();
		int frameHeight = ImGui::GetWindowHeight() - 100;

		// SCREEN SIZE HAS CHANGED 
		if ((frameWidth != _lastFrameWidth) || (frameHeight != _lastFrameHeight))
		{
			_framebuffer = nullptr;
			_lastFrameHeight = frameHeight;
			_lastFrameWidth = frameWidth;
			_rerender = true;
		}

		auto scene = _volumeViewContext->scene;
		if (scene->FrameReady())
		{
			if ((scene->GetFrameHeight() == frameHeight) && (scene->GetFrameWidth() == frameWidth))
			{
				_framebuffer = scene->GetFrame();
				_framesCount++;				
			}
		}

		if (!scene->RenderingInProgress())
		{
			_volumeViewContext->sceneEditable.Notify();
			if (_volumeViewContext->scene->DataChanged() || _rerender)
			{
				scene->GetCamera()->ChangeObserverAxis(_observerAxis, _axisRotate);

				scene->GetCamera()->Observe(-_xAngleDelta / 100, -_yAngleDelta / 100, -_wheelDelta * 100, -_xCenterDelta, -_yCenterDelta, -_zCenterDelta);
				_yAngleDelta = 0;
				_xAngleDelta = 0;
				_wheelDelta = 0;
				_xCenterDelta = 0;
				_yCenterDelta = 0;
				_zCenterDelta = 0;

				scene->ComputeFrame(frameWidth, frameHeight, _fast);				

				if (_fast)
				{
					_rerender = true;
				}
				else
				{
					_rerender = false;
				}
				_fast = false;
			}
		}

		// RELEASE OLD VIEW
		if (_view != NULL)
		{
			_view->Release();
			_view = NULL;
		}

		// SHOW FRAME
		if (_framebuffer != nullptr)
		{
			RGBAToTexture(_framebuffer.get(), &_view, frameWidth, frameHeight);

			ImGui::Image((void*)_view, ImVec2(frameWidth, frameHeight));
		}

		// FPS AND SCREEN SIZE
		//std::string fpsLabel = "FPS:" + std::to_string(fps);
		//ImGui::Text(fpsLabel.c_str());
	}

	// FRAME-COUNTER
	if (clock() - _fps_start > 45000)
	{
		_fps = _framesCount / ((clock() - _fps_start) / 1000.0);
		_fps_start = clock();
		_framesCount = 0;
	}

	ImGui::End();
}