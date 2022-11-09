cmake_minimum_required(VERSION 3.10)

message("Retrieving 'windows-only' dependencies")
message("Mainly because library packaging on windows sucks or just beyond my comprehension")

# Oh boy
# We need to cache:
# thirdparty/touches
# thirdparty/GLFW
# thirdparty/glm
# thirdparty/ImGui
# libraries/glfw
set(IMGUI_VERSION 1.88)
set(GLFW_VERSION 3.3.8.bin.WIN64)
set(GLM_VERSION 0.9.9.8)

# XXX: Could we move that to the build step maybe??
# Via ExternalProject

file(MAKE_DIRECTORY "${CMAKE_SOURCE_DIR}/libraries/")
file(MAKE_DIRECTORY "${CMAKE_SOURCE_DIR}/thirdparty/touches/")
if(NOT EXISTS "${CMAKE_SOURCE_DIR}/thirdparty/touches/imgui-${IMGUI_VERSION}")
    message("ImGui will now be downloaded")
    file(DOWNLOAD https://github.com/ocornut/imgui/archive/refs/tags/v${IMGUI_VERSION}.zip
        "${CMAKE_SOURCE_DIR}/tmp/imgui-${IMGUI_VERSION}.zip"
        EXPECTED_HASH SHA256=81087a74599e5890a07b636887cee73a7dc1a9eb9e1f19a4a0d82a76090bf4c2
        SHOW_PROGRESS
    )

    message("ImGui will now be extracted")
    file(ARCHIVE_EXTRACT INPUT "${CMAKE_SOURCE_DIR}/tmp/imgui-${IMGUI_VERSION}.zip"
        DESTINATION "${CMAKE_SOURCE_DIR}/tmp/"
    )

    message("ImGui includes will now be emplaced")
    file(RENAME
        "${CMAKE_SOURCE_DIR}/tmp/imgui-${IMGUI_VERSION}"
        "${CMAKE_SOURCE_DIR}/thirdparty/imgui"
    )

    file(TOUCH "${CMAKE_SOURCE_DIR}/thirdparty/touches/imgui-${IMGUI_VERSION}")
endif(NOT EXISTS "${CMAKE_SOURCE_DIR}/thirdparty/touches/imgui-${IMGUI_VERSION}")

if(NOT EXISTS "${CMAKE_SOURCE_DIR}/thirdparty/touches/glfw-${GLFW_VERSION}")
    message("GLFW will now be downloaded")
    file(DOWNLOAD https://github.com/glfw/glfw/releases/download/3.3.8/glfw-${GLFW_VERSION}.zip
        "${CMAKE_SOURCE_DIR}/tmp/glfw-${GLFW_VERSION}.zip"
        EXPECTED_HASH SHA256=7851c068b63c3cebf11a3b52c9e7dbdb6159afe32666b0aad268e4a258a9bdd1
        SHOW_PROGRESS
    )

    message("GLFW will now be extracted")
    file(ARCHIVE_EXTRACT INPUT "${CMAKE_SOURCE_DIR}/tmp/glfw-${GLFW_VERSION}.zip"
        DESTINATION "${CMAKE_SOURCE_DIR}/tmp/"
    )

    message("GLFW includes will now be emplaced")
    file(RENAME
        "${CMAKE_SOURCE_DIR}/tmp/glfw-${GLFW_VERSION}/include/GLFW"
        "${CMAKE_SOURCE_DIR}/thirdparty/GLFW"
    )

    message("GLFW libraries will now be emplaced")
    file(RENAME
        "${CMAKE_SOURCE_DIR}/tmp/glfw-${GLFW_VERSION}/lib-mingw-w64/"
        "${CMAKE_SOURCE_DIR}/libraries/glfw"
    )

    file(GLOB GLFW_VISUAL_C_LISTS "${CMAKE_SOURCE_DIR}/tmp/glfw-${GLFW_VERSION}/lib-vc2022/*")
    file(COPY "${GLFW_VISUAL_C_LISTS}"
        DESTINATION "${CMAKE_SOURCE_DIR}/libraries/"
    )

    file(TOUCH "${CMAKE_SOURCE_DIR}/thirdparty/touches/glfw-${GLFW_VERSION}")
endif(NOT EXISTS "${CMAKE_SOURCE_DIR}/thirdparty/touches/glfw-${GLFW_VERSION}")

if(NOT EXISTS "${CMAKE_SOURCE_DIR}/thirdparty/touches/glm-${GLM_VERSION}")
    message("glm will now be downloaded")
    file(DOWNLOAD https://github.com/g-truc/glm/releases/download/0.9.9.8/glm-${GLM_VERSION}.zip
        "${CMAKE_SOURCE_DIR}/tmp/glm-${GLM_VERSION}.zip"
        EXPECTED_HASH SHA256=37e2a3d62ea3322e43593c34bae29f57e3e251ea89f4067506c94043769ade4c
        SHOW_PROGRESS
    )

    message("glm will now be extracted")
    file(ARCHIVE_EXTRACT INPUT "${CMAKE_SOURCE_DIR}/tmp/glm-${GLM_VERSION}.zip"
        DESTINATION "${CMAKE_SOURCE_DIR}/tmp/"
    )

    message("glm includes will now be emplaced")
    file(RENAME
        "${CMAKE_SOURCE_DIR}/tmp/glm/glm/"
        "${CMAKE_SOURCE_DIR}/thirdparty/glm"
    )

    file(TOUCH "${CMAKE_SOURCE_DIR}/thirdparty/touches/glm-${GLM_VERSION}")
endif(NOT EXISTS "${CMAKE_SOURCE_DIR}/thirdparty/touches/glm-${GLM_VERSION}")