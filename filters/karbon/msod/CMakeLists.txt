
include_directories( ${KWMF_INCLUDES} )



########### next target ###############

set(karbonmsodimport_PART_SRCS msodimport.cc msod.cc )


kde4_add_plugin(karbonmsodimport ${karbonmsodimport_PART_SRCS})



target_link_libraries(karbonmsodimport kwmf komain)

install(TARGETS karbonmsodimport  DESTINATION ${PLUGIN_INSTALL_DIR})


########### install files ###############

install( FILES  karbon_msod_import.desktop  DESTINATION ${SERVICES_INSTALL_DIR})





#original Makefile.am contents follow:

######## General stuff
#
#INCLUDES= -I$(srcdir) $(KOFFICE_INCLUDES) $(all_includes)
#libkarbonmsodimport_la_LDFLAGS	= -module -avoid-version -no-undefined $(all_libraries)
#libkarbonmsodimport_la_LIBADD = $(KOFFICE_LIBS) $(LIBZ)
#
######## Files
#
#kde_module_LTLIBRARIES = libkarbonmsodimport.la
#
#libkarbonmsodimport_la_SOURCES = msodimport.cc msod.cc
#
#noinst_HEADERS = msodimport.h msod.h
#
#METASOURCES = AUTO
#
#service_DATA = karbon_msod_import.desktop
#servicedir = $(kde_servicesdir)
