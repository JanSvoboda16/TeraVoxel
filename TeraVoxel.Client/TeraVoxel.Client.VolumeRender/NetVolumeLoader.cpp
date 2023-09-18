#include "NetVolumeLoader.h"

template <typename T>
NetVolumeLoader<T>::NetVolumeLoader(const ProjectInfo& projectInfo, int threadCount, const ProjectManager &projectManager) : VolumeLoaderBase<T>(projectInfo, threadCount) {
	_projectManager = projectManager;
}

template <typename T>
NetVolumeLoader<T>::~NetVolumeLoader() {
	
}

template <typename T>
T* NetVolumeLoader<T>::LoadSegment(int x, int y, int z, int downscale)
{
	int downscaledSegmentSize = this->_projectInfo.segmentSize / (int)pow(2, downscale);
	bool loaded = false;
	int segmentSize = downscaledSegmentSize * downscaledSegmentSize * downscaledSegmentSize;
	std::vector<unsigned char> byteData;

	// Segment loading from the server
	try
	{
		byteData = _projectManager.GetSegment(this->_projectInfo.name, x, y, z, downscale, segmentSize * sizeof(T));
		loaded = true;
	}
	catch (const std::exception& ex)
	{
		throw ex;
	}

	T* data = new T[segmentSize];

	bool sameEndianites = false;
	if ((this->_projectInfo.isLittleEndian && std::endian::native == std::endian::little)
		|| (!this->_projectInfo.isLittleEndian && std::endian::native == std::endian::big))
	{
		sameEndianites = true;
	}

	// Deserialization
	for (int i = 0; i < segmentSize; i++)
	{
		uint_fast16_t zpos = i / (downscaledSegmentSize * downscaledSegmentSize);
		uint_fast32_t mod = i % (downscaledSegmentSize * downscaledSegmentSize);
		uint_fast16_t ypos = mod / downscaledSegmentSize;
		uint_fast16_t xpos = mod % downscaledSegmentSize;

		auto index = Serialization::GetZCurveIndex(xpos, ypos, zpos);
		data[index] = 0;

		unsigned char* value = reinterpret_cast<unsigned char*>(&data[index]);

		for (int j = 0; j < sizeof(T); j++)
		{
			if (sameEndianites)
			{
				value[j] = (unsigned char)byteData[i * sizeof(T) + j];
			}
			else
			{
				value[sizeof(T) - j - 1] = (unsigned char)byteData[i * sizeof(T) + j];
			}
		}
	}

	return data;
}

template NetVolumeLoader<uint8_t>;
template NetVolumeLoader<uint16_t>;
template NetVolumeLoader<uint32_t>;
template NetVolumeLoader<uint64_t>;
template NetVolumeLoader<float>;
template NetVolumeLoader<double>;
template NetVolumeLoader<int8_t>;
template NetVolumeLoader<int16_t>;
template NetVolumeLoader<int32_t>;
template NetVolumeLoader<int64_t>;