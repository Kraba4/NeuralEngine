#pragma once

#include <game/Game.h>
#include <graphics/IRenderEngine.h>
#include "Timer.h"

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
	void showFPS(Timer& a_timer);

	GLFWwindow* m_window{ nullptr };
	std::shared_ptr<game::GameEngine> m_game;
	std::shared_ptr<graphics::IRenderEngine> m_renderer;

};
}