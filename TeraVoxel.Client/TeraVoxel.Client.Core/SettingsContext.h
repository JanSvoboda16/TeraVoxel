/*
 * Author: Jan Svoboda
 * University: BRNO UNIVERSITY OF TECHNOLOGY, FACULTY OF INFORMATION TECHNOLOGY
 */
#pragma once
#include <stdint.h>
#include <atomic>

class SettingsContext
{
public:
	std::atomic<int> renderingThreadCount = 13;
	std::atomic<int> loadingThreadCount = 5;
	std::atomic<int> preloadingThreadCount = 16;

	static SettingsContext& GetInstance()
	{
		static SettingsContext instance;
		return instance;
	}
};

