project(spellcheck)
add_subdirectory( tests )

include_directories( ${KDE4_INCLUDE_DIR} ${QT_INCLUDES} ${KOTEXT_INCLUDES} ${KOMAIN_INCLUDES} ${FLAKE_INCLUDES} )

set(spellcheck_SRCS
    SpellCheckPlugin.cpp
    SpellCheck.cpp
    SpellCheckFactory.cpp
    BgSpellCheck.cpp
    SpellCheckMenu.cpp
)

kde4_add_plugin(spellcheck ${spellcheck_SRCS})

target_link_libraries(spellcheck kotext)

install(TARGETS spellcheck DESTINATION ${PLUGIN_INSTALL_DIR})

install( FILES spellcheck.desktop DESTINATION ${SERVICES_INSTALL_DIR} )
