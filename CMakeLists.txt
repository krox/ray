cmake_minimum_required (VERSION 2.8.9)
project (ray)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -O2 -Wall -Wextra -Werror -pedantic")

add_subdirectory(util)
add_subdirectory(fmt)
add_subdirectory(glm)

include_directories(src)
include_directories(util/src)
include_directories(util/src)
include_directories(glm)

file(GLOB files_cpp "src/ray/*.cpp")

add_executable(ray src/main.cpp ${files_cpp})
target_link_libraries(ray util fmt SDL2)
