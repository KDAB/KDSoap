## This file is part of the KD Soap library.
##
## SPDX-FileCopyrightText: 2010-2020 Klarälvdalens Datakonsult AB, a KDAB Group company <info@kdab.com>
##
## SPDX-License-Identifier: LGPL-2.1-only OR LGPL-3.0-only OR GPL-2.0-only OR GPL-3.0-only OR LicenseRef-KDAB-KDSoap OR LicenseRef-KDAB-KDSoap-US
##
## Licensees holding valid commercial KD Soap licenses may use this file in
## accordance with the KD Soap Commercial License Agreement provided with
## the Software.
##
## Contact info@kdab.com if any conditions of this licensing are not clear to you.
##

TEMPLATE=subdirs
SUBDIRS=libkode/code_generation libkode/common libkode/schema wsdl src
wsdl.depends = libkode/common libkode/schema
src.depends = wsdl libkode/code_generation

# common should be "libxmlcommon" ideally, but the code uses #include "common/file.h" everywhere.
# same for libwsdl
