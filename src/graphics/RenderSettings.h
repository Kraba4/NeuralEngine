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
    int selectedMesh = 0;
    int selectedCamera = -1;
    bool showGUI = true;
    bool doScreenShot = false;
    bool ml = false;
    bool bakeLightGrid = false;
    bool showLightGrid = true;
    bool showProbeEnvironment = false;
};
}