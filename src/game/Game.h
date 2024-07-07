#pragma once

namespace neural::game {
class GameEngine {
	bool m_isRunning{ false };
public:
	void initialize();
	void processInputs();
	void drawFrame();
};
}