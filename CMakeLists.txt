cmake_minimum_required(VERSION 3.11)

set( CMAKE_CXX_COMPILER "g++" )
set( CMAKE_C_COMPILER "gcc" )

project( Chip8 )
add_executable( Chip8 ./src/main.cpp ./src/chip8.cpp)

