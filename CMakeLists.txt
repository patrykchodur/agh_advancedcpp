cmake_minimum_required(VERSION 3.7)
project(game_of_life)

include_directories(include /usr/bin/include)
find_package(Boost COMPONENTS program_options REQUIRED)
include_directories(${Boost_INCLUDE_DIR})
find_package(Curses REQUIRED)
include_directories(${Curses_INCLUDE_DIR})

set(CMAKE_CXX_STANDARD 17)

set(source_files src/main.cpp src/board.cpp src/engine.cpp src/rtl_parser.cpp)

add_executable(${PROJECT_NAME} ${source_files})

target_link_libraries(${PROJECT_NAME} ${Boost_LIBRARIES})
target_link_libraries(${PROJECT_NAME} ${CURSES_LIBRARIES})
