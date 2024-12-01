cmake_minimum_required(VERSION 3.22)
project(newDB)

set(CMAKE_CXX_STANDARD 17)
link_directories(lib)
include_directories(include)

add_executable(newDB main.cpp
        include/token.h
        include/newDB.h
        include/utility.h
        src/lexer.cpp
        include/parser.h
        src/parser.cpp)