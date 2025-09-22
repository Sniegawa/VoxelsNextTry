cmake_minimum_required (VERSION 3.16)

project(Voxels)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

add_subdirectory(Libs/GLAD)
add_subdirectory(Libs/glfw)


# glm is header-only so only include_directories needed
include_directories(Libs/glm)

add_executable(Voxels "src/main.cpp" "src/Application.h" "src/Application.cpp" "src/Renderer/Shader.h" "src/Renderer/Shader.cpp" "src/Renderer/Buffers.h" "src/Renderer/Buffers.cpp")

target_link_libraries (Voxels PUBLIC
		GLAD
		glfw
)

target_include_directories(Voxels PUBLIC
	${CMAKE_SOURCE_DIR}/Libs/GLAD/include
	${CMAKE_SOURCE_DIR}/Libs/glfw/include
	${CMAKE_SOURCE_DIR}/Libs/glm
)

add_custom_command(
    TARGET Voxels POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_directory
        ${CMAKE_SOURCE_DIR}/src/Shaders
        $<TARGET_FILE_DIR:Voxels>/Shaders
)