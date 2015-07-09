
## Add common files :
file(GLOB TESTS_SRC_FILES "Common.cpp")
file(GLOB TESTS_INC_FILES "Common.h")

# add subdirectories :
add_subdirectory("UnitTests/ColorTest")
add_subdirectory("UnitTests/LayerUtilsTest")
add_subdirectory("UnitTests/DataProviderTest")
add_subdirectory("UnitTests/FiltersTest")
add_subdirectory("UnitTests/ImageRendererTest")
add_subdirectory("UnitTests/ImageOpenerTest")
add_subdirectory("UnitTests/ImageWriterTest")

