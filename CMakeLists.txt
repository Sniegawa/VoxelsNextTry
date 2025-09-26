cmake_minimum_required (VERSION 3.16)

project(Voxels)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

add_subdirectory(Libs/GLAD)
add_subdirectory(Libs/glfw)


# glm is header-only so only include_directories needed
include_directories(Libs/glm)

set(IMGUI_DIR ${CMAKE_CURRENT_SOURCE_DIR}/Libs/imgui)

set(IMGUI_SOURCES
	${IMGUI_DIR}/imgui.cpp
	${IMGUI_DIR}/imgui_draw.cpp
	${IMGUI_DIR}/imgui_widgets.cpp
	${IMGUI_DIR}/imgui_tables.cpp
	${IMGUI_DIR}/imgui_demo.cpp
	${IMGUI_DIR}/imgui_impl_glfw.cpp
	${IMGUI_DIR}/imgui_impl_opengl3.cpp
)

add_executable(Voxels 
${IMGUI_SOURCES} 
"src/main.cpp" 
"src/Application.h" 
"src/Application.cpp"
"src/Renderer/Shader.h" 
"src/Renderer/Shader.cpp"
"src/Renderer/Buffers.h"
"src/Renderer/Buffers.cpp"
"src/Renderer/Camera.h"
"src/Renderer/Camera.cpp"
"src/PerlinNoise.hpp" "src/Renderer/Chunk.h" "src/Renderer/Chunk.cpp" "src/Renderer/World.h" "src/Renderer/World.cpp")

target_link_libraries (Voxels PUBLIC
		GLAD
		glfw
)

target_include_directories(Voxels PUBLIC
	${CMAKE_SOURCE_DIR}/Libs/GLAD/include
	${CMAKE_SOURCE_DIR}/Libs/glfw/include
	${CMAKE_SOURCE_DIR}/Libs/glm
	${IMGUI_DIR}
)

add_custom_command(
    TARGET Voxels POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_directory
        ${CMAKE_SOURCE_DIR}/src/Shaders
        $<TARGET_FILE_DIR:Voxels>/Shaders
)