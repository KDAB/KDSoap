project(keep_unused_types)

#this is the option given to KDWSDL2CPP when generating cpp from wsdl
set(KSWSDL2CPP_OPTION "-keep-unused-types")

set(WSDL_FILES keep_unused_types.wsdl)
set(keep_unused_types_SRCS keep_unused_types.cpp)

add_unittest(${keep_unused_types_SRCS})
