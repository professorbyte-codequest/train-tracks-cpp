include(CTest)

# Find GTest
find_package(GTest REQUIRED)
include_directories(${GTEST_INCLUDE_DIRS})

# Specify the test executables

file(GLOB TEST_SOURCES "*.cpp")
foreach(TEST_SOURCE ${TEST_SOURCES})
    get_filename_component(TEST_NAME ${TEST_SOURCE} NAME_WE)
    add_executable(${TEST_NAME} ${TEST_SOURCE})
    target_link_libraries(${TEST_NAME} ${GTEST_LIBRARIES} pthread TrainTracks)
    add_test(${TEST_NAME}.test ${TEST_NAME})
endforeach()