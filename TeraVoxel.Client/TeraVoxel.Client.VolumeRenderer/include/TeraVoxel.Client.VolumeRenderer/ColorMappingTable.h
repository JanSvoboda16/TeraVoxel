/*
 * Author: Jan Svoboda
 * University: BRNO UNIVERSITY OF TECHNOLOGY, FACULTY OF INFORMATION TECHNOLOGY
 */
#pragma once
#include <Eigen/Dense>
#include <json.hpp>

class ColorMappingItem {
public:
	float range[2]; // Min and max values of the interval
	float colorFrom[4]; // Color of max (RGBA)
	float colorTo[4]; // Color of min (RGBA)

	NLOHMANN_DEFINE_TYPE_INTRUSIVE(ColorMappingItem, range, colorFrom, colorTo)

	// Recomputes all precomputed values
	void RecomputeDeltas() {
		_deltaRange = range[1] - range[0];
		_dreDivDra = (colorTo[0] - colorFrom[0]) / _deltaRange;
		_dgrDivDra = (colorTo[1] - colorFrom[1]) / _deltaRange;
		_dblDivDra = (colorTo[2] - colorFrom[2]) / _deltaRange;
		_dalDivDra = (colorTo[3] - colorFrom[3]) / _deltaRange;
	};

	float DeltaRange() const { return _deltaRange; };
	float DreDivDra() const { return _dreDivDra; };
	float DgrDivDra() const { return _dgrDivDra; };
	float DblDivDra() const { return _dblDivDra; };
	float DalDivDra() const { return _dalDivDra; };

private:
	// Precomputed values
	float _deltaRange;
	float _dreDivDra;
	float _dgrDivDra;
	float _dblDivDra;
	float _dalDivDra;
};

class ColorMappingTable
{
public:
	std::vector<ColorMappingItem> table; // Mapping table rows
	NLOHMANN_DEFINE_TYPE_INTRUSIVE(ColorMappingTable, table)

	// Recomputes all precomputed values of all rows
	void RecomputeDeltas() {
		
		for (size_t i = 0;  i < table.size(); i++)
		{
			table[i].RecomputeDeltas();
		}
	}
};

