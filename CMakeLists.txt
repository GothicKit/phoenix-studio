cmake_minimum_required(VERSION 3.10)
project(phoenix-studio)

option(PSTUDIO_DISABLE_SANITIZERS "Build without sanitizers in debug mode" OFF)

set(CMAKE_CXX_STANDARD 17)

if (MSVC)
    # enable all warnings
    set(PSTUDIO_CXX_FLAGS "/W4")

    # in debug mode, enable sanitizers
    if (${CMAKE_BUILD_TYPE} MATCHES "Debug" AND NOT ${PSTUDIO_DISABLE_SANITIZERS})
        set(PSTUDIO_CXX_FLAGS ${PSTUDIO_CXX_FLAGS} "/fsanitize=address")
    endif ()
else ()
    # enable all warnings
    set(PSTUDIO_CXX_FLAGS -Wall -Wextra -Werror -Wconversion)

    # in debug mode, enable sanitizers
    if (${CMAKE_BUILD_TYPE} MATCHES "Debug" AND NOT ${PSTUDIO_DISABLE_SANITIZERS})
        set(PSTUDIO_CXX_FLAGS ${PSTUDIO_CXX_FLAGS} -fsanitize=address -fsanitize=undefined)

        # when not compiling for MacOS, enable leak sanitizer
        if (NOT APPLE)
            set(PSTUDIO_CXX_FLAGS ${PSTUDIO_CXX_FLAGS} -fsanitize=leak)
        endif ()
    endif ()

    # in debug mode on Clang to get proper debugging support, add -fstandalone-debug
    if (${CMAKE_BUILD_TYPE} MATCHES "Debug" AND ${CMAKE_CXX_COMPILER_ID} MATCHES "Clang")
        set(PSTUDIO_CXX_FLAGS ${PSTUDIO_CXX_FLAGS} -fstandalone-debug)
    endif ()
endif ()

add_subdirectory(vendor)
add_subdirectory(src)
