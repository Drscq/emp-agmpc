cmake_minimum_required(VERSION 4.0)
project (emp-agmpc)
set(NAME "emp-agmpc")

find_path(CMAKE_FOLDER NAMES cmake/emp-tool-config.cmake)
include(${CMAKE_FOLDER}/cmake/emp-base.cmake)

FIND_PACKAGE(emp-ot REQUIRED)
INCLUDE_DIRECTORIES(${EMP-OT_INCLUDE_DIRS})

install(FILES cmake/emp-agmpc-config.cmake DESTINATION cmake/)
install(DIRECTORY emp-agmpc DESTINATION include/)

ENABLE_TESTING()
ADD_SUBDIRECTORY(test)

message(STATUS "CMAKE_RUNTIME_OUTPUT_DIRECTORY: '${CMAKE_RUNTIME_OUTPUT_DIRECTORY}'")
message(STATUS "Test executables will be in: ${CMAKE_CURRENT_BINARY_DIR}/test/")