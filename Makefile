
all:
	clang++ -std=c++17 -g -lncurses -lboost_program_options -Wall main.cpp board.cpp engine.cpp rtl_parser.cpp -o a.out

clean:
	rm -f a.out
