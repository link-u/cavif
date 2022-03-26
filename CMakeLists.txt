cmake_minimum_required(VERSION 3.13)
project(cavif LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

if (WIN32)
  # https://stackoverflow.com/questions/47690822/possible-to-force-cmake-msvc-to-use-utf-8-encoding-for-source-files-without-a-bo
  add_compile_options("$<$<C_COMPILER_ID:MSVC>:/utf-8>")
  add_compile_options("$<$<CXX_COMPILER_ID:MSVC>:/utf-8>")

  # https://stackoverflow.com/a/60410369
  set(ENV{PKG_CONFIG_PATH} "${CMAKE_CURRENT_SOURCE_DIR}/_deps/lib/pkgconfig\;$ENV{PKG_CONFIG_PATH}")
else(WIN32)
  set(ENV{PKG_CONFIG_PATH} "${CMAKE_CURRENT_SOURCE_DIR}/_deps/lib/pkgconfig:$ENV{PKG_CONFIG_PATH}")
endif(WIN32)

# pkg-config path for dependencies
include_directories("${CMAKE_CURRENT_SOURCE_DIR}/_deps/include")
link_directories("${CMAKE_CURRENT_SOURCE_DIR}/_deps/lib")
###############################################################################
add_subdirectory(external/libavif-container EXCLUDE_FROM_ALL)
###############################################################################
find_package(PkgConfig)
pkg_check_modules(ZLIB REQUIRED zlib)
pkg_check_modules(LIBPNG REQUIRED libpng)
###############################################################################
set(ENABLE_TESTS OFF CACHE BOOL "" FORCE)
set(ENABLE_EXAMPLES OFF CACHE BOOL "" FORCE)
set(ENABLE_TOOLS OFF CACHE BOOL "" FORCE)
# See: external/libaom/build/cmake/aom_config_defaults.cmake
set(CONFIG_TUNE_VMAF 1 CACHE INTERNAL "Enable encoding tuning for VMAF." FORCE)
set(CONFIG_NN_V2 1 CACHE INTERNAL "Fully-connected neural nets ver.2." FORCE)
set(CONFIG_COLLECT_PARTITION_STATS 1 CACHE INTERNAL "AV1 experiment: Collect partition timing stats. Can be 1 or 2." FORCE)
set(CONFIG_PARTITION_SEARCH_ORDER 1 CACHE INTERNAL "AV1 experiment: Use alternative partition search order." FORCE)
# FIXME(ledyba-z): CONFIG_DIST_8X8 conflicts with CONFIG_MULTITHREAD.
# --- aom_configure: Detected CPU: x86_64
# CMake Warning at external/libaom/build/cmake/util.cmake:57 (message):
#  --- Disabled CONFIG_DIST_8X8, incompatible with CONFIG_MULTITHREAD.
#
set(CONFIG_AV1_DECODER 0 CACHE STRING "Enable AV1 decoder." FORCE)
add_subdirectory(external/libaom EXCLUDE_FROM_ALL)
###############################################################################

add_executable(cavif
    src/main.cpp

    src/math/Fraction.hpp

    src/img/Conversion.hpp
    src/img/png/Reader.cpp
    src/img/png/Reader.hpp

    src/Config.cpp
    src/Config.hpp

    src/AVIFBuilder.cpp
    src/AVIFBuilder.hpp

    src/ext/ExternalPartitionModelFactoryContainer.cpp
    src/ext/ExternalPartitionModelFactoryContainer.hpp
    src/ext/ExternalPartitionModel.hpp

    src/ext/models/NonSplitPartitionModel.cpp
    src/ext/models/NonSplitPartitionModel.hpp
)

# https://cmake.org/cmake/help/latest/manual/cmake-compile-features.7.html#requiring-language-standards
# https://stackoverflow.com/questions/45688522/how-to-enable-c17-in-cmake
target_compile_features(cavif PRIVATE cxx_std_17)
# https://cmake.org/cmake/help/latest/prop_tgt/CXX_STANDARD.html
set_property(TARGET cavif PROPERTY CXX_STANDARD 17)
set_property(TARGET cavif PROPERTY CXX_STANDARD_REQUIRED ON)

if (CMAKE_CXX_COMPILER_ID STREQUAL "GNU" OR CMAKE_CXX_COMPILER_ID STREQUAL "Clang")
  set_property(TARGET cavif PROPERTY CXX_FLAGS_DEBUG "-g3 -O0 -fno-omit-frame-pointer")
endif()

target_link_libraries(cavif PRIVATE aom)
target_link_libraries(cavif PRIVATE fmt::fmt)
target_link_libraries(cavif PRIVATE libavif-container)
target_include_directories(cavif PRIVATE external/libaom)
target_include_directories(cavif PRIVATE "${CMAKE_CURRENT_BINARY_DIR}/external/libaom")
target_include_directories(cavif PRIVATE external/libavif-container/src)
target_include_directories(cavif PRIVATE "${CMAKE_BINARY_DIR}/include")

target_include_directories(cavif PRIVATE ${LIBPNG_INCLUDE_DIRS})
target_include_directories(cavif PRIVATE ${ZLIB_INCLUDE_DIRS})
target_link_libraries(cavif PRIVATE ${LIBPNG_LDFLAGS} ${LIBPNG_LIBRARIES})
target_link_libraries(cavif PRIVATE ${ZLIB_LDFLAGS} ${ZLIB_LIBRARIES})
if (CMAKE_CXX_COMPILER_ID STREQUAL "GNU")
  # FIXME(ledyba-z): workaround for gcc-8
  target_link_libraries(cavif PRIVATE stdc++fs)
endif (CMAKE_CXX_COMPILER_ID STREQUAL "GNU")

###############################################################################

install(TARGETS cavif
    RUNTIME DESTINATION bin
)

install(DIRECTORY external/vmaf/model
    DESTINATION share/cavif
)
