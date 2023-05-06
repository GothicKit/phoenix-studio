option(PHOENIX_BUILD_TESTS OFF)
option(PHOENIX_BUILD_EXAMPLES OFF)
option(PHOENIX_BUILD_TOOLS OFF)
option(PHOENIX_DISABLE_SANITIZERS ${PSTUDIO_DISABLE_SANITIZERS})

add_subdirectory(phoenix)
add_subdirectory(json)
add_subdirectory(fmt)
add_subdirectory(glad)
add_subdirectory(glfw)

set(CLI11_PRECOMPILED ON)
add_subdirectory(CLI11)

# add stb as a library
add_library(stb INTERFACE)
target_include_directories(stb INTERFACE stb)
