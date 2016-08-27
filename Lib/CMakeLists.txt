project( GIVLib )

## include & link to OpenCV :
include_directories(${OpenCV_INCLUDE_DIRS})
link_directories(${OpenCV_LIB_DIR})
link_libraries(${OpenCV_LIBS})

## include & link to GDAL :
include_directories(${GDAL_INCLUDE_DIR})
link_libraries(${GDAL_LIBRARY})

## include & link to Qt :
if(WIN32)
    SET(INSTALL_QT_DLLS OFF)
endif(WIN32)
include(Qt)

## search files:
    # define subfolders
SET(modules 3rdparty Core Gui Tools Filters)

SET(SRC_FILES "")
SET(INC_FILES "")
SET(UI_FILES "")
foreach(module ${modules})
    file(GLOB_RECURSE src "${module}/*.cpp")
    file(GLOB_RECURSE src2 "${module}/*.c")
    file(GLOB_RECURSE inc "${module}/*.h")
    file(GLOB_RECURSE ui "${module}/*.ui")
    list(APPEND SRC_FILES ${src})
    list(APPEND SRC_FILES ${src2})
    list(APPEND INC_FILES ${inc})
    list(APPEND UI_FILES ${ui})
endforeach()


## Use .qrc files:
file(GLOB QRC_FILES "Resources/*.qrc")
QT5_ADD_RESOURCES (ResourceSources ${QRC_FILES})

# add ui_<source>.h
INCLUDE_DIRECTORIES(${CMAKE_BINARY_DIR}/Lib)

#### for debug purposes: ################
if (NOT WIN32)
    if (CMAKE_BUILD_TYPE MATCHES Debug)
        message("Define _DEBUG symbol")
        add_definitions("-D_DEBUG")
    endif (CMAKE_BUILD_TYPE MATCHES Debug)
endif(NOT WIN32)
##########################################

## create library :
add_definitions("-DLIB_EXPORT")
add_library( ${PROJECT_NAME} SHARED ${SRC_FILES} ${INC_FILES} ${UI_FILES} ${ResourceSources})
set_target_properties(${PROJECT_NAME} PROPERTIES DEBUG_POSTFIX ".d")

## installation :
install(TARGETS ${PROJECT_NAME} RUNTIME DESTINATION bin ARCHIVE DESTINATION lib LIBRARY DESTINATION lib)
foreach(module ${modules})
    INSTALL(DIRECTORY ${CMAKE_SOURCE_DIR}/Lib/${module} DESTINATION include FILES_MATCHING PATTERN "*.h" )
endforeach()

## install editable function resources
install(DIRECTORY ${CMAKE_SOURCE_DIR}/Lib/Resources/EditableFunction DESTINATION bin/Resources)


