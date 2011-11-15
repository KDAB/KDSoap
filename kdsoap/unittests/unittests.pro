TEMPLATE = subdirs
SUBDIRS = basic builtinhttp wsdl_rpc sugar_wsdl salesforce_wsdl groupwise_wsdl logbook_wsdl messagereader servertest msexchange_wsdl wsdl_document dwservice_wsdl dwservice_12_wsdl
# These need internet access
SUBDIRS += webcalls webcalls_wsdl

WIN_BINDIR=release
CONFIG(debug, debug|release) {
    WIN_BINDIR=debug
}

test.target=test
unix:!macx {
    LIB_PATH=../../lib:\$\$LD_LIBRARY_PATH
    test.commands=for d in $${SUBDIRS}; do cd "\$$d" && LD_LIBRARY_PATH=$$LIB_PATH && $(MAKE) test && cd .. || exit 1; done
}
unix:macx {
    LIB_PATH=../../lib:\$\$DYLD_LIBRARY_PATH
    test.commands=for d in $${SUBDIRS}; do ( cd "\$$d" && export DYLD_LIBRARY_PATH=$$LIB_PATH && $(MAKE) test ) || exit 1; done
}
win32:test.commands=for %d in ($${SUBDIRS}); do runTest.bat "%d" $$WIN_BINDIR || exit 1; don
unix:test.commands=for d in $${SUBDIRS}; do cd "\$$d" && $(MAKE) test && cd .. || exit 1; done
test.depends = first
QMAKE_EXTRA_TARGETS += test

