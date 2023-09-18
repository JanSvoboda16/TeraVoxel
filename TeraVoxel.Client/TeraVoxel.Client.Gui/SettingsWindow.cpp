/*
 * Author: Jan Svoboda
 * University: BRNO UNIVERSITY OF TECHNOLOGY, FACULTY OF INFORMATION TECHNOLOGY
 */
#include "SettingsWindow.h"

void SettingsWindow::Update()
{
	ImGui::Begin("Settings");
	ImGui::PushItemWidth(100);

	int ram = MemoryContext::GetInstance().maxMemory.load(std::memory_order::acquire) / 1000000000;
	ImGui::InputInt("Max data RAM (GB)", &ram);
	MemoryContext::GetInstance().maxMemory.store((int64_t)(ram < 1 ? 1 : ram) * 1000000000, std::memory_order::release);

	int renderingThreadCount = SettingsContext::GetInstance().renderingThreadCount.load(std::memory_order::acquire);
	ImGui::InputInt("Rendering thread count", &renderingThreadCount);
	if (renderingThreadCount < 1)
	{
		renderingThreadCount = 1;
	}
	SettingsContext::GetInstance().renderingThreadCount.store(renderingThreadCount, std::memory_order::release);

	int loadingThreadCount = SettingsContext::GetInstance().loadingThreadCount.load(std::memory_order::acquire);
	ImGui::InputInt("Loading thread count", &loadingThreadCount);
	if (loadingThreadCount < 1)
	{
		loadingThreadCount = -1;
	}
	SettingsContext::GetInstance().loadingThreadCount.store(loadingThreadCount, std::memory_order::release);

	int preloadingThreadCount = SettingsContext::GetInstance().preloadingThreadCount.load(std::memory_order::acquire);
	ImGui::InputInt("Preloading thread count", &preloadingThreadCount);
	if (preloadingThreadCount < 1)
	{
		preloadingThreadCount = 1;
	}
	SettingsContext::GetInstance().preloadingThreadCount.store(preloadingThreadCount, std::memory_order::release);

	ImGui::PopItemWidth();
	ImGui::End();
}
