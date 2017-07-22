cmake_minimum_required(VERSION 3.2 FATAL_ERROR)
project(lanxc-core-macos CXX)

add_library(lanxc-core-macos
            include/lanxc/core-macos/event_loop.hpp
            include/lanxc/core-macos/event_service.hpp
            include/lanxc/core-macos/event_channel.hpp
            src/event_loop.cpp
            src/event_channel.cpp
            src/connection.hpp
            src/connection.cpp)
target_include_directories(lanxc-core-macos PUBLIC
                           $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
                           $<INSTALL_INTERFACE:include>
                           )
target_compile_options(lanxc-core-macos PRIVATE -Wall )

target_link_libraries(lanxc-core-macos lanxc-core lanxc-core-posix)
