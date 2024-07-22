#include "Timer.h"

namespace neural {
void Timer::setTime(double a_currentTime) {
    m_lastTime = a_currentTime;
    m_avgTime = 0;
    m_avgCounter = 0;
    m_fps = 0;
}

double Timer::calculateDT(double a_currentTime) {
    m_dt = a_currentTime - m_lastTime;
    m_lastTime = a_currentTime;
    m_avgTime += m_dt;
    m_avgCounter++;
    return m_dt;
}

bool Timer::tryRecalculateFPS() {
    if (m_avgCounter >= NAverage) {
        m_fps = static_cast<int>(1.0 / (m_avgTime / static_cast<double>(NAverage)));
        m_avgTime = 0.0;
        m_avgCounter = 0;
        return true;
    }
    return false;
}

int Timer::getLastFPS() const { return m_fps; }
double Timer::getLastTime() const { return m_lastTime; }
double Timer::getLastDeltaTime() const { return m_dt; }
}  // namespace neural
