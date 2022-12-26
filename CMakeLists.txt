cmake_minimum_required(VERSION 3.11)

# The following entries are gracefully borrowed
cmake_policy(SET CMP0042 NEW)

file(STRINGS "version" REAL_VERSION LIMIT_COUNT 1)
string(REGEX MATCH "([0-9]+.[0-9]+.[0-9]+)" SEMVER ${REAL_VERSION}) # Strips out metadata, but oh well
project(LPPShow VERSION ${SEMVER})

include(ExternalProject)
include(FetchContent)

option(USE_CDDLIB "Use cddlib and enable compute step" ON)
option(USE_OBJ_LOADER "Use wavefront OBJ Loader (shouldn't be necessary)" OFF)
option(USE_BAKED_SHADERS "Bake in shaders for easier distribution" OFF)

if(NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE Release)
endif(NOT CMAKE_BUILD_TYPE)

message(VERBOSE "Listing useful enviroment details:")
message(VERBOSE "* Building: LPPShow version ${REAL_VERSION}")
message(VERBOSE "* Using mode: ${CMAKE_BUILD_TYPE}")
message(VERBOSE "* Using generator: ${CMAKE_GENERATOR}")
message(VERBOSE "* Is MSVC: ${MSVC}")
message(VERBOSE "* Is UNIX: ${UNIX}")
message(VERBOSE "* Is WIN32: ${WIN32}")
message(VERBOSE "* Is APPLE: ${APPLE}")

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
if (MSVC AND CMAKE_BUILD_TYPE MATCHES Release) # Only on full releases we go without console
    set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} /SUBSYSTEM:WINDOWS /ENTRY:mainCRTStartup")
endif()
add_executable(LPPShow "${CMAKE_SOURCE_DIR}/src/LPPShow.cpp") # "${CMAKE_SOURCE_DIR}/metadata/metadata.rc")
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
set(CDDLIB_LIBRARY_NAME "libcdd.a")
target_link_directories(LPPShow PUBLIC libraries) # "Fallback", if anything goes wrong

# Now the real fun begins
if (WIN32)
    message("Condolences.")

    set(GLFW_LIBRARY_NAME "glfw3")
    set(CDDLIB_LIBRARY_NAME "cdd") # And hope it'll use .lib properly

    list(APPEND LIBRARIES opengl32 gdi32)

    # I have nothing.
    # ///////////////////////////////////////
    # The plastic world has won.
    # The mockup has proven to be stronger.
    # ///////////////////////////////////////
    # At some point it's just copyright infrigement

    # Either way, we have to separate the link libraries because yes. Either here or better yet, during configure step.
    if(MSVC)
        if (CMAKE_GENERATOR MATCHES "2019$")
            target_link_directories(LPPShow PUBLIC libraries/glfw-vc2019)
            list(APPEND LIBRARIES legacy_stdio_definitions)
        elseif (CMAKE_GENERATOR MATCHES "2017$")
            target_link_directories(LPPShow PUBLIC libraries/glfw-vc2017)
            list(APPEND LIBRARIES legacy_stdio_definitions)
        else()
            target_link_directories(LPPShow PUBLIC libraries/glfw-vc2022)
            list(APPEND LIBRARIES legacy_stdio_definitions)
        endif()
    else() # (CMAKE_CXX_COMPILER_ID MATCHES GNU|CLANG), hopefully
        list(APPEND LIBRARIES -static) # Force static links only on MinGW/Unix/whatever else, because VC is magic, apparently
        target_link_directories(LPPShow PUBLIC libraries/glfw-mingw-w64)
    endif()
elseif(UNIX AND NOT APPLE)
    list(APPEND LIBRARIES rt m dl)
elseif(APPLE)
    target_link_directories(LPPShow PUBLIC /usr/local/lib)
    target_link_directories(LPPShow PUBLIC /opt/local/lib)
    target_link_directories(LPPShow PUBLIC /opt/homebrew/lib)

    target_link_libraries(LPPShow PUBLIC "-framework OpenGL")
    target_link_libraries(LPPShow PUBLIC "-framework Cocoa")
    target_link_libraries(LPPShow PUBLIC "-framework IOKit")
    target_link_libraries(LPPShow PUBLIC "-framework CoreVideo")
    
    target_include_directories(LPPShow PUBLIC /usr/local/include)
    target_include_directories(LPPShow PUBLIC /opt/local/include)
    target_include_directories(LPPShow PUBLIC /opt/homebrew/include)
endif()

list(APPEND LIBRARIES ${GLFW_LIBRARY_NAME})

if(USE_CDDLIB)
    list(APPEND LIBRARIES quickhull ${CDDLIB_LIBRARY_NAME})
endif()

message(VERBOSE "Will link to ${LIBRARIES}")
target_link_libraries(LPPShow PUBLIC ${LIBRARIES})

install(TARGETS LPPShow DESTINATION "${CMAKE_SOURCE_DIR}/dist")
