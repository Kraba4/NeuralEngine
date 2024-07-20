#include <GLFW/glfw3.h>
#include <iostream>
#include <d3d12.h>
#include <dxgi1_6.h>
#include "Application.h"

constexpr int32_t WIDTH = 980;
constexpr int32_t HEIGHT = 620;


int main() {
	neural::Application app;
	app.initialize("Neural", WIDTH, HEIGHT);
	app.mainLoop();
}