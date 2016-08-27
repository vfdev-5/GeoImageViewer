project( EditableFunction )
cmake_minimum_required( VERSION 3.1.0 FATAL_ERROR)

# Find OPENCV
find_package( OpenCV 2.4.0 REQUIRED )

## include & link to OpenCV :
include_directories(${OpenCV_INCLUDE_DIRS})
link_directories(${OpenCV_LIB_DIR})
link_libraries(${OpenCV_LIBS})

add_definitions("-DEF_EXPORT_DEF")

## build configuration
file(GLOB INC_FILES "EditableFunction.h")
file(GLOB SRC_FILES "EditableFunction.cpp")

## create dll :
add_library( ${PROJECT_NAME} SHARED ${SRC_FILES} ${INC_FILES})
set_target_properties(${PROJECT_NAME} PROPERTIES DEBUG_POSTFIX ".d")

## installation :
install(TARGETS ${PROJECT_NAME} RUNTIME DESTINATION . LIBRARY DESTINATION .)

## install OPENCV Libraries for win32
if(WIN32)
	SET(version "${OpenCV_VERSION_MAJOR}${OpenCV_VERSION_MINOR}${OpenCV_VERSION_PATCH}")
    get_filename_component(OpenCV_BIN_DIR "${OpenCV_LIB_PATH}/../bin" ABSOLUTE)
    INSTALL(FILES ${OpenCV_BIN_DIR}/opencv_core${version}.dll ${OpenCV_BIN_DIR}/opencv_imgproc${version}.dll DESTINATION .)
endif(WIN32)







