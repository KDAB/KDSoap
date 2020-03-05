set(optionaltype_regular_SRCS testregularapi.cpp)
set(WSDL_FILES test.wsdl)
set(KSWSDL2CPP_OPTION -namespace KDAB)
add_unittest(${optionaltype_regular_SRCS})

