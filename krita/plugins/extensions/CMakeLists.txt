add_subdirectory( bigbrother )
add_subdirectory( backgrounds )
#add_subdirectory( bracketing2hdr )
add_subdirectory( colorrange )
add_subdirectory( colorspaceconversion )
add_subdirectory( compose )
add_subdirectory( dropshadow )
add_subdirectory( histogram )
add_subdirectory( imagesize )
add_subdirectory( metadataeditor )
add_subdirectory( modify_selection )
add_subdirectory( rotateimage )
if(SHOULD_BUILD_SCRIPTING)
   add_subdirectory( scripting ) # TODO: port it to the change of KisFilter, KisPaintOpPreset
endif(SHOULD_BUILD_SCRIPTING)
add_subdirectory( separate_channels )
add_subdirectory( shearimage )
add_subdirectory( together )
add_subdirectory( tonemapping )
add_subdirectory( dockers )
add_subdirectory( extensionsmanager )

if(OPENEXR_FOUND)
  add_subdirectory( painterlyframework )
endif(OPENEXR_FOUND)

find_package(OpenShiva)
find_package(QtShiva )

if(HAVE_OPENSHIVA AND HAVE_QTSHIVA)
 add_subdirectory(shiva)
endif(HAVE_OPENSHIVA AND HAVE_QTSHIVA)

macro_log_feature(HAVE_OPENSHIVA "OpenShiva" "OpenShiva interpreter for the Shiva Kernel Language (part of OpenGTL)" "http://www.opengtl.org" FALSE "" "Required for Shiva based Generators and Filters")
macro_log_feature(HAVE_QTSHIVA "QtShiva" "Qt bindings for the OpenShiva interpreter (part of libQtGTL)" "http://www.opengtl.org" FALSE "" "Required for Shiva based Generators and Filters")

