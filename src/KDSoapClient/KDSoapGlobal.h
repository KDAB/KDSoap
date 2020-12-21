/****************************************************************************
**
** This file is part of the KD Soap library.
**
** SPDX-FileCopyrightText: 2010-2020 Klarälvdalens Datakonsult AB, a KDAB Group company <info@kdab.com>
**
** SPDX-License-Identifier: LGPL-2.1-only OR LGPL-3.0-only OR GPL-2.0-only OR GPL-3.0-only OR LicenseRef-KDAB-KDSoap OR LicenseRef-KDAB-KDSoap-US
**
** Licensees holding valid commercial KD Soap licenses may use this file in
** accordance with the KD Soap Commercial License Agreement provided with
** the Software.
**
** Contact info@kdab.com if any conditions of this licensing are not clear to you.
**
****************************************************************************/
#ifndef KDSOAPGLOBAL_H
#define KDSOAPGLOBAL_H

#include <QtCore/QtGlobal>

# ifdef KDSOAP_STATICLIB
#  undef KDSOAP_SHARED
#  define KDSOAP_EXPORT
# else
#  ifdef KDSOAP_BUILD_KDSOAP_LIB
#   define KDSOAP_EXPORT Q_DECL_EXPORT
#  else
#   define KDSOAP_EXPORT Q_DECL_IMPORT
#  endif
# endif

#endif /* KDSOAPGLOBAL_H */

