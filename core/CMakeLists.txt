cmake_minimum_required(VERSION 3.1)

add_library(lanxc-core INTERFACE)

target_include_directories(lanxc-core INTERFACE
	$<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
	)
target_compile_features(lanxc-core INTERFACE
    cxx_alias_templates
    cxx_auto_type
    cxx_constexpr
    cxx_defaulted_functions
    cxx_deleted_functions
    cxx_delegated_constructors
    cxx_lambdas
    cxx_noexcept
    cxx_nullptr
    cxx_rvalue_references
    cxx_right_angle_brackets
    cxx_static_assert
    cxx_variadic_templates
    )

install(FILES
	${CMAKE_CURRENT_SOURCE_DIR}/include/lanxc/functional.hpp
	${CMAKE_CURRENT_SOURCE_DIR}/include/lanxc/intrus.hpp
	DESTINATION include/lanxc
	)

install(FILES
	${CMAKE_CURRENT_SOURCE_DIR}/include/lanxc/intrus/forward_list_config.hpp
	${CMAKE_CURRENT_SOURCE_DIR}/include/lanxc/intrus/forward_list_define.hpp
	${CMAKE_CURRENT_SOURCE_DIR}/include/lanxc/intrus/forward_list_node.hpp
	${CMAKE_CURRENT_SOURCE_DIR}/include/lanxc/intrus/forward_list_iterator.hpp
	${CMAKE_CURRENT_SOURCE_DIR}/include/lanxc/intrus/forward_list.hpp
	${CMAKE_CURRENT_SOURCE_DIR}/include/lanxc/intrus/list_config.hpp
	${CMAKE_CURRENT_SOURCE_DIR}/include/lanxc/intrus/list_define.hpp
	${CMAKE_CURRENT_SOURCE_DIR}/include/lanxc/intrus/list_node.hpp
	${CMAKE_CURRENT_SOURCE_DIR}/include/lanxc/intrus/list_iterator.hpp
	${CMAKE_CURRENT_SOURCE_DIR}/include/lanxc/intrus/list.hpp
	${CMAKE_CURRENT_SOURCE_DIR}/include/lanxc/intrus/rbtree_config.hpp
	${CMAKE_CURRENT_SOURCE_DIR}/include/lanxc/intrus/rbtree_define.hpp
	${CMAKE_CURRENT_SOURCE_DIR}/include/lanxc/intrus/rbtree_node.hpp
	${CMAKE_CURRENT_SOURCE_DIR}/include/lanxc/intrus/rbtree_iterator.hpp
	${CMAKE_CURRENT_SOURCE_DIR}/include/lanxc/intrus/rbtree.hpp
	DESTINATION include/lanxc/intrus
	)

include(CMakePackageConfigHelpers)

# Write version file to build dir
write_basic_package_version_file(
	"${CMAKE_CURRENT_BINARY_DIR}/lanxc-core-config-version.cmake"
	VERSION 1.0
	COMPATIBILITY AnyNewerVersion
	)

# Install version file
install(FILES
	"${CMAKE_CURRENT_BINARY_DIR}/lanxc-core-config-version.cmake"
	DESTINATION lib/cmake/lanxc-core
	)

# Export targets
install(TARGETS lanxc-core
    EXPORT lanxc-core-targets
    ARCHIVE DESTINATION lib)

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


