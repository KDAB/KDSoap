/****************************************************************************
**
** This file is part of the KD Soap project..
**
** SPDX-FileCopyrightText: 2010-2022 Klarälvdalens Datakonsult AB, a KDAB Group company <info@kdab.com>
**
** SPDX-License-Identifier: MIT
**
****************************************************************************/
#ifndef KDSOAPGLOBAL_H
#define KDSOAPGLOBAL_H

#include <QtCore/QtGlobal>

#ifdef KDSOAP_STATICLIB
#undef KDSOAP_SHARED
#define KDSOAP_EXPORT
#else
#ifdef KDSOAP_BUILD_KDSOAP_LIB
#define KDSOAP_EXPORT Q_DECL_EXPORT
#else
#define KDSOAP_EXPORT Q_DECL_IMPORT
#endif
#endif

#endif /* KDSOAPGLOBAL_H */
