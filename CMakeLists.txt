## Project GeoImageViewer
project( GeoImageViewer )
cmake_minimum_required( VERSION 2.8.11 FATAL_ERROR)


# Add custom Cmake Modules
set(CMAKE_MODULE_PATH "${CMAKE_SOURCE_DIR}/CMakeModules")

# Find includes in corresponding build directories
set(CMAKE_INCLUDE_CURRENT_DIR ON)
# Instruct CMake to run moc automatically when needed.
set(CMAKE_AUTOMOC ON)
# Instruct CMake to run uic automatically when needed.
set(CMAKE_AUTOUIC ON)



# Find OpenCV, GDAL, Qt:
find_package( OpenCV REQUIRED )
find_package( GDAL REQUIRED )
find_package( OpenGL REQUIRED )

# This is works for Qt5 only 
set( QT_COMPONENT_LIST Widgets OpenGL Test Qml Quick QuickWidgets)
find_package( Qt REQUIRED )




# add library :
#add_subdirectory("3rdparty")
add_subdirectory("Lib")
add_subdirectory("Plugins")
add_subdirectory("App")
add_subdirectory("Sandbox")
add_subdirectory("Tests")

if(WIN32)
    ## install opencv library dlls :
    get_filename_component(OpenCV_BIN_DIR "${OpenCV_LIB_PATH}/../bin" ABSOLUTE)
#    message("OpenCV_BIN_DIR : ${OpenCV_BIN_DIR}")
    INSTALL(DIRECTORY ${OpenCV_BIN_DIR}/ DESTINATION bin CONFIGURATIONS Debug FILES_MATCHING PATTERN "*d.dll" )
    INSTALL(DIRECTORY ${OpenCV_BIN_DIR}/ DESTINATION bin CONFIGURATIONS Release FILES_MATCHING PATTERN "*.dll" PATTERN "*d.dll" EXCLUDE )

    ## install gdal library dlls :
    get_filename_component(GDAL_BIN_DIR "${GDAL_INCLUDE_DIR}/../bin" ABSOLUTE)
#    message("GDAL_BIN_DIR : ${GDAL_BIN_DIR}")
    INSTALL(DIRECTORY ${GDAL_BIN_DIR}/ DESTINATION bin FILES_MATCHING PATTERN "*.dll" )
    INSTALL(DIRECTORY ${GDAL_BIN_DIR}/gdaldata DESTINATION bin PATTERN "*.png" EXCLUDE PATTERN "*.svg" EXCLUDE PATTERN "data/*")
    INSTALL(DIRECTORY ${GDAL_BIN_DIR}/gdalplugins DESTINATION bin PATTERN "*.dll")
endif(WIN32)




