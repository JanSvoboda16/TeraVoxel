#pragma once
#include "VolumeVisualizerBase.h"

template<typename T>
class IVolumeVisualizerFactory
{
public:
	virtual std::unique_ptr<VolumeVisualizerBase<T>> Create(std::shared_ptr<Camera> camera, std::shared_ptr<VolumeObjectMemory<T>> memory) = 0;
};

