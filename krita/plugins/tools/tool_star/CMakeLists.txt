

set(kritatoolstar_PART_SRCS
    tool_star.cc
    kis_tool_star.cc
    )


kde4_add_plugin(kritatoolstar ${kritatoolstar_PART_SRCS})

target_link_libraries(kritatoolstar kritaui)

install(TARGETS kritatoolstar  DESTINATION ${PLUGIN_INSTALL_DIR})


########### install files ###############

install( FILES  tool_star.png  	tool_star_cursor.png  DESTINATION ${DATA_INSTALL_DIR}/krita/pics)
install( FILES  kritatoolstar.desktop  DESTINATION ${SERVICES_INSTALL_DIR})


