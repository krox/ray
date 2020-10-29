cmake_minimum_required (VERSION 2.8.9)
project (ray)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)
# NOTE on compiler flags:
#   * -fno-math-errno does not change any results (unlike -ffast-math or
#     -ffinite-math-only). It only assumes we don't need errno set by math
#     functions. This enables using AVX sqrt instructions and such without
#     branching, which is faster than the software sqrt.
#   * with -O2, our own vector implementation is significantly slower than
#     the GLM library. With -O3 it is very close. (tested on gcc 7.5).
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -O3 -Wall -Wextra -Werror -pedantic -Wno-type-limits -march=native -fno-math-errno")

add_subdirectory(util)
add_subdirectory(fmt)
add_subdirectory(json)
add_subdirectory(CLI11)

include_directories(src)
include_directories(util/src)
include_directories(util/src)
include_directories(CLI11/include)

file(GLOB files_cpp "src/*/*.cpp")

add_executable(ray src/main.cpp ${files_cpp})
target_link_libraries(ray util fmt SDL2 nlohmann_json::nlohmann_json pthread)
