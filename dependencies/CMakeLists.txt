cmake_minimum_required(VERSION 3.6.0 FATAL_ERROR)

project(DiffusionExpertDeeps C CXX)

set(CMAKE_CXX_STANDARD 17)
set(FLTK_BUILD_FLUID ON)
set(FLTK_BUILD_EXAMPLES OFF)
set(FLTK_BUILD_TEST OFF)
set(OPTION_BUILD_SHARED_LIBS OFF)


include_directories(DiffusionExpertDeps
    "${CMAKE_CURRENT_LIST_DIR}/fltk"
)

add_subdirectory("fltk")