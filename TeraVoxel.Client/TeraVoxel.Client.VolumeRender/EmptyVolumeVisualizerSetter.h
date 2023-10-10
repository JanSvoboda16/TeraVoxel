#pragma once
#include "VolumeScene.h"
#include "EmptyVolumeVisualizerFactory.h"

class EmptyVolumeVisualizerSetter
{
public:
	template <typename T>
	static void Set(std::shared_ptr<IVolumeScene> scene, std::shared_ptr<VolumeVisualizerSettingsBase> settings) {
		VolumeScene<T>* nscene = (VolumeScene<T>*)(scene.get());
		std::shared_ptr<IVolumeVisualizerFactory<T>> visfac = std::make_shared<EmptyVolumeVisualizerFactory<T>>(std::static_pointer_cast<EmptyVolumeVisualizerSettings>(settings));
		nscene->ChangeVisualizer(visfac);
	}
};

