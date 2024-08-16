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
    int screenshotCounter = 0;
    Camera camera;
    std::string meshName = "cat";
    bool showGUI = true;
    bool doScreenShot = false;
    bool ml = true;
};
}