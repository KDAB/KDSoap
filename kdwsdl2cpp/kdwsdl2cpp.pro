TEMPLATE=subdirs
SUBDIRS=libkode/code_generation libkode/common libkode/schema wsdl src
wsdl.depends = libkode/common libkode/schema
src.depends = wsdl libkode/code_generation

# common should be "libxmlcommon" ideally, but the code uses #include "common/file.h" everywhere.
# same for libwsdl
