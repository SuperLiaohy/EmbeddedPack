cmake_minimum_required(VERSION 3.15)

project(Bsp)

file(GLOB SOURCES "./*.cpp")

add_library(Bsp "${SOURCES}")

target_include_directories(Bsp PUBLIC ".")
