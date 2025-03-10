#pragma once
#include <cstdint>
#include <immintrin.h>
#include <Eigen/Dense>

constexpr unsigned int Z_CURVE_MASK = 0b1001001001001001001001001001;

class Serialization
{
public:
	static __forceinline uint_fast32_t GetZCurveIndex(uint_fast16_t xpos, uint_fast16_t ypos, uint_fast16_t zpos)
	{
		return _pdep_u32(xpos, Z_CURVE_MASK) | _pdep_u32(ypos, Z_CURVE_MASK << 1) | _pdep_u32(zpos, Z_CURVE_MASK << 2);
	}

	static __forceinline Eigen::Vector3i IdxToXYZ(uint64_t index, Eigen::Vector3i dims)
	{
		return Eigen::Vector3i(
			index % dims[0],
			(index % (dims[0] * dims[1])) / dims[0],
			index / (dims[0] * dims[1])
		);
	}
};

