cmake_minimum_required(VERSION 3.6.0 FATAL_ERROR)

project(DiffusionExpert C CXX)

set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} ${CMAKE_CURRENT_SOURCE_DIR}/cmake-modules)
set(CMAKE_CXX_STANDARD 17)
set(FLTK_BUILD_FLUID ON)
set(FLTK_BUILD_EXAMPLES OFF)
set(FLTK_BUILD_TEST OFF)
set(OPTION_BUILD_SHARED_LIBS OFF)

macro(print_all_variables)
    message(STATUS "print_all_variables------------------------------------------{")
    get_cmake_property(_variableNames VARIABLES)
    foreach (_variableName ${_variableNames})
        message(STATUS "${_variableName}=${${_variableName}}")
    endforeach()
    message(STATUS "print_all_variables------------------------------------------}")
endmacro()

set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${PROJECT_SOURCE_DIR}/build/lib)
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${PROJECT_SOURCE_DIR}/build/lib)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${PROJECT_SOURCE_DIR}/build/bin)

include_directories(DiffusionExpert
                    ${PROJECT_SOURCE_DIR}
                    # ${FLTK_INCLUDE_DIR}
                    ${PYTHON_INCLUDE_DIRS}
                    "${CMAKE_CURRENT_LIST_DIR}/dependencies/cimg"
                    "${CMAKE_CURRENT_LIST_DIR}/dependencies/nlohmann_json/include"
                    "${CMAKE_CURRENT_LIST_DIR}/dependencies/pybind11/include"
                    "${CMAKE_CURRENT_LIST_DIR}/build/fltk/include"
                    "${CMAKE_CURRENT_LIST_DIR}/dependencies/python3.10.11/include"
                    )

link_directories(DiffusionExpert
    "${CMAKE_CURRENT_LIST_DIR}/build/fltk/lib"
    "${CMAKE_CURRENT_LIST_DIR}/dependencies/python3.10.11/libs"
)

add_subdirectory("src2")
add_subdirectory("src")

enable_testing()
add_subdirectory("tests")