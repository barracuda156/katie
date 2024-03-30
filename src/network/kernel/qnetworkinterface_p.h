/****************************************************************************
**
** Copyright (C) 2015 The Qt Company Ltd.
** Copyright (C) 2016 Ivailo Monev
**
** This file is part of the QtNetwork module of the Katie Toolkit.
**
** $QT_BEGIN_LICENSE:LGPL$
**
** GNU Lesser General Public License Usage
** This file may be used under the terms of the GNU Lesser
** General Public License version 2.1 as published by the Free Software
** Foundation and appearing in the file LICENSE.LGPL included in the
** packaging of this file.  Please review the following information to
** ensure the GNU Lesser General Public License version 2.1 requirements
** will be met: http://www.gnu.org/licenses/old-licenses/lgpl-2.1.html.
**
** $QT_END_LICENSE$
**
****************************************************************************/

#ifndef QNETWORKINTERFACEPRIVATE_H
#define QNETWORKINTERFACEPRIVATE_H

//
//  W A R N I N G
//  -------------
//
// This file is not part of the Katie API.  It exists for the convenience
// of the QLibrary class.  This header file may change from
// version to version without notice, or even be removed.
//
// We mean it.
//

#include <QtCore/qatomic.h>
#include <QtCore/qlist.h>
#include <QtCore/qstring.h>
#include <QtNetwork/qhostaddress.h>
#include <QtNetwork/qabstractsocket.h>
#include "qhostaddress_p.h"

#ifndef QT_NO_NETWORKINTERFACE

QT_BEGIN_NAMESPACE

class QNetworkAddressEntryPrivate
{
public:
    QHostAddress address;
    QHostAddress netmask;
    QHostAddress broadcast;
};

class QNetworkInterfacePrivate: public QSharedData
{
public:
    QNetworkInterfacePrivate()
        : index(0), flags(0)
    {
    }

    int index; // interface index, if known
    QNetworkInterface::InterfaceFlags flags;
    QString name;
    QString hardwareAddress;
    QList<QNetworkAddressEntry> addressEntries;

    static QList<QNetworkInterfacePrivate *> scan();
private:
    Q_DISABLE_COPY(QNetworkInterfacePrivate);
};

QT_END_NAMESPACE

#endif // QT_NO_NETWORKINTERFACE

#endif // QNETWORKINTERFACEPRIVATE_H
