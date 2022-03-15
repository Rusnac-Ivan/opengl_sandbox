cmake_minimum_required(VERSION 3.12)

project(TestGL C CXX)

set(GLFW_BUILD_DOCS OFF CACHE BOOL "" FORCE)
set(GLFW_BUILD_TESTS OFF CACHE BOOL "" FORCE)
set(GLFW_BUILD_EXAMPLES OFF CACHE BOOL "" FORCE)
add_subdirectory(${CMAKE_SOURCE_DIR}/Libraries/glfw-3.3.3)

set(BUILD_STATIC_LIBS ON)
add_subdirectory(${CMAKE_SOURCE_DIR}/Libraries/glm-0.9.9.8)

find_package(OpenGL REQUIRED)

include_directories(${CMAKE_SOURCE_DIR}/Sources)
include_directories(${CMAKE_SOURCE_DIR}/Libraries/glad-0.1.34/include)
include_directories(${CMAKE_SOURCE_DIR}/Libraries/imgui-1.82)
include_directories(${CMAKE_SOURCE_DIR}/Libraries/imgui-1.82/backends)

file(GLOB SOURCES_FILES ${CMAKE_SOURCE_DIR}/Libraries/glad-0.1.34/src/glad.c)
file(GLOB SOURCES_FILES ${SOURCES_FILES} ${CMAKE_SOURCE_DIR}/Sources/main.cpp)

file(GLOB ImGui ${CMAKE_SOURCE_DIR}/Libraries/imgui-1.82/*.cpp ${CMAKE_SOURCE_DIR}/Libraries/imgui-1.82/*.h)
file(GLOB Backend  ${CMAKE_SOURCE_DIR}/Libraries/imgui-1.82/backends/imgui_impl_glfw.cpp ${CMAKE_SOURCE_DIR}/Libraries/imgui-1.82/backends/imgui_impl_glfw.h)
file(GLOB Backend ${Backend} ${CMAKE_SOURCE_DIR}/Libraries/imgui-1.82/backends/imgui_impl_opengl3.cpp ${CMAKE_SOURCE_DIR}/Libraries/imgui-1.82/backends/imgui_impl_opengl3.h)

file(GLOB Core ${CMAKE_SOURCE_DIR}/Sources/Core/*.cpp ${CMAKE_SOURCE_DIR}/Sources/Core/*.h)
file(GLOB GLObjects ${CMAKE_SOURCE_DIR}/Sources/GLObjects/*.cpp ${CMAKE_SOURCE_DIR}/Sources/GLObjects/*.h)
file(GLOB Rendering ${CMAKE_SOURCE_DIR}/Sources/Rendering/*.cpp ${CMAKE_SOURCE_DIR}/Sources/Rendering/*.h)
file(GLOB Tools ${CMAKE_SOURCE_DIR}/Sources/Tools/*.cpp ${CMAKE_SOURCE_DIR}/Sources/Tools/*.h)

file(GLOB Pieces ${CMAKE_SOURCE_DIR}/Sources/Pieces/*.h)

set(LIBS glm glfw opengl32)

message("Project name: ${PROJECT_NAME}\n")
message("Source files: ${SOURCES_FILES}\n")

add_executable(${PROJECT_NAME} ${SOURCES_FILES} ${ImGui} ${Backend} ${Core} ${GLObjects} ${Rendering} ${Tools} ${Pieces})

source_group("Backend" FILES ${Backend})
source_group("ImGui" FILES ${ImGui})
source_group("Core" FILES ${Core})
source_group("GLObjects" FILES ${GLObjects})
source_group("Rendering" FILES ${Rendering})
source_group("Tools" FILES ${Tools})
source_group("Pieces" FILES ${Pieces})

target_link_libraries(${PROJECT_NAME} ${LIBS})


