cmake_minimum_required(VERSION 3.12)
project(cavif)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_FLAGS_DEBUG "-g3 -O0 -fno-omit-frame-pointer")

###############################################################################
set(CONFIG_AV1_DECODER 0 CACHE STRING "Enable AV1 decoder." FORCE)
set(ENABLE_TESTS OFF CACHE BOOL "" FORCE)
set(ENABLE_EXAMPLES OFF CACHE BOOL "" FORCE)
set(ENABLE_TOOLS OFF CACHE BOOL "" FORCE)
add_subdirectory(external/libaom)
###############################################################################

add_executable(cavif src/main.cpp)
target_link_libraries(cavif aom)
target_include_directories(cavif PRIVATE external/libaom)
target_include_directories(cavif PRIVATE "${CMAKE_CURRENT_BINARY_DIR}/external/libaom")
