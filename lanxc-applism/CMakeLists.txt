cmake_minimum_required(VERSION 3.2 FATAL_ERROR)
project(lanxc-applism CXX)

add_library(lanxc-applism
            include/lanxc-applism/config.hpp
            include/lanxc-applism/event_loop.hpp
            include/lanxc-applism/event_service.hpp
            include/lanxc-applism/event_channel.hpp
            src/event_loop.cpp
            src/event_channel.cpp
            src/connection.hpp
            src/connection.cpp include/lanxc-applism/event_source.hpp src/event_source.cpp)
target_include_directories(lanxc-applism PUBLIC
                           $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
                           $<INSTALL_INTERFACE:include>
                           )

if (BUILD_SHARED_LIBS)
  target_compile_definitions(lanxc-applism PUBLIC LANXC_APPLISM_SHARED_LIBRARY)
endif()
target_compile_options(lanxc-applism PRIVATE -Wall )

target_link_libraries(lanxc-applism lanxc::core lanxc::unixy)
