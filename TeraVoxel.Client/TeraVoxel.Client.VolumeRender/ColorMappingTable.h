/*
 * Author: Jan Svoboda
 * University: BRNO UNIVERSITY OF TECHNOLOGY, FACULTY OF INFORMATION TECHNOLOGY
 */
#pragma once
#include <Eigen/Dense>
#include "../TeraVoxel.Client.Core/nlohman/json.hpp"

class ColorMappingItem {
public:
	float Range[2]; // Min and max values of the interval
	float ColorFrom[4]; // Color of max (RGBA)
	float ColorTo[4]; // Color of min (RGBA)

	NLOHMANN_DEFINE_TYPE_INTRUSIVE(ColorMappingItem, Range, ColorFrom, ColorTo)

		// Recomputes all precomputed values
	void RecomputeDeltas() {
		deltaRange = Range[1] - Range[0];
		dreDivDra = (ColorTo[0] - ColorFrom[0]) / deltaRange;
		dgrDivDra = (ColorTo[1] - ColorFrom[1]) / deltaRange;
		dblDivDra = (ColorTo[2] - ColorFrom[2]) / deltaRange;
		dalDivDra = (ColorTo[3] - ColorFrom[3]) / deltaRange;
	};

	float DeltaRange() const { return deltaRange; };
	float DreDivDra() const { return dreDivDra; };
	float DgrDivDra() const { return dgrDivDra; };
	float DblDivDra() const { return dblDivDra; };
	float DalDivDra() const { return dalDivDra; };

private:
	// Precomputed values
	float deltaRange;
	float dreDivDra;
	float dgrDivDra;
	float dblDivDra;
	float dalDivDra;
};

class ColorMappingTable
{
public:
	std::vector<ColorMappingItem> Table; // Mapping table rows
	NLOHMANN_DEFINE_TYPE_INTRUSIVE(ColorMappingTable, Table)

	// Recomputes all precomputed values of all rows
	void RecomputeDeltas() {
		
		for (size_t i = 0;  i < Table.size(); i++)
		{
			Table[i].RecomputeDeltas();
		}
	}
};

