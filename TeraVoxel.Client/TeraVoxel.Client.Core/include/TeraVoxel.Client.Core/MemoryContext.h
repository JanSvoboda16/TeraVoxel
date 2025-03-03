#pragma once
#include <mutex>

class MemoryContext
{
public:
	std::atomic<uint64_t> usedMemory = 0;
	std::atomic<uint64_t> maxMemory = 6000000000;
	std::mutex memoryInfoWriteMutex;

	static MemoryContext& GetInstance()
	{
		static MemoryContext instance;
		return instance;
	}
};

