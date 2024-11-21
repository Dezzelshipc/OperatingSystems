cmake_minimum_required(VERSION 3.5.0)

PROJECT(task3)

set (CMAKE_CXX_STANDARD 20)

ADD_EXECUTABLE(temp serial.hpp processes.hpp main.cpp)
add_executable(sim serial.hpp simulator.cpp)
# IF (NOT WIN32)
#     # В POSIX нужно линковаться к rt для shm_open() и к pthread для sem_open()
#     TARGET_LINK_LIBRARIES(shmem rt pthread)
# ENDIF (NOT WIN32)

ADD_EXECUTABLE(test test.cpp)
IF (WIN32)
    TARGET_LINK_LIBRARIES(test ws2_32)
ENDIF(WIN32)


