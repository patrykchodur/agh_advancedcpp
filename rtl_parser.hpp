#ifndef RTL_PARSER_HPP
#define RTL_PARSER_HPP

#include <iostream>
#include <string>
#include <optional>

#include "board.hpp"

std::optional<Board> parse_from_file(const std::string& filename);
std::optional<Board> parse_from_stdin();

#endif
