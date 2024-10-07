cmake_minimum_required(VERSION 3.5.0)

project(main)

set(APP_SRCS main.cpp)
set(LIB_SRCS func.cpp)
set(LIB_HDRS func.hpp)

add_library(lib SHARED ${LIB_HDRS} ${LIB_SRCS})
add_executable(main main.cpp ${LIB_HDRS} ${LIB_SRCS})

target_link_libraries(main lib)