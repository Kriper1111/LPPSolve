add_library(glad STATIC "glad.c")
target_include_directories(glad PUBLIC "")

if (WIN32)
    add_subdirectory("cache-win32")
else() # Has proper package detection
    find_package(glfw3 REQUIRED)
    find_package(glm REQUIRED)
endif(WIN32)

# Here be bears
FetchContent_Declare(imgui
    URL https://github.com/ocornut/imgui/archive/refs/tags/v1.88.zip
    URL_HASH SHA256=81087a74599e5890a07b636887cee73a7dc1a9eb9e1f19a4a0d82a76090bf4c2
    SOURCE_DIR "${CMAKE_SOURCE_DIR}/thirdparty/imgui"
    PREFIX "${CMAKE_SOURCE_DIR}/external_projects"
)
message("Fecthing ImGui")
FetchContent_MakeAvailable(imgui)

FetchContent_Declare(mfr
    URL https://github.com/AnotherFoxGuy/MofileReader/releases/download/1.2.0/moFileReader.hpp
    URL_HASH SHA256=c001fd7e7483f9bde9be186287ec72629e64bc9b288088c8b1fbd930237f38a7
    DOWNLOAD_NO_EXTRACT on
    SOURCE_DIR "${CMAKE_SOURCE_DIR}/thirdparty/mfr"
    PREFIX "${CMAKE_SOURCE_DIR}/external_projects"
)
message("Fecthing moFileReader")
FetchContent_MakeAvailable(mfr)

set(IMGUI_SOURCES
    imgui/imgui.cpp imgui/backends/imgui_impl_glfw.cpp imgui/backends/imgui_impl_opengl3.cpp
    imgui/imgui_draw.cpp imgui/imgui_tables.cpp imgui/imgui_widgets.cpp
)
add_library(imgui STATIC ${IMGUI_SOURCES})
target_include_directories(imgui PUBLIC "")
target_include_directories(imgui PUBLIC "imgui")

if (WIN32)
    add_dependencies(imgui dep_glfw) # GLFW is satisfied on Linux by apt
endif(WIN32)

if(USE_CDDLIB)
    add_library(quickhull STATIC "quickhull/QuickHull.cpp")
    target_include_directories(quickhull PUBLIC "thirdparty/quickhull/")
    # somehow make this thing build cddlib which only uses automake and is a PITA to build on Windows

    set(UNWANTED_FILES_PREFIX "/tmp")

    # I have finally got enough courage to push out my own cmake-enabled fork
    # Complete with (hopefully) proper path detections/fillings
    # So we can skip the shuffle step
    # Again, not a carbon copy, but close enough for my intents and purposes
    # We could also have probably just set -DBUILD_SHARED_LIBS=off and had
    #   the previous script build static libs just as well.. BuuUUut hey we use
    #   latest version at least.
    ExternalProject_Add(dep_cddlib
        GIT_REPOSITORY https://github.com/Kriper1111/cddlib-cmake
        GIT_TAG master
        GIT_SHALLOW on
        BUILD_IN_SOURCE on
        CMAKE_ARGS "-DBUILD_BINARIES=off" "-DCMAKE_INSTALL_PREFIX=${UNWANTED_FILES_PREFIX}" "-DCMAKE_INSTALL_INCLUDEDIR=${CMAKE_SOURCE_DIR}/thirdparty" "-DCMAKE_INSTALL_LIBDIR=${CMAKE_SOURCE_DIR}/libraries" "-DCMAKE_INSTALL_DOCDIR=/tmp" "-DCMAKE_INSTALL_BINDIR=/tmp"
    )
endif()