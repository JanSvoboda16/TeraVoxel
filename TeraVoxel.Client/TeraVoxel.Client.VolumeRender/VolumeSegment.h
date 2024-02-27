/*
 * Author: Jan Svoboda
 * University: BRNO UNIVERSITY OF TECHNOLOGY, FACULTY OF INFORMATION TECHNOLOGY
 */
#pragma once
#include<memory>
#include<shared_mutex>

template <typename T>
struct VolumeSegment
{
	std::atomic<float> priority = 0;
	std::atomic<int> unusedCount = 0;				// How many times was not used (continuously)
	std::atomic<bool> used = false;					// Was used in actual frame
	std::atomic<bool> waitsToBeReloaded = false;	// Wait in the reload stack or is being reloaded
	std::atomic<short> futureDownscale = 500;		// Dowsncale that the segment will have in future (after reload) or has now
	
	short actualDownscale = 500;					// High value -> will be always reloaded first
	short x, y, z;	//READONLY						// Indexes of this segment
	short requiredDownscale = 0;				// Downscale that is requiews for actual view
	
	T* data;								// DATA

	VolumeSegment(short x, short y, short z, T* data = nullptr) :
		x(x), y(y), z(z), data(data)
	{
	}

	~VolumeSegment() 
	{
		delete[] data;
	}
};

