include( $${TOP_SOURCE_DIR}/examples/examples.pri )

HEADERS = mainwindow.h
SOURCES = bank_gui.cpp mainwindow.cpp
RESOURCES = resources.qrc
KDWSDL = BLZService.wsdl

# This is our only GUI program :)
greaterThan(QT_MAJOR_VERSION, 4) {
    QT += widgets
} else {
    QT += gui
}
CONFIG -= console
macx: CONFIG += app_bundle
