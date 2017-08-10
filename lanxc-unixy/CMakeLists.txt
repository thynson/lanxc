cmake_minimum_required(VERSION 3.2 FATAL_ERROR)
project(lanxc-unixy CXX)
add_library(lanxc-unixy
            include/lanxc-unixy/config.hpp
            include/lanxc-unixy/unixy.hpp
            src/unixy.cpp)
add_library(lanxc::unixy ALIAS lanxc-unixy)

if (BUILD_SHARED_LIBS)
  target_compile_definitions(lanxc-unixy PRIVATE BUILD_LANXC_UNIXY_SHARED_LIBRARY)
  target_compile_definitions(lanxc-unixy PUBLIC LANXC_UNIXY_SHARED_LIBRARY)
  set_target_properties(lanxc-unixy PROPERTIES CXX_VISIBILITY_PRESET hidden)
  set_target_properties(lanxc-unixy PROPERTIES C_VISIBILITY_PRESET hidden)
endif()
#
#configure_file(${CMAKE_CURRENT_SOURCE_DIR}/include/lanxc/core-unix/config.hpp.in
#               ${CMAKE_CURRENT_BINARY_DIR}/include/lanxc/core-unix/config.hpp)


target_include_directories(lanxc-unixy PUBLIC
                           $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
                           $<INSTALL_INTERFACE:include>
                           )
target_link_libraries(lanxc-unixy lanxc-core)

