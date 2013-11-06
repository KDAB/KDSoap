TEMPLATE = subdirs
SUBDIRS += KDSoapClient KDSoapServer
KDSoapServer.depends = KDSoapClient
