cmake_minimum_required(VERSION 3.2 FATAL_ERROR)
project(lanxc-core-unix CXX)
add_library(lanxc-core-posix
            include/lanxc/core-unix/core-posix.hpp
            src/core-posix.cpp)

target_include_directories(lanxc-core-posix PUBLIC
                           $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
                           $<INSTALL_INTERFACE:include>
                           )
target_link_libraries(lanxc-core-posix lanxc-core)
