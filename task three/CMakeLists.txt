cmake_minimum_required(VERSION 3.5.0)

PROJECT(task3)

set (CMAKE_CXX_STANDARD 20)

ADD_EXECUTABLE(shmem shmem.hpp processes.hpp main.cpp)
IF (NOT WIN32)
    # В POSIX нужно линковаться к rt для shm_open() и к pthread для sem_open()
    TARGET_LINK_LIBRARIES(shmem rt pthread)
ENDIF (NOT WIN32)
