cmake_minimum_required(VERSION 3.10)
project(pstudio.ztex VERSION 0.1.0)


execute_process(
		COMMAND git log -1 --format=%h
		WORKING_DIRECTORY .
		OUTPUT_VARIABLE GIT_HASH
		OUTPUT_STRIP_TRAILING_WHITESPACE
)

configure_file(config.hh.in ${CMAKE_CURRENT_BINARY_DIR}/config.hh @ONLY)

add_executable(ztex main.cc)
target_link_libraries(ztex PRIVATE phoenix CLI11 stb fmt)
target_include_directories(ztex PRIVATE ${CMAKE_CURRENT_BINARY_DIR})

set_target_properties(ztex PROPERTIES
		ARCHIVE_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/lib"
		LIBRARY_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/lib"
		RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/bin")
