# Set the library name
set(LIBRARY_NAME TrainTracks)

# Find all .cpp files in the current directory
file(GLOB SOURCES "*.cpp")

# Add the static library
add_library(${LIBRARY_NAME} STATIC ${SOURCES})

# Specify the include directory for headers
target_include_directories(${LIBRARY_NAME} PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/include)

# Install the static library
install(TARGETS ${LIBRARY_NAME}
    ARCHIVE DESTINATION lib)

# Install the header files
install(DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/include/
    DESTINATION include)