#include "engine.hpp"

#include <curses.h>
#include <chrono>
#include <thread>

#include <iostream>

using namespace std::chrono_literals;

Engine::Engine(WINDOW* scr, const Board& board) :
		m_board(board), m_scr(scr), m_iteration_duration(1000ms) {
	setupNcurses();
	if (!m_scr)
		m_scr = stdscr;
	m_max_iterations = -1;
}

Engine::Engine(WINDOW* scr, Board&& board) :
		m_board(board), m_scr(scr), m_iteration_duration(1000ms) {
	setupNcurses();
	if (!m_scr)
		m_scr = stdscr;
	m_max_iterations = -1;
}



void Engine::setSpeed(double seconds) {
	m_iteration_duration = std::chrono::milliseconds(
			static_cast<int>(seconds * 1000));
}

void Engine::setMaxIterations(int max) {
	m_max_iterations = max;
}

void Engine::setupNcurses() {
	static bool was_done = false;
	if (was_done)
		return;

	initscr();
	cbreak();
	noecho();
	keypad(stdscr, TRUE);
	// nonblocking, -1 if no char
	timeout(0);

}

void Engine::disableNcurses() {
	endwin();
}


void Engine::loop() {
	auto now = []() { return std::chrono::system_clock::now(); };
	auto timer = now();
	
	bool exit_loop = false;
	bool pause = false;

	auto refresh_rate = m_iteration_duration;


	int posx = 0;
	int posy = 0;

	int iterations = 0;

	while (!exit_loop && 
			(m_max_iterations == -1 || iterations < m_max_iterations)) {
		m_board.draw(m_scr);
		::wmove(m_scr, posy, posx);
		::wrefresh(m_scr);

		int ch = ::getch();
		switch (ch) {
			case 'q':
				exit_loop = true;
				break;
			case ' ':
				pause = !pause;
				if (!pause)
					timer = now() - refresh_rate;
				break;
			case 'x':
			case 'k':
			{
				int y, x;
				getyx(m_scr, y, x);
				if (ch == 'x')
					m_board.add_at(y, x);
				else
					m_board.kill_at(y, x);
				break;
			}
			case KEY_LEFT:
				if (posx)
					posx--;
				break;
			case KEY_RIGHT:
				posx++;
				break;
			case KEY_UP:
				if (posy)
					posy--;
				break;
			case KEY_DOWN:
				posy++;
				break;
			case KEY_F(1):
				display_help();
				timer = now() - refresh_rate;
				break;
			case 's':
				display_save();
				timer = now() - refresh_rate;
				break;

		}
		std::this_thread::sleep_for(10ms);
		

		if (!pause) {
			auto elapsed = now() - timer;
			if (elapsed > refresh_rate) {
				m_board.iterate();
				iterations++;
				timer += refresh_rate;
			}
		}

	}
	disableNcurses();
}

void Engine::display_help() {
	::wclear(m_scr);
	::wmove(m_scr, 0, 0);
	auto print = [=](const std::string& string) {
		::waddstr(m_scr, string.c_str());
		::waddstr(m_scr, "\n");
	};

	print("HELP\n");

	print("- use arrows to move");
	print("- to diplay this help message press F1");
	print("- to quit press 'q'");
	print("- to pause game press spacebar");
	print("- to kill cell press 'k'");
	print("- to resurrect cell press 'x'");
	print("- to save game press 's'");

	::wrefresh(m_scr);

	while (::getch() != 'q')
		std::this_thread::sleep_for(10ms);

}

void Engine::display_save() {
	::wclear(m_scr);
	::wmove(m_scr, 0, 0);
	::waddstr(m_scr, "Save file\n\nPlese enter save location "
			"(esc to quit)\n");
	cbreak();
	echo();
	timeout(-1);
	::wrefresh(m_scr);

	std::string location;
	::keypad(m_scr, FALSE);

	int c;
	bool no_save = false;
	while (true) {
		c = ::getch();
		// std::cerr << c << ' ';
		// check if escape
		if (c == 27) {
			no_save = true;
			break;
		}
		if (c == KEY_ENTER) {
			break;
		}
		if (c == '\n') {
			break;
		}
		location += c;
	}

	if (!no_save)
		m_board.dump_to_file(location);

	keypad(m_scr, TRUE);
	cbreak();
	noecho();
	timeout(0);
	::wclear(m_scr);
}
