#include "rtl_parser.hpp"
#include <optional>
#include <istream>
#include <fstream>
#include <sstream>
#include <cassert>
#include <initializer_list>
#include <map>

#define PARSE_ANYWAY
#define MAX_ERROR_COUNT 4

enum Token : int {
	_STRING_ERROR = -2,
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
	_MULTIPLY,
	_PLUS,
	_MINUS,
	_IDENTIFIER, // [a-zA-Z ~oOsSbB] [a-zA-Z_]*
	_O_BRACKET,
	_C_BRACKET,
	_PERCENT, // %
	_SLASH, // /
	_EXCLAMATION_MARK, // !
	_NUMBER, // [0-9]\+
	_COMMENT, // #\.\$
	_STRING,
	_CALL,
	_RULE, // rule
	_TOKEN_LAST
};

std::string pretty_token(const std::pair<int, std::string>& tok) {
	switch (tok.first) {
	// case _ERROR:
		// return {"error"};
	case _EOF:
		return {"eof"};
	case _X:
		return {"'x'"};
	case _Y:
		return {"'y'"};
	case _COMMA:
		return {"','"};
	case _EQUALS:
		return {"'='"};
	case _DOLAR:
		return {"'$'"};
	case _B:
		return {"'b'"};
	case _O:
		return {"'o'"};
	case _S:
		return {"'s'"};
	case _MULTIPLY:
		return {"'*'"};
	case _PLUS:
		return {"'+'"};
	case _MINUS:
		return {"'-'"};
	case _SLASH:
		return {"'/'"};
	case _EXCLAMATION_MARK:
		return {"'!'"};
	case _PERCENT:
		return {"'%'"};
	case _NUMBER:
		return {"number '" + tok.second + '\''};
	case _COMMENT:
		return {"comment '" + tok.second + '\''};
	case _STRING:
		return {"string '" + tok.second + '\''};
	case _CALL:
		return {"call"};
	case _RULE:
		return {"rule"};
	case _IDENTIFIER:
		return {"identifier '" + tok.second + '\''};
	case _O_BRACKET:
		return {"'('"};
	case _C_BRACKET:
		return {"')'"};
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
		       (c >= 'A' && c <= 'Z') ||
			    c == '_') {
			result += c;
			c = get_char();
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
	decltype(get_char()) c;
	do {
		c = get_char();
		get_more = false;

		// these letters cannot start identifier
		switch (c) {
		case 'b':
		case 'B':
			return { _B, std::string(1, c) };
		case 'o':
		case 'O':
			return { _O, std::string(1, c) };
		case 's':
		case 'S':
			return { _S, std::string(1, c) };
		}

		if ((c >= 'a' && c <= 'z') ||
			(c >= 'A' && c <= 'Z') ||
			 c == '_') {
			auto word = get_word();
			auto whole_word = static_cast<char>(c) + word;

			if (whole_word == "call")
				return { _CALL, whole_word };
			return { _IDENTIFIER, static_cast<char>(c) + word };
		}

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
		case '*':
			return { _MULTIPLY, std::string(1, c) };
		case '+':
			return { _PLUS, std::string(1, c) };
		case '-':
			return { _MINUS, std::string(1, c) };
		case '$':
			return { _DOLAR, std::string(1, c) };
		case '/':
			return { _SLASH, std::string(1, c) };
		case '!':
			return { _EXCLAMATION_MARK, std::string(1, c) };
		case '"':
		case '\'':
		{
			std::string result;

			auto c2 = c;
			while ((c2 = get_char()) != c) {
				if (c2 == eof)
					return { _STRING_ERROR, "" };
				result += c2;
			}
			return { _STRING, result };
		}
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
		case '(':
			return { _O_BRACKET, std::string(1, c) };
		case ')':
			return { _C_BRACKET, std::string(1, c) };
		case '%':
			return { _PERCENT, std::string(1, c) };
		case ' ':
		case '\n':
			get_more = true;
		}
	}
	while (get_more);
	return { _ERROR, std::string(1, c) };
}

/*
 * grammar
 *
 * rtl_file = comments_section, header_section, pattern_section;
 *
 * comment_section = { _COMMENT };
 *
 * header_section = [ statement ], { _COMMA, statement };
 *
 * statement = value_description | (_CALL, function_call);
 *
 * function_call = _IDENTIFIER, _O_BRACKET, argument, _C_BRACKET
 *
 * argument = [ math_expression | _STRING ]
 *
 * value_description = _IDENTIFIER, _EQUALS, ( [ _MINUS ], _NUMBER | expression_value );
 *
 * rule_description = _B, _NUMBER, _SLASH, _S, _NUMBER;
 *
 * pattern_section = line_pattern, { _DOLAR, line_pattern }, _EXCLAMATION_MARK;
 *
 * line_pattern = pattern, { pattern };
 *
 * pattern = [ _NUMBER | expression_value ], ( _B | _O );
 *
 * expression_value = _PERCENT, _O_BRACKET, math_expression _C_BRACKET;
 *
 * math_expression = _IDENTIFIER 
 *                  | _NUMBER
 *                  | _MINUS math_expression
 *                  | _PLUS math_expression
 *                  | _O_BRACKET math_expression _C_BRACKET
 *                  | math_expression _MULTIPLY math_expression
 *                  | math_expression _SLASH math_expression
 *                  | math_expression _PLUS math_expression
 *                  | math_expression _MINUS math_expression
 *
 */

