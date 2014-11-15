cmake_minimum_required(VERSION 3.0)

add_library(lanxc-core INTERFACE)

target_include_directories(lanxc-core INTERFACE
	$<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
	)
target_compile_options(lanxc-core INTERFACE -std=c++11)

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

#export(EXPORT lanxc-core-targets
#	FILE "${CMAKE_CURRENT_BINARY_DIR}/lanxc-core/lanxc-core-targets.cmake"
#	NAMESPACE lanxc:: )
install(EXPORT lanxc-core-targets
	FILE lanxc-core-targets.cmake
	NAMESPACE lanxc::
	DESTINATION lib/cmake/lanxc-core
	)
install(DIRECTORY include/lanxc DESTINATION include)

# Package config file
configure_file("lanxc-core-config.cmake"
	"${CMAKE_CURRENT_BINARY_DIR}/lanxc-core/lanxc-core-config.cmake"
	COPY_ONLY)

# Install package config file
install(FILES lanxc-core-config.cmake
	DESTINATION lib/cmake/lanxc-core
	)


