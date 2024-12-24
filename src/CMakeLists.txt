
cmake_minimum_required(VERSION 3.8)

# Copy datafiles into the location of the executable
# WARNING: It doesn't react to changes in data files
FILE(COPY ../data/ DESTINATION "${CMAKE_CURRENT_BINARY_DIR}/data")


add_executable(JourneyPlanner main.cpp DataTypes.hpp Raptor.cpp Timetable.cpp Route.cpp Raptor.hpp
        Timetable.hpp InputReader.hpp InputReader.cpp )
