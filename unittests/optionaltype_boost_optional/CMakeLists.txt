
set(optionaltype_boost_optional_SRCS testboostapi.cpp)
set(KSWSDL2CPP_OPTION -optional-element-type boost-optional)
set(WSDL_FILES test.wsdl)
add_unittest(${optionaltype_boost_optional_SRCS})

