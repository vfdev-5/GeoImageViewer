#############################################################
## Qt CMakeImport file
#############################################################


if(NOT QT_COMPONENT_LIST)
    set(QT_COMPONENT_LIST Core)
endif(NOT QT_COMPONENT_LIST)


# For Qt5:
foreach(component ${QT_COMPONENT_LIST})
    find_package( Qt5${component} REQUIRED )
endforeach()

	
