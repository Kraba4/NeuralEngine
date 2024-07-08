#pragma once

namespace neural::game {
class GameEngine {
public:
	void initialize();
	void processInputs();
	void drawFrame();
private:
	bool m_isRunning{ false };
};
}