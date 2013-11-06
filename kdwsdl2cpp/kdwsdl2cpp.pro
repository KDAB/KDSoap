TEMPLATE=subdirs
SUBDIRS=libkode common schema wsdl src
wsdl.depends = common schema
src.depends = wsdl libkode

# common should be "libxmlcommon" ideally, but the code uses #include "common/file.h" everywhere.
# same for libwsdl
