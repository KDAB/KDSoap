TEMPLATE = subdirs
SUBDIRS = basic builtinhttp wsdl_rpc sugar_wsdl salesforce_wsdl groupwise_wsdl logbook_wsdl messagereader servertest msexchange_wsdl wsdl_document dwservice_wsdl dwservice_12_wsdl dwservice_combined_wsdl issue1 issue4 soap12
# These need internet access
SUBDIRS += webcalls webcalls_wsdl

SUBDIRS += wsdl_rpc-server

test.target=test
unix:!macx {
    LIB_PATH=$${TOP_BUILD_DIR}/lib:\$\$LD_LIBRARY_PATH
    test.commands=for d in $${SUBDIRS}; do cd "\$$d" && LD_LIBRARY_PATH=$$LIB_PATH && $(MAKE) test && cd .. || exit 1; done
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
unix:test.commands=for d in $${SUBDIRS}; do cd "\$$d" && $(MAKE) test && cd .. || exit 1; done
test.depends = first
QMAKE_EXTRA_TARGETS += test

