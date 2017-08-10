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


target_include_directories(lanxc-unixy PUBLIC
                           $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
                           $<INSTALL_INTERFACE:include>
                           )
target_link_libraries(lanxc-unixy lanxc::core pthread)


install(DIRECTORY include/lanxc-unixy DESTINATION include/lanxc-unixy)

include(CMakePackageConfigHelpers)

# Write version file to build dir
write_basic_package_version_file(
    "${CMAKE_CURRENT_BINARY_DIR}/${PROJECT_NAME}-config-version.cmake"
    VERSION 1.0
    COMPATIBILITY SameMajorVersion
)

# Install version file
install(FILES
        "${CMAKE_CURRENT_BINARY_DIR}/${PROJECT_NAME}-config-version.cmake"
        DESTINATION lib/cmake/lanxc/
        )

# Export targets
install(TARGETS ${PROJECT_NAME}
        EXPORT ${PROJECT_NAME}-targets
        DESTINATION lib)

install(EXPORT ${PROJECT_NAME}-targets
        FILE ${PROJECT_NAME}-targets.cmake
        NAMESPACE lanxc::
        DESTINATION lib/cmake/lanxc
        )

# Package config file
configure_file("${PROJECT_NAME}-config.cmake" "${CMAKE_CURRENT_BINARY_DIR}/${PROJECT_NAME}-config.cmake")

# Install package config file
install(FILES ${PROJECT_NAME}-config.cmake
        DESTINATION lib/cmake/lanxc
        )

