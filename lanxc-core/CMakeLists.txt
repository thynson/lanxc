cmake_minimum_required(VERSION 3.2 FATAL_ERROR)
project(lanxc-core CXX)

add_library(lanxc-core
            include/lanxc/config.hpp
            include/lanxc/type_traits.hpp
            include/lanxc/function.hpp
            include/lanxc/functional.hpp
            include/lanxc/unique_tuple.hpp
            include/lanxc/link.hpp
            include/lanxc/link/list_config.hpp
            include/lanxc/link/list_define.hpp
            include/lanxc/link/list_node.hpp
            include/lanxc/link/list_iterator.hpp
            include/lanxc/link/list.hpp
            include/lanxc/link/rbtree_config.hpp
            include/lanxc/link/rbtree_define.hpp
            include/lanxc/link/rbtree_node.hpp
            include/lanxc/link/rbtree_iterator.hpp
            include/lanxc/link/rbtree.hpp
            include/lanxc/core/clock_context.hpp
            include/lanxc/core/io_context.hpp
            include/lanxc/core/task_context.hpp
            include/lanxc/core/network_context.hpp
            include/lanxc/core/future.hpp
            include/lanxc/core/buffer.hpp
            src/main.cpp
            src/buffer.cpp)
add_library(lanxc::core ALIAS lanxc-core)

if (BUILD_SHARED_LIBS)
  target_compile_definitions(lanxc-core PRIVATE BUILD_LANXC_CORE_SHARED_LIBRARY)
  target_compile_definitions(lanxc-core PUBLIC LANXC_CORE_SHARED_LIBRARY)
  set_target_properties(lanxc-core PROPERTIES CXX_VISIBILITY_PRESET hidden)
  set_target_properties(lanxc-core PROPERTIES C_VISIBILITY_PRESET hidden)
endif()

target_compile_features(lanxc-core PUBLIC
    cxx_alias_templates
    cxx_auto_type
    cxx_constexpr
    cxx_defaulted_functions
    cxx_deleted_functions
    cxx_delegating_constructors
    cxx_lambdas
    cxx_noexcept
    cxx_nullptr
    cxx_rvalue_references
    cxx_right_angle_brackets
    cxx_static_assert
    cxx_variadic_templates
    )

target_include_directories(lanxc-core PUBLIC
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
    $<INSTALL_INTERFACE:include>
    )

install(DIRECTORY include/lanxc DESTINATION include/lanxc)

include(CMakePackageConfigHelpers)

# Write version file to build dir
write_basic_package_version_file(
    "${CMAKE_CURRENT_BINARY_DIR}/lanxc-core-config-version.cmake"
    VERSION 1.0
    COMPATIBILITY SameMajorVersion
    )

# Install version file
install(FILES
    "${CMAKE_CURRENT_BINARY_DIR}/lanxc-core-config-version.cmake"
    DESTINATION lib/cmake/lanxc-core
    )

# Export targets
install(TARGETS lanxc-core
    EXPORT lanxc-core-targets
    DESTINATION lib)

install(EXPORT lanxc-core-targets
    FILE lanxc-core-targets.cmake
    NAMESPACE lanxc::
    DESTINATION lib/cmake/lanxc-core
    )
install(DIRECTORY include/lanxc DESTINATION include)

# Package config file
configure_file("lanxc-core-config.cmake" "${CMAKE_CURRENT_BINARY_DIR}/lanxc-core/lanxc-core-config.cmake")

# Install package config file
install(FILES lanxc-core-config.cmake
    DESTINATION lib/cmake/lanxc-core
    )

