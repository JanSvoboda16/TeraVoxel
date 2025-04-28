/*
 * Author: Jan Svoboda
 * University: BRNO UNIVERSITY OF TECHNOLOGY, FACULTY OF INFORMATION TECHNOLOGY
 */
#pragma once
#include <memory>
#include <shared_mutex>
#include <Eigen/Dense>

enum RequestState
{
	WaitingToBeLoaded,
	BeingLoaded,
	Loaded,
	UnableToLoadOrSkipped
};

struct VolumeBlockRequestTicket
{
	std::mutex mutex;

	int downscale;
	Eigen::Vector3i coordinates;

	float priority;
	bool needed = true;
	RequestState state;
};

template <typename T>
struct VolumeBlock
{	
	short downscale = 500;					    // High value -> will be always reloaded first
	short x, y, z;	//READONLY					// Indexes of this segment

	T* data;								// DATA

	VolumeBlock(short x, short y, short z, T* data = nullptr) :
		x(x), y(y), z(z), data(data)
	{
	}

	~VolumeBlock() 
	{
		delete[] data;
	}
};

