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
set_target_properties(lanxc-unixy PROPERTIES EXPORT_NAME unixy)
#
#configure_file(${CMAKE_CURRENT_SOURCE_DIR}/include/lanxc/core-unix/config.hpp.in
#               ${CMAKE_CURRENT_BINARY_DIR}/include/lanxc/core-unix/config.hpp)


target_include_directories(lanxc-unixy PUBLIC
                           $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
                           $<INSTALL_INTERFACE:include>
                           )
target_link_libraries(lanxc-unixy lanxc-core)


install(DIRECTORY include/lanxc-unixy DESTINATION include/lanxc-unixy)

include(CMakePackageConfigHelpers)

# Write version file to build dir
write_basic_package_version_file(
    "${CMAKE_CURRENT_BINARY_DIR}/lanxc-unixy-config-version.cmake"
    VERSION 1.0
    COMPATIBILITY SameMajorVersion
)

# Install version file
install(FILES
        "${CMAKE_CURRENT_BINARY_DIR}/lanxc-unixy-config-version.cmake"
        DESTINATION lib/cmake/lanxc/
        )

# Export targets
install(TARGETS lanxc-unixy
        EXPORT lanxc-unixy-targets
        DESTINATION lib)

install(EXPORT lanxc-unixy-targets
        FILE lanxc-unixy-targets.cmake
        NAMESPACE lanxc::
        DESTINATION lib/cmake/lanxc
        )

# Package config file
configure_file("lanxc-unixy-config.cmake" "${CMAKE_CURRENT_BINARY_DIR}/lanxc-unixy-config.cmake")

# Install package config file
install(FILES lanxc-unixy-config.cmake
        DESTINATION lib/cmake/lanxc
        )

