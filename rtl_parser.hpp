#ifndef RTL_PARSER_HPP
#define RTL_PARSER_HPP

#include <iostream>
#include <string>

#include "board.hpp"

Board parse_from_file(const std::string& filename);
Board parse_from_stdin();

#endif
