#pragma once
#include <atomic>

class GPUEntity
{
public:
	virtual ~GPUEntity() = default;
	void IncrementVersionId() { _versionId++; };
	uint64_t VersionId() { return _versionId; }
private:
	uint64_t _versionId = 0;
};

