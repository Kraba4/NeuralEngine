#include "Game.h"
#include <graphics/d3d12/DX12RenderEngine.h>
#include <iostream>
#include <DirectXMath.h>
#include <iomanip>

namespace neural::game {
void GameEngine::processInputs(const AppInput& a_appInput, double dt)
{
    if (a_appInput.keyPressed[GLFW_KEY_F2]) {
        m_pRenderSettings->showGUI = !m_pRenderSettings->showGUI;
    }
    if (a_appInput.keyPressed[GLFW_KEY_1]) {
        m_pRenderSettings->meshName = "cat";
    }
    if (a_appInput.keyPressed[GLFW_KEY_2]) {
        m_pRenderSettings->meshName = "bird";
    }
	double rotateAcceleration = 1;
	if (a_appInput.keyPressed[GLFW_KEY_R]) {
		m_pRenderSettings->enableRotating = !m_pRenderSettings->enableRotating;
	}

	if (a_appInput.keyPressed[GLFW_KEY_UP] || a_appInput.keyHold[GLFW_KEY_UP]) {
		m_pRenderSettings->rotateSpeedX += rotateAcceleration * dt;
	}

	if (a_appInput.keyPressed[GLFW_KEY_DOWN] || a_appInput.keyHold[GLFW_KEY_DOWN]) {
		m_pRenderSettings->rotateSpeedX = max(0, m_pRenderSettings->rotateSpeedX - rotateAcceleration * dt);
	}

	if (a_appInput.keyPressed[GLFW_KEY_RIGHT] || a_appInput.keyHold[GLFW_KEY_RIGHT]) {
		m_pRenderSettings->rotateSpeedY += rotateAcceleration * dt;
	}

	if (a_appInput.keyPressed[GLFW_KEY_LEFT] || a_appInput.keyHold[GLFW_KEY_LEFT]) {
		m_pRenderSettings->rotateSpeedY = max(0, m_pRenderSettings->rotateSpeedY - rotateAcceleration * dt);
	}

	if (a_appInput.mouse.capture) {
		auto& camera = m_pRenderSettings->camera;
		double mouseSensitivity = DirectX::XM_1DIV2PI / 100;
		double horizontalAngle = a_appInput.mouse.deltaX * mouseSensitivity;
		double verticalAngle = a_appInput.mouse.deltaY * mouseSensitivity;
		camera.rotateHorizontal(horizontalAngle);
		camera.rotateVertical(verticalAngle);

		double cameraSpeed = a_appInput.keyHold[GLFW_KEY_LEFT_SHIFT] ? 30 : 10;
		if (a_appInput.keyPressed[GLFW_KEY_W] || a_appInput.keyHold[GLFW_KEY_W]) {
			camera.moveForward(cameraSpeed * dt);
		}
		if (a_appInput.keyPressed[GLFW_KEY_S] || a_appInput.keyHold[GLFW_KEY_S]) {
			camera.moveForward(-cameraSpeed * dt);
		}
		if (a_appInput.keyPressed[GLFW_KEY_D] || a_appInput.keyHold[GLFW_KEY_D]) {
			camera.moveRight(cameraSpeed * dt);
		}
		if (a_appInput.keyPressed[GLFW_KEY_A] || a_appInput.keyHold[GLFW_KEY_A]) {
			camera.moveRight(-cameraSpeed * dt);
		}
	}

}
}