#pragma once
#include <GLFW/glfw3.h>
#include <Windows.h>

namespace neural::graphics {
class IRenderEngine {
public:
	virtual void init(HWND a_window, int a_width, int a_heigh)            = 0;
	virtual void render(double a_dt)          = 0;
	virtual void shutdown()        = 0;
};
}