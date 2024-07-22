#pragma once
#include <GLFW/glfw3.h>
#include <cstring>
#include <bitset>

namespace neural {
struct AppInput
{
    static constexpr int MAX_KEYS = GLFW_KEY_LAST;
    AppInput() {}

    std::bitset<MAX_KEYS> keyPressed;
    std::bitset<MAX_KEYS> keyReleased;
    std::bitset<MAX_KEYS> keyHold;

    struct {
        double posX;
        double posY;
        int deltaX;
        int deltaY;
        bool capture = false;
    } mouse;

    void clearKeys() {
        keyPressed.reset();
        keyReleased.reset();
        keyHold.reset();
    }
};
}