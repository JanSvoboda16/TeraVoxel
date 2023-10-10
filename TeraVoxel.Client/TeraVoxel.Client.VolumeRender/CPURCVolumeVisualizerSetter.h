#pragma once
#include "CPURCVolumeVisualizerFactory.h"
#include "VolumeScene.h"

class CPURCVolumeVisualizerSetter
{
public:
	template <typename T>
	static void Set(const std::shared_ptr<IVolumeScene>& scene, const std::shared_ptr<VolumeVisualizerSettingsBase>& settings)
	{
		VolumeScene<T>* nscene = (VolumeScene<T>*)(scene.get());
		std::shared_ptr<IVolumeVisualizerFactory<T>> visfac = std::make_shared<CPURCVolumeVisualizerFactory<T>>(std::static_pointer_cast<CPURCVolumeVisualizerSettings>(settings));
		nscene->ChangeVisualizer(visfac);
	}
};

