#ifndef BOARD_HPP
#define BOARD_HPP

#include <vector>
#include <curses.h>
#include <set>


class Board {
	int m_width;
	int m_height;

	using board_array_t = std::vector<std::vector<bool>>;
public:
	Board(int width, int height);

	void set_rules(const std::set<int>& survives, const std::set<int>& born) {
		m_survives = survives;
		m_born = born;
	}

	void iterate();
	// with bound checking
	void add_at(int row, int col);
	void kill_at(int row, int col);
	void draw(WINDOW*) const;
private:
	board_array_t m_board;
	board_array_t m_temporary_board;

	std::set<int> m_survives;
	std::set<int> m_born;

public:
	struct Position {
		int row;
		int col;

		Position(const Board& board) {
			m_board = &board;
		}

		Position() {
			m_board = nullptr;
		}

		Position& operator++() {
			if (col < m_board->m_width - 1)
				++col;
			else {
				++row;
				col ^= col;
			}
			return *this;
		}

		Position operator++(int) {
			auto tmp = *this;
			++(*this);
			return tmp;
		}

		bool operator==(const Position& other) {
			return row == other.row && col == other.col;
		}
	
		bool operator!=(const Position& other) {
			return !(*this == other);
		}
	private:
		const Board* m_board;
	};
	Position begin() {
		auto result = Position(*this);
		result.row = 0;
		result.col = 0;
		return result;
	}
	Position end() {
		auto result = Position(*this);
		result.row = m_height;
		result.col = 0;
		return result;
	}

	void add_at(const Position& position) {
		return add_at(position.row, position.col);
	}

	void kill_at(const Position& position) {
		return kill_at(position.row, position.col);
	}
};


#endif // BOARD_HPP
