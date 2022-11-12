cmake_minimum_required(VERSION 3.11)

file(STRINGS "version" projectVersion LIMIT_COUNT 1)
project(LPPShow VERSION ${projectVersion})

include(ExternalProject)
include(FetchContent)

option(USE_CDDLIB "Use cddlib and enable compute step" ON)
option(USE_OBJ_LOADER "Use wavefront OBJ Loader (shouldn't be necessary)" OFF)
option(USE_BAKED_SHADERS "Bake in shaders for easier distribution" OFF)

if(NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE Release)
endif(NOT CMAKE_BUILD_TYPE)

message("Building LPPShow version ${projectVersion} mode: ${CMAKE_BUILD_TYPE}")
message("Using ${CMAKE_GENERATOR} as the generator.")

if(CMAKE_BUILD_TYPE MATCHES Deb)
    add_definitions(-DDEBUG)
endif()

if(CMAKE_BUILD_TYPE MATCHES Rel)
	set(USE_BAKED_SHADERS ON)
endif()

# I think C++11 is the minimum, either way my GCC is c++14 default
set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUIRED True)

add_subdirectory(assets)
add_subdirectory(src)
add_subdirectory(thirdparty)

configure_file("config.h.in" "include/config.h")
if(CMAKE_BUILD_TYPE MATCHES Deb)
    add_executable(LPPShow)
else()
    add_executable(LPPShow WIN32)
endif(CMAKE_BUILD_TYPE MATCHES Deb)
target_sources(LPPShow PUBLIC "${CMAKE_SOURCE_DIR}/src/LPPShow.cpp") # "${CMAKE_SOURCE_DIR}/metadata/metadata.rc")
# In that order, to shadow Makefile fallback stub
target_include_directories(LPPShow PRIVATE "${PROJECT_BINARY_DIR}/include")
target_include_directories(LPPShow PRIVATE "include")

if (USE_BAKED_SHADERS)
    # target_sources(LPPShow PRIVATE "${PROJECT_SOURCE_DIR}/include/baked_shaders.h")
    add_dependencies(LPPShow bake)
endif(USE_BAKED_SHADERS)

set(LIBRARIES framework glad imgui) # "Core" libraries, same *names* across all builds.

# Linux defaults
set(GLFW_LIBRARY_NAME "glfw")
set(CDDLIB_LIBRARY_NAME "cdd")

# Now the real fun begins
if (WIN32)
    message("Condolences.")

    set(GLFW_LIBRARY_NAME "glfw3")
    # Because *apparently* it's libcdd on Windows, thanks guys, very cool.
    set(CDDLIB_LIBRARY_NAME "cdd")

    list(APPEND LIBRARIES opengl32 gdi32)
    target_link_directories(LPPShow PUBLIC libraries) # "Fallback", if anything goes wrong

    # I have nothing.
    # ///////////////////////////////////////
    # The plastic world has won.
    # The mockup has proven to be stronger.
    # ///////////////////////////////////////
    # At some point it's just copyright infrigement

    # Either way, we have to separate the link libraries because yes. Either here or better yet, during configure step.
    if (CMAKE_GENERATOR MATCHES "Visual Studio .+ 2022")
        target_link_directories(LPPShow PUBLIC libraries/glfw-vc2022)
        list(APPEND LIBRARIES legacy_stdio_definitions)
    elseif (CMAKE_GENERATOR MATCHES "Visual Studio .+ 2019")
        target_link_directories(LPPShow PUBLIC libraries/glfw-vc2019)
        list(APPEND LIBRARIES legacy_stdio_definitions)
    elseif (CMAKE_GENERATOR MATCHES "Visual Studio .+ 2017")
        target_link_directories(LPPShow PUBLIC libraries/glfw-vc2017)
        list(APPEND LIBRARIES legacy_stdio_definitions)
    elseif(CMAKE_GENERATOR MATCHES "Makefiles" AND NOT CMAKE_GENERATOR MATCHES "NMake")
        list(APPEND LIBRARIES -static) # Force static links only on MinGW/Unix/whatever else, because VC is magic, apparently
        target_link_directories(LPPShow PUBLIC libraries/glfw-mingw-w64)
    else()
        target_link_directories(LPPShow PUBLIC libraries/glfw-vc2022)
        list(APPEND LIBRARIES legacy_stdio_definitions)
    endif()
elseif(UNIX AND NOT APPLE)
    list(APPEND LIBRARIES rt m dl)
endif()

list(APPEND LIBRARIES ${GLFW_LIBRARY_NAME})

if(USE_CDDLIB)
    list(APPEND LIBRARIES quickhull ${CDDLIB_LIBRARY_NAME})
endif()

target_link_libraries(LPPShow PUBLIC ${LIBRARIES})

install(TARGETS LPPShow DESTINATION "${CMAKE_SOURCE_DIR}/dist")
