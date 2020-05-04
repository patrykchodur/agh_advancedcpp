#include <curses.h>
#include <algorithm>

#include <iostream>

#include "board.hpp"

// #define BOARD_OVERLAP

Board::Board(int height, int width) : m_width(width), m_height(height){
	m_board.resize(m_height);

	for (auto&& iter : m_board)
		iter.resize(m_width);

	m_temporary_board = m_board;
	m_survives.insert({2, 3});
	m_born.insert(3);

	// m_board[5][5] = true;
	// m_board[5][6] = true;
	// m_board[6][6] = true;
	// m_board[7][7] = true;
}

void Board::iterate() {
	auto count_neighbours = [&](auto row, auto col) {
#ifdef BOARD_OVERLAP
		auto mod = [](int a, int b) {
			return a < 0 ? a + b : a % b;
		};
		int result = 0;
		for (auto row_iter = row - 1;
				row_iter < row + 2; ++row_iter) {
			for (auto col_iter = col - 1;
					col_iter < col + 2; ++col_iter) {
				if (row_iter == row && col_iter == col)
					continue;
				if (m_board[mod(row_iter ,m_height)][mod(col_iter, m_width)])
					++result;
			}
		}
		return result;
#else
		int result = 0;
		for (auto row_iter = std::max(1, row) - 1;
				row_iter < std::min(row + 2, m_height); ++row_iter) {
			for (auto col_iter = std::max(1, col) - 1;
					col_iter < std::min(col + 2, m_width); ++col_iter) {
				if (row_iter == row && col_iter == col)
					continue;
				if (m_board[row_iter][col_iter])
					++result;
			}
		}
		return result;
#endif // BOARD_OVERLAP
	};


	for (int row = 0; row < m_height; ++row) {
		for (int col = 0; col < m_width; ++col) {
			auto neighbours_no = count_neighbours(row, col);
			if (m_board[row][col]) {
				if (m_survives.count(neighbours_no))
					m_temporary_board[row][col] = true;
				else
					m_temporary_board[row][col] = false;
			}
			else {
				if (m_born.count(neighbours_no))
					m_temporary_board[row][col] = true;
				else
					m_temporary_board[row][col] = false;
			}
		}
	}

	m_board = m_temporary_board;
}

void Board::add_at(int row, int col) {
	if (row < m_height && col < m_width)
		m_board[row][col] = true;
}

void Board::kill_at(int row, int col) {
	if (row < m_height && col < m_width)
		m_board[row][col] = false;
}

void Board::draw(WINDOW* scr) const {
	for (int row = 0; row < m_height; ++row) {
		::wmove(scr, row, 0);
		for (int col = 0; col < m_width; ++col) {
			::waddch(scr, m_board[row][col] ? 'X' : ' ');
		}
		::waddch(scr, '|');
	}
	::wmove(scr, m_height, 0);
	for (int col = 0; col < m_width; ++col)
		::waddch(scr, '-');
	::waddch(scr, '+');
}







