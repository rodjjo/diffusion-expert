file(GLOB Sources
    "${CMAKE_CURRENT_SOURCE_DIR}/data/*.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/config/*.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/controls/*.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/windows/*.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/python/*.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/geometry/*.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/dialogs/*.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/panels/*.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/console/*.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/opengl_utils/*.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/*.cpp")

file(GLOB SourcesEntry
    "${CMAKE_CURRENT_SOURCE_DIR}/entry-point/*.cpp")

if(WIN32)
add_executable(diffusion-exp WIN32 ${Sources})
else(WIN32)
add_executable(diffusion-exp ${Sources})
endif(WIN32)

if(WIN32)
add_executable(diffusion-exp-win WIN32 ${SourcesEntry})
endif(WIN32)

target_link_libraries(diffusion-exp stdc++ ${FLTK_LIBRARIES} ${PYTHON_LIBRARIES})

set_target_properties(diffusion-exp
    PROPERTIES
    LINKER_LANGUAGE C
)

if(WIN32)
set(SUBSYSTEM_LINKER_OPTIONS "-mconsole")
endif(WIN32)

target_link_options(diffusion-exp PRIVATE ${SUBSYSTEM_LINKER_OPTIONS})

install(TARGETS diffusion-exp
        RUNTIME DESTINATION bin
        COMPONENT runtime)

if(WIN32)
install(TARGETS diffusion-exp-win
        RUNTIME DESTINATION bin
        COMPONENT runtime)
endif()