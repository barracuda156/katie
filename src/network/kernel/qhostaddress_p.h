/****************************************************************************
**
** Copyright (C) 2024 Ivailo Monev
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

#ifndef QHOSTADDRESSPRIVATE_H
#define QHOSTADDRESSPRIVATE_H

//
//  W A R N I N G
//  -------------
//
// This file is not part of the Katie API.  It exists for the convenience
// of the QHostAddress and QNetworkInterface classes.  This header file may change from
// version to version without notice, or even be removed.
//
// We mean it.
//

QT_BEGIN_NAMESPACE

#include "qhostaddress.h"
#include "qabstractsocket.h"

class QHostAddressPrivate
{
public:
    QHostAddressPrivate();

    QAbstractSocket::NetworkLayerProtocol protocol;
    QByteArray ipString;
    QByteArray scopeId;

    friend class QHostAddress;
};

QT_END_NAMESPACE

#endif
