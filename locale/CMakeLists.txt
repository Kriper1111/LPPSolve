if(WIN32 AND FALSE)
    FetchContent_Declare(gettext_runtime
        URL https://github.com/mlocati/gettext-iconv-windows/releases/download/v0.21-v1.16/gettext0.21-iconv1.16-shared-64.zip
        URL_HASH SHA256=b500cb5b3f3fdeb388d55cebce2f2b0302edb1bf3f7a7b5d1bf1526c73b031bb
        SOURCE_DIR "${CMAKE_SOURCE_DIR}/thirdparty/gettext"
        PREFIX "${CMAKE_SOURCE_DIR}/external_projects"
    )
    message("Fecthing gettext runtime")
    FetchContent_MakeAvailable(gettext_runtime)
endif(WIN32 AND FALSE)

file(GLOB TRANSLATION_SOURCES ${CMAKE_SOURCE_DIR}/locale/translated/*.po)
file(MAKE_DIRECTORY ${CMAKE_SOURCE_DIR}/locale/compiled)
foreach(TRANSLATION_SOURCE IN LISTS TRANSLATION_SOURCES)
    get_filename_component(TRANSLATION_TARGET "${TRANSLATION_SOURCE}" NAME_WE)
    string(APPEND TRANSLATION_TARGET .mo)
    string(PREPEND TRANSLATION_TARGET ${CMAKE_SOURCE_DIR}/locale/compiled/)

    add_custom_command(
        OUTPUT "${TRANSLATION_TARGET}"
        COMMAND bash -c "msgfmt -o '${TRANSLATION_TARGET}' '${TRANSLATION_SOURCE}' --" # msgfmt seems to be bundled with git-bash
        DEPENDS "${TRANSLATION_SOURCE}"
    )
    list(APPEND TRANSLATION_TARGETS ${TRANSLATION_TARGET})
endforeach(TRANSLATION_SOURCE IN LISTS TRANSLATION_SOURCES)


add_custom_target(compiled_pots
    DEPENDS "${TRANSLATION_TARGETS}"
    SOURCES "${TRANSLATION_SOURCES}"
)
if (WIN32 AND FALSE)
    add_dependencies(compiled_pots gettext_runtime)
endif(WIN32 AND FALSE)

install(FILES ${TRANSLATION_TARGETS} DESTINATION "${CMAKE_SOURCE_DIR}/dist/locale/compiled")
