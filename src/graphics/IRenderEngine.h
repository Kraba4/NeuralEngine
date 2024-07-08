#pragma once
#include <GLFW/glfw3.h>
#include <Windows.h>
#include <a_main/Timer.h>

namespace neural::graphics {
class IRenderEngine {
public:
	virtual void initialize(HWND a_window, int a_width, int a_heigh) = 0;
	virtual void render(const Timer& timer)                          = 0;
	virtual void shutdown()                                          = 0;
};
}