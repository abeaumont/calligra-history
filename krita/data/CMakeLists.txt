add_subdirectory( brushes )
add_subdirectory( paintoppresets )
add_subdirectory( patterns )
add_subdirectory( gradients )
add_subdirectory( profiles )
add_subdirectory( palettes )
add_subdirectory( templates )
add_subdirectory( shaders )
add_subdirectory( images )
add_subdirectory( backgrounds )
add_subdirectory( cursors )
########### install files ###############

install( FILES  
    krita_filter.desktop      
    krita_brush.desktop      
    krita_paintop.desktop      
    krita_plugin.desktop      
    krita_tool.desktop
    krita_generator.desktop
    krita_dock.desktop
    DESTINATION ${SERVICETYPES_INSTALL_DIR})

install( FILES
    kritarc
    krita.knsrc
    DESTINATION ${CONFIG_INSTALL_DIR} )
