#pragma once
#include <a_main/Camera.h>

namespace neural {
struct RenderSettings {
    float angleX;
    float angleY;
    bool enableRotating = true;
    double rotatingTimeX = 0;
    double rotatingTimeY = 0;
    double rotateSpeedX = 1;
    double rotateSpeedY = 1;
    Camera camera;
};
}