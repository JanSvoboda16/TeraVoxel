/*
 * Author: Jan Svoboda
 * University: BRNO UNIVERSITY OF TECHNOLOGY, FACULTY OF INFORMATION TECHNOLOGY
 */
#pragma once
#include <Eigen/Dense>
#include "../TeraVoxel.Client.Core/nlohman/json.hpp"
#include <cuda_runtime.h>
#include <device_launch_parameters.h>

class MaterialTableItem {
public:
	float range[2] = { 0.f, 0.f };  // Min and max values of the interval
	float reflectionColorFrom[3] = { 0.f, 0.f, 0.f }; // Color of max (RGBA)
	float reflectionColorTo[3] = { 0.f, 0.f, 0.f }; // Color of min (RGBA)

	float translucencyColorFrom[3] = { 0.99f, 0.99f, 0.99f }; // Color of max (RGBA)
	float translucencyColorTo[3] = { 0.99f, 0.99f, 0.99f }; // Color of min (RGBA)

	float diffuseReflectionFrom = 0.f;
	float diffuseReflectionTo = 0.f;
	float specularReflectionFrom = 0.f;
	float specularReflectionTo = 0.f;
	float specularRoughnessFrom = 0.f;
	float specularRoughnessTo = 0.f;

	NLOHMANN_DEFINE_TYPE_INTRUSIVE(MaterialTableItem, range, reflectionColorFrom, reflectionColorTo, diffuseReflectionFrom, specularReflectionTo, specularRoughnessFrom, specularRoughnessTo)

	// Recomputes all precomputed values
	__host__ void RecomputeDeltas() 
	{
		_deltaRange = range[1] - range[0];
		_redRefDivRange = (reflectionColorTo[0] - reflectionColorFrom[0]) / _deltaRange;
		_greenRefDivRange = (reflectionColorTo[1] - reflectionColorFrom[1]) / _deltaRange;
		_blueRefDivRange = (reflectionColorTo[2] - reflectionColorFrom[2]) / _deltaRange;

		_redTransDivRange = (translucencyColorTo[0] - translucencyColorFrom[0]) / _deltaRange;
		_greenTransDivRange = (translucencyColorTo[1] - translucencyColorFrom[1]) / _deltaRange;
		_blueTransDivRange = (translucencyColorTo[2] - translucencyColorFrom[2]) / _deltaRange;

		_ddiffDivDra = (diffuseReflectionTo - diffuseReflectionFrom) / _deltaRange;
		_dspecRefDivDra = (specularReflectionTo - specularReflectionFrom) / _deltaRange;
		_dspecRouDivDra = (specularRoughnessTo - specularRoughnessFrom) / _deltaRange;
	};

	__host__ __device__ float DeltaRange() const { return _deltaRange; };

	__host__ __device__ float RedReflectionDivRange() const { return _redRefDivRange; };
	__host__ __device__ float GreenReflectionDivRange() const { return _greenRefDivRange; };
	__host__ __device__ float BlueReflectionDivRange() const { return _blueRefDivRange; };

	__host__ __device__ float RedTranslucencyDivRange() const { return _redTransDivRange; };
	__host__ __device__ float GreenTranslucencyDivRange() const { return _greenTransDivRange; };
	__host__ __device__ float BlueTranslucencyDivRange() const { return _blueTransDivRange; };

	__host__ __device__ float DdiffDivDra() const { return _ddiffDivDra; };
	__host__ __device__ float DspecRefDivDra() const { return _dspecRefDivDra; };
	__host__ __device__ float DspecRouDivDra() const { return _dspecRouDivDra; };

private:
	// Precomputed values
	float _deltaRange;

	float _redRefDivRange;
	float _greenRefDivRange;
	float _blueRefDivRange;

	float _redTransDivRange;
	float _greenTransDivRange;
	float _blueTransDivRange;

	float _ddiffDivDra;
	float _dspecRefDivDra;
	float _dspecRouDivDra;
};

class MaterialTable
{
public:
	std::vector<MaterialTableItem> table; // Mapping table rows
	NLOHMANN_DEFINE_TYPE_INTRUSIVE(MaterialTable, table)

	// Recomputes all precomputed values of all rows
	void RecomputeDeltas() 
	{
		for (size_t i = 0; i < table.size(); i++)
		{
			table[i].RecomputeDeltas();
		}
	}

	MaterialTableItem* CreateTableOnDevice() 
	{
		MaterialTableItem* t;

		cudaMalloc(&t, sizeof(MaterialTableItem) * table.size());

		cudaMemcpy(t, table.data(), sizeof(MaterialTableItem) * table.size(), cudaMemcpyHostToDevice);

		return t;
	}
};

