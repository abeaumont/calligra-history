

include_directories( ${CMAKE_SOURCE_DIR}/kivio/kiviopart ${CMAKE_SOURCE_DIR}/kivio/kiviopart/kiviosdk ${CMAKE_SOURCE_DIR}/kivio/kiviopart/config ${KDE4_INCLUDES}  )


########### next target ###############

set(kivioimageexport_PART_SRCS kivio_imageexport.cpp kivio_imageexportdialog.cpp )


kde4_add_ui3_files(kivioimageexport_PART_SRCS kivio_imageexportwidget.ui )

kde4_add_plugin(kivioimageexport ${kivioimageexport_PART_SRCS})



target_link_libraries(kivioimageexport  kiviocommon )

install(TARGETS kivioimageexport  DESTINATION ${PLUGIN_INSTALL_DIR})


########### install files ###############

install( FILES  kivio_image_export.desktop  DESTINATION ${SERVICES_INSTALL_DIR})
