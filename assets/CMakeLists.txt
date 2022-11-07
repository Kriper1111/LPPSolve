find_package(Python3 REQUIRED)
# find_package(PythonInterp3 REQUIRED)

if (CMAKE_BUILD_TYPE MATCHES "Release" OR BAKE_SHADERS)
    message("Using PYTHON_EXECUTABLE = ${Python3_EXECUTABLE}")
    message("COMMAND ${Python3_EXECUTABLE} ${CMAKE_SOURCE_DIR}/preconfigure/bake_obj.py ${CMAKE_SOURCE_DIR}/assets/* ${CMAKE_SOURCE_DIR}/include/baked_shaders.h")
    add_custom_target(
        bake ALL
        COMMAND ${Python3_EXECUTABLE} ${CMAKE_SOURCE_DIR}/preconfigure/bake_obj.py ${CMAKE_SOURCE_DIR}/assets/* ${CMAKE_SOURCE_DIR}/include/baked_shaders.h
        BYPRODUCTS "{CMAKE_SOURCE_DIR}/include/baked_shaders.h"
        COMMENT "Baking shaders"
    )
    add_definitions(-DUSE_BAKED_SHADERS)
else()
    install(
        DIRECTORY "${CMAKE_SOURCE_DIR}/assets"
        DESTINATION "${CMAKE_SOURCE_DIR}/dist"
        PATTERN PATTERN "*.txt" EXCLUDE
    )
    # I'm not sure about that, but it would make sense to have it installed to the
    # build dir as well. Dist is more for github to make the package out of.
    # Since we'd have to exclude a bunch of stuff from the artifact
    install(
        DIRECTORY "${CMAKE_SOURCE_DIR}/assets"
        DESTINATION "${PROJECT_BINARY_DIR}"
        PATTERN PATTERN "*.txt" EXCLUDE
    )
endif()