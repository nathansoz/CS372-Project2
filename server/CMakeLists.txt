cmake_minimum_required(VERSION 2.6)
project(server)

set(CMAKE_C_COMPILER "clang")


set(SOURCE_FILES main.c server.h server.c)
add_executable(fserve ${SOURCE_FILES})

target_link_libraries(fserve pthread)
