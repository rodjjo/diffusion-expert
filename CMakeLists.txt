cmake_minimum_required(VERSION 3.6.0 FATAL_ERROR)

project(DiffusionExpert C CXX)

include(${CMAKE_CURRENT_LIST_DIR}/build/conanbuildinfo.cmake)
conan_basic_setup(TARGETS)

set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} ${CMAKE_CURRENT_SOURCE_DIR}/cmake-modules)
set(CMAKE_CXX_STANDARD 17)
set(FLTK_SKIP_FLUID true)

find_package(FLTK REQUIRED)
find_package(nlohmann_json REQUIRED)
find_package(PythonLibs REQUIRED)

macro(print_all_variables)
    message(STATUS "print_all_variables------------------------------------------{")
    get_cmake_property(_variableNames VARIABLES)
    foreach (_variableName ${_variableNames})
        message(STATUS "${_variableName}=${${_variableName}}")
    endforeach()
    message(STATUS "print_all_variables------------------------------------------}")
endmacro()


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
                    ${PYTHON_INCLUDE_DIRS}
                    ${nlohmann_json_INCLUDE_DIRS_RELEASE})


add_subdirectory("src")

enable_testing()
add_subdirectory("tests")