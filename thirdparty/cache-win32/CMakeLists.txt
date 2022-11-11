cmake_minimum_required(VERSION 3.10)

message("Retrieving 'windows-only' dependencies")
message("Mainly because library packaging on windows sucks or just beyond my comprehension")

# We need to cache, hopefully, just:
# ./external_project
set(IMGUI_VERSION 1.88)
set(GLFW_VERSION 3.3.8.bin.WIN64)
set(GLM_VERSION 0.9.9.8)
set(CDDLIB_VERSION 0.94m)

include(ExternalProject)

set(UNWANTED_FILES_PREFIX "${CMAKE_SOURCE_DIR}/tmp")

set_directory_properties(PROPERTIES EP_BASE "${CMAKE_SOURCE_DIR}/external_projects")

if(POLICY CMP0135)
    cmake_policy(SET CMP0135 NEW)
endif()

# We only need includes and libraries
# and ${CMAKE_SOURCE_DIR}/tmp will not get cached by actions, hopefully.
ExternalProject_Add(dep_cddlib
    URL https://github.com/cddlib/cddlib/releases/download/0.94m/cddlib-0.94m.tar.gz
    URL_HASH SHA256=70dffdb3369b8704dc75428a1b3c42ab9047b81ce039f12f427e2eb2b1b0dee2
    BUILD_IN_SOURCE on

    CONFIGURE_COMMAND "bash" -c "./configure --disable-dependency-tracking --prefix=${UNWANTED_FILES_PREFIX} --includedir=${CMAKE_SOURCE_DIR}/thirdparty --libdir=${CMAKE_SOURCE_DIR}/libraries"
    BUILD_COMMAND "bash" -c "make CFLAGS='-fno-stack-check' '-fno-stack-protector' '-mno-stack-arg-probe'"# FIXME: VERY VERY VERY VERY BAD
)
# HACK: While CMake port haven't consolidated yet, we have to use 'sed' to make autoconf be more friendly towards
#       compilers/shells installed in, say, "C:/Program Files/"
ExternalProject_Get_Property(dep_cddlib SOURCE_DIR)
ExternalProject_Add_Step(dep_cddlib prebuild
    COMMAND "bash" -c "sed -i -f '${CMAKE_SOURCE_DIR}/thirdparty/cache-win32/sed.txt' '${SOURCE_DIR}/lib-src/Makefile' '${SOURCE_DIR}/src/Makefile'"
    DEPENDEES "configure"
    DEPENDERS "build"
)
ExternalProject_Add_Step(dep_cddlib compat
    COMMAND "${CMAKE_COMMAND}" -E copy "${CMAKE_SOURCE_DIR}/libraries/libcdd.a" "${CMAKE_SOURCE_DIR}/libraries/cdd.lib"
    DEPENDEES "install"
)
# So the final layout looks like CONFIGURE -> PREBUILD -> BUILD -> INSTALL -> COMPAT

# Download and unpack
ExternalProject_Add(dep_glfw
    URL https://github.com/glfw/glfw/releases/download/3.3.8/glfw-3.3.8.bin.WIN64.zip
    URL_HASH SHA256=7851c068b63c3cebf11a3b52c9e7dbdb6159afe32666b0aad268e4a258a9bdd1
    CONFIGURE_COMMAND "" BUILD_COMMAND "" INSTALL_COMMAND ""
)
ExternalProject_Get_Property(dep_glfw SOURCE_DIR)
ExternalProject_Add_Step(dep_glfw shuffle
    COMMAND "${CMAKE_COMMAND}" -E make_directory "${CMAKE_SOURCE_DIR}/libraries"
    COMMAND "${CMAKE_COMMAND}" -E copy_if_different "${SOURCE_DIR}/lib-mingw-w64" "${CMAKE_SOURCE_DIR}/libraries/glfw-mingw-w64"
    COMMAND "${CMAKE_COMMAND}" -E copy_if_different "${SOURCE_DIR}/lib-vc2017" "${CMAKE_SOURCE_DIR}/libraries/glfw-vc2017"
    COMMAND "${CMAKE_COMMAND}" -E copy_if_different "${SOURCE_DIR}/lib-vc2019" "${CMAKE_SOURCE_DIR}/libraries/glfw-vc2019"
    COMMAND "${CMAKE_COMMAND}" -E copy_if_different "${SOURCE_DIR}/lib-vc2022" "${CMAKE_SOURCE_DIR}/libraries/glfw-vc2022"
    COMMAND "${CMAKE_COMMAND}" -E copy_if_different "${SOURCE_DIR}/include/GLFW" "${CMAKE_SOURCE_DIR}/thirdparty/GLFW"
    DEPENDEES "install"
)

# Download and unpack
ExternalProject_Add(dep_glm
    URL https://github.com/g-truc/glm/releases/download/0.9.9.8/glm-0.9.9.8.zip
    URL_HASH SHA256=37e2a3d62ea3322e43593c34bae29f57e3e251ea89f4067506c94043769ade4c
    CONFIGURE_COMMAND "" BUILD_COMMAND "" INSTALL_COMMAND ""
)
ExternalProject_Get_Property(dep_glm SOURCE_DIR)
ExternalProject_Add_Step(dep_glm shuffle
    COMMAND "${CMAKE_COMMAND}" -E copy_if_different "${SOURCE_DIR}/glm" "${CMAKE_SOURCE_DIR}/thirdparty/glm"
    DEPENDEES "install"
)
