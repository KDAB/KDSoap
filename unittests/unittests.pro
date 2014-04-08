TEMPLATE = subdirs
SUBDIRS = \
  basic \
  builtinhttp \
  wsdl_rpc \
  sugar_wsdl \
  ihc_wsdl \
  salesforce_wsdl \
  groupwise_wsdl \
  logbook_wsdl \
  messagereader \
  servertest \
  msexchange_noservice_wsdl \
  msexchange_wsdl \
  wsdl_document \
  dwservice_wsdl \
  dwservice_12_wsdl \
  dwservice_combined_wsdl \
  tech3356_wsdl \
  issue1 \
  issue4 \
  issue38 \
  issue43/regular \
  issue43/pointer \
  issue43/boost-optional \
  enum_escape \
  soap12 \
  literal_true_false \
  import_definition \
  unqualified_formdefault \

# These need internet access
SUBDIRS += webcalls webcalls_wsdl

SUBDIRS += wsdl_rpc-server

test.target=test
unix:!macx {
    LIB_PATH=$${TOP_BUILD_DIR}/lib:\$\$LD_LIBRARY_PATH
    test.commands=for d in $${SUBDIRS}; do pushd . && cd "\$$d" && LD_LIBRARY_PATH=$$LIB_PATH && $(MAKE) test && popd || exit 1; done
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
	test.commands=for %d in ($${SUBDIRS}); do $$RUNTEST "%d" $$WIN_BINDIR || exit 1; done
}
unix:test.commands=for d in $${SUBDIRS}; do pushd . && cd "\$$d" && $(MAKE) test && popd || exit 1; done
test.depends = first
QMAKE_EXTRA_TARGETS += test

