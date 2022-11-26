if (USE_BAKED_SHADERS)
    find_package(Python3 REQUIRED)

    file(GLOB UNBAKED_SHADERS ${CMAKE_SOURCE_DIR}/assets/*.vert ${CMAKE_SOURCE_DIR}/assets/*.frag)
    # XXX: still rebuilds every time
    add_custom_command(
        OUTPUT "{CMAKE_SOURCE_DIR}/include/baked_shaders.h"
        DEPENDS "${UNBAKED_SHADERS}"
        COMMAND "${Python3_EXECUTABLE}" "${CMAKE_SOURCE_DIR}/preconfigure/bake_shaders.py" ${UNBAKED_SHADERS} "${CMAKE_SOURCE_DIR}/include/baked_shaders.h"
        COMMENT "[Ba]king [S]haders"
        VERBATIM
    )
    add_custom_target(bake
        DEPENDS "{CMAKE_SOURCE_DIR}/include/baked_shaders.h"
        SOURCES "${UNBAKED_SHADERS}"
    )
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
endif(USE_BAKED_SHADERS)
