#include "rtl_parser.hpp"
#include <optional>
#include <istream>
#include <fstream>
#include <sstream>
#include <cassert>
#include <initializer_list>


enum Token : int {
	_ERROR = -1,
	_EOF = 0,
	_X, // x | X
	_Y, // y | Y
	_COMMA, // ,
	_EQUALS, // =
	_DOLAR, // $
	_B, // b | B
	_O, // o | O
	_S, // s | S
	_SLASH, // /
	_EXCLAMATION_MARK, // !
	_NUMBER, // [0-9]\+
	_COMMENT, // #\.\$
	_RULE, // rule
	_TOKEN_LAST
};

std::string pretty_token(const std::pair<int, std::string>& tok) {
	switch (tok.first) {
	case _ERROR:
		return std::string("error");
	case _EOF:
		return std::string("eof");
	case _X:
		return std::string("x");
	case _Y:
		return std::string("y");
	case _COMMA:
		return std::string("comma");
	case _EQUALS:
		return std::string("equals");
	case _DOLAR:
		return std::string("dolar");
	case _B:
		return std::string("b");
	case _O:
		return std::string("o");
	case _S:
		return std::string("s");
	case _SLASH:
		return std::string("slash");
	case _EXCLAMATION_MARK:
		return std::string("exclamation_mark");
	case _NUMBER:
		return std::string("number");
	case _COMMENT:
		return std::string("comment");
	case _RULE:
		return std::string("rule");
	default:
		return tok.second;
	}
}

class Lexer {
public:
	explicit Lexer(std::istream& stream);
	std::pair<Token, std::string> lex();

	int get_line_number() {
		return m_line_nr;
	}

	int get_col_number() {
		return m_col_nr;
	}

private:
	std::istream& m_stream;
	int m_line_nr;
	int m_col_nr;
	// one previous col
	int m_previous_col_len;

	auto get_char() {
		auto result = m_stream.get();
		if (result == '\n') {
			m_previous_col_len = m_col_nr;
			m_col_nr = 1;
			m_line_nr++;
		}
		else {
			m_col_nr++;
		}
		return result;
	}
	
	void unget_char(int count = 1) {
		bool previous_line = false;
		while (count--) {
			m_stream.unget();
			auto c = m_stream.peek();
			if (c == '\n') {
				if (!previous_line) {
					previous_line = true;
					m_col_nr = m_previous_col_len;
				}
				else {
					m_col_nr = -1;
				}
				m_line_nr--;
			}
			else
				m_col_nr--;
		}
	}

	auto get_line() {
		std::string result;
		std::getline(m_stream, result);
		m_previous_col_len = m_col_nr + result.size();
		m_col_nr = 1;
		m_line_nr++;
		return result;
	}

	auto get_word() {
		std::string result;
		auto c = get_char();
		while ((c >= 'a' && c <= 'z') ||
		       (c >= 'A' && c <= 'Z')) {
			result += c;
			c = get_char();
			m_col_nr++;
		}
		unget_char();
		return result;
	}
};

Lexer::Lexer(std::istream& stream) : m_stream(stream) {
	m_line_nr = 1;
	m_col_nr = 1;
}

std::pair<Token, std::string> Lexer::lex() {
	constexpr auto eof = std::istream::traits_type::eof();
	bool get_more = false;
	do {
		auto c = get_char();
		get_more = false;

		switch (c) {
		case eof:
			return { _EOF, std::string() };
		case 'x':
		case 'X':
			return { _X, std::string(1, c) };
		case 'y':
		case 'Y':
			return { _Y, std::string(1, c) };
		case ',':
			return { _COMMA, std::string(1, c) };
		case '=':
			return { _EQUALS, std::string(1, c) };
		case '$':
			return { _DOLAR, std::string(1, c) };
		case 'b':
		case 'B':
			return { _B, std::string(1, c) };
		case 'o':
		case 'O':
			return { _O, std::string(1, c) };
		case 's':
		case 'S':
			return { _S, std::string(1, c) };
		case '/':
			return { _SLASH, std::string(1, c) };
		case '!':
			return { _EXCLAMATION_MARK, std::string(1, c) };
		case '0':
		case '1':
		case '2':
		case '3':
		case '4':
		case '5':
		case '6':
		case '7':
		case '8':
		case '9':
			// get number
		{
			std::string result(1, c);
			while (std::isdigit(c = get_char())) {
				result += c;
			}
			unget_char();
			return { _NUMBER, result };
		}
		case '#':
			return { _COMMENT, static_cast<char>(c) + get_line() };
		case 'r':
		{
			auto word = static_cast<char>(c) + get_word();
			if (word == "rule") {
				return { _RULE, word };
			}
			else
				unget_char(word.size());
		}
		case ' ':
		case '\n':
			get_more = true;
		}
	}
	while (get_more);
	assert(0);
	return { _ERROR, std::string() };
}

