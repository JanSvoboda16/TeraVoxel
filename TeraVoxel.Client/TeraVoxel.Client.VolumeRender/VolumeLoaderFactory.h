#pragma once
#include <memory>
#include "VolumeLoaderBase.h"

template <typename T>
class VolumeLoaderFactory
{
public:
	virtual std::unique_ptr<VolumeLoaderBase<T>> Create(const ProjectInfo& projectInfo, int threadCount) const = 0;
};

