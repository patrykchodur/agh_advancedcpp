#include <curses.h>
#include <chrono>
#include <thread>
#include <optional>
#include <boost/program_options.hpp>
#include <string>
#include <vector>
#include <iostream>
#include <SFML/Graphics.hpp>

#include "board.hpp"
#include "engine.hpp"
#include "rtl_parser.hpp"

namespace po = boost::program_options;

struct coordinates_pair {
	explicit coordinates_pair(int y = 0, int x = 0) :
		x(x), y(y) { }

	friend std::istream& operator>>(
			std::istream& stream, 
			coordinates_pair& pair
			);
	int x;
	int y;
};

std::istream& operator>>(std::istream& stream, coordinates_pair& pair) {
	std::string first_coordinate;
	std::string second_coordinate;

	char c;
	// read first
	while ((c = stream.get()) != '-') {
		first_coordinate += c;
	}

	stream >> second_coordinate;

	pair.y = std::stoi(first_coordinate);
	pair.x = std::stoi(second_coordinate);
	
	return stream;
}

int main(int argc, char* argv[]) {
	
	po::options_description desc("Allowed options");
	desc.add_options()
		("help", "print this text")
		("width,w", po::value<int>(), "set board width")
		("height,h", po::value<int>(), "set board height")
		("speed,s", po::value<double>(),
			"set speed of simmulation (time needed for one change)")
		("max-iterations,m", po::value<int>(), 
			"set max number of iterations")
		("init-values,I",
			po::value<std::vector<coordinates_pair>>()->multitoken(), 
			"init values as pairs of ints y-x: 5-3")
		("input-file,i", po::value<std::string>(),
			".rtl input file")
		;

	po::variables_map vm;
	po::store(po::parse_command_line(argc, argv, desc), vm);
	po::notify(vm);

	// help message
	if (vm.count("help")) {
		std::cout << desc << '\n';
		return EXIT_SUCCESS;
	}

	std::optional<Board> board;

	if (vm.count("input-file")) {
		board = parse_from_file(vm["input-file"].as<std::string>());
		if (!board)
			return EXIT_FAILURE;
	}

	if (!board) {
		// setting board
		int height = 10;
		int width = 10;
		if (vm.count("height"))
			height = vm["height"].as<int>();
		if (vm.count("width"))
			width = vm["width"].as<int>();

		board = Board(height, width);

		//setting leaving cells
		if (vm.count("init-values")) {
			auto&& vec = vm["init-values"].as<std::vector<coordinates_pair>>();
			for (auto&& iter : vec) {
				board->add_at(iter.y, iter.x);
				std::cerr << "x: " << iter.x << ", y: " << iter.y << '\n';
			}
		}
	}


	sf::RenderWindow window(sf::VideoMode(800, 600), "My window");
	Engine<sf::RenderWindow&> engine(window, std::move(*board));

	// setting engine
	if (vm.count("max-iterations"))
		engine.setMaxIterations(vm["max-iterations"].as<int>());
	if (vm.count("speed"))
		engine.setSpeed(vm["speed"].as<double>());


	engine.loop();
	/*

	Engine<WINDOW*> engine(stdscr, std::move(*board));

	// setting engine
	if (vm.count("max-iterations"))
		engine.setMaxIterations(vm["max-iterations"].as<int>());
	if (vm.count("speed"))
		engine.setSpeed(vm["speed"].as<double>());


	engine.loop();
	*/


}
