#include <GLFW/glfw3.h>
#include <d3d12.h>
#include <dxgi1_6.h>

#include <iostream>

#include "Application.h"
#pragma warning(1:4242)
#pragma warning(1:4267)

constexpr int32_t WIDTH = 800;
constexpr int32_t HEIGHT = 600;

int main() {
    neural::Application app;
    app.initialize("Neural", WIDTH, HEIGHT);
    app.mainLoop();
}
