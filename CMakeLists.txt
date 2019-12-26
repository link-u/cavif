cmake_minimum_required(VERSION 3.12)
project(cavif)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_FLAGS_DEBUG "-g3 -O0 -fno-omit-frame-pointer")

add_executable(cavif src/main.cpp)
