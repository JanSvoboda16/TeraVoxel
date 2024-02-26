#pragma once
#include <string>
#include "../TeraVoxel.Client.Core/TypeToString.h"

#define CALL_TEMPLATED_FUNCTION(functionName, typeName, ...) \
    ( \
        (!strcmp(typeName, STRING_INT8_T)) ? functionName<int8_t>(__VA_ARGS__) : \
        (!strcmp(typeName, STRING_UINT8_T)) ? functionName<uint8_t>(__VA_ARGS__) : \
        (!strcmp(typeName, STRING_INT16_T)) ? functionName<int16_t>(__VA_ARGS__) : \
        (!strcmp(typeName, STRING_UINT16_T)) ? functionName<uint16_t>(__VA_ARGS__) : \
        (!strcmp(typeName, STRING_INT32_T)) ? functionName<int32_t>(__VA_ARGS__) : \
        (!strcmp(typeName, STRING_UINT32_T)) ? functionName<uint32_t>(__VA_ARGS__) : \
        (!strcmp(typeName, STRING_FLOAT_T)) ? functionName<float>(__VA_ARGS__) : \
        (!strcmp(typeName, STRING_DOUBLE_T)) ? functionName<double>(__VA_ARGS__) : \
        (!strcmp(typeName, STRING_INT64_T)) ? functionName<int64_t>(__VA_ARGS__) : \
        (!strcmp(typeName, STRING_UINT64_T)) ? functionName<uint64_t>(__VA_ARGS__) : \
        functionName<uint8_t>(__VA_ARGS__) \
    )

