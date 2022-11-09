cmake_minimum_required(VERSION 3.10)

file(STRINGS "version" projectVersion LIMIT_COUNT 1)
project(LPPShow VERSION ${projectVersion})

include(ExternalProject)

option(USE_CDDLIB "Use cddlib and enable compute step" OFF)
option(USE_OBJ_LOADER "Use wavefront OBJ Loader (shouldn't be necessary)" OFF)
option(BAKE_SHADERS "Force baking in shaders" OFF)

message("Building LPPShow version ${projectVersion} mode: ${CMAKE_BUILD_TYPE}")

if(CMAKE_BUILD_TYPE MATCHES Debug)
    add_definitions(-DDEBUG)
endif()

# I think C++11 is the minimum, either way my GCC is c++14 default
set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUIRED True)

add_subdirectory(assets)
add_subdirectory(src)
add_subdirectory(thirdparty)

configure_file("config.h.in" "include/config.h")
add_executable(LPPShow src/LPPShow.cpp) 
target_include_directories(LPPShow PRIVATE "include")
target_include_directories(LPPShow PRIVATE "${PROJECT_BINARY_DIR}/include")

if (CMAKE_BUILD_TYPE MATCHES "Release" OR BAKE_SHADERS)
    add_dependencies(LPPShow bake)
endif()

set(LIBRARIES framework glad imgui)

if(USE_CDDLIB)
    list(APPEND LIBRARIES quickhull cddlib)
endif()

if (WIN32)
    message("Condolences.")
    list(APPEND LIBRARIES opengl32 gdi32 glfw3)
    list(APPEND LIBRARIES -static)
    target_link_directories(LPPShow PUBLIC libraries) # Fallback
    # I have nothing.
    # Plastic world has won.
    target_link_directories(LPPShow PUBLIC libraries/glfw-vc2022)
endif()

if (UNIX AND NOT APPLE)
    list(APPEND LIBRARIES glfw)
endif()

target_link_libraries(LPPShow PUBLIC ${LIBRARIES})
target_link_libraries(LPPShow PUBLIC rt m dl)

install(TARGETS LPPShow DESTINATION "${CMAKE_SOURCE_DIR}/dist")
