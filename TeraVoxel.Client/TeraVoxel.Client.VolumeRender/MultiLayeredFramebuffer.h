#pragma once
#include <memory>
#include <vector>
#include "FramebufferLayer.h"


class MultiLayeredFramebuffer
{
	std::vector<std::vector<std::shared_ptr<FramebufferLayer>>> _layers;
	FramebufferLayer _mainLayer;
	uint16_t _width, _height, _tileSize=16;

public:
	MultiLayeredFramebuffer(uint16_t width, uint16_t height);
	void Resize(uint16_t width, uint16_t height);
	uint16_t GetWidth() { return _width; }
	uint16_t GetHeight() { return _height; }
	void SetValue(uint16_t x, uint16_t y, uint8_t r, uint8_t g, uint8_t b, uint8_t a, float depth);
	void SetValue(uint16_t x, uint16_t y, const Fragment& fragment);
	void Clear();
	std::vector<Fragment> GetFragmentsOrdered(uint16_t x, uint16_t y);
};


__forceinline void MultiLayeredFramebuffer::SetValue(uint16_t x, uint16_t y, uint8_t r, uint8_t g, uint8_t b, uint8_t a, float depth)
{
	_mainLayer.SetValue(x, y, r, g, b, a, depth);
}

__forceinline void MultiLayeredFramebuffer::SetValue(uint16_t x, uint16_t y, const Fragment& fragment)
{
	_mainLayer.SetValue(x, y, fragment);
}

__forceinline std::vector<Fragment> MultiLayeredFramebuffer::GetFragmentsOrdered(uint16_t x, uint16_t y)
{
	std::vector<Fragment> fragments;
	auto fragment = _mainLayer.GetValue(x, y);
	
	if (fragment.depth >= -1.f)
	{
		fragments.push_back(fragment);
	}
	
	return fragments;
}