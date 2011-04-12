# User-configurable variables
isEmpty(WSDL_DIR):WSDL_DIR = .
isEmpty(WSDL_SOURCES_DIR):WSDL_SOURCES_DIR = $$WSDL_DIR
isEmpty(WSDL_HEADERS_DIR):WSDL_HEADERS_DIR = $$WSDL_DIR
isEmpty(KD_MOD_WSDL):KD_MOD_WSDL = wsdl_
isEmpty(KD_MOD_SWSDL):KD_MOD_SWSDL = swsdl_

win32* {
    isEmpty(KDSOAPDIR): KDWSDL2CPP = $$KDSOAP_PATH/bin/kdwsdl2cpp.exe
    !isEmpty(KDSOAPDIR): KDWSDL2CPP = $$KDSOAPDIR/bin/kdwsdl2cpp.exe
} else {
    isEmpty(KDSOAPDIR): KDWSDL2CPP = $$KDSOAP_PATH/bin/kdwsdl2cpp
    !isEmpty(KDSOAPDIR): KDWSDL2CPP = $$KDSOAPDIR/bin/kdwsdl2cpp
}

kdwsdl_h.commands = $$KDWSDL2CPP $$KDWSDL_OPTIONS ${QMAKE_FILE_IN} -o ${QMAKE_FILE_OUT}
#kdwsdl_h.depend_command = "$$KDWSDL2CPP" -d "${QMAKE_FILE_IN}"
kdwsdl_h.depends = $$KDWSDL2CPP
kdwsdl_h.output = $$WSDL_HEADERS_DIR/$${KD_MOD_WSDL}${QMAKE_FILE_BASE}$${first(QMAKE_EXT_H)}
kdwsdl_h.input = KDWSDL
kdwsdl_h.variable_out = KDWSDL_HEADERS
kdwsdl_h.CONFIG += no_link target_predeps
kdwsdl_h.name = KDWsdl2Cpp HEADER ${QMAKE_FILE_IN}
silent:kdwsdl_h.commands = @echo kdwsdl2cpp ${QMAKE_FILE_IN} && $$kdwsdl_h.commands
QMAKE_EXTRA_COMPILERS += kdwsdl_h

kdwsdl_impl.commands = $$KDWSDL2CPP $$KDWSDL_OPTIONS -impl $${KD_MOD_WSDL}${QMAKE_FILE_BASE}$${first(QMAKE_EXT_H)} ${QMAKE_FILE_IN} -o ${QMAKE_FILE_OUT}
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

# Server code (with generated type classes) (KDWSDL_SERVER)

#skdwsdl_h.commands = $$KDWSDL2CPP -server ${QMAKE_FILE_IN} -o ${QMAKE_FILE_OUT}
##skdwsdl_h.depend_command = "$$KDWSDL2CPP" -d "${QMAKE_FILE_IN}"
#skdwsdl_h.depends = $$KDWSDL2CPP
#skdwsdl_h.output = $$WSDL_HEADERS_DIR/$${KD_MOD_SWSDL}${QMAKE_FILE_BASE}$${first(QMAKE_EXT_H)}
#skdwsdl_h.input = KDWSDL_SERVER
#skdwsdl_h.variable_out = KDWSDL_HEADERS
#skdwsdl_h.CONFIG += no_link target_predeps
#skdwsdl_h.name = KDWsdl2Cpp -server HEADER ${QMAKE_FILE_IN}
#silent:skdwsdl_h.commands = @echo kdwsdl2cpp -server ${QMAKE_FILE_IN} && $$kdwsdl_h.commands
#QMAKE_EXTRA_COMPILERS += skdwsdl_h

#skdwsdl_impl.commands = $$KDWSDL2CPP -server -impl $${KD_MOD_SWSDL}${QMAKE_FILE_BASE}$${first(QMAKE_EXT_H)} ${QMAKE_FILE_IN} -o ${QMAKE_FILE_OUT}
##skdwsdl_impl.depend_command = "$$KDWSDL2CPP" -d "${QMAKE_FILE_IN}"
#skdwsdl_impl.output = $$WSDL_SOURCES_DIR/$${KD_MOD_SWSDL}${QMAKE_FILE_BASE}$${first(QMAKE_EXT_CPP)}
#skdwsdl_impl.depends = $$WSDL_HEADERS_DIR/$${KD_MOD_SWSDL}${QMAKE_FILE_BASE}$${first(QMAKE_EXT_H)} $$KDWSDL2CPP
#skdwsdl_impl.input = KDWSDL_SERVER
#skdwsdl_impl.variable_out = SOURCES
#skdwsdl_impl.dependency_type = TYPE_C
#skdwsdl_impl.CONFIG += target_predeps
#skdwsdl_impl.name = KDWsdl2Cpp SOURCE ${QMAKE_FILE_IN}
#silent:skdwsdl_impl.commands = @echo kdwsdl2cpp -server -impl ${QMAKE_FILE_IN} && $$kdwsdl_impl.commands
#QMAKE_EXTRA_COMPILERS += skdwsdl_impl

#skdwsdl_server.commands = $$KDWSDL2CPP -server $${KD_MOD_SWSDL}${QMAKE_FILE_BASE}$${first(QMAKE_EXT_H)} ${QMAKE_FILE_IN} -o ${QMAKE_FILE_OUT}
#skdwsdl_server.output = $$WSDL_SOURCES_DIR/$${KD_MOD_SWSDL}${QMAKE_FILE_BASE}$${first(QMAKE_EXT_CPP)}
#skdwsdl_server.depends = $$KDWSDL2CPP
#skdwsdl_server.input = KDWSDL_SERVER
#skdwsdl_server.variable_out = WHATEVER_SOURCES
#skdwsdl_server.dependency_type = TYPE_C
#skdwsdl_server.CONFIG += target_predeps
#skdwsdl_server.name = KDWsdl2Cpp SOURCE ${QMAKE_FILE_IN}
#silent:skdwsdl_server.commands = @echo kdwsdl2cpp -server ${QMAKE_FILE_IN} && $$kdwsdl_server.commands
#QMAKE_EXTRA_COMPILERS += skdwsdl_server

# Moc files
load(moc)
kdwsdl_moc.commands = $$moc_header.commands
kdwsdl_moc.output = $$moc_header.output
kdwsdl_moc.depends = $$WSDL_SOURCES_DIR/${QMAKE_FILE_BASE}$${first(QMAKE_EXT_CPP)}
kdwsdl_moc.input = KDWSDL_HEADERS
kdwsdl_moc.variable_out = GENERATED_SOURCES
kdwsdl_moc.name = $$moc_header.name
QMAKE_EXTRA_COMPILERS += kdwsdl_moc
