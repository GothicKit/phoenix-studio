cmake_minimum_required(VERSION 3.10)
project(glad)

add_library(glad STATIC src/gl.c src/vulkan.c)
target_include_directories(glad PUBLIC include)
