#ifndef ENGINE_HPP
#define ENGINE_HPP

#include "board.hpp"
#include <chrono>

class Engine {
public:
	Engine(WINDOW* scr, const Board& board);
	Engine(WINDOW* scr, Board&& board);

	void setSpeed(double seconds);
	void setMaxIterations(int max);
	void initializeField(int y, int x);

	void loop();

private:
	static void setupNcurses();
	static void disableNcurses();
	Board m_board;
	WINDOW* m_scr;

	int m_max_iterations;
	std::chrono::milliseconds m_iteration_duration;
};



#endif // ENGINE_HPP
