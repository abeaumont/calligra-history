include_directories( ${KRITA_INCLUDES} ${GIF_INCLUDE_DIR})

set(libkritaconverter_LIB_SRCS
    gif_converter.cpp
    kis_gif_writer_visitor.cpp
    )

set(kritagifimport_PART_SRCS
    gif_import.cpp
    ${libkritaconverter_LIB_SRCS}
    )

kde4_add_plugin(kritagifimport ${kritagifimport_PART_SRCS})

target_link_libraries(kritagifimport kritaui ${GIF_LIBRARIES})

install(TARGETS kritagifimport  DESTINATION ${PLUGIN_INSTALL_DIR})

########### next target ###############

set(kritagifexport_PART_SRCS
    gif_export.cpp
    ${libkritaconverter_LIB_SRCS}
    )

kde4_add_plugin(kritagifexport ${kritagifexport_PART_SRCS})


target_link_libraries(kritagifexport kritaui ${GIF_LIBRARIES})

#install(TARGETS kritagifexport  DESTINATION ${PLUGIN_INSTALL_DIR})


########### install files ###############

install( FILES  krita_gif_import.desktop DESTINATION ${SERVICES_INSTALL_DIR})
#install( FILES  krita_gif_export.desktop  DESTINATION ${SERVICES_INSTALL_DIR})
install( PROGRAMS  krita_gif.desktop  DESTINATION ${XDG_APPS_INSTALL_DIR})
