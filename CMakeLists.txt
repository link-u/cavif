cmake_minimum_required(VERSION 3.13)
project(cavif)

enable_language(C CXX ASM)
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_FLAGS_DEBUG "-g3 -O0 -fno-omit-frame-pointer")

if (WIN32)
    # https://stackoverflow.com/questions/47690822/possible-to-force-cmake-msvc-to-use-utf-8-encoding-for-source-files-without-a-bo
    add_compile_options("$<$<C_COMPILER_ID:MSVC>:/utf-8>")
    add_compile_options("$<$<CXX_COMPILER_ID:MSVC>:/utf-8>")
endif(WIN32)

###############################################################################
add_subdirectory(external/libavif-container EXCLUDE_FROM_ALL)
###############################################################################
# zlib
set(ASM686L OFF CACHE BOOL "Enable building i686 assembly implementation" FORCE)
set(ASM686 OFF CACHE BOOL "Enable building amd64 assembly implementation" FORCE)
add_subdirectory(external/zlib EXCLUDE_FROM_ALL)
set(ZLIB_INCLUDE_DIR "${CMAKE_CURRENT_SOURCE_DIR}/external/zlib" "${CMAKE_CURRENT_BINARY_DIR}/external/zlib")
set(ZLIB_ROOT "${CMAKE_BINARY_DIR}/zlib")
set(ZLIB_DIR "${CMAKE_BINARY_DIR}/zlib")
###############################################################################
set(SKIP_INSTALL_ALL ON)

set(PNG_BUILD_ZLIB ON CACHE BOOL "Custom zlib Location, else find_package is used" FORCE)
set(PNG_SHARED OFF CACHE BOOL "Build shared lib" FORCE)
set(PNG_STATIC ON CACHE BOOL "Build static lib" FORCE)
set(PNG_TESTS OFF CACHE BOOL "Build libpng tests" FORCE)
add_subdirectory(external/libpng EXCLUDE_FROM_ALL)
#FIXME(ledyba-z): Workaround to include "pnglibconf.h"
target_include_directories(png_static PUBLIC "${CMAKE_CURRENT_BINARY_DIR}/external/libpng")
# zlib
target_link_directories(png_static PRIVATE "${CMAKE_CURRENT_BINARY_DIR}/external/zlib")
target_include_directories(png_static PRIVATE external/zlib)
target_link_libraries(png_static zlibstatic)
add_dependencies(png_static zlibstatic)
###############################################################################
# fmt
set(FMT_TEST OFF CACHE BOOL "" FORCE)
set(FMT_FUZZ OFF CACHE BOOL "" FORCE)
add_subdirectory(external/fmt EXCLUDE_FROM_ALL)
###############################################################################
set(CONFIG_AV1_DECODER 0 CACHE STRING "Enable AV1 decoder." FORCE)
set(ENABLE_TESTS OFF CACHE BOOL "" FORCE)
set(ENABLE_EXAMPLES OFF CACHE BOOL "" FORCE)
set(ENABLE_TOOLS OFF CACHE BOOL "" FORCE)
add_subdirectory(external/libaom EXCLUDE_FROM_ALL)
###############################################################################

add_executable(cavif
        src/main.cpp

        src/img/PNGReader.cpp
        src/img/PNGReader.hpp
        src/img/Convert.hpp

        src/Config.cpp
        src/Config.hpp

        src/AVIFBuilder.cpp
        src/AVIFBuilder.hpp
)
target_link_libraries(cavif PRIVATE aom)
target_link_libraries(cavif PRIVATE png_static)
target_link_libraries(cavif PRIVATE fmt::fmt)
target_link_libraries(cavif PRIVATE libavif-container)
target_include_directories(cavif PRIVATE external/libpng)
target_include_directories(cavif PRIVATE external/libaom)
target_include_directories(cavif PRIVATE "${CMAKE_CURRENT_BINARY_DIR}/external/libaom")
target_include_directories(cavif PRIVATE external/libyuv/include)
target_include_directories(cavif PRIVATE external/libavif-container/src)
if (UNIX)
    # FIXME(ledyba-z): workaround for gcc-8
    target_link_libraries(cavif PRIVATE stdc++fs)
endif (UNIX)

install(TARGETS cavif
    RUNTIME DESTINATION bin
)

