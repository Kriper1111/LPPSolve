add_library(framework "assets.cpp" "camera.cpp" "solver.cpp")
target_include_directories(framework PRIVATE "${PROJECT_BINARY_DIR}/include")
target_include_directories(framework PRIVATE "../include")

# If on windows, glm is required before we can build our own code
if (WIN32)
    add_dependencies(framework dep_glm)
endif(WIN32)

# Additionally, if we want to use cddlib, we have to fetch&build it first
if (USE_CDDLIB)
    add_dependencies(framework dep_cddlib)
endif(USE_CDDLIB)

target_link_libraries(framework PUBLIC glad imgui)