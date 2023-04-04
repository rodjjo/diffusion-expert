cmake_minimum_required(VERSION 3.6.0 FATAL_ERROR)

project(DiffusionExpert C CXX)

include(${CMAKE_CURRENT_LIST_DIR}/build/conanbuildinfo.cmake)
conan_basic_setup(TARGETS)

set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} ${CMAKE_CURRENT_SOURCE_DIR}/cmake-modules)
set(Boost_USE_STATIC_LIBS ON)
set(CMAKE_CXX_STANDARD 14)
set(FLTK_SKIP_FLUID true)

find_package(FLTK REQUIRED)
find_package(PythonLibs REQUIRED)

if ($ENV{DEBUG_ENABLED})
    message("Compliling in Debug build type")
    set(CMAKE_BUILD_TYPE Debug)
else ($ENV{DEBUG_ENABLED})
    message("Compliling in Release build type")
    set(CMAKE_BUILD_TYPE Release)
endif ($ENV{DEBUG_ENABLED})


set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${PROJECT_SOURCE_DIR}/build/lib)
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${PROJECT_SOURCE_DIR}/build/lib)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${PROJECT_SOURCE_DIR}/build/bin)


include_directories(DiffusionExpert
                    ${PROJECT_SOURCE_DIR}
                    ${FLTK_INCLUDE_DIR}
                    ${PYTHON_INCLUDE_DIRS})


add_subdirectory("src")

enable_testing()
add_subdirectory("tests")