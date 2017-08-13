cmake_minimum_required(VERSION 2.6)
project(server)

set(CMAKE_C_STANDARD 99)

set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -Wall -Werror -pedantic")

set(SOURCE_FILES main.c server.h server.c)
add_executable(fserve ${SOURCE_FILES})

target_link_libraries(fserve pthread)