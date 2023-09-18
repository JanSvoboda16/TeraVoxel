/*
 * Author: Jan Svoboda
 * University: BRNO UNIVERSITY OF TECHNOLOGY, FACULTY OF INFORMATION TECHNOLOGY
 */
#pragma once
#include<memory>
#include<shared_mutex>

template <typename T>
class VolumeSegment
{
public:
	std::atomic<int> unusedCount = 0;				// How many times was not used (continuously)
	std::atomic<bool> used = false;					// Was used in actual frame
	std::atomic<bool> waitsToBeReloaded = false;	// Wait in the reload stack or is being reloaded
	std::atomic<short> futureDownscale = 500;		// Dowsncale that the segment will have in future (after reload) or has now
	
	short actualDownscale = 500;					// High value -> will be always reloaded first
	short x, y, z;	//READONLY						// Indexes of this segment
	short lastRequiredDownscale = 0;				// Downscale that is requiews for actual view
	
	T* data = nullptr;								// DATA

	VolumeSegment(short x, short y, short z) 
	{
		this->x = x; this->y = y; this->z = z;
	}

	~VolumeSegment() 
	{
		delete[] data;
	}
};

