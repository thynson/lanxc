cmake_minimum_required(VERSION 3.2 FATAL_ERROR)
project(lanxc-applism CXX)

add_library(lanxc-applism
            include/lanxc-applism/config.hpp
            include/lanxc-applism/event_source.hpp
            include/lanxc-applism/event_service.hpp
            include/lanxc-applism/event_channel.hpp
            include/lanxc-applism/network_connection.hpp
            include/lanxc-applism/event_loop.hpp
            src/event_loop.cpp
            src/event_channel.cpp
            src/event_source.cpp
            src/network_connection.cpp)

add_library(lanxc::applism ALIAS lanxc-applism)

target_include_directories(lanxc-applism PUBLIC
                           $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
                           $<INSTALL_INTERFACE:include>
                           )

set_target_properties(lanxc-applism PROPERTIES EXPORT_NAME applism)

if (BUILD_SHARED_LIBS)
  target_compile_definitions(lanxc-applism PUBLIC LANXC_APPLISM_SHARED_LIBRARY)
endif()
target_compile_options(lanxc-applism PRIVATE -Wall )

target_link_libraries(lanxc-applism lanxc::core lanxc::unixy)

install(DIRECTORY include/lanxc-applism DESTINATION include/lanxc-applism)

include(CMakePackageConfigHelpers)

# Write version file to build dir
write_basic_package_version_file(
    "${CMAKE_CURRENT_BINARY_DIR}/lanxc-applism-config-version.cmake"
    VERSION 1.0
    COMPATIBILITY SameMajorVersion
)

# Install version file
install(FILES
        "${CMAKE_CURRENT_BINARY_DIR}/lanxc-applism-config-version.cmake"
        DESTINATION lib/cmake/lanxc/
        )

# Export targets
install(TARGETS lanxc-applism
        EXPORT lanxc-applism-targets
        DESTINATION lib)

install(EXPORT lanxc-applism-targets
        FILE lanxc-applism-targets.cmake
        NAMESPACE lanxc::
        DESTINATION lib/cmake/lanxc
        )

# Package config file
configure_file("lanxc-applism-config.cmake" "${CMAKE_CURRENT_BINARY_DIR}/lanxc-applism-config.cmake")

# Install package config file
install(FILES lanxc-applism-config.cmake
        DESTINATION lib/cmake/lanxc
        )

