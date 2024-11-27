cmake_minimum_required(VERSION 3.5.0)

PROJECT(task4_5)

set (CMAKE_CXX_STANDARD 20)

set (CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall -Werror")

if (CMAKE_CXX_COMPILER_ID STREQUAL "Clang")
    # set (CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fsanitize=undefined")
    # set (CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fsanitize=address")
    set (CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fsanitize=thread")
endif()

ADD_EXECUTABLE(temp serial.hpp server.hpp utility.hpp main.cpp)
add_executable(sim serial.hpp simulator.cpp)
ADD_EXECUTABLE(test server.hpp utility.hpp test.cpp)

IF (WIN32)
    TARGET_LINK_LIBRARIES(test ws2_32)
    TARGET_LINK_LIBRARIES(temp ws2_32)
ENDIF(WIN32)


