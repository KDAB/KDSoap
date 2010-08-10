#    Note: KDSOAP_PATH is set in the calling .pro file
include (../examples/examples.pri)

# Unittests shouldn't be in ../bin, it breaks 'nmake test' on Windows and makes things more difficult for developing on linux
DESTDIR=

CONFIG += qtestlib

