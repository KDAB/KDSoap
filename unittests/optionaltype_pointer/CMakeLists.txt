
set(optionaltype_pointer_SRCS testpointerapi.cpp)
set(KSWSDL2CPP_OPTION -optional-element-type raw-pointer)
set(WSDL_FILES test.wsdl)
add_unittest(${optionaltype_pointer_SRCS})

