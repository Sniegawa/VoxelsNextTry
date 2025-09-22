cmake_minimum_required(VERSION 3.8)

project(GLAD)

add_library(GLAD STATIC 
"${CMAKE_CURRENT_SOURCE_DIR}/src/glad.c"
"${CMAKE_CURRENT_SOURCE_DIR}/include/glad/glad.h"
"${CMAKE_CURRENT_SOURCE_DIR}/include/KHR/khrplatform.h"
)

target_include_directories(GLAD PRIVATE
    ${CMAKE_SOURCE_DIR}/Libs/GLAD/include)
