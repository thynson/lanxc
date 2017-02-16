cmake_minimum_required(VERSION 3.1)

function(lanxc_unit_test )
    foreach (t ${ARGN})
        add_executable("${t}" EXCLUDE_FROM_ALL "${t}.cpp")
        target_link_libraries(${t} lanxc::lanxc-core)
        add_test(compile-${t} "${CMAKE_COMMAND}" --build ${CMAKE_BINARY_DIR} --target ${t})
        add_test(test-${t} ${t})
        set_tests_properties(test-${t} PROPERTIES DEPENDS compile-${t})
    endforeach()
endfunction()

lanxc_unit_test(list-01 rbtree-01 rbtree-02 rbtree-03 rbtree-04 function-01
        scheduler-01)

