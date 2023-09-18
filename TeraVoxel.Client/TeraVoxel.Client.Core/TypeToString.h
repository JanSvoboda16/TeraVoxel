#pragma once
#include <unordered_map>
#include <string>
#include <typeinfo>
#include <cstdint>
#define STRING_INT8_T "System.Sbyte"
#define STRING_UINT8_T "System.Byte"
#define STRING_INT16_T "System.Int16"
#define STRING_UINT16_T "System.UInt16"
#define STRING_INT32_T "System.Int32"
#define STRING_UINT32_T "System.UInt32"
#define STRING_INT64_T "System.Int64"
#define STRING_UINT64_T "System.UInt64"
#define STRING_FLOAT_T "System.Single"
#define STRING_DOUBLE_T "System.Double"

class TypeToString
{
	inline static std::unordered_map<std::size_t, const char*> typeStrings = {
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


public:
	template <typename T>
	static const char* ToString()
	{
		auto result = typeStrings.find(typeid(T).hash_code());
		return result->second;
	}
};

