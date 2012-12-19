include( $${TOP_SOURCE_DIR}/examples/examples.pri )

HEADERS = mainwindow.h
SOURCES = holidays_gui.cpp mainwindow.cpp
RESOURCES = resources.qrc
KDWSDL = holidays.wsdl

# This is our only GUI program :)
greaterThan(QT_MAJOR_VERSION, 4) {
    QT += widgets
} else {
    QT += gui
}
CONFIG -= console
macx: CONFIG += app_bundle
