#include "Timer.h"

namespace neural {
void Timer::setTime(double a_currentTime) {
	m_lastTime = a_currentTime;
	m_avgTime = 0;
	m_avgCounter = 0;
	m_fps = 0;
}

double Timer::calculateDT(double a_currentTime) {
	double dt = a_currentTime - m_lastTime;
	m_lastTime = a_currentTime;
	m_avgTime += dt;
	m_avgCounter++;
	return dt;
}

bool Timer::tryRecalculateFPS() {
	if (m_avgCounter >= NAverage)
	{
		m_fps = int(1.0 / (m_avgTime / double(NAverage)));
		m_avgTime = 0.0;
		m_avgCounter = 0;
		return true;
	}
	return false;
}

int Timer::getLastFPS() {
	return m_fps;
}
}