#ifndef ENGINE_HPP
#define ENGINE_HPP

#include "board.hpp"
#include <chrono>

// Engine<WINDOW*>
// Engine<sf::RenderWindow&>
template <class Window>
class Engine {
public:
	Engine(Window scr, const Board& board);
	Engine(Window scr, Board&& board);

	void setSpeed(double seconds);
	void setMaxIterations(int max);
	void initializeField(int y, int x);

	void loop();

private:
	static void setupDisplay();
	static void disableDisplay();
	void display_help();
	void display_save();
	Board m_board;
	Window m_scr;

	int m_max_iterations;
	std::chrono::milliseconds m_iteration_duration;
};



#endif // ENGINE_HPP
