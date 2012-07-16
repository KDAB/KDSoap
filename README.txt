KD Soap is a Qt-based client-side and server-side SOAP component.

It can be used to create client applications for web services and also provides
the means to create web services without the need for any further component such
as a dedicated web server.

For a full description of KDSoap, see http://www.kdab.com/kdab-products/kd-soap/

KD Soap is (C) 2010-2012, Klaralvdalens Datakonsult AB, and is available
under the terms of:
* the LGPL (see LICENSE.LGPL.txt for details)
* the GPL (see LICENSE.GPL.txt for details)
* the KDAB commercial license, provided that you buy a license, see
http://www.kdab.com/kdab-products/prices/

KD Soap requires Qt 4.6.0 or newer.

Note: the use of the cookie jar requires Qt 4.7.0 or newer.
Note: large multithreaded kdsoap servers should use Qt 4.7.0 or newer.

To build KD Soap from a github clone, do the following:
* git clone https://github.com/KDAB/KDSoap.git
* cd KDSoap
* git submodule update --init
* ./autogen.py [options]
* make

autogen.py supports a number of command-line options, including -prefix <dir>
to specify an installation prefix for KD Soap. This affects make install.

For more information, please refer to the KD Soap Programmer's Manual.

With best regards,

the KDAB KD Soap team.

