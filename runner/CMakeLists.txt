# Find all .cpp files in the current directory
file(GLOB SOURCES "*.cpp")

add_executable(Runner ${SOURCES})
target_link_libraries(Runner TrainTracks)

install(TARGETS Runner DESTINATION bin)