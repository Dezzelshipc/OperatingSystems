cmake_minimum_required(VERSION 3.5.0)

PROJECT(task3)

set (CMAKE_CXX_STANDARD 23)

ADD_EXECUTABLE(temp serial.hpp server.hpp utility.hpp main.cpp)
add_executable(sim serial.hpp simulator.cpp)
ADD_EXECUTABLE(test server.hpp utility.hpp test.cpp)

IF (WIN32)
    TARGET_LINK_LIBRARIES(test ws2_32)
    TARGET_LINK_LIBRARIES(temp ws2_32)
ENDIF(WIN32)


