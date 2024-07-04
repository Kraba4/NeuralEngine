#include "Application.h"
#include <graphics/d3d12/DX12RenderEngine.h>

#define GLFW_EXPOSE_NATIVE_WIN32
#include <GLFW/glfw3native.h>

#include <sstream>
//GLFWwindow* m_window{ nullptr };

namespace neural {

void Application::showFPS(Timer& a_timer) {
	if (a_timer.tryRecalculateFPS())
	{
		std::stringstream strout;
		strout << "FPS = " << a_timer.getLastFPS();
		glfwSetWindowTitle(m_window, strout.str().c_str());
	}
}

void Application::settingGLFW() {
	glfwInit();
	glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
	glfwWindowHint(GLFW_RESIZABLE, GLFW_TRUE);

	// callbacks
	// ...
	glfwSwapInterval(0);
}

void Application::init(std::string_view a_name, int a_width, int a_height)
{
	settingGLFW();
	m_window = glfwCreateWindow(a_width, a_height, a_name.data(), nullptr, nullptr);

	m_game = std::make_shared<game::GameEngine>();
	m_game->init();

	m_renderer = std::make_shared<graphics::DX12RenderEngine>();
	m_renderer->init(glfwGetWin32Window(m_window), a_width, a_height);
}

void Application::mainLoop()
{
	Timer timer; timer.setTime(glfwGetTime());

	while (!glfwWindowShouldClose(m_window))
	{
		glfwPollEvents();
		showFPS(timer);

		double dt = timer.calculateDT(glfwGetTime());
		m_game->processInputs();
		m_renderer->render(dt);
	}
}

Application::~Application()
{
	m_renderer->shutdown();
	glfwDestroyWindow(m_window);
	glfwTerminate();
}
}