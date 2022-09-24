option(PHOENIX_BUILD_TESTS OFF)
option(PHOENIX_BUILD_EXAMPLES OFF)
option(PHOENIX_BUILD_TOOLS OFF)
option(PHOENIX_DISABLE_SANITIZERS ${PSTUDIO_DISABLE_SANITIZERS})

add_subdirectory(phoenix)
add_subdirectory(flags)

# add stb as a library
add_library(stb INTERFACE)
target_include_directories(stb INTERFACE stb)
