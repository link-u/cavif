cmake_minimum_required(VERSION 3.13)
project(cavif)

enable_language(C CXX ASM)
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_FLAGS_DEBUG "-g3 -O0 -fno-omit-frame-pointer")

###############################################################################
add_subdirectory(external/libyuv EXCLUDE_FROM_ALL)
add_subdirectory(external/libavif-container EXCLUDE_FROM_ALL)
###############################################################################
set(SKIP_INSTALL_ALL ON)
set(PNG_SHARED OFF CACHE BOOL "Build shared lib" FORCE)
set(PNG_STATIC ON CACHE BOOL "Build static lib" FORCE)
set(PNG_TESTS OFF CACHE BOOL "Build libpng tests" FORCE)
add_subdirectory(external/libpng EXCLUDE_FROM_ALL)
#FIXME(ledyba-z): Workaround to include "pnglibconf.h"
target_include_directories(png_static PUBLIC ${CMAKE_CURRENT_BINARY_DIR}/external/libpng)
###############################################################################
# fmt
set(FMT_TEST OFF CACHE BOOL "" FORCE)
set(FMT_FUZZ OFF CACHE BOOL "" FORCE)
add_subdirectory(external/fmt)
###############################################################################
set(CONFIG_AV1_DECODER 0 CACHE STRING "Enable AV1 decoder." FORCE)
set(ENABLE_TESTS OFF CACHE BOOL "" FORCE)
set(ENABLE_EXAMPLES OFF CACHE BOOL "" FORCE)
set(ENABLE_TOOLS OFF CACHE BOOL "" FORCE)
add_subdirectory(external/libaom)
###############################################################################

add_executable(cavif
        src/main.cpp

        src/PNGReader.cpp
        src/PNGReader.hpp

        src/Image.cpp
        src/Image.hpp

        src/AVIFBuilder.cpp
        src/AVIFBuilder.hpp
)
target_link_libraries(cavif PRIVATE aom)
target_link_libraries(cavif PRIVATE png_static)
target_link_libraries(cavif PRIVATE fmt::fmt)
target_link_libraries(cavif PRIVATE yuv)
target_link_libraries(cavif PRIVATE libavif-container)
target_include_directories(cavif PRIVATE external/libpng)
target_include_directories(cavif PRIVATE external/libaom)
target_include_directories(cavif PRIVATE "${CMAKE_CURRENT_BINARY_DIR}/external/libaom")
target_include_directories(cavif PRIVATE external/libyuv/include)
target_include_directories(cavif PRIVATE external/libavif-container/src)
