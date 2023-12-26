#include "FramebufferLayer.h"

void FramebufferLayer::Clear()
{
	std::memset(_r.get(), 0, _width * _height * sizeof(uint8_t));
	std::memset(_g.get(), 0, _width * _height * sizeof(uint8_t));
	std::memset(_b.get(), 0, _width * _height * sizeof(uint8_t));
	std::memset(_a.get(), 0, _width * _height * sizeof(uint8_t));
	for (int i = 0; i < _width * _height; ++i)
	{
		_depth.get()[i] = -2.f;
	}
}