/*
 * grammar
 *
 * rtl_file = comments_section, header_section, pattern_section;
 *
 * comment_section = { _COMMENT };
 *
 * header_section = [ value_description ], { _COMMA, value_description };
 *
 * value_description = _X, _EQUALS, _NUMBER
 *                    | _Y, _EQUALS, _NUMBER
 *                    | _RULE, _EQUALS, rule_description
 *
 * rule_description = _B, _NUMBER, _SLASH, _S, _NUMBER;
 *
 * pattern_section = line_pattern, { _DOLAR, line_pattern }, _EXCLAMATION_MARK;
 *
 * line_pattern = pattern, { pattern };
 *
 * pattern = [ _NUMBER ], ( _B | _O );
 */

class Parser {
public:
	Parser();
	Board parse_stream(std::istream& stream, const std::string& fname);

private:
	std::string m_file_name;

	int m_x;
	int m_y;
	std::optional<Lexer> m_lex;
	std::optional<Board> m_board;
	std::set<int> m_survives;
	std::set<int> m_born;
	Board::Position m_position;

	Token m_current_symbol;
	std::string m_current_text;
	std::string m_cached_text;

	bool accept(Token symbol);
	bool expect(Token symbol);
	bool ask(Token symbol);
	bool ask(std::initializer_list<Token> token_list);
	void next_symbol();

	void error(const std::string& message);
	void warning(const std::string& message);
	void unexpected_token(Token token);
	int m_error_count;

	void rtl_file();
	void comment_section();
	void header_section();
	void value_description();
	void rule_description();
	void pattern_section();
	void line_pattern();
	void pattern();

	int finnish_line();
};

Parser::Parser() {
	m_x = 0;
	m_y = 0;
}

bool Parser::accept(Token symbol) {
	if (m_current_symbol == symbol) {
		next_symbol();
		return true;
	}
	return false;
}

bool Parser::ask(Token symbol) {
	return m_current_symbol == symbol;
}

bool Parser::ask(std::initializer_list<Token> token_list) {
	for (auto&& iter : token_list)
		if (ask(iter))
			return true;
	return false;
}

bool Parser::expect(Token symbol) {
	if (accept(symbol))
		return true;

	unexpected_token(symbol);
	return false;
}

void Parser::next_symbol() {
	auto pair = m_lex->lex();
	m_current_symbol = pair.first;
	m_current_text = m_cached_text;
	m_cached_text = pair.second;
}

void Parser::rtl_file() {
	comment_section();
	header_section();
	pattern_section();
}

void Parser::comment_section() {
	while (accept(_COMMENT));
}

void Parser::header_section() {
	if (ask({_X, _Y, _RULE})) {
		value_description();
		while (accept(_COMMA))
			value_description();
	}

	if (m_x <= 0) {
		m_x = 10;
		warning("x not set. Using default - " + std::to_string(m_x));
	}
	if (m_y <= 0) {
		m_y = 10;
		warning("y not set. Using default - " + std::to_string(m_y));
	}
	if (m_born.empty() || m_survives.empty()) {
		warning("rules not set. Using default - B3/S23");
		m_born.insert(3);
		m_survives.insert({2, 3});
	}
	m_board = Board(m_y, m_x);
	m_position = m_board->begin();
}

void Parser::value_description() {
	if (accept(_X)) {
		expect(_EQUALS);
		expect(_NUMBER);
		m_x = std::stoi(m_current_text);
		return;
	}
	else if (accept(_Y)) {
		expect(_EQUALS);
		expect(_NUMBER);
		m_y = std::stoi(m_current_text);
		return;
	}
	else {
		expect(_RULE);
		expect(_EQUALS);
		rule_description();
	}
}

void Parser::rule_description() {
	expect(_B);
	expect(_NUMBER);
	for (auto&& iter : m_current_text)
		m_born.insert(iter - '0');
	expect(_SLASH);
	expect(_S);
	expect(_NUMBER);
	for (auto&& iter : m_current_text)
		m_survives.insert(iter - '0');
}

void Parser::pattern_section() {
	line_pattern();
	while (accept(_DOLAR)) {
		finnish_line();
		line_pattern();
	}
	finnish_line();
	expect(_EXCLAMATION_MARK);
}

void Parser::line_pattern() {
	pattern();
	while (ask({_NUMBER, _B, _O}))
		pattern();
}

