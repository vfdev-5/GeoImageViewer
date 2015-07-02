
add_definitions("-DPLUGIN_EXPORT")

## Add common files :
#file(GLOB PLUGINS_SRC_FILES "*.cpp")
file(GLOB PLUGINS_INC_FILES "PluginExport.h")
#file(GLOB PLUGINS_UI_FILES "*.ui")


## Add subdirectories :
#add_subdirectory("LassoFilter")
#add_subdirectory("HistogramThresholdFilter")
add_subdirectory("FloodThresholdFilterTool")
add_subdirectory("DarkPixelFilterTool")


