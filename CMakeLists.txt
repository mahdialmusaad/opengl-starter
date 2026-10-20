cmake_minimum_required(VERSION 3.16)
project(gltutorial LANGUAGES C)

set_property(GLOBAL PROPERTY USE_FOLDERS ON)

set(GLFW_BUILD_DOCS OFF)
set(GLFW_INSTALL OFF)

add_subdirectory(external/glfw)
add_executable(gltutorial main.c gl.c)

target_include_directories(gltutorial PRIVATE external)
target_link_libraries(gltutorial PRIVATE glfw)
target_compile_definitions(gltutorial PRIVATE GLFW_INCLUDE_NONE)

set_target_properties(gltutorial PROPERTIES C_STANDARD 99 C_EXTENSIONS OFF C_STANDARD_REQUIRED ON)

if(NOT MSVC)
	target_compile_options(gltutorial PRIVATE -Wall -Wextra)
else()
	target_compile_options(gltutorial PRIVATE /W4)
endif()

if(UNIX)
	target_link_libraries(gltutorial PRIVATE m)
	target_compile_definitions(gltutorial PRIVATE _GNU_SOURCE)
endif()
