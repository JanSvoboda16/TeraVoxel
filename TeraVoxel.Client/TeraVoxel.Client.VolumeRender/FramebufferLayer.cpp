#include "FramebufferLayer.h"

void FramebufferLayer::SetValue(uint16_t x, uint16_t y, uint8_t r, uint8_t g, uint8_t b, uint8_t a, float depth)
{
	auto index = y * _width + x;
	
	if (_depth[index] < -1.f || depth <= _depth[index])
	{
		_r[index] = r;
		_g[index] = g;
		_b[index] = b;
		_a[index] = a;
		_depth[index] = depth;
	}
}

void FramebufferLayer::SetValue(uint16_t x, uint16_t y, const Fragment& fragment)
{
	auto index = y * _width + x;

	if (_depth[index] < -1.f || fragment.depth <= _depth[index])
	{
		_r[index] = fragment.r;
		_g[index] = fragment.g;
		_b[index] = fragment.b;
		_a[index] = fragment.a;
		_depth[index] = fragment.depth;
	}
}

void FramebufferLayer::Clear()
{
	std::memset(_r.get(), 0, _width * _height * sizeof(uint8_t));
	std::memset(_g.get(), 0, _width * _height * sizeof(uint8_t));
	std::memset(_b.get(), 0, _width * _height * sizeof(uint8_t));
	std::memset(_a.get(), 0, _width * _height * sizeof(uint8_t));
	std::memset(_depth.get(), -2.f, _width * _height * sizeof(float));
}

Fragment FramebufferLayer::GetValue(uint16_t x, uint16_t y)
{
	auto index = y * _width + x;

	Fragment fragment;
	
	fragment.r = _r[index];
	fragment.g = _g[index];
	fragment.b = _b[index];
	fragment.a = _a[index];
	fragment.depth = _depth[index];

	return fragment;
}
