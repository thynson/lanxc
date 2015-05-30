cmake_minimum_required(VERSION 3.1)
#
#add_executable(test-01 test-01.cpp)
#add_executable(test-02 test-rbtree-01.cpp)
#target_link_libraries(test-01 lanxc-core)
#target_link_libraries(test-rbtree-01 lanxc-core)
#add_test(ctest test-01)

function(lanxc_unit_test )
    foreach (t ${ARGN})
        add_executable("${t}" "${t}.cpp")
        target_link_libraries(${t} lanxc-core)
        add_test(test-${t} ${t})
    endforeach()
endfunction()

lanxc_unit_test(test-rbtree-01)
lanxc_unit_test(test-rbtree-02)

