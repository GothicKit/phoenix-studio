cmake_minimum_required(VERSION 3.10)
project(pstudio.editor VERSION 0.1.0)

execute_process(
        COMMAND git log -1 --format=%h
        WORKING_DIRECTORY .
        OUTPUT_VARIABLE GIT_HASH
        OUTPUT_STRIP_TRAILING_WHITESPACE
)

configure_file(Config.hh.in ${CMAKE_CURRENT_BINARY_DIR}/Config.hh @ONLY)

add_executable(editor Main.cc Window.cc Resources.cc gl/GlShader.cc gl/GlTexture.cc gl/GlArray.cc)
target_link_libraries(editor PRIVATE phoenix fmt glad glfw)
target_include_directories(editor PRIVATE ${CMAKE_CURRENT_BINARY_DIR} ${CMAKE_CURRENT_SOURCE_DIR})

set_target_properties(editor PROPERTIES
        ARCHIVE_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/lib"
        LIBRARY_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/lib"
        RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/bin")
