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

#ifndef QHOSTADDRESS_H
#define QHOSTADDRESS_H

#include <QtCore/qbytearray.h>
#include <QtNetwork/qabstractsocket.h>

struct sockaddr;

QT_BEGIN_NAMESPACE

class QHostAddressPrivate;

class Q_NETWORK_EXPORT QHostAddress
{
public:
    enum SpecialAddress {
        Null,
        Broadcast,
        LocalHost,
        LocalHostIPv6,
        Any,
        AnyIPv6
    };

    enum FormattingOption {
        None = 0x0,
        RemoveScope = 0x1,
    };
    Q_DECLARE_FLAGS(FormattingOptions, FormattingOption)

    QHostAddress();
    explicit QHostAddress(const sockaddr *sockaddr);
    explicit QHostAddress(const QByteArray &address);
    QHostAddress(const QHostAddress &copy);
    QHostAddress(SpecialAddress address);
    ~QHostAddress();

    QHostAddress &operator=(const QHostAddress &other);

    bool isNull() const;
    void clear();

    bool setAddress(const sockaddr *sockaddr);
    bool setAddress(const QByteArray &address);

    QAbstractSocket::NetworkLayerProtocol protocol() const;

    QByteArray toString(FormattingOptions options = None) const;

    QByteArray scopeId() const;
    void setScopeId(const QByteArray &id);

    bool operator ==(const QHostAddress &address) const;
    inline bool operator !=(const QHostAddress &address) const
    { return !operator==(address); }

private:
    QHostAddressPrivate *d;
};
Q_DECLARE_OPERATORS_FOR_FLAGS(QHostAddress::FormattingOptions)

#ifndef QT_NO_DEBUG_STREAM
Q_NETWORK_EXPORT QDebug operator<<(QDebug, const QHostAddress &);
#endif

Q_NETWORK_EXPORT uint qHash(const QHostAddress &key);

#ifndef QT_NO_DATASTREAM
Q_NETWORK_EXPORT QDataStream &operator<<(QDataStream &, const QHostAddress &);
Q_NETWORK_EXPORT QDataStream &operator>>(QDataStream &, QHostAddress &);
#endif

QT_END_NAMESPACE

#endif // QHOSTADDRESS_H
