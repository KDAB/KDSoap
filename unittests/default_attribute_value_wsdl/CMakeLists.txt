set(default_attribute_value_wsdl_SRCS default_attribute_value_wsdl.cpp)
set(WSDL_FILES default_attribute_value.wsdl)
set(KSWSDL2CPP_OPTION -optional-element-type boost-optional)
add_unittest(${default_attribute_value_wsdl_SRCS})
