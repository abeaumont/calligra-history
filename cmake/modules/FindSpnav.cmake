FIND_PATH(SPNAV_INCLUDE_DIR spnav.h )

FIND_LIBRARY(SPNAV_LIBRARY NAMES spnav ) 

include(FindPackageHandleStandardArgs)
FIND_PACKAGE_HANDLE_STANDARD_ARGS(Spnav DEFAULT_MSG SPNAV_INCLUDE_DIR SPNAV_LIBRARY )
