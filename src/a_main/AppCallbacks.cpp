#pragma once
#include <GLFW/glfw3.h>
#include "Application.h"

namespace neural {
void Application::onKeyboardPressedBasic(GLFWwindow* window, int key, int, int action, int)
{
    switch (key)
    {
    case GLFW_KEY_ESCAPE:
        if (action == GLFW_PRESS)
            glfwSetWindowShouldClose(window, static_cast<int>(true));
        break;
    //case GLFW_KEY_LEFT_SHIFT:
    //    g_inputDesktop.camMoveSpeed = 10.0f;
    //    break;

    //case GLFW_KEY_LEFT_CONTROL:
    //    g_inputDesktop.camMoveSpeed = 1.0f;
    //    break;

    default:
        if (key >= 0 && key < AppInput::MAX_KEYS)
        {
            if (action == GLFW_RELEASE)
                g_appInput.keyReleased[key] = true;
            else if (action == GLFW_PRESS)
                g_appInput.keyPressed[key] = true;
            else if (action == GLFW_REPEAT) {
                g_appInput.keyRepeated[key] = true;
            }
        }
        break;
    }
}
}