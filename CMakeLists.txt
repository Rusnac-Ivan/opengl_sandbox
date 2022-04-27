cmake_minimum_required(VERSION 3.12)

project(opengl_sandbox C CXX)

# Include cmake modules
if(NOT DEFINED EMSCRIPTEN)
    list(APPEND CMAKE_MODULE_PATH "${CMAKE_CURRENT_SOURCE_DIR}/cmake")
endif()

# Set src and include root path
set(SRC_ROOT_PATH "${CMAKE_CURRENT_SOURCE_DIR}/sources/")
# Set src and include root path
set(LIBS_ROOT_PATH "${CMAKE_CURRENT_SOURCE_DIR}/libraries/")

file(
    GLOB_RECURSE SOURCE_FILES
    LIST_DIRECTORIES false
    "${SRC_ROOT_PATH}/*.c"
    "${SRC_ROOT_PATH}/*.cpp"
    "${SRC_ROOT_PATH}/*.h"
    "${SRC_ROOT_PATH}/*.hpp"
)

file(
    GLOB_RECURSE LIBS_FILES
    LIST_DIRECTORIES false
    "${LIBS_ROOT_PATH}/*.c"
    "${LIBS_ROOT_PATH}/*.cpp"
    "${LIBS_ROOT_PATH}/*.h"
    "${LIBS_ROOT_PATH}/*.hpp"
)

foreach(_source IN ITEMS ${SOURCE_FILES})
    get_filename_component(_source_path "${_source}" PATH)
    file(RELATIVE_PATH _source_path_rel "${SRC_ROOT_PATH}" "${_source_path}")
    string(REPLACE "/" "\\" _group_path "${_source_path_rel}")
    source_group("source\\${_group_path}" FILES "${_source}")
endforeach()

foreach(_source IN ITEMS ${LIBS_FILES})
    get_filename_component(_source_path "${_source}" PATH)
    file(RELATIVE_PATH _source_path_rel "${LIBS_ROOT_PATH}" "${_source_path}")
    string(REPLACE "/" "\\" _group_path "${_source_path_rel}")
    source_group("source\\${_group_path}" FILES "${_source}")
endforeach()

set(GLAD_INCLUDE_DIR "${LIBS_ROOT_PATH}/glad-0.1.34/include")


if(DEFINED EMSCRIPTEN)
    if (CMAKE_BUILD_TYPE MATCHES Debug)	
        set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -s NO_EXIT_RUNTIME=1 -s USE_WEBGL2=1  -s USE_GLFW=3 -s DEMANGLE_SUPPORT=1 -s ASSERTIONS=2 --bind -s WASM=1 -s DISABLE_DEPRECATED_FIND_EVENT_TARGET_BEHAVIOR=1 -s TOTAL_MEMORY=1GB -o out.html")
    else()
        add_definitions(-DNDEBUG)
        set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -s NO_EXIT_RUNTIME=1 -s USE_WEBGL2=1 -s USE_GLFW=3 --bind -s WASM=1 -s INITIAL_MEMORY=1GB -s MAXIMUM_MEMORY=3GB -s ALLOW_MEMORY_GROWTH=1 -o out.html")
    endif()
endif()

if(NOT DEFINED EMSCRIPTEN)
    add_executable(${PROJECT_NAME} ${INCLUDE_LIST} ${SOURCE_FILES} ${LIBS_FILES})
else()
    add_executable(${PROJECT_NAME} ${INCLUDE_LIST} ${SOURCE_FILES})
endif()

#target_include_directories(${PROJECT_NAME} PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/src/ ${GLAD_INCLUDE_DIR})
include_directories(${SRC_ROOT_PATH}/engine)

if(NOT DEFINED EMSCRIPTEN)
    target_include_directories(${PROJECT_NAME} PRIVATE ${GLAD_INCLUDE_DIR})

    find_package(OpenGL REQUIRED)

    find_package(GLFW3 REQUIRED)
    target_include_directories(${PROJECT_NAME} PRIVATE ${GLFW3_INCLUDE_DIR})
    target_link_libraries(${PROJECT_NAME} PRIVATE ${GLFW3_LIBRARY})
    message(STATUS "GLFW3_INCLUDE_DIR: ${GLFW3_INCLUDE_DIR}")
    message(STATUS "GLFW3_LIBRARY: ${GLFW3_LIBRARY}")
endif()




if(IMGUI_PATH)
    #message(STATUS "IMGUI_PATH: ${IMGUI_PATH}")
    set(IMGUI_SRC 
            ${IMGUI_PATH}/imconfig.h
            ${IMGUI_PATH}/imgui.cpp
            ${IMGUI_PATH}/imgui.h
            ${IMGUI_PATH}/imgui_demo.cpp
            ${IMGUI_PATH}/imgui_draw.cpp
            ${IMGUI_PATH}/imgui_internal.h
            ${IMGUI_PATH}/imgui_tables.cpp
            ${IMGUI_PATH}/imgui_widgets.cpp
            ${IMGUI_PATH}/imstb_rectpack.h
            ${IMGUI_PATH}/imstb_textedit.h
            ${IMGUI_PATH}/imstb_truetype.h
            ${IMGUI_PATH}/backends/imgui_impl_opengl3.cpp
            ${IMGUI_PATH}/backends/imgui_impl_opengl3.h
            ${IMGUI_PATH}/backends/imgui_impl_glfw.cpp
            ${IMGUI_PATH}/backends/imgui_impl_glfw.h
    )
    
    message(STATUS "IMGUI_SRC: ${IMGUI_SRC}")
    target_sources(${PROJECT_NAME} PRIVATE ${IMGUI_SRC})
    target_include_directories(${PROJECT_NAME} PRIVATE ${IMGUI_PATH})
else()
    message(FATAL_ERROR "IMGUI_PATH is not set!")
endif()

if(OPENFBX_PATH)
    set(OPENFBX_SRC 
            ${OPENFBX_PATH}/miniz.c
            ${OPENFBX_PATH}/miniz.h
            ${OPENFBX_PATH}/ofbx.cpp
            ${OPENFBX_PATH}/ofbx.h
    )
    message(STATUS "OPENFBX_PATH: ${OPENFBX_PATH}")
    
    target_sources(${PROJECT_NAME} PRIVATE ${OPENFBX_SRC})
    target_include_directories(${PROJECT_NAME} PRIVATE ${OPENFBX_PATH})
else()
    message(STATUS "OPENFBX_PATH is not set!")
endif()

if(TINYGLTF_PATH)
    message(STATUS "TINYGLTF_PATH: ${TINYGLTF_PATH}")
    set(TINYGLTF_SRC 
            ${TINYGLTF_PATH}/json.hpp
            ${TINYGLTF_PATH}/stb_image.h
            ${TINYGLTF_PATH}/stb_image_write.h
            ${TINYGLTF_PATH}/tiny_gltf.h
            #${TINYGLTF_PATH}/tiny_gltf.cc
    )
    target_sources(${PROJECT_NAME} PRIVATE ${TINYGLTF_SRC})
    target_include_directories(${PROJECT_NAME} PRIVATE ${TINYGLTF_PATH})
else()
     message(TINYGLTF_PATH "TINYGLTF_PATH is not set!")
endif()

#find_package(tinygltf REQUIRED)
#target_include_directories(${PROJECT_NAME} PRIVATE ${TinyGLTF_INCLUDE})
#target_link_libraries(${PROJECT_NAME} PRIVATE tinygltf)
#message(STATUS "TinyGLTF_INCLUDE: ${TinyGLTF_INCLUDE}")

find_package(glm REQUIRED)
target_link_libraries(${PROJECT_NAME} PRIVATE glm::glm)

