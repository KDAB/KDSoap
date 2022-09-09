/****************************************************************************
**
** This file is part of the KD Soap project.
**
** SPDX-FileCopyrightText: 2010-2022 Klarälvdalens Datakonsult AB, a KDAB Group company <info@kdab.com>
**
** SPDX-License-Identifier: MIT
**
****************************************************************************/
#ifndef KDSOAPSERVERGLOBAL_H
#define KDSOAPSERVERGLOBAL_H

#include <QtCore/QtGlobal>

#ifdef KDSOAPSERVER_STATICLIB
#undef KDSOAPSERVER_SHAREDLIB
#define KDSOAPSERVER_EXPORT
#else
#ifdef KDSOAP_BUILD_KDSOAPSERVER_LIB
#define KDSOAPSERVER_EXPORT Q_DECL_EXPORT
#else
#define KDSOAPSERVER_EXPORT Q_DECL_IMPORT
#endif
#endif

#endif /* KDSOAPSERVERGLOBAL_H */
