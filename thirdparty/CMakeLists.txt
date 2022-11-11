add_library(glad STATIC "glad.c")
target_include_directories(glad PUBLIC "")

if (WIN32)
    add_subdirectory("cache-win32")
else() # Has proper package detection
    find_package(glfw3 REQUIRED)
    find_package(glm REQUIRED)
endif(WIN32)

set(IMGUI_SOURCES
    imgui/imgui.cpp imgui/backends/imgui_impl_glfw.cpp imgui/backends/imgui_impl_opengl3.cpp
    imgui/imgui_draw.cpp imgui/imgui_tables.cpp imgui/imgui_widgets.cpp
)
add_library(imgui STATIC ${IMGUI_SOURCES})
target_include_directories(imgui PUBLIC "")
target_include_directories(imgui PUBLIC "imgui")
if (WIN32)
    add_dependencies(imgui imgui_fetch)
endif(WIN32)

if(USE_CDDLIB)
    add_library(quickhull STATIC "quickhull/QuickHull.cpp")
    target_include_directories(quickhull PUBLIC "thirdparty/quickhull/")
    # somehow make this thing build cddlib which only uses automake and is a PITA to build on Windows
endif()