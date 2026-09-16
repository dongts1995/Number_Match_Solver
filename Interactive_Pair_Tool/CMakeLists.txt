cmake_minimum_required(VERSION 3.16)
project(interactive_pair_tool LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

add_executable(interactive_pair_tool main.cpp)