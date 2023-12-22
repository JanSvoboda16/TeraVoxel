#include "MultiLayeredFramebuffer.h"

MultiLayeredFramebuffer::MultiLayeredFramebuffer(uint16_t width, uint16_t height): _width(width), _height(height), _layers(), _mainLayer(width, height)
{
	
}

void MultiLayeredFramebuffer::Resize(uint16_t width, uint16_t height)
{
	_mainLayer = FramebufferLayer(width, height);
}

void MultiLayeredFramebuffer::SetValue(uint16_t x, uint16_t y, uint8_t r, uint8_t g, uint8_t b, uint8_t a, float depth)
{
	_mainLayer.SetValue(x, y, r, g , b , a , depth);
}

void MultiLayeredFramebuffer::SetValue(uint16_t x, uint16_t y, const Fragment& fragment)
{
	_mainLayer.SetValue(x , y, fragment);
}

void MultiLayeredFramebuffer::Clear()
{
	_mainLayer.Clear();
}

std::vector<Fragment> MultiLayeredFramebuffer::GetFragmentsOrdered(uint16_t x, uint16_t y)
{
	std::vector<Fragment> fragments;
	fragments.push_back(_mainLayer.GetValue(x, y));
	return fragments;
}
