add_subdirectory( rotate )

install( FILES
         webslideshow-sidebar.png
         layout-elements.svg
         DESTINATION ${DATA_INSTALL_DIR}/kpresenter/pics)

kde4_install_icons( ${ICON_INSTALL_DIR} )

