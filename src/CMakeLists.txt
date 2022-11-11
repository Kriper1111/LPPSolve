add_library(framework "assets.cpp" "camera.cpp" "solver.cpp")
target_include_directories(framework PRIVATE "${PROJECT_BINARY_DIR}/include")
target_include_directories(framework PRIVATE "../include")
target_link_libraries(framework PUBLIC glad imgui)