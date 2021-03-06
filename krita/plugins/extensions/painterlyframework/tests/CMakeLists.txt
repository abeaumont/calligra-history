set( EXECUTABLE_OUTPUT_PATH ${CMAKE_CURRENT_BINARY_DIR} )
include_directories( ${KOMAIN_INCLUDES} "../" )

########### KisIlluminantProfile Test ###############

set(kis_illuminant_profile_test_SRCS
    kis_illuminant_profile_test.cpp
    ../kis_illuminant_profile.cpp)

kde4_add_unit_test(KisIlluminantProfileTest
                   TESTNAME krita-plugins-KisIlluminantProfileTest
                   ${kis_illuminant_profile_test_SRCS})

target_link_libraries(KisIlluminantProfileTest
    kritaimage ${KDE4_KDEUI_LIBS} ${QT_QTTEST_LIBRARY})

########### KisKSColorSpace Test ###############

set(kis_ks_colorspace_test_SRCS
    kis_ks_colorspace_test.cpp
    ../kis_illuminant_profile.cpp)

kde4_add_unit_test(KisKSColorSpaceTest
                   TESTNAME krita-plugins-KisKSColorSpaceTest
                   ${kis_ks_colorspace_test_SRCS})

target_link_libraries(KisKSColorSpaceTest
    kritaimage ${KDE4_KDEUI_LIBS} ${QT_QTTEST_LIBRARY})

########### Mixing Test ###############

set(mixing_test_SRCS
    mixing_test.cpp
    ../kis_illuminant_profile.cpp)

kde4_add_unit_test(MixingTest
                   TESTNAME krita-plugins-MixingTest
                   ${mixing_test_SRCS})

target_link_libraries(MixingTest
    kritaimage ${KDE4_KDEUI_LIBS} ${QT_QTTEST_LIBRARY})
