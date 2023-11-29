file(GLOB Sources
    "${CMAKE_CURRENT_SOURCE_DIR}/misc/*.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/windows/*.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/python/*.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/terminal/*.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/messagebus/*.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/components/*.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/components/xpm/*.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/*.cpp")

file(GLOB SourcesEntry
    "${CMAKE_CURRENT_SOURCE_DIR}/entrypoint/*.cpp")

if(WIN32)
add_executable(diffusion-exp2 WIN32 ${Sources})
else(WIN32)
add_executable(diffusion-exp2 ${Sources})
endif(WIN32)

if(WIN32)
add_executable(diffusion-exp2-win WIN32 ${SourcesEntry})
endif(WIN32)

target_include_directories(diffusion-exp2 PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})
target_include_directories(diffusion-exp2-win PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})

target_link_libraries(
    diffusion-exp2
    stdc++ 
    fltk
    fltk_forms 
    fltk_images 
    fltk_jpeg 
    fltk_png 
    fltk_z 
    fltk_gl 
#   python3.lib
    python310.lib
    opengl32
    user32
    comctl32
    gdiplus
    ws2_32
    glu32
)

set_target_properties(diffusion-exp2
    PROPERTIES
    LINKER_LANGUAGE C
)

if(WIN32)
set(SUBSYSTEM_LINKER_OPTIONS "-mconsole")
endif(WIN32)

target_link_options(diffusion-exp2 PRIVATE ${SUBSYSTEM_LINKER_OPTIONS})

install(TARGETS diffusion-exp2
        RUNTIME DESTINATION bin
        COMPONENT runtime)

if(WIN32)
install(TARGETS diffusion-exp2-win
        RUNTIME DESTINATION bin
        COMPONENT runtime)
endif()