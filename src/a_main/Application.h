#pragma once

#include <game/Game.h>
#include <graphics/IRenderEngine.h>
#include "Timer.h"
#include "AppInput.h"

#include <GLFW/glfw3.h>

#include <memory>
#include <string_view>

namespace neural {
class Application {
public:
    void initialize(std::string_view a_name, int a_width, int a_height);
    void mainLoop();
    ~Application();
private:
    void settingGLFW();
    void showFPS(Timer& a_timer, bool a_enableStatistics);

    GLFWwindow* m_window{ nullptr };
    std::shared_ptr<game::GameEngine> m_game;
    std::shared_ptr<graphics::IRenderEngine> m_renderer;

    inline static AppInput g_appInput;
    static void onKeyboardPressedBasic(GLFWwindow* window, int key, int, int action, int);
    static void onMouseButtonClickedBasic(GLFWwindow* window, int button, int action, int);
    //static void onMouseMoveBasic(GLFWwindow*, double xpos, double ypos);
    static void receiveTickInputs(GLFWwindow* a_window);
};
}