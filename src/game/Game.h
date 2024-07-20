#pragma once
#include <a_main/AppInput.h>
#include <graphics/RenderSettings.h>

namespace neural::game {
class GameEngine {
public:
	void initialize() {
		m_isRunning = true;
	}
	void processInputs(const AppInput& a_appInput, double dt);
	void drawFrame();
	void setRenderSettingsPtr(RenderSettings* a_pRenderSettings) {
		m_pRenderSettings = a_pRenderSettings;
	}
private:
	bool m_isRunning{ false };
	RenderSettings* m_pRenderSettings;
};
}