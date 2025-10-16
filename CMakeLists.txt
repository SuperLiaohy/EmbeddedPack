cmake_minimum_required(VERSION 3.31)
project(EmbeddedPack)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)


add_library(EmbeddedPack library.cpp)

#add_subdirectory(GPIO)
#target_link_libraries(EmbeddedPack PUBLIC GPIO)
#
#add_subdirectory(I2C)
#target_link_libraries(EmbeddedPack PUBLIC I2C)

add_subdirectory(Bsp)
target_link_libraries(EmbeddedPack PUBLIC Bsp)

add_executable(test library.cpp)

