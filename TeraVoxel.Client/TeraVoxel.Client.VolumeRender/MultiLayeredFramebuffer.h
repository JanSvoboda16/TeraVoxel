#pragma once
#include <memory>
#include <vector>
#include "FramebufferLayer.h"
#include <algorithm>


class MultiLayeredFramebuffer
{
	std::vector<FramebufferLayer> _alphaLayers;
	FramebufferLayer _mainLayer;
	uint16_t _width, _height, _tileSize = 16, _usedAlphaLayers = 0;

public:
	MultiLayeredFramebuffer(uint16_t width, uint16_t height);;
	void Resize(uint16_t width, uint16_t height);
	uint16_t GetWidth() { return _width; }
	uint16_t GetHeight() { return _height; }
	void SetValue(uint16_t x, uint16_t y, uint8_t r, uint8_t g, uint8_t b, uint8_t a, float depth);
	void SetValue(uint16_t x, uint16_t y, const Fragment& fragment);
	void Clear();
	std::vector<Fragment> GetFragmentsOrdered(uint16_t x, uint16_t y);
};

inline MultiLayeredFramebuffer::MultiLayeredFramebuffer(uint16_t width, uint16_t height) :
	_width(width), _height(height), _alphaLayers(), _mainLayer(width, height)
{
}

__forceinline void MultiLayeredFramebuffer::SetValue(uint16_t x, uint16_t y, uint8_t r, uint8_t g, uint8_t b, uint8_t a, float depth)
{
	if (a >= 253)
	{
		_mainLayer.SetValue(x,y,r,g,b,a,depth);
	}
	else
	{
		// Behind not transparent fragment
		if (_mainLayer.GetValue(x, y).depth < depth)
		{
			return;
		}

		int layerIndex = 0;
		while (true)
		{
			//float enabledDepthError = (2 / ((depth + 1.f))*10) * 0.001;
			if (layerIndex + 1 > _alphaLayers.size())
			{
				_alphaLayers.push_back(FramebufferLayer(_width, _height));
				_alphaLayers[layerIndex].SetValue(x, y, r, g, depth*b, a, depth);
				break;
			}
			else
			{
				auto& layer = _alphaLayers[layerIndex];
				auto layerValue = layer.GetValue(x, y);

				if (layerValue.depth > 1.f)
				{
					layer.SetValue(x, y, r, g, depth*b, a, depth);
					break;
				}
				else
				{
					layerIndex++;
				}
			}			
		}
		if (layerIndex + 1 > _usedAlphaLayers)
		{
			_usedAlphaLayers = layerIndex + 1;
		}
	}
}

__forceinline void MultiLayeredFramebuffer::SetValue(uint16_t x, uint16_t y, const Fragment& fragment)
{
	SetValue(x, y, fragment.r, fragment.g, fragment.b, fragment.a, fragment.depth);
}

__forceinline void SortFragments(std::vector<Fragment>& fragments)
{
	int n = fragments.size();
	for (int i = 0; i < n - 1; i++)
	{
		int minIndex = i;
		for (int j = i + 1; j < n; j++)
		{
			if (fragments[j].depth < fragments[minIndex].depth)
			{
				minIndex = j;
			}
		}
		if (minIndex != i)
		{
			std::swap(fragments[i], fragments[minIndex]);
		}
	}
}

__forceinline std::vector<Fragment> MultiLayeredFramebuffer::GetFragmentsOrdered(uint16_t x, uint16_t y)
{
	// pokud alfavrstev je 0, tak vrat hodnotu, jinak nacti vsechny hodnoty a serad je

	std::vector<Fragment> fragments;
	if (_alphaLayers.size() == 0)
	{
		auto fragment = _mainLayer.GetValue(x, y);
		if (fragment.depth <= 1.f)
		{
			fragments.push_back(fragment);
		}
	}
	else
	{
		auto fragment = _mainLayer.GetValue(x, y);
		if (fragment.depth <= 1.f)
		{
			fragments.push_back(fragment);
		}

		for (size_t i = 0; i < _usedAlphaLayers; i++)
		{
			fragment = _alphaLayers[i].GetValue(x, y);
			if (fragment.depth <= 1.f)
			{
				fragments.push_back(fragment);
			}
		}

		SortFragments(fragments);
	}	
	
	
	return fragments;
}

inline void MultiLayeredFramebuffer::Resize(uint16_t width, uint16_t height)
{
	_mainLayer.Resize(width, height);

	for (auto& layer: _alphaLayers)
	{
		layer.Resize(width, height);
	}

	_width = width;
	_height = height;
	Clear();
}

inline void MultiLayeredFramebuffer::Clear()
{
	_mainLayer.Clear();
	for (size_t i = 0; i < _usedAlphaLayers; i++)
	{
		_alphaLayers[i].Clear();
	}

	int alphaLayersSize = _alphaLayers.size();
	for (int i = 0; i < alphaLayersSize - _usedAlphaLayers; i++)
	{
		_alphaLayers.pop_back();
	}
	_usedAlphaLayers = 0;
}
