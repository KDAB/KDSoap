# This file is part of the KD Soap project.
#
# SPDX-FileCopyrightText: 2012-2023 Klarälvdalens Datakonsult AB, a KDAB Group company <info@kdab.com>
#
# SPDX-License-Identifier: MIT
#

if(NOT ${PROJECT_NAME}_QT6)
    install(FILES kdsoap.prf DESTINATION ${INSTALL_DATADIR}/mkspecs/features)
endif()
