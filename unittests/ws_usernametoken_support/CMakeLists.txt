project(ws_usernametoken_support)

set(WSDL_FILES wsusernametoken.wsdl)
set(ws_usernametoken_support_SRCS wsusernametokentest.cpp)

add_unittest(${ws_usernametoken_support_SRCS})
