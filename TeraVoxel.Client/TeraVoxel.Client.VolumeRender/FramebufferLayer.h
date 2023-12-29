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

	/// <summary>
	/// Sets value to the internal buffer if fragment is in front of the old value. 
	/// </summary>
	/// <param name="x"></param>
	/// <param name="y"></param>
	/// <param name="r"></param>
	/// <param name="g"></param>
	/// <param name="b"></param>
	/// <param name="a"></param>
	/// <param name="depth"></param>
	void SetValue(uint16_t x, uint16_t y, uint8_t r, uint8_t g, uint8_t b, uint8_t a, float depth);

	/// <summary>
	/// Sets a value to the internal buffer if the fragment is in front of the old one. 
	/// </summary>
	/// <param name="x"></param>
	/// <param name="y"></param>
	/// <param name="fragment"></param>
	void SetValue(uint16_t x, uint16_t y, const Fragment &fragment);

	/// <summary>
	/// Clears buffer.
	/// Depth is set to 2.0f;
	/// </summary>
	void Clear();

	/// <summary>
	/// Changes the virtual size of the buffer (real buffer can be larger)
	/// </summary>
	/// <param name="width"></param>
	/// <param name="height"></param>
	void Resize(int width, int height);

	/// <summary>
	/// Returns a fragment on a given position. 
	/// </summary>
	/// <param name="x"></param>
	/// <param name="y"></param>
	/// <returns></returns>
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