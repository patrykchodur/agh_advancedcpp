cmake_minimum_required(VERSION 3.7)
project(game_of_life)

find_package(Boost COMPONENTS program_options REQUIRED)
find_package(Curses REQUIRED)
find_package(SFML COMPONENTS REQUIRED graphics window system)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_FLAGS "-Wall -g")

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

set(source_files src/main.cpp src/board.cpp src/engine.cpp src/rtl_parser.cpp)

add_executable(${PROJECT_NAME} ${source_files})

target_include_directories(${PROJECT_NAME} PUBLIC include)
target_include_directories(${PROJECT_NAME} PUBLIC /usr/local/include)
target_include_directories(${PROJECT_NAME} PUBLIC ${Boost_INCLUDE_DIR})
target_include_directories(${PROJECT_NAME} PUBLIC ${Curses_INCLUDE_DIR})
target_include_directories(${PROJECT_NAME} PUBLIC ${SFML_INCLUDE_DIR})

target_link_libraries(${PROJECT_NAME} ${Boost_LIBRARIES})
target_link_libraries(${PROJECT_NAME} ${CURSES_LIBRARIES})
target_link_libraries(${PROJECT_NAME} sfml-graphics)
