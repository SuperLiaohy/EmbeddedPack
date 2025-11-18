cmake_minimum_required(VERSION 3.31)
project(EmbeddedPack)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

#set(CMAKE_CXX_FLAGS "-O2")

add_library(EmbeddedPack library.cpp)
target_include_directories(EmbeddedPack PUBLIC Bsp)

add_executable(test library.cpp)

