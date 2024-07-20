cmake_minimum_required(VERSION 3.6.0 FATAL_ERROR)

project(DiffusionExpert C CXX)

set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} ${CMAKE_CURRENT_LIST_DIR}/cmake-modules)
set(CMAKE_CXX_STANDARD 17)
set(FLTK_BUILD_FLUID ON)
set(FLTK_BUILD_EXAMPLES OFF)
set(FLTK_BUILD_TEST OFF)
set(OPTION_BUILD_SHARED_LIBS OFF)
set(APP_NAME diffusion-expert)

macro(print_all_variables)
    message(STATUS "print_all_variables------------------------------------------{")
    get_cmake_property(_variableNames VARIABLES)
    foreach (_variableName ${_variableNames})
        message(STATUS "${_variableName}=${${_variableName}}")
    endforeach()
    message(STATUS "print_all_variables------------------------------------------}")
endmacro()

file(GLOB Sources
    "${CMAKE_CURRENT_LIST_DIR}/diffusion/*.cpp"
    "${CMAKE_CURRENT_LIST_DIR}/diffusion/gui/*.cpp"
    "${CMAKE_CURRENT_LIST_DIR}/diffusion/data/*.cpp"
    "${CMAKE_CURRENT_LIST_DIR}/diffusion/python/*.cpp"
    "${CMAKE_CURRENT_LIST_DIR}/diffusion/application/*.cpp"
)

include_directories(
    DiffusionExpert
    ${PROJECT_SOURCE_DIR}
    ${PYTHON_INCLUDE_DIRS}
    "${CMAKE_CURRENT_LIST_DIR}/dependencies/cimg"
    "${CMAKE_CURRENT_LIST_DIR}/dependencies/nlohmann_json/include"
    "${CMAKE_CURRENT_LIST_DIR}/dependencies/pybind11/include"
    "${CMAKE_CURRENT_LIST_DIR}/dependencies/sfml/include"
)

add_subdirectory("${CMAKE_CURRENT_LIST_DIR}/dependencies/sfml")
add_subdirectory("${CMAKE_CURRENT_LIST_DIR}/diffusion/gui")


add_executable(
    ${APP_NAME}
    ${Sources}
)


target_include_directories(
    ${APP_NAME} PUBLIC 
    ${CMAKE_CURRENT_SOURCE_DIR}
    "${CMAKE_CURRENT_SOURCE_DIR}/diffusion/gui/include"
)

target_link_libraries(
    ${APP_NAME}
    simple-ui
    stdc++ 
    sfml-system
    sfml-graphics
)

set_target_properties(${APP_NAME}
    PROPERTIES
    LINKER_LANGUAGE C
)

if(WIN32)
    target_link_libraries(
        ${APP_NAME}
        python3.lib
        python310.lib
        opengl32
        user32
        comctl32
        gdiplus
        ws2_32
        glu32
        Shcore
    )
    set(SUBSYSTEM_LINKER_OPTIONS "-mconsole")
    target_link_options(${APP_NAME} PRIVATE ${SUBSYSTEM_LINKER_OPTIONS})
endif(WIN32)


install(TARGETS ${APP_NAME}
        RUNTIME DESTINATION bin
        COMPONENT runtime)
