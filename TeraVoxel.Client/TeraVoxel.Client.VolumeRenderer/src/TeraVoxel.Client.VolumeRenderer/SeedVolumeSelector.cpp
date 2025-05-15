/*
 * Author: Jan Svoboda
 * University: BRNO UNIVERSITY OF TECHNOLOGY, FACULTY OF INFORMATION TECHNOLOGY
 */

#include "TeraVoxel.Client.VolumeRenderer/SeedVolumeSelector.h"
#include "TeraVoxel.Client.VolumeRenderer/Serialization.h"
#include "TeraVoxel.Client.VolumeRenderer/VolumeLoaderGenericBase.h"

using Eigen::Vector3i;

void SeedVolumeSelector::Select(const Vector3f &seed, float lowerBound, float upperBound, float maxDifference, bool erase)
{
	CALL_TEMPLATED_FUNCTION2(SelectTemplated, _datasetInfo.dataType, seed, lowerBound, upperBound, maxDifference, erase);
}

std::vector<Vector3i> directions = {
	{1, 0, 0}, {-1, 0, 0}, 
	{0, 1, 0}, {0, -1, 0}, 
	{0, 0, 1}, {0, 0, -1}  
};

template <typename T>
void SeedVolumeSelector::SelectTemplated(const Vector3f seedf, float lowerBound, float upperBound, float maxDifference, bool erase)
{
	auto cache = std::dynamic_pointer_cast<VolumeCache<T>>(_volumeCache);
	std::queue<Vector3i> queue;

	Vector3i seed = (seedf.array() / Vector3f(_datasetInfo.voxelDimensions).array()).cast<int>();

	T value = 0;
	T nextValue = 0;

	if (GetData<T>(seed, value, cache) && value >= lowerBound && value <= upperBound)
	{
		queue.push(seed);
		auto maskIndex = seed.x() + seed.y() * _mask->x + seed.z() * _mask->x * _mask->y;
		_mask->data[maskIndex] = !erase;
	}

	int iters = 0;
	while (!queue.empty())
	{		
		Vector3i pos = queue.front();
		queue.pop();
		if (GetData(pos, value, cache))
		{
			for (size_t i = 0; i < 6; i++)
			{
				Vector3i nextPos = pos + directions[i];
				if (GetData<T>(nextPos, nextValue, cache))
				{
					auto maskIndex = nextPos.x() + nextPos.y() * _mask->x + nextPos.z() * _mask->x * _mask->y;
					if (nextValue >= lowerBound && nextValue <= upperBound && std::fabs(value - nextValue) <= maxDifference && (erase ? _mask->data[maskIndex] : !_mask->data[maskIndex]))
					{
						_mask->data[maskIndex] = !erase;
						queue.push(nextPos);
					}
				}
			}			
		}
	}
}

template <typename T>
__forceinline bool SeedVolumeSelector::GetData(const Vector3i &position, T &value, const std::shared_ptr<VolumeCache<T>> &cache)
{	
	if (position.x() < _datasetInfo.dataSizeX && position.y() < _datasetInfo.dataSizeY && position.z() <  _datasetInfo.dataSizeZ && position.x()>=0 && position.y() >= 0 && position.z() >= 0)
	{
		value = cache->GetValue(position.x(), position.y(), position.z());

		return true;
	}
	return false;
}


