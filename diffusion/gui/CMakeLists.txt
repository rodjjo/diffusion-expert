cmake_minimum_required(VERSION 3.6.0 FATAL_ERROR)

project(SimpleUI C CXX)

set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} ${CMAKE_CURRENT_LIST_DIR}/cmake-modules)
set(CMAKE_CXX_STANDARD 17)
set(FLTK_BUILD_FLUID ON)
set(FLTK_BUILD_EXAMPLES OFF)
set(FLTK_BUILD_TEST OFF)
set(OPTION_BUILD_SHARED_LIBS OFF)
set(APP_NAME simple-ui)


file(GLOB Sources
    "${CMAKE_CURRENT_LIST_DIR}/*.cpp"
    "${CMAKE_CURRENT_LIST_DIR}/xpm/*.cpp"
)

add_library(${APP_NAME} STATIC ${Sources})

target_include_directories(${APP_NAME} PUBLIC 
     ${CMAKE_CURRENT_SOURCE_DIR}
    "${CMAKE_CURRENT_LIST_DIR}/include"
)
