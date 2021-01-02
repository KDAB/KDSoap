/****************************************************************************
**
** This file is part of the KD Soap library.
**
** SPDX-FileCopyrightText: 2010-2021 Klarälvdalens Datakonsult AB, a KDAB Group company <info@kdab.com>
**
** SPDX-License-Identifier: LicenseRef-KDAB-KDSoap-AGPL3-Modified OR LicenseRef-KDAB-KDSoap OR LicenseRef-KDAB-KDSoap-US
**
** Licensees holding valid commercial KD Soap licenses may use this file in
** accordance with the KD Soap Commercial License Agreement provided with
** the Software.
**
** Contact info@kdab.com if any conditions of this licensing are not clear to you.
**
****************************************************************************/
#ifndef KDSOAPSERVERGLOBAL_H
#define KDSOAPSERVERGLOBAL_H

#include <QtCore/QtGlobal>

# ifdef KDSOAPSERVER_STATICLIB
#  undef KDSOAPSERVER_SHAREDLIB
#  define KDSOAPSERVER_EXPORT
# else
#  ifdef KDSOAP_BUILD_KDSOAPSERVER_LIB
#   define KDSOAPSERVER_EXPORT Q_DECL_EXPORT
#  else
#   define KDSOAPSERVER_EXPORT Q_DECL_IMPORT
#  endif
# endif

#endif /* KDSOAPSERVERGLOBAL_H */

