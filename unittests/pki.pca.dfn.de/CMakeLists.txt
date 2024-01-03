# This file is part of the KD Soap project.
#
# SPDX-FileCopyrightText: 2019 Klarälvdalens Datakonsult AB, a KDAB Group company <info@kdab.com>
#
# SPDX-License-Identifier: MIT
#

set(pki.pca.dfn.de_SRCS test_pki.cpp)
set(WSDL_FILES pki.pca.dfn.de.wsdl)
add_unittest(${pki.pca.dfn.de_SRCS})
