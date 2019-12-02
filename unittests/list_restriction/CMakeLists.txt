project(list_restriction)

set(list_restriction_SRCS test_list_restriction.cpp)
set(EXTRA_LIBS kdwsdl2cpp_lib)
add_unittest(${list_restriction_SRCS})
