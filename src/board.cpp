#include <curses.h>
#include <algorithm>

#include <iostream>
#include <fstream>

#include <SFML/Graphics.hpp>

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

template<>
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
template<>
void Board::draw(sf::RenderTarget& target) const {
	// sf::Texture x_texture;
	// sf::Texture blank_texture;
	// x_texture.loadFromFile("assets/x_texture.png");
	// blank_texture.loadFromFile("assets/blank_texture.png");
	sf::Font font;
	font.loadFromFile("assets/arial.ttf");
	sf::Text x_text;
	x_text.setFont(font);
	x_text.setString("x");
	x_text.setFillColor(sf::Color::Black);
	x_text.setCharacterSize(24);
	// x_text.setStyle(sf::Text::Bold | sf::Text::Underlined);
	double x_mul = 10;
	double y_mul = 15;
	for (int row = 0; row < m_height; ++row){
		for (int col = 0; col < m_width; ++col) {
			if (!m_board[row][col])
				continue;

			x_text.setPosition(x_mul * col, y_mul * row);
			target.draw(x_text);
		}
	}


}

void Board::dump_to_file(const std::string& name) {
	std::ofstream file(name);
	file << "# Auto generated map file\n";
	file << "x = " << m_width << ", y = " << m_height << ", ";
	file << "rule = B";
	for (auto&& iter : m_born)
		file << iter;
	file << "/S";
	for (auto&& iter : m_survives)
		file << iter;
	file << std::endl;

	int max_line_len = 80;
	int line_length = 0;

	auto out_char = [&](char c) {
		if (line_length > max_line_len) {
			file << '\n';
			line_length = 0;
		}
		++line_length;
		file << c;
	};

	auto out_string = [&](auto string) {
		if (line_length + string.size() > (size_t)max_line_len) {
			file << '\n';
			line_length = 0;
		}
		line_length += string.size();
		file << string;
	};

	char previous_char = 0;
	int previous_char_counter = 0;

	auto out_marker = [&](char c) {
		if (previous_char == c)
			++previous_char_counter;
		else {
			if (previous_char_counter > 1) {
				out_string(std::to_string(previous_char_counter) +
						previous_char);
			}
			else if (previous_char_counter == 1) {
				out_char(previous_char);
			}
			if (c == 'o' || c == 'b') {
				previous_char = c;
				previous_char_counter = 1;
			}
			else {
				previous_char = 0;
				previous_char_counter = 0;
				out_char(c);
			}
		}
	};

	for (auto iter = begin(); iter != end(); ++iter) {
		if (iter != begin() && iter.col == 0) {
			out_marker('$');
		}
		if (*iter)
			out_marker('o');
		else
			out_marker('b');
	}
	out_marker('!');
	file << "\n";
}






