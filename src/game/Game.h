#pragma once

namespace neural::game {
class GameEngine {
	bool m_isRunning{ false };
public:
	void init();
	void processInputs();
	void drawFrame();
};
}