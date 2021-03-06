#add_definitions(-DQT3_SUPPORT -DQT3_SUPPORT_WARNINGS)

set(kritalibpaintop_LIB_SRCS
    kis_auto_brush_widget.cpp
    kis_bidirectional_mixing_option.cpp
    kis_bidirectional_mixing_option_widget.cpp
    kis_brush_based_paintop.cpp
    kis_brush_chooser.cpp
    kis_brush_option_widget.cpp
    kis_brush_size_option.cpp
    kis_brush_option.cpp
    kis_brush_selection_widget.cpp
    kis_color_option.cpp
    kis_color_source.cpp
    kis_curve_option.cpp
    kis_curve_option_widget.cpp
    kis_custom_brush_widget.cpp
    kis_dynamic_sensor.cc
    kis_filter_option.cpp
    kis_paint_action_type_option.cpp
    kis_airbrush_option.cpp
    kis_paintop_option.cpp
    kis_paintop_options_model.cpp
    kis_paintop_options_widget.cpp
    kis_pressure_darken_option.cpp
    kis_pressure_hsv_option.cpp
    kis_pressure_opacity_option.cpp
    kis_pressure_mirror_option.cpp
    kis_pressure_scatter_option.cpp
    kis_pressure_scatter_option_widget.cpp
    kis_pressure_sharpness_option.cpp
    kis_pressure_sharpness_option_widget.cpp
    kis_pressure_mirror_option_widget.cpp
    kis_pressure_rate_option.cpp
    kis_pressure_rate_option_widget.cpp
    kis_pressure_rotation_option.cpp
    kis_pressure_size_option.cpp
    kis_pressure_softness_option.cpp
    kis_pressure_mix_option.cpp
    kis_sensor_selector.cc
    kis_text_brush_chooser.cpp
    kis_brush_based_paintop_options_widget.cpp
    kis_brush_based_paintop_settings.cpp
    sensors/kis_dynamic_sensors.cc
    sensors/kis_dynamic_sensor_distance.cc
    sensors/kis_dynamic_sensor_time.cc
    sensors/kis_dynamic_sensor_fade.cpp
)

kde4_add_ui_files(kritalibpaintop_LIB_SRCS
    wdgautobrush.ui
    wdgBrushSizeOptions.ui
    wdgcurveoption.ui
    wdgcustombrush.ui
    wdgtextbrush.ui
    wdgincremental.ui
    wdgairbrush.ui
    wdgfilteroption.ui
    wdgcoloroptions.ui
    wdgbrushchooser.ui
    sensors/SensorDistanceConfiguration.ui
    sensors/SensorTimeConfiguration.ui
    sensors/SensorFadeConfiguration.ui
    SensorSelector.ui
)

kde4_add_library(kritalibpaintop SHARED ${kritalibpaintop_LIB_SRCS} )

target_link_libraries(kritalibpaintop kritalibbrush kritaui)
target_link_libraries(kritalibpaintop LINK_INTERFACE_LIBRARIES kritalibbrush kritaui)

set_target_properties(kritalibpaintop PROPERTIES
    VERSION ${GENERIC_KOFFICE_LIB_VERSION} SOVERSION ${GENERIC_KOFFICE_LIB_SOVERSION}
)

install(TARGETS kritalibpaintop  ${INSTALL_TARGETS_DEFAULT_ARGS})


add_subdirectory(tests)
