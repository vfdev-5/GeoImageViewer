#############################################################
## Qt CMakeImport file
#############################################################

if(NOT QT_COMPONENT_LIST)
    set(QT_COMPONENT_LIST Core)
endif(NOT QT_COMPONENT_LIST)


# For Qt5: 
if (DEFINED ENV{QT_DIR})
	set(CMAKE_PREFIX_PATH "$ENV{QT_DIR}" ${CMAKE_PREFIX_PATH})
	#message("CMAKE_PREFIX_PATH : ${CMAKE_PREFIX_PATH}")
else()
	message (FATAL_ERROR "ERROR: Environment variable QT_DIR is not set. Example: '/home/user/Qt5.4.2/5.4/gcc_64'") 
endif()
#message("Find Qt ...")
foreach(component ${QT_COMPONENT_LIST})
    #message("Find_package Qt5${component}")
    find_package( Qt5${component} REQUIRED )
endforeach()