cmake_minimum_required(VERSION 3.5)
project(armpit)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c11")

set(SOURCE_FILES
    armpit.c
    armpit.h)

add_executable(armpit ${SOURCE_FILES})