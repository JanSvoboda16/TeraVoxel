#pragma once
#include "../TeraVoxel.Client.Core/TypeToString.h"

template <class Concrete>
class VolumeVisualizerSetter
{
public:
	static void Set(std::shared_ptr<IVolumeScene> scene, std::shared_ptr<VolumeVisualizerSettingsBase> settings)
	{
		const char* typeName = scene->GetDataTypeName();

		if (!strcmp(typeName, STRING_INT8_T))
		{
			Concrete::template Set<int8_t>(scene, settings);
		}
		else if (!strcmp(typeName, STRING_UINT8_T))
		{
			Concrete::template Set<uint8_t>(scene, settings);
		}
		else if (!strcmp(typeName, STRING_INT16_T))
		{
			Concrete::template Set<int16_t>(scene, settings);
		}
		else if (!strcmp(typeName, STRING_UINT16_T))
		{
			Concrete::template Set<uint16_t>(scene, settings);
		}
		else if (!strcmp(typeName, STRING_INT32_T))
		{
			Concrete::template Set<int32_t>(scene, settings);
		}
		else if (!strcmp(typeName, STRING_UINT32_T))
		{
			Concrete::template Set<uint32_t>(scene, settings);
		}
		else if (!strcmp(typeName, STRING_FLOAT_T))
		{
			Concrete::template Set<float>(scene, settings);
		}
		else if (!strcmp(typeName, STRING_DOUBLE_T))
		{
			Concrete::template Set<double>(scene, settings);
		}
		else if (!strcmp(typeName, STRING_INT64_T))
		{
			Concrete::template Set<int64_t>(scene, settings);
		}
		else if (!strcmp(typeName, STRING_UINT64_T))
		{
			Concrete::template Set<uint64_t>(scene, settings);
		}
		else
		{
			throw std::template exception("Unknown data type");
		}
	}
};

