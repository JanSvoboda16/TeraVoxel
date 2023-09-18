#pragma once
#include "FastRCVolumeVisualizerFactory.h"
#include "VolumeScene.h"

class FastRcVolumeVisualizerSetter
{
public:
	template <typename T>
	static void Set(std::shared_ptr<IVolumeScene> scene, std::shared_ptr<VolumeVisualizerSettingsBase> settings)
	{
		VolumeScene<T>* nscene = (VolumeScene<T>*)(scene.get());
		std::shared_ptr<IVolumeVisualizerFactory<T>> visfac = std::shared_ptr<IVolumeVisualizerFactory<T>>((IVolumeVisualizerFactory<T>*) new FastRCVolumeVisualizerFactory<T>(std::static_pointer_cast<FastRCVolumeVisualizerSettings>(settings)));
		nscene->ChangeVisualizer(visfac);
	}
};

