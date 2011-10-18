/****************************************************************************
** Copyright (C) 2010-2011 Klaralvdalens Datakonsult AB.  All rights reserved.
**
** This file is part of the KD Soap library.
**
** Licensees holding valid commercial KD Soap licenses may use this file in
** accordance with the KD Soap Commercial License Agreement provided with
** the Software.
**
**
** This file may be distributed and/or modified under the terms of the
** GNU General Public License version 2 and version 3 as published by the
** Free Software Foundation and appearing in the file LICENSE.GPL.txt included.
**
** This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
** WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
**
** Contact info@kdab.com if any conditions of this licensing are not
** clear to you.
**
**********************************************************************/
#ifndef KDSOAPJOB_H
#define KDSOAPJOB_H

#include "KDSoapGlobal.h"

#include <QtCore/QObject>

class KDSoapMessage;

class KDSOAP_EXPORT KDSoapJob : public QObject {
    Q_OBJECT
public:
    explicit KDSoapJob(QObject *parent=0);
    ~KDSoapJob();

    bool isFault() const;
    KDSoapMessage reply() const;

    void start();

Q_SIGNALS:
    void finished(KDSoapJob *);

protected:
    Q_INVOKABLE virtual void doStart() = 0;
    void emitFinished(const KDSoapMessage &reply);

private:
    class Private;
    Private * const d;
};

#endif // KDSOAPJOB_H
