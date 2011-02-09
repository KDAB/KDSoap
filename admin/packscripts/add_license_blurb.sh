#!/bin/bash

find "$@" -name '*.h' -o -name '*.cpp' | while read FILE; do
    if grep -qE "Copyright \(C\) 2010-[0-9]{4} Klar.*lvdalens Datakonsult AB" "$FILE" ; then continue; fi
    cat <<EOF > "$FILE".tmp
/****************************************************************************
** Copyright (C) 2010-$(date +%Y) Klaralvdalens Datakonsult AB.  All rights reserved.
**
** This file is part of the KD SOAP library.
**
** Licensees holding valid commercial KD Soap licenses may use this file in
** accordance with the KD Soap Commercial License Agreement provided with
** the Software.
**
**
** This file may be distributed and/or modified under the terms of the
** GNU General Public License version 2 and version 3 as published by the
** Free Software Foundation and appearing in the file LICENSE.GPL included.
**
** This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
** WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
**
** Contact info@kdab.com if any conditions of this licensing are not
** clear to you.
**
**********************************************************************/

EOF
    cat "$FILE" >> "$FILE".tmp
    mv "$FILE".tmp "$FILE"
done
