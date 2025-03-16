#include "TeraVoxel.Client.VolumeRenderer/NetVolumeLoader.h"
#include <NeuroVoxel/MultiHashDataReader.h>

template <typename T>
NetVolumeLoader<T>::NetVolumeLoader(const ProjectInfo& projectInfo, int threadCount, const ProjectManager& projectManager) : 
	VolumeLoaderBase<T>(projectInfo.ToBlockBasedDatasetInfo(), threadCount),
	_projectInfo(projectInfo)
{
	_projectManager = projectManager;
}

template <typename T>
NetVolumeLoader<T>::~NetVolumeLoader()
{
	// Must be called also here, because _project mnager would be destroyed before ending threads.
	this->_endLoopingThreads = true;
	this->_loadingTreads.clear();
}

template <typename T>
T* NetVolumeLoader<T>::LoadSegmentData(int x, int y, int z, int downscale)
{
	Logger::GetInstance()->LogEvent("NetVolumeLoader", "SegmentLoading:Started", "", std::format("{0},{1},{2},{3}", x, y, z, downscale));

	int downscaledSegmentSize = _projectInfo.segmentSize / (int)pow(2, downscale);
	int segmentSize = downscaledSegmentSize * downscaledSegmentSize * downscaledSegmentSize;
	std::vector<unsigned char> byteData;

	// Segment loading from the server
	try
	{
		byteData = _projectManager.GetSegment(_projectInfo.name, x, y, z, downscale, segmentSize * sizeof(T), _projectInfo.compressed);
	}
	catch (const std::exception& ex)
	{
		throw ex;
	}

	T* data = new T[segmentSize];

	bool sameEndianites = false;
	if ((_projectInfo.isLittleEndian && std::endian::native == std::endian::little)
		|| (!_projectInfo.isLittleEndian && std::endian::native == std::endian::big))
	{
		sameEndianites = true;
	}

	Logger::GetInstance()->LogEvent("NetVolumeLoader", "SegmentLoading:Reserialization:Started", "", std::format("{0},{1},{2},{3}", x, y, z, downscale));

	// Reserialization
	if (_projectInfo.zTransformed)
	{		
		if (sameEndianites)
		{
			for (int i = 0; i < segmentSize; i++)
			{
				unsigned char* value = reinterpret_cast<unsigned char*>(&data[i]);
				for (int j = 0; j < sizeof(T); j++)
				{
					value[j] = (unsigned char)byteData[i * sizeof(T) + j];
				}
			}
		}
		else
		{
			for (int i = 0; i < segmentSize; i++)
			{
				unsigned char* value = reinterpret_cast<unsigned char*>(&data[i]);
				for (int j = 0; j < sizeof(T); j++)
				{
					value[sizeof(T) - j - 1] = (unsigned char)byteData[i * sizeof(T) + j];
				}
			}
		}
	}
	else
	{
		for (int i = 0; i < segmentSize; i++)
		{
			uint_fast16_t zpos = i / (downscaledSegmentSize * downscaledSegmentSize);
			uint_fast32_t mod = i % (downscaledSegmentSize * downscaledSegmentSize);
			uint_fast16_t ypos = mod / downscaledSegmentSize;
			uint_fast16_t xpos = mod % downscaledSegmentSize;

			auto index = Serialization::GetZCurveIndex(xpos, ypos, zpos);
			data[index] = 0;

			unsigned char* value = reinterpret_cast<unsigned char*>(&data[index]);

			if (sameEndianites)
			{
				for (int j = 0; j < sizeof(T); j++)
				{
					value[j] = (unsigned char)byteData[i * sizeof(T) + j];
				}
			}
			else
			{
				for (int j = 0; j < sizeof(T); j++)
				{
					value[sizeof(T) - j - 1] = (unsigned char)byteData[i * sizeof(T) + j];
				}
			}			
		}
	}
	
	Logger::GetInstance()->LogEvent("NetVolumeLoader", "SegmentLoading:Reserialization:Ended", "", std::format("{0},{1},{2},{3}", x, y, z, downscale));

	Logger::GetInstance()->LogEvent("NetVolumeLoader", "SegmentLoading:Ended", "", std::format("{0},{1},{2},{3}", x, y, z, downscale));

	return data;
}

template NetVolumeLoader<uint8_t>;
template NetVolumeLoader<uint16_t>;
template NetVolumeLoader<uint32_t>;
template NetVolumeLoader<uint64_t>;
template NetVolumeLoader<float>;
template NetVolumeLoader<int8_t>;
template NetVolumeLoader<int16_t>;
template NetVolumeLoader<int32_t>;
template NetVolumeLoader<int64_t>;