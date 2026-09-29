cmake_minimum_required(VERSION 3.16)
project(ImageProcessor LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

add_executable(ImageProcessor
    src/main.cpp
    src/BmpParser.cpp
    src/CommandLineParser.cpp
    src/Filters.cpp
)

target_include_directories(ImageProcessor PRIVATE include)

if (MSVC)
    target_compile_options(ImageProcessor PRIVATE /W4 /permissive-)
else()
    target_compile_options(ImageProcessor PRIVATE -Wall -Wextra -Wpedantic)
endif()
