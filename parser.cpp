

enum Symbol : int {
	node_description = _TOKEN_LAST + 1,
	nodes_description,
	list_of_node_descriptions,
	line_description,
	list_of_line_descriptions,
	lines_section,
	property,
	rules_entity,
	rules_scheme,
	format_value,
	format_entry,
	list_of_format_entries,
	format_section,
	rtl_section,
	list_of_rtl_sections,
	rtl_file,
};

/*           *************************************
 *           *                                   *
 *           *          PARSER GRAMMAR           *
 *           *                                   *
 *           *************************************
 *
 *
 * node_description :  _B
 *                   | _O
 *
 *
 * nodes_description :  node_description
 *                    | _NUMBER node_description
 *
 *
 * list_of_node_descriptions :  nodes_description
 *                            | nodes_description list_of_node_descriptions
 *
 *
 * line_description :  list_of_nodes_descriptions
 *
 *
 *
 * list_of_line_descriptions :  line_description
 *                            | line_description _DOLAR list_of_node_descriptions
 *
 *
 * lines_section : list_of_line_descriptions _EXCLAMATION_MARK
 *
 *
 *
 * property :  _X
 *           | _Y
 *           | _RULE
 *
 * rules_entity :  _B _NUMBER
 *               | _S _NUMBER
 *
 *
 * rules_scheme : rules_entity _SLASH rules_entity
 *
 *
 *
 * format_value :  _NUMBER
 *               | rules_scheme
 *
 *
 * format_entry : property _EQUALS format_value
 *
 *
 *
 * list_of_format_entries :  format_entry
 *                         | format_entry _COMMA list_of_format_entries
 *
 *
 * format_section : list_of_format_entries
 *
 *
 *
 * rtl_section :  _COMMENT
 *              | format_section
 *              | lines_section
 *
 * list_of_rtl_sections :  rtl_section
 *                       | rtl_section list_of_rtl_sections
 *
 *
 * rtl_file : list_of_rtl_sections
 *
 *
 *
 */




class Parser {
public:
	Parser();
	Board parse_stream(std::istream& stream);
	using token_ptr = std::shared_ptr<std::pair<int, std::any>>;
private:
	std::optional<Lexer> m_lex;
	std::optional<Board> m_board;
	int m_x;
	int m_y;
	std::set<int> m_survives;
	std::set<int> m_born;
	int single_parse();
	int reduce(const std::vector<int>& token_stack);

	token_ptr lex();
	token_ptr next();
	void unget(token_ptr token);
	std::stack<token_ptr> m_aquired_tokens;
	std::vector<token_ptr> m_token_stack;
}

token_ptr Parser::lex() {
	if (m_aquired_tokens.empty())
		return { m_lex->lex() };
	auto result = m_aquired_tokens.top();
	m_aquired_tokens.pop();
	return result;
}

void Parser::unget(token_ptr token) {
	m_aquired_tokens.push(token);
}

Parser::Parser() {
	m_x = 10;
	m_y = 10;
	m_survives.insert(3);
	m_born.insert({2, 3});
}

Parser::reduce(std::vector<token_ptr>::iterator start) {
	auto token = (*start)->first;

	switch (token) {
	case _B:
	{
		auto next_token = lex();
		if (next_token->


int Parser::parse(const std::vector<int>& token_stack) {

	if (token_stack.empty())
		return _ERROR;
	switch (*stack_position) {
	case _B:
	{
		auto save_position = stack_position;
		if (reduce_next() == _NUMBER && 
			reduce_next() == _SLASH &&
			reduce_next() == _S &&
			reduce_next() == _NUMBER) 
			return rules_scheme;
		else {
			stack_position = save_position;
			return node_description;
	}
	case _O:




}

int Parser::single_parse(int token) {
	switch (token) {


	}


}

#define UNEXPECTED_TOKEN(lexer, token) do {                        \
	std::cerr << "Error: unexpected token " << #token << '\n' <<   \
	"Position: line = " << (lexer).get_line_number() <<            \
	", column = " << (lexer).get_col_number() << '\n';             \
	} while (0)

Board parse_from_istream(std::istream& stream) {
	Lexer lexer(stream);

	auto assert_token = [&](token actual_token, token expected_token) {
		if (actual_token != expected_token) {
			UNEXPECTED_TOKEN(lexer, actual_token);
			return false;
		}
		return true;
	};

	auto lexer_pair = lexer.lex();
	auto& current_token = lexer_pair.first;
	auto& text = lexer_pair.second;
	auto next = [&]() {
		lexer_pair = lexer.lex();
	};

	// skip comments
	while (current_token == _COMMENT)
		lexer_pair = lexer.lex();

	// description of format
	assert_token(current_token, _X);
	assert_token(current_token, _EQUALS);
	assert_token(current_token, );
	



}

Board parse_from_istream1(std::istream& stream) {
	constexpr auto max = std::numeric_limits<std::streamsize>::max();

	int line_nr = 0;
	int x = -1;
	int y = -1;

	std::optional<Board> board;
	std::optional<Board::Position> position;
	int error = 0;

	int repeat = -1;

	std::set<int> survives;
	std::set<int> born;

	while (!stream.eof()) {
		if (error)
			break;

		++line_nr;
		auto c = stream.get();

		switch (c) {
		case '#':
			stream.ignore(max, '\n');
			continue;
		case '\n':
			continue;
		case 'y':
			if (y != -1 || board)
				error = 1;
			else {
				char test;
				stream >> test;
				if (test != '=')
					error = 1;
				stream >> y;
				if (x != -1) {
					board = Board(y, x);
					position = board->begin();
				}
			}
			continue;
		case 'x':
			if (x != -1 || board)
				error = 1;
			else {
				char test;
				stream >> test;
				if (test != '=')
					error = 1;
				stream >> x;
				if (y != -1) {
					board = Board(y, x);
					position = board->begin();
				}
			}
			continue;

		case 'r': {
			std::string test;
			stream >> test;
			if (test != "ule") {
				error = 1;
				break;
			}
			stream >> test;
			if (test != "=") {
				error = 1;
				break;
			}
			char c2;
			stream >> c2;

			if (c2 != 'B') {
				error = 1;
				break;
			}
			while ((c2 = stream.get()) != '/')
				born.emplace(c2 - '0');
			if ((c2 = stream.get()) != 'S') {
				error = 1;
				break;
			}
			while (std::isdigit(c2 = stream.get())) {
				survives.emplace(c2 - '0');
			}
			continue;

		}


				
			default:
				break;
		}

		switch (c) {
		case 'b':
			if (repeat == -1)
				board->kill_at((*position)++);
			else while(repeat--)
				board->kill_at((*position)++);
			repeat = -1;
			break;
		case 'o':
			if (repeat == -1)
				board->add_at((*position)++);
			else while (repeat--)
				board->add_at((*position)++);
			repeat = -1;
			break;
		case '$': {
			auto col = position->col;
			while (col == position->col)
				board->kill_at((*position)++);
			break;
		}
		case '!': {
			board->set_rules(survives, born);
			return *board;
		}

		if (std::isdigit(c)) {
			if (repeat != -1)
				repeat *= 10;
			else
				repeat = 0;
			repeat += c - '0';
		}
		}


	}
	
	if (!survives.empty() && !born.empty())
		board->set_rules(survives, born);
	return *board;
}
