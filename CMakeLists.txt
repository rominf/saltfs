cmake_minimum_required(VERSION 2.8.4)
project(saltfs)

#set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11")

#set(SOURCE_FILES main.cpp)
#set(SOURCE_FILES super.c)
#add_executable(saltfs ${SOURCE_FILES})

#set(CMAKE_RUNTIME_OUTPUT_DIRECTORY "/data/romas/bmstu/term_project/07_operating_systems/bin")

subdirs(src/kern)
