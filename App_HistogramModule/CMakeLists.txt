project( App_HistogramModule )

## include & link to OpenCV :
include_directories(${OpenCV_INCLUDE_DIRS})
link_directories(${OpenCV_LIB_DIR})
link_libraries(${OpenCV_LIBS})

## include & link to GDAL :
include_directories(${GDAL_INCLUDE_DIRS})
link_libraries(${GDAL_LIBRARY})

## include & link to Qt :
SET(INSTALL_QT_DLLS ON)
include(Qt)

## include & link to project library
include_directories(${CMAKE_SOURCE_DIR}/Lib)
include_directories(${CMAKE_BINARY_DIR}/Lib)
link_directories(${CMAKE_BINARY_DIR}/Lib)
link_libraries(optimized "GIVLib" debug "GIVLib.d")

## get files
file(GLOB SRC_FILES "*.cpp")
file(GLOB INC_FILES "*.h")
file(GLOB UI_FILES "*.ui")

## create application
add_executable( ${PROJECT_NAME} ${SRC_FILES} ${INC_FILES} ${UI_FILES})
set_target_properties(${PROJECT_NAME} PROPERTIES DEBUG_POSTFIX ".d")

## install application
install(TARGETS ${PROJECT_NAME} RUNTIME DESTINATION bin)