void Parser::pattern() {
	int repetitions = 1;
	if (accept(_NUMBER))
		repetitions = std::stoi(m_current_text);
	if (accept(_B)) {
		while (repetitions--)
			m_board->kill_at(m_position++);
	}
	else {
		expect(_O);
		while (repetitions--)
			m_board->add_at(m_position++);
	}
}

void Parser::error(const std::string& message) {
	std::cerr << "ERROR: " << 
		message << "  " <<
		m_file_name << ':' <<
		m_lex->get_line_number() << ':' << m_lex->get_col_number() <<
		'\n';

	++m_error_count;
}

void Parser::warning(const std::string& message) {
	std::cerr << "WARNING: " << 
		message << "  " <<
		m_file_name << ':' <<
		m_lex->get_line_number() << ':' << m_lex->get_col_number() <<
		'\n';
}

int Parser::finnish_line() {
	if (!m_position.col)
		return 0;
	auto start_row = m_position.row;
	int added = 0;
	while (start_row == m_position.row) {
		added++;
		m_board->kill_at(m_position++);
	}
	return added;
}

void Parser::unexpected_token(Token symbol) {
	error("Unexpected token: " + pretty_token({symbol, m_cached_text}));
}

Board Parser::parse_stream(std::istream& stream, const std::string& name) {
	m_file_name = name;
	m_lex.emplace(stream);
	next_symbol();
	rtl_file();
	return std::move(*m_board);
}

Board parse_from_file(const std::string& name) {
	std::ifstream stream(name);
	Parser parser;
	return parser.parse_stream(stream, name);
}

Board parse_from_stdin() {
	Parser parser;
	return parser.parse_stream(std::cin, "stdin");
}





/*

#define UNEXPECTED_TOKEN(lexer, token) do {                        \
	std::cerr << "Error: unexpected token '" << pretty_token(token)\
	<< "'\n" <<                                                    \
	"Position: line = " << (lexer).get_line_number() <<            \
	", column = " << (lexer).get_col_number() << '\n';             \
	} while (0)


Board parse_from_istream(std::istream& stream) {
	Lexer lexer(stream);

	int error_nr = 0;
	std::pair<Token, std::string> current_token;

	auto assert_next = [&](Token expected) {
		current_token = lexer.lex();
		if (current_token.first != expected) {
			UNEXPECTED_TOKEN(lexer, current_token);
			++error_nr;
		}
		return current_token.first;
	};

	auto assert_current = [&](Token expected) {
		if (current_token.first != expected) {
			UNEXPECTED_TOKEN(lexer, current_token);
			++error_nr;
		}
	};

	auto update_next = [&]() {
		current_token = lexer.lex();
		return current_token.first;
	};

	// skip comments
	while (update_next() == _COMMENT);

	std::set<int> born;
	std::set<int> survives;
	int x = 10;
	int y = 10;

	if (current_token.first == _X) {
		assert_next(_EQUALS);
		assert_next(_NUMBER);

		x = std::stoi(current_token.second);

		assert_next(_COMMA);
		assert_next(_Y);
		assert_next(_EQUALS);
		assert_next(_NUMBER);

		y = std::stoi(current_token.second);
		update_next();
		if (current_token.first == _COMMA)
			update_next();
	}

	if (current_token.first == _RULE) {
		assert_next(_EQUALS);
		assert_next(_B);
		assert_next(_NUMBER);

		for (auto&& iter : current_token.second)
			born.insert(iter - '0');

		assert_next(_SLASH);
		assert_next(_S);
		assert_next(_NUMBER);

		for (auto&& iter : current_token.second)
			survives.insert(iter - '0');
		update_next();
	}

	Board board(y, x);
	auto position = board.begin();

	while (current_token.first != _EXCLAMATION_MARK) {
		switch (current_token.first) {
		case _NUMBER:
		{
			auto repetitions = std::stoi(current_token.second);
			update_next();
			if (!(current_token.first == _B ||
				  current_token.first == _O))
				UNEXPECTED_TOKEN(lexer, current_token);
			if (current_token.first == _B) {
				while (repetitions--) {
					board.kill_at(position++);
				}
			}
			else if (current_token.first == _O) {
				while (repetitions--) {
					board.add_at(position++);
				}
			}
			else
				UNEXPECTED_TOKEN(lexer, current_token);
			break;
		}
		case _O:
			board.add_at(position++);
			break;
		case _B:
			board.kill_at(position++);
			break;
		case _DOLAR:
		{
			auto start = position.row;
			while (position.row == start)
				board.kill_at(position++);
			break;
		}
		default:
			UNEXPECTED_TOKEN(lexer, current_token);
			break;
		}
		update_next();
	}
	return board;
}
*/
