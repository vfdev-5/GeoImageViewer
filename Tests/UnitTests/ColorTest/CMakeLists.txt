project( ColorTest )

enable_testing()

## include & link to OpenCV :
include_directories(${OpenCV_INCLUDE_DIRS})
link_directories(${OpenCV_LIB_DIR})
link_libraries(${OpenCV_LIBS})

## include & link to GDAL :
include_directories(${GDAL_INCLUDE_DIRS})
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

## add common test files
list(APPEND INC_FILES "${TESTS_INC_FILES}")
list(APPEND SRC_FILES "${TESTS_SRC_FILES}")

## create app :
add_executable( ${PROJECT_NAME} ${SRC_FILES} ${INC_FILES} ${UI_FILES})
set_target_properties(${PROJECT_NAME} PROPERTIES DEBUG_POSTFIX ".d")
add_test(NAME ${PROJECT_NAME} COMMAND ${PROJECT_NAME} ${CMAKE_SOURCE_DIR}/Tests/Data)

## install application
install(TARGETS ${PROJECT_NAME} RUNTIME DESTINATION bin)
