add_subdirectory( kritacore )
add_subdirectory( scripts )

include_directories(${KOKROSS_INCLUDES})

########### next target ###############

set(kritascripting_PART_SRCS scriptingpart.cpp kis_script_filter.cc kis_script_dock.cc kis_script_decoration.cc )
kde4_add_plugin(kritascripting ${kritascripting_PART_SRCS})
target_link_libraries(kritascripting kritaui kokross krossmodulekrita)
# install(TARGETS kritascripting DESTINATION ${PLUGIN_INSTALL_DIR})

########### install files ###############

# install(FILES scripting.rc DESTINATION ${DATA_INSTALL_DIR}/kritaplugins)
# install(FILES kritascripting.desktop DESTINATION ${SERVICES_INSTALL_DIR})
