cmake_minimum_required(VERSION 3.0)

add_library(lanxc-core INTERFACE)
target_include_directories(lanxc-core INTERFACE
	$<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
	)


install(TARGETS lanxc-core EXPORT lanxc-core)
install(EXPORT lanxc-core NAMESPACE lanxc::
	DESTINATION lib/cmake/lanxc/core )

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

