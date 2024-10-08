#pragma once

namespace neural {
class Timer {
public:
    void setTime(double a_currentTime);
    double calculateDT(double a_currentTime);
    bool tryRecalculateFPS();
    int getLastFPS() const;
    double getLastTime() const;
    double getLastDeltaTime() const;

private:
    static constexpr int NAverage = 60;
    double m_avgTime;
    int m_avgCounter;
    double m_lastTime;
    int m_fps;
    double m_dt;
};
}  // namespace neural
