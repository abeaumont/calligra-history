

include_directories( ${CMAKE_SOURCE_DIR}/karbon ${CMAKE_SOURCE_DIR}/karbon/core ${KDE4_INCLUDES}  )


########### next target ###############

set(karbonkontourimport_PART_SRCS kontourimport.cpp )


kde4_add_plugin(karbonkontourimport ${karbonkontourimport_PART_SRCS})



target_link_libraries(karbonkontourimport  ${KDE4_KDECORE_LIBS} karboncommon )

install(TARGETS karbonkontourimport  DESTINATION ${PLUGIN_INSTALL_DIR})


########### install files ###############

install( FILES  karbon_kontour_import.desktop  DESTINATION ${SERVICES_INSTALL_DIR})





#original Makefile.am contents follow:

######## General stuff
#
#INCLUDES= -I$(srcdir) $(KOFFICE_INCLUDES) $(KOPAINTER_INCLUDES) -I$(top_srcdir)/karbon -I$(top_srcdir)/karbon/core $(all_includes)
#
######## Files
## Obviously you have to change "foo" (and maybe "import") to
## reflect the name of your filter. If you have more files
## than just fooimport.cc please add them to the _SOURCES line.
#
#kde_module_LTLIBRARIES = libkarbonkontourimport.la
#
#libkarbonkontourimport_la_SOURCES = kontourimport.cpp
#libkarbonkontourimport_la_LDFLAGS = -module $(KDE_PLUGIN)
#libkarbonkontourimport_la_LIBADD = $(KOFFICE_LIBS) ../../../karbon/libkarboncommon.la
#
#METASOURCES = AUTO
#
#service_DATA = karbon_kontour_import.desktop
#servicedir = $(kde_servicesdir)
#
## Note: If your filter imports or exports some special file
## which KDE doesn't have a mimetype for, yet, you'll have to
## create a mimetype and install it using those two lines.
## In case of doubt please ask koffice@kde.org or
## koffice-devel@kde.org. Thanks.
## mydata_DATA = x-foo.desktop
## mydatadir = $(kde_mimedir)/text
