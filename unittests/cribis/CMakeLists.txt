# This file is part of the KD Soap project.
#
# SPDX-FileCopyrightText: 2016 Klarälvdalens Datakonsult AB, a KDAB Group company <info@kdab.com>
#
# SPDX-License-Identifier: MIT
#

set(cribis_SRCS test_cribis.cpp)
set(WSDL_FILES CribisSKWS.wsdl)
add_unittest(${cribis_SRCS})
