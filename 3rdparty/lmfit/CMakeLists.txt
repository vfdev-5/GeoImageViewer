project( Lmfit )

## search files:
file(GLOB_RECURSE SRC_FILES "*.c")
file(GLOB_RECURSE INC_FILES "*.h")

## create library :
add_library( ${PROJECT_NAME} STATIC ${SRC_FILES} ${INC_FILES})
set_target_properties(${PROJECT_NAME} PROPERTIES DEBUG_POSTFIX ".d")

## installation :
install(TARGETS ${PROJECT_NAME} RUNTIME DESTINATION bin ARCHIVE DESTINATION lib LIBRARY DESTINATION lib)
install(DIRECTORY ${CMAKE_SOURCE_DIR}/3rdparty/lmfit DESTINATION include FILES_MATCHING PATTERN "*.h" )



