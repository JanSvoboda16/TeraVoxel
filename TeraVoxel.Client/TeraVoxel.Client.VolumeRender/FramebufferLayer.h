#pragma once
#include <memory>
#include <vector>

struct Fragment
{
	uint8_t r{};
	uint8_t g{};
	uint8_t b{};
	uint8_t a{};
	float depth{};

	Fragment() {};
	Fragment(uint8_t r, uint8_t g, uint8_t b, uint8_t a, float depth)
		: r(r), g(g), b(b), a(a), depth(depth)
	{
	};
};

class  FramebufferLayer
{
	std::vector<uint8_t> _r;
	std::vector<uint8_t> _g;
	std::vector<uint8_t> _b;
	std::vector<uint8_t> _a;
	std::vector<float> _depth;
	uint16_t _width;
	uint16_t _height;
	

public:

	FramebufferLayer(uint16_t width, uint16_t height) : _width(width), _height(height)
	{
		int itemCount = width * height;
		_r.resize(itemCount);
		_g.resize(itemCount);
		_b.resize(itemCount);
		_a.resize(itemCount);
		_depth.resize(itemCount);
		Clear();
	}

	void SetValue(uint16_t x, uint16_t y, uint8_t r, uint8_t g, uint8_t b, uint8_t a, float depth);
	void SetValue(uint16_t x, uint16_t y, const Fragment &fragment);
	void Clear();
	void Resize(int width, int height);
	Fragment GetValue(uint16_t x, uint16_t y);
	uint16_t GetWidht() { return _width; }
	uint16_t GetHeight() { return _height; }
};


__forceinline void FramebufferLayer::SetValue(uint16_t x, uint16_t y, uint8_t r, uint8_t g, uint8_t b, uint8_t a, float depth)
{
	auto index = y * _width + x;

	if (depth <= _depth[index])
	{
		_r[index] = r;
		_g[index] = g;
		_b[index] = b;
		_a[index] = a;
		_depth[index] = depth;
	}
}

__forceinline void FramebufferLayer::SetValue(uint16_t x, uint16_t y, const Fragment& fragment)
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

__forceinline Fragment FramebufferLayer::GetValue(uint16_t x, uint16_t y)
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