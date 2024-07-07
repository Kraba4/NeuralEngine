#pragma once

namespace neural {
class Timer {
	static constexpr int NAverage = 60;
	double m_avgTime;
	int    m_avgCounter;
	double m_lastTime;
	int    m_fps;
public:
	void   setTime(double a_currentTime);
	double calculateDT(double a_currentTime);
	bool   tryRecalculateFPS();
	int    getLastFPS();
	double getLastTime() const;
};
}