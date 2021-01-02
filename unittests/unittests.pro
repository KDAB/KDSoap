## This file is part of the KD Soap library.
##
## SPDX-FileCopyrightText: 2009-2021 Klarälvdalens Datakonsult AB, a KDAB Group company <info@kdab.com>
##
## SPDX-License-Identifier: LGPL-2.1-only OR LGPL-3.0-only OR GPL-2.0-only OR GPL-3.0-only OR LicenseRef-KDAB-KDSoap OR LicenseRef-KDAB-KDSoap-US
##
## Licensees holding valid commercial KD Soap licenses may use this file in
## accordance with the KD Soap Commercial License Agreement provided with
## the Software.
##
## Contact info@kdab.com if any conditions of this licensing are not clear to you.
##

TEMPLATE = subdirs
SUBDIRS = \
  basic \
  builtinhttp \
  wsdl_rpc \
  wsdl_rpc-server \
  sugar_wsdl \
  ihc_wsdl \
  salesforce_wsdl \
  groupwise_wsdl \
  logbook_wsdl \
  messagereader \
  servertest \
  msexchange_noservice_wsdl \
  msexchange_wsdl \
  multiple_input_param \
  wsdl_document \
  dwservice_wsdl \
  dwservice_12_wsdl \
  dwservice_combined_wsdl \
  tech3356_wsdl \
  empty_response_wsdl \
  element_ns_wsdl \
  specialchars_wsdl \
  optionaltype_regular \
  optionaltype_pointer \
  enum_escape \
  clearbooks \
  soap12 \
  literal_true_false \
  import_definition \
  unqualified_formdefault \
  uitapi \
  encapsecurity \
  prefix_wsdl \
  vidyo \
  pki.pca.dfn.de \
  kddatetime \
  empty_list_wsdl \
  empty_element_wsdl \
  fault_namespace \
  enzo \
  date_example \
  dv_terminalauth \
  test_calc \
  ws_addressing_support \
  ws_usernametoken_support \
  list_restriction \
  QSharedPointer_include

greaterThan(QT_MAJOR_VERSION, 4): SUBDIRS += soap_over_udp

# These need internet access
SUBDIRS += webcalls webcalls_wsdl

# TODO: If boost optional is installed
#SUBDIRS += optionaltype_boost_optional
#SUBDIRS += default_attribute_value_wsdl

#with msvc, cribis requires the /bigobj option
win32-msvc*: QMAKE_CXXFLAGS += /bigobj

test.target=test
unix:!macx {
    LIB_PATH=$${TOP_BUILD_DIR}/lib:\$\$LD_LIBRARY_PATH
    test.commands=for d in $${SUBDIRS}; do origdir="\$$PWD" && cd "\$$d" && LD_LIBRARY_PATH=$$LIB_PATH && $(MAKE) test && cd "\$$origdir" || exit 1; done
}
unix:macx {
    LIB_PATH=$${TOP_BUILD_DIR}/lib:\$\$DYLD_LIBRARY_PATH
    test.commands=for d in $${SUBDIRS}; do ( cd "\$$d" && export DYLD_LIBRARY_PATH=$$LIB_PATH && $(MAKE) test ) || exit 1; done
}
win32 {
    WIN_BINDIR=
    debug_and_release {
        WIN_BINDIR=release
    }

    RUNTEST=$${TOP_SOURCE_DIR}/unittests/runTest.bat
    RUNTEST=$$replace(RUNTEST, /, \\)
    LIB_PATH=$$TOP_BUILD_DIR/lib
    LIB_PATH=$$replace(LIB_PATH, /, \\)
    test.commands=for %d in ($${SUBDIRS}); do $$RUNTEST $$LIB_PATH "%d" $$WIN_BINDIR || exit 1; done
}
test.depends = first
QMAKE_EXTRA_TARGETS += test
