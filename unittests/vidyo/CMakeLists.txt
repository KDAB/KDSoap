#
# This file is part of the KD Soap project.
#
# SPDX-FileCopyrightText: 2015-2022 Klarälvdalens Datakonsult AB, a KDAB Group company <info@kdab.com>
#
# SPDX-License-Identifier: MIT
#

set(vidyo_SRCS test_vidyo.cpp)
set(WSDL_FILES VidyoPortalGuestService.wsdl)
add_unittest(${vidyo_SRCS})
