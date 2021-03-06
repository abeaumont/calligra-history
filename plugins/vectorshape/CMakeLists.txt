project( vectorshape)

# The EMF library is directly under this directory.
# When it becomes an entity on its own we have to create
# some cmake code here to find it.
SET( EMF_INCLUDE_DIR ${CMAKE_CURRENT_SOURCE_DIR/libemf} )

# Duplicate from filters/CMakeLists.txt
set(KWMF_INCLUDES ${CMAKE_SOURCE_DIR}/filters/libkowmf)

include_directories( ${KOMAIN_INCLUDES} ${FLAKE_INCLUDES} ${KWMF_INCLUDES} ${EMF_INCLUDE_DIR})

SET ( VectorShape_SRCS
    VectorShapePlugin.cpp
    VectorShape.cpp
    VectorShapeFactory.cpp
    #VectorCollection.cpp
    #VectorData.cpp
    # No tool yet.
    #VectorTool.cpp
    #VectorToolFactory.cpp

    libemf/EmfRecords.cpp
    libemf/EmfHeader.cpp
    libemf/BitmapHeader.cpp
    libemf/Bitmap.cpp
    libemf/EmfParser.cpp
    libemf/EmfOutput.cpp
    libemf/EmfOutputDebugStrategy.cpp
    libemf/EmfOutputPainterStrategy.cpp
)

#kde4_add_ui_files(VectorShape_SRCS
#)

kde4_add_plugin(vectorshape ${VectorShape_SRCS})

target_link_libraries(vectorshape flake kowmf)

install(TARGETS vectorshape DESTINATION ${PLUGIN_INSTALL_DIR})

########### install files ###############

install( FILES vectorshape.desktop DESTINATION ${SERVICES_INSTALL_DIR})
#kde4_install_icons( ${DATA_INSTALL_DIR}/koffice/icons )


########### subdirectories ###############

#add_subdirectory( libemf )
