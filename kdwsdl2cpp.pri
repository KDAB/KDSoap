isEmpty(WSDL_DIR):WSDL_DIR = .
isEmpty(WSDL_SOURCES_DIR):WSDL_SOURCES_DIR = $$WSDL_DIR
isEmpty(WSDL_HEADERS_DIR):WSDL_HEADERS_DIR = $$WSDL_DIR
isEmpty(KD_MOD_WSDL):KD_MOD_WSDL = wsdl_
KDWSDL2CPP = $$KDSOAP_PATH/bin/kdwsdl2cpp

kdwsdl_h.commands = $$KDWSDL2CPP ${QMAKE_FILE_IN} -o ${QMAKE_FILE_OUT}
#kdwsdl_h.depend_command = "$$KDWSDL2CPP" -d "${QMAKE_FILE_IN}"
kdwsdl_h.depends = $$KDWSDL2CPP
kdwsdl_h.output = $$WSDL_HEADERS_DIR/$${KD_MOD_WSDL}${QMAKE_FILE_BASE}$${first(QMAKE_EXT_H)}
kdwsdl_h.input = KDWSDL
kdwsdl_h.variable_out = GENERATED_FILES
kdwsdl_h.CONFIG += no_link target_predeps
kdwsdl_h.name = KDWsdl2Cpp HEADER ${QMAKE_FILE_IN}
silent:kdwsdl_h.commands = @echo kdwsdl2cpp ${QMAKE_FILE_IN} && $$kdwsdl_h.commands
QMAKE_EXTRA_COMPILERS += kdwsdl_h

kdwsdl_impl.commands = $$KDWSDL2CPP -impl $${KD_MOD_WSDL}${QMAKE_FILE_BASE}$${first(QMAKE_EXT_H)} ${QMAKE_FILE_IN} -o ${QMAKE_FILE_OUT}
#kdwsdl_impl.depend_command = "$$KDWSDL2CPP" -d "${QMAKE_FILE_IN}"
kdwsdl_impl.output = $$WSDL_SOURCES_DIR/$${KD_MOD_WSDL}${QMAKE_FILE_BASE}$${first(QMAKE_EXT_CPP)}
kdwsdl_impl.depends = $$WSDL_HEADERS_DIR/$${KD_MOD_WSDL}${QMAKE_FILE_BASE}$${first(QMAKE_EXT_H)} $$KDWSDL2CPP
kdwsdl_impl.input = KDWSDL
kdwsdl_impl.variable_out = SOURCES
kdwsdl_impl.dependency_type = TYPE_C
kdwsdl_impl.CONFIG += target_predeps
kdwsdl_impl.name = KDWsdl2Cpp SOURCE ${QMAKE_FILE_IN}
silent:kdwsdl_impl.commands = @echo kdwsdl2cpp -impl ${QMAKE_FILE_IN} && $$kdwsdl_impl.commands
QMAKE_EXTRA_COMPILERS += kdwsdl_impl

