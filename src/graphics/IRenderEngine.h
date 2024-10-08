#pragma once
#include <GLFW/glfw3.h>
#include <Windows.h>
#include <a_main/Timer.h>
#include <a_main/AppInput.h>
#include "RenderSettings.h"

namespace neural::graphics {
class IRenderEngine {
public:
    virtual void initialize(HWND a_window, int a_width, int a_height) = 0;
    virtual void render(const Timer& a_timer) = 0;
    virtual void shutdown() = 0;
    RenderSettings* getRenderSettingsPtr() {
        return &m_settings;
    }
protected:
    RenderSettings m_settings;
};
}