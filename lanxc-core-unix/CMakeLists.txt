cmake_minimum_required(VERSION 3.2 FATAL_ERROR)
project(lanxc-core-unix CXX)
add_library(lanxc-core-unix
            include/lanxc/core-unix/core-unix.hpp
            src/core-unix.cpp)

target_include_directories(lanxc-core-unix PUBLIC
                           $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
                           $<INSTALL_INTERFACE:include>
                           )
target_link_libraries(lanxc-core-unix lanxc-core)
