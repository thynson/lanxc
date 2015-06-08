cmake_minimum_required(VERSION 3.1)

function(lanxc_unit_test )
    foreach (t ${ARGN})
        add_executable("${t}" "${t}.cpp")
        target_link_libraries(${t} lanxc-core)
        add_test(test-${t} ${t})
    endforeach()
endfunction()

lanxc_unit_test(rbtree-01 rbtree-02 rbtree-03)

