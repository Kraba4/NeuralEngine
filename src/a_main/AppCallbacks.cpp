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
        }
        break;
    }
}

void Application::onMouseButtonClickedBasic(GLFWwindow* window, int button, int action, int)
{
    if (button == GLFW_MOUSE_BUTTON_RIGHT && action == GLFW_RELEASE) {
        g_appInput.mouse.capture = !g_appInput.mouse.capture;
    }

    if (g_appInput.mouse.capture) {
        glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
    }
    else {
        glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_NORMAL);
    }
}

//void Application::onMouseMoveBasic(GLFWwindow*, double xpos, double ypos)
//{
//    static bool hasOldValues = false;
//    if (hasOldValues) {
//        g_appInput.deltaX = xpos - g_appInput.mousePosX;
//        g_appInput.deltaY = ypos - g_appInput.mousePosY;
//    }
//    g_appInput.mousePosX = xpos;
//    g_appInput.mousePosY = ypos;
//
//    hasOldValues = true;
//}

void Application::receiveTickInputs(GLFWwindow* a_window) {
    double newPosX = 0;
    double newPosY = 0;
    glfwGetCursorPos(a_window, &newPosX, &newPosY);

    g_appInput.mouse.deltaX = newPosX - g_appInput.mouse.posX;
    g_appInput.mouse.deltaY = newPosY - g_appInput.mouse.posY;

    g_appInput.mouse.posX = newPosX;
    g_appInput.mouse.posY = newPosY;

    constexpr int keysToCheckHold[] = {
        GLFW_KEY_W, GLFW_KEY_S, GLFW_KEY_D, GLFW_KEY_A, 
        GLFW_KEY_LEFT_SHIFT,
        GLFW_KEY_LEFT, GLFW_KEY_RIGHT, GLFW_KEY_UP, GLFW_KEY_DOWN
    };

    for (auto key : keysToCheckHold) {
        g_appInput.keyHold[key] = glfwGetKey(a_window, key);
    }
   
}
}