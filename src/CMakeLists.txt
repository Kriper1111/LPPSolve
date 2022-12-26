add_library(framework "assets.cpp" "camera.cpp" "solver.cpp" "display.cpp")
target_include_directories(framework PRIVATE "${PROJECT_BINARY_DIR}/include")
target_include_directories(framework PRIVATE "../include")
target_include_directories(framework PRIVATE "/usr/local/include")
target_link_directories(framework PRIVATE "/usr/local/lib")

# If on windows, glm is required before we can build our own code
if (WIN32)
    add_dependencies(framework dep_glm)
endif(WIN32)

# Additionally, if we want to use cddlib, we have to fetch&build it first
if (USE_CDDLIB)
    if (NOT APPLE) # R E L Y I N G on macos having sane cddlib, which it probably won't but i do really care
        add_dependencies(framework dep_cddlib quickhull)
    else()
        add_dependencies(framework quickhull)
    endif(NOT APPLE)
endif(USE_CDDLIB)

target_link_libraries(framework PUBLIC glad imgui)