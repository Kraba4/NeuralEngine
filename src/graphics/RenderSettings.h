#pragma once
#include <a_main/Camera.h>
#include <string>

namespace neural {
struct RenderSettings {
    bool enableRotating = true;
    float rotatingTimeX = 0;
    float rotatingTimeY = 0;
    int rotateSpeedX = 1;
    int rotateSpeedY = 1;
    Camera camera;
    std::string meshName = "cat";
};
}