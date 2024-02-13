#include "FramebufferLayer.h"

void FramebufferLayer::Clear()
{
	auto itemCount = _width * _height;
	std::memset(_r.data(), 0, itemCount * sizeof(uint8_t));
	std::memset(_g.data(), 0, itemCount * sizeof(uint8_t));
	std::memset(_b.data(), 0, itemCount * sizeof(uint8_t));
	std::memset(_a.data(), 0, itemCount * sizeof(uint8_t));

	for (int i = 0; i < itemCount; ++i)
	{
		_depth[i] = 2.f;
	}
}

void FramebufferLayer::Resize(int width, int height)
{
	if (width * height > _width * _height)
	{
		_r.resize(width * height);
		_g.resize(width * height);
		_b.resize(width * height);
		_a.resize(width * height);
		_depth.resize(width * height);
	}
	_width = width;
	_height = height;
}