class Parser {
public:
	Parser();
	std::optional<Board> parse_stream(std::istream& stream, const std::string& fname);

private:
	std::string m_file_name;

	int m_x;
	int m_y;
	std::optional<Lexer> m_lex;
	std::optional<Board> m_board;
	std::set<int> m_survives;
	std::set<int> m_born;
	Board::Position m_position;
	std::map<std::string, int> m_values;

	Token m_current_symbol;
	std::string m_current_text;
	std::pair<int, int> m_source_position;
	std::string m_cached_text;

	bool accept(Token symbol);
	bool expect(Token symbol);
	bool ask(Token symbol);
	bool ask(std::initializer_list<Token> token_list);
	void next_symbol();
	void token_error();
	void update_position();

	void error(const std::string& message);
	void warning(const std::string& message);
	void unexpected_token(Token token);
	int m_error_count;

	void rtl_file();
	void comment_section();
	void header_section();
	void statement();
	void function_call();
	void print();
	bool m_during_print_call;
	void value_description();
	void rule_description();
	void pattern_section();
	void line_pattern();
	void pattern();
	int expression_value();
	std::string m_expression_desc;
	bool m_inside_expression;
	int math_expression();

	int finnish_line();
};

Parser::Parser() {
	m_x = 0;
	m_y = 0;

	m_during_print_call = false;
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
#ifdef PARSE_ANYWAY
	while (!accept(symbol)) {
		if (m_current_symbol == _EOF)
			return false;
		unexpected_token(m_current_symbol);
		next_symbol();
	}
	return true;
#else
	if (accept(symbol))
		return true;

	unexpected_token(m_current_symbol);
	return false;
#endif // !PARSE_ANYWAY
}

void Parser::next_symbol() {
	update_position();
	auto pair = m_lex->lex();
	m_current_symbol = pair.first;
	m_current_text = m_cached_text;
	m_cached_text = pair.second;

	if (m_inside_expression)
		m_expression_desc += m_current_text;

	if (m_current_symbol < 0)
		token_error();
}

void Parser::token_error() {
	switch (m_current_symbol) {
	case _STRING_ERROR:
		error("Unmached pair of string brackets");
		break;

	default:
		error("Unknown error in file");
	}
}

void Parser::update_position() {
	m_source_position = { m_lex->get_line_number(), m_lex->get_col_number() };
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
	if (ask({_IDENTIFIER, _CALL})) {
		statement();
		while (accept(_COMMA))
			statement();

	}

	int x, y;
	bool valid = true;
	if (!m_values.count("x")) {
		x = 10;
		warning("x not set. Using default - " + std::to_string(x));
	}
	else
		x = m_values["x"];

	if (!m_values.count("y")) {
		y = 10;
		warning("y not set. Using default - " + std::to_string(y));
	}
	else
		y = m_values["y"];

	if (m_born.empty() || m_survives.empty()) {
		warning("rule not set. Using default - B3/S23");
		m_born.insert(3);
		m_survives.insert({2, 3});
	}
	if (x < 1) {
		error("Value of x is invalid - " + std::to_string(x));
		valid = false;
	}
	if (y < 1) {
		error("Value of y is invalid - " + std::to_string(y));
		valid = false;
	}
	if (!valid)
		return;
	m_board = Board(y, x);
	m_position = m_board->begin();
}

void Parser::statement() {
	if (accept(_CALL))
		function_call();
	else
		value_description();
}

void Parser::function_call() {
	expect(_IDENTIFIER);
	auto ident = m_current_text;

	if (ident != "print") {
		error("no function named " + ident);
		return;
	}

	expect(_O_BRACKET);
	print();
	expect(_C_BRACKET);
}

void Parser::print() {
	m_during_print_call = true;
	if (ask(_C_BRACKET)) {
		if (!m_error_count)
			std::cout << '\n';
		return;
	}
	do {
		if (accept(_STRING)) {
			auto to_print = m_current_text;
			if (!m_error_count)
				std::cout << to_print;
		}
		else {
			auto to_print = math_expression();
			if (!m_error_count)
				std::cout << to_print;
		}
	} while (accept(_COMMA));
	if (!m_error_count)
		std::cout << '\n';

	m_during_print_call = false;
}

