cmake_minimum_required(VERSION 3.16)
project(armpit LANGUAGES C VERSION 0.0.1 DESCRIPTION "ARM emulator")


# Note: by default ENABLE_DEVELOPER_MODE is True
# This means that all analysis (sanitizers, static analysis)
# is enabled and all warnings are treated as errors
# if you want to switch this behavior, change TRUE to FALSE
set(ENABLE_DEVELOPER_MODE
    TRUE
    CACHE BOOL "Enable 'developer mode'")


set(CMAKE_C_STANDARD 17)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF) # turns off compiler specific extensions

set(SOURCE_FILES
    armpit.c
    armpit.h)

add_executable(armpit ${SOURCE_FILES})