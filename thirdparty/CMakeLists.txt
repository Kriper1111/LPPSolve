find_package(glfw3 REQUIRED)
find_package(glm REQUIRED)

add_library(glad STATIC "glad.c")
target_include_directories(glad PUBLIC "")

# file(DOWNLOAD https://github.com/ocornut/imgui/archive/refs/tags/v1.88.zip
#     "imgui-v1.88.zip"
#     EXPECTED_HASH SHA256=81087a74599e5890a07b636887cee73a7dc1a9eb9e1f19a4a0d82a76090bf4c2
# )
# ExternalProject_Add(imgui
#     URL https://github.com/ocornut/imgui/archive/refs/tags/v1.88.zip
#     URL_HASH SHA256=81087a74599e5890a07b636887cee73a7dc1a9eb9e1f19a4a0d82a76090bf4c2
#     # DOWNLOAD_EXTRACT_TIMESTAMP on
#     SOURCE_DIR imgui
#     CONFIGURE_COMMAND ""
#     BUILD_COMMAND ""
#     INSTALL_COMMAND ""
# )

set(IMGUI_SOURCES
    imgui/imgui.cpp imgui/backends/imgui_impl_glfw.cpp imgui/backends/imgui_impl_opengl3.cpp
    imgui/imgui_draw.cpp imgui/imgui_tables.cpp imgui/imgui_widgets.cpp
)
add_library(imgui STATIC ${IMGUI_SOURCES})
target_include_directories(imgui PUBLIC "")
target_include_directories(imgui PUBLIC "imgui")

add_library(quickhull STATIC "quickhull/QuickHull.cpp")
target_include_directories(quickhull PUBLIC "thirdparty/quickhull/")

if(USE_CDDLIB)
    # somehow make this thing build cddlib which only uses automake and is a PITA to build on Windows
endif()