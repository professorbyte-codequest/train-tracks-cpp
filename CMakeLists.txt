cmake_minimum_required(VERSION 3.10)

# Project name and version
project(TrainTracks++ VERSION 1.0 LANGUAGES CXX)

# Set the C++ standard
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED True)

# Add the source directory
add_subdirectory(src)
add_subdirectory(runner)

# Add the source directory to includes
include_directories(src)

# Add the tests directory
enable_testing()
add_subdirectory(tests)