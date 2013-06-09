cmake_minimum_required(VERSION 2.8)
project(SimpleVideoInput)

# Output settings

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${PROJECT_BINARY_DIR}/bin)
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${PROJECT_BINARY_DIR}/bin)
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${PROJECT_BINARY_DIR}/lib)
set(CMAKE_MODULE_PATH "${PROJECT_SOURCE_DIR}/cmake/modules;${CMAKE_MODULE_PATH}")

set(SVI_SRC_DIR ${PROJECT_SOURCE_DIR}/src)

# Use C++11

if (NOT MSVC)
	set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11")
endif (NOT MSVC)

if (APPLE)
	set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -stdlib=libc++")
endif (APPLE)

# Packages

find_package(OpenCV REQUIRED core)
find_package(FFmpeg REQUIRED)

# Options

option(WITH_EXAMPLES "add example projects" FALSE)
if (WITH_EXAMPLES)
	add_subdirectory(examples)
endif(WITH_EXAMPLES)

add_subdirectory(src)
