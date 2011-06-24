set path=%path%;C:\Data\Docs\EWOCC\Dev\KDSoapEval\kdsoap-20110527\kdsoap\bin
kdwsdl2cpp IntegrationTest.wsdl -o integrationTestClient.h
kdwsdl2cpp IntegrationTest.wsdl -o integrationTestClient.cpp -impl integrationTestClient.h
kdwsdl2cpp IntegrationTest.wsdl -o integrationTestServer.h -server
kdwsdl2cpp IntegrationTest.wsdl -o integrationTestServer.cpp -server -impl integrationTestServer.h
ECHO code files created
PAUSE