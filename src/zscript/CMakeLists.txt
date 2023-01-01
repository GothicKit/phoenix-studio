cmake_minimum_required(VERSION 3.10)
project(phoenix.tools.zscript VERSION 0.1.0)


execute_process(
		COMMAND git log -1 --format=%h
		WORKING_DIRECTORY .
		OUTPUT_VARIABLE GIT_HASH
		OUTPUT_STRIP_TRAILING_WHITESPACE
)

configure_file(config.hh.in ${CMAKE_CURRENT_BINARY_DIR}/config.hh @ONLY)

add_executable(zscript main.cc decompiler.cc)
target_link_libraries(zscript PRIVATE phoenix flags fmt)
target_include_directories(zscript PRIVATE ${CMAKE_CURRENT_BINARY_DIR})

set_target_properties(zscript
		PROPERTIES
		ARCHIVE_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/lib"
		LIBRARY_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/lib"
		RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/bin"
		)
