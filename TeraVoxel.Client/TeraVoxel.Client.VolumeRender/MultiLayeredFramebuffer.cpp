#include "MultiLayeredFramebuffer.h"

MultiLayeredFramebuffer::MultiLayeredFramebuffer(uint16_t width, uint16_t height) : _width(width), _height(height), _layers(), _mainLayer(width, height)
{

}

void MultiLayeredFramebuffer::Resize(uint16_t width, uint16_t height)
{
	_mainLayer = FramebufferLayer(width, height);
}

void MultiLayeredFramebuffer::Clear()
{
	_mainLayer.Clear();
}