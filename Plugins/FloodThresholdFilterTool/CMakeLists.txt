project( GIVFloodThresholdFilterToolPlugin )

## include & link to OpenCV :
include_directories(${OpenCV_INCLUDE_DIRS})
link_directories(${OpenCV_LIB_DIR})
link_libraries(${OpenCV_LIBS})

## include & link to GDAL :
include_directories(${GDAL_INCLUDE_DIR})
link_libraries(${GDAL_LIBRARY})


## include & link to Qt :
SET(INSTALL_QT_DLLS OFF)
include(Qt)

## include & link to project library
include_directories(${CMAKE_SOURCE_DIR}/Lib)
include_directories(${CMAKE_BINARY_DIR}/Lib)
link_directories(${CMAKE_BINARY_DIR}/Lib)
link_libraries(optimized "GIVLib" debug "GIVLib.d")


## search files:
file(GLOB_RECURSE SRC_FILES "*.cpp")
file(GLOB_RECURSE INC_FILES "*.h")
file(GLOB_RECURSE UI_FILES "*.ui")

## create dll :
add_library( ${PROJECT_NAME} SHARED ${SRC_FILES} ${INC_FILES} ${UI_FILES})
set_target_properties(${PROJECT_NAME} PROPERTIES DEBUG_POSTFIX ".d")

## installation :
#install(TARGETS ${PROJECT_NAME} RUNTIME DESTINATION bin/Plugins/Tools ARCHIVE DESTINATION lib LIBRARY DESTINATION lib)
install(TARGETS ${PROJECT_NAME} RUNTIME DESTINATION bin/Plugins/Tools ARCHIVE DESTINATION lib LIBRARY DESTINATION bin/Plugins/Tools)
