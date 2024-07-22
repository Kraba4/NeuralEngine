#include "Application.h"
#include <graphics/d3d12/DX12RenderEngine.h>

#define GLFW_EXPOSE_NATIVE_WIN32
#include <GLFW/glfw3native.h>

#include <sstream>

namespace neural {

void Application::showFPS(Timer& a_timer, bool a_enableStatistics) {
	static int maxFPS = 0;
	static int minFPS = INT_MAX;
	static int sumFPS = 0;
	static int countCheckFPS = 0;
	static constexpr int NEED_CHECK_FPS = 150;
	if (a_timer.tryRecalculateFPS())
	{
		int fps = a_timer.getLastFPS();
		if (a_enableStatistics) {
			maxFPS = max(maxFPS, fps);
			minFPS = std::min(minFPS, fps);
			sumFPS += a_timer.getLastFPS();
			++countCheckFPS;
			if (countCheckFPS >= NEED_CHECK_FPS) {
				std::stringstream strout;
				strout << "FPS: AVG = " << sumFPS / countCheckFPS << " MIN = " << minFPS << " MAX = " << maxFPS;
				glfwSetWindowTitle(m_window, strout.str().c_str());
				minFPS = INT_MAX;
				maxFPS = 0;
				sumFPS = 0;
				countCheckFPS = 0;
			}
		}
		else {
			minFPS = INT_MAX;
			maxFPS = 0;
			sumFPS = 0;
			countCheckFPS = 0;
			std::stringstream strout;
			strout << "FPS = " << fps;
			glfwSetWindowTitle(m_window, strout.str().c_str());
		}
	}
}

void Application::settingGLFW() {
	glfwInit();
	glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
	glfwWindowHint(GLFW_RESIZABLE, GLFW_TRUE);
	glfwSwapInterval(0);
}

void Application::initialize(std::string_view a_name, int a_width, int a_height)
{
	settingGLFW();
	m_window = glfwCreateWindow(a_width, a_height, a_name.data(), nullptr, nullptr);
	glfwSetKeyCallback(m_window, onKeyboardPressedBasic);
	glfwSetMouseButtonCallback(m_window, onMouseButtonClickedBasic);
	//glfwSetCursorPosCallback(m_window, onMouseMoveBasic);

	m_renderer = std::make_shared<graphics::DX12RenderEngine>();
	m_renderer->initialize(glfwGetWin32Window(m_window), a_width, a_height);

	m_game = std::make_shared<game::GameEngine>();
	m_game->initialize();
	m_game->setRenderSettingsPtr(m_renderer->getRenderSettingsPtr());
}

void Application::mainLoop()
{
	Timer timer; timer.setTime(glfwGetTime());

	while (!glfwWindowShouldClose(m_window))
	{
		g_appInput.clearKeys();
		glfwPollEvents();            // g_appInput receives inputs from callbacks
		receiveTickInputs(m_window); // g_appInput receives current tick inputs

		static bool enableStatisticsFPS = true;
		if (g_appInput.keyPressed[GLFW_KEY_GRAVE_ACCENT]) enableStatisticsFPS = !enableStatisticsFPS;
		showFPS(timer, enableStatisticsFPS);

		double dt = timer.calculateDT(glfwGetTime());

		m_game->processInputs(g_appInput, dt);

		m_renderer->render(timer);
	}
}

Application::~Application()
{
	m_renderer->shutdown();
	glfwDestroyWindow(m_window);
	glfwTerminate();
}
}