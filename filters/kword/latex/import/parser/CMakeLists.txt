




########### next target ###############

set(latexparser_PART_SRCS 
   command.cc 
   comment.cc 
   element.cc 
   env.cc 
   group.cc 
   latexparser.cc 
   param.cc 
   text.cc 
   texparser.cc 
   texscaner.cc )


kde4_add_plugin(latexparser ${latexparser_PART_SRCS})



target_link_libraries(latexparser  ${KDE4_KDECORE_LIBS} )

install(TARGETS latexparser  DESTINATION ${PLUGIN_INSTALL_DIR})


########### next target ###############

set(texlauncher_SRCS texlauncher.cc )

if(KDE4_BUILD_TESTS)

kde4_add_executable(texlauncher ${texlauncher_SRCS})

target_link_libraries(texlauncher  ${KDE4_KDECORE_LIBS} latexexport )

endif(KDE4_BUILD_TESTS)

########### install files ###############






#original Makefile.am contents follow:

######## General stuff
#
#INCLUDES= -I$(srcdir) $(all_includes)
#liblatexparser_la_LDFLAGS = $(all_libraries) -module -avoid-version -no-undefined
#liblatexparser_la_LIBADD  = -lm $(KOFFICE_LIBS)
#
######## Files
#
#kde_module_LTLIBRARIES = liblatexparser.la
#
#liblatexparser_la_SOURCES = command.cc comment.cc element.cc env.cc group.cc \
#														latexparser.cc param.cc text.cc texparser.cc \
#														texscaner.cc
#
#noinst_HEADERS = command.h comment.h element.h env.h group.h \
#								 latexparser.h param.h text.h
#
#texparser.cc: $(srcdir)/texparser.l texscaner.cc
#		flex -B -8 -otexparser.cc $(srcdir)/texparser.l
#
#texscaner.cc: $(srcdir)/texscaner.y
#		bison --debug --verbose -d -o texscaner.cc $(srcdir)/texscaner.y && mv -f texscaner.cc.h texscaner.hh
## -p texparser
######### Debug
#check_PROGRAMS = texlauncher
#texlauncher_SOURCES = texlauncher.cc
#texlauncher_LDADD = liblatexexport.la
#texlauncher_LDFLAGS = $(KDE_RPATH) $(all_libraries)
#
#
#METASOURCES = AUTO
#
