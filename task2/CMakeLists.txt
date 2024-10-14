cmake_minimum_required(VERSION 3.5.0)

project(task2)

set(APP_SRCS main.cpp)
set(LIB_SRCS background.cpp)
set(LIB_HDRS background.hpp)

add_library(background SHARED ${LIB_HDRS} ${LIB_SRCS})
add_executable(main main.cpp ${LIB_HDRS} ${LIB_SRCS})

target_link_libraries(main background)