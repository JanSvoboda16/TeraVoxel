#pragma once
#include <unordered_map>
#include <string>
#include <typeinfo>
#include <cstdint>
constexpr auto STRING_INT8_T = "System.Sbyte";
constexpr auto STRING_UINT8_T = "System.Byte";
constexpr auto STRING_INT16_T = "System.Int16";
constexpr auto STRING_UINT16_T = "System.UInt16";
constexpr auto STRING_INT32_T = "System.Int32";
constexpr auto STRING_UINT32_T = "System.UInt32";
constexpr auto STRING_INT64_T = "System.Int64";
constexpr auto STRING_UINT64_T = "System.UInt64";
constexpr auto STRING_FLOAT_T = "System.Single";
constexpr auto STRING_DOUBLE_T = "System.Double";

static std::unordered_map<std::size_t, const char*> typeStrings = {
	{typeid(int8_t).hash_code(), STRING_INT8_T},
	{typeid(uint8_t).hash_code(), STRING_UINT8_T},
	{typeid(int16_t).hash_code(), STRING_INT16_T},
	{typeid(uint16_t).hash_code(), STRING_UINT16_T},
	{typeid(int32_t).hash_code(), STRING_INT32_T},
	{typeid(uint32_t).hash_code(), STRING_UINT32_T},
	{typeid(float).hash_code(), STRING_FLOAT_T},
	{typeid(double).hash_code(), STRING_DOUBLE_T},
	{typeid(int64_t).hash_code(), STRING_INT64_T},
	{typeid(uint64_t).hash_code(), STRING_UINT64_T}
};

class TypeToString
{

public:
	template <typename T>
	static const char* ToString()

	{
		auto result = typeStrings.find(typeid(T).hash_code());
		return result->second;
	}

};