void Parser::value_description() {
	expect(_IDENTIFIER);
	auto identifier = m_current_text;
	expect(_EQUALS);
	if (identifier == "rule")
		rule_description();
	else {
		int result;
		bool unary_minus = false;
		bool unary_plus = false;
		if (accept(_MINUS))
			unary_minus = true;
		else if (accept(_PLUS))
			unary_plus = true;
		if (accept(_NUMBER))
			result = std::stoi(m_current_text);
		else {
			if (unary_minus)
				error("Unary minus not allowed here");
			if (unary_plus)
				error("Unary plus not allowed here");
			result = expression_value();
		}
		if (unary_minus)
			result = -result;
		m_values.insert_or_assign(identifier, result);
	}
	/*
	if (accept(_X)) {
		expect(_EQUALS);
		expect(_NUMBER);
		if (m_x != 0)
			error("x redefinition");
		m_x = std::stoi(m_current_text);
		return;
	}
	else if (accept(_Y)) {
		expect(_EQUALS);
		expect(_NUMBER);
		if (m_y != 0)
			error("y redefinition");
		m_y = std::stoi(m_current_text);
		return;
	}
	else {
		expect(_RULE);
		expect(_EQUALS);
		if (!m_survives.empty() && !m_born.empty())
			error("rule redefinition");
		rule_description();
	}
	*/
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
	if (m_error_count)
		return;
	auto row = m_position.row;
	line_pattern();
	while (accept(_DOLAR)) {
		if (!(row == m_position.row ||
			  (row + 1 == m_position.row &&
			   0 == m_position.col)))
			error("line too long");

		finnish_line();

		row = m_position.row;
		line_pattern();
	}
	if (!(row == m_position.row ||
		  (row + 1 == m_position.row &&
		   0 == m_position.col)))
		error("line too long");

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
	else if (ask(_PERCENT)) {
		repetitions = expression_value();
		error("Negative value of expression '" + m_expression_desc + '\'');
		return;
	}
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

int Parser::expression_value() {
	m_inside_expression = true;
	m_expression_desc.clear();
	int result;
	expect(_PERCENT);
	expect(_O_BRACKET);
	result = math_expression();
	expect(_C_BRACKET);
	m_inside_expression = false;
	return result;


	/*
	expect(_IDENTIFIER);
	auto identifier = m_current_text;
	expect(_C_BRACKET);
	if (!m_values.count(identifier)) {
		error("Unknown variable " + identifier);
		return 0;
	}
	return m_values[identifier];
	*/
}

int Parser::math_expression() {
	int result = 0;
	if (accept(_IDENTIFIER)) {
		if (!m_values.count(m_current_text)) {
			error("Unknown variable " + m_current_text);
			result = 0;
		}
		result = m_values[m_current_text];
	}
	else if (accept(_MINUS)) {
		result = math_expression();
		result = -result;
	}
	else if (accept(_PLUS))
		result = math_expression();
	else if (accept(_NUMBER))
		result = std::stoi(m_current_text);
	else if (accept(_O_BRACKET)){
		result = math_expression();
		expect(_C_BRACKET);
	}
	else {
		error("Wrong argument in print call " + m_current_text);
	}
	if (accept(_MULTIPLY))
		return result * math_expression();
	if (accept(_SLASH))
		return result / math_expression();
	if (accept(_PLUS))
		return result + math_expression();
	if (accept(_MINUS))
		return result - math_expression();
	return result;
}


void Parser::error(const std::string& message) {
	if (m_during_print_call)
		std::cout << std::endl;

	if (m_error_count == MAX_ERROR_COUNT)
		std::cerr << "ERROR: max error count exceeded\n";

	else if (m_error_count < MAX_ERROR_COUNT)
		std::cerr << "ERROR: " << 
			message << "  " <<
			m_file_name << ':' <<
			m_source_position.first << ':' <<
			m_source_position.second  - 1 <<
			'\n';

	++m_error_count;
}

void Parser::warning(const std::string& message) {
	if (m_during_print_call)
		std::cout << std::endl;

	std::cerr << "WARNING: " << 
		message << "  " <<
		m_file_name << ':' <<
		m_source_position.first << ':' << m_source_position.second - 1 <<
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
	update_position();
	error("Unexpected token: " + pretty_token({symbol, m_cached_text}));
}

std::optional<Board> Parser::parse_stream(std::istream& stream, const std::string& name) {
	m_file_name = name;
	m_lex.emplace(stream);
	next_symbol();
	rtl_file();
	if (!m_error_count)
		return std::move(*m_board);
	else
		return { };

}

std::optional<Board> parse_from_file(const std::string& name) {
	std::ifstream stream(name);
	Parser parser;
	return parser.parse_stream(stream, name);
}

std::optional<Board> parse_from_stdin() {
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
