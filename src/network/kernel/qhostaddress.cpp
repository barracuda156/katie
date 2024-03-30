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

#include "qhostaddress.h"
#include "qhostaddress_p.h"
#include "qdebug.h"
#include "qcorecommon_p.h"

#ifndef QT_NO_DATASTREAM
#include "qdatastream.h"
#endif

#include <sys/socket.h>
#include <arpa/inet.h>
#include <netinet/in.h>

#ifndef QT_NO_IPV6IFNAME
#  include <net/if.h>
#endif

QT_BEGIN_NAMESPACE

static const char addrscopeseparator = '%';

QHostAddressPrivate::QHostAddressPrivate()
    : protocol(QAbstractSocket::UnknownNetworkLayerProtocol)
{
}

/*!
    \class QHostAddress
    \brief The QHostAddress class provides an IP address.
    \ingroup network
    \inmodule QtNetwork

    This class holds an IPv4 or IPv6 address in a platform- and
    protocol-independent manner.

    QHostAddress is normally used with the QTcpSocket, QTcpServer,
    and QUdpSocket to connect to a host or to set up a server.

    A host address is set with setAddress(), and retrieved with
    toIPv4Address(), toIPv6Address(), or toString(). You can check the
    type with protocol().

    \note Please note that QHostAddress does not do DNS lookups.
    QHostInfo is needed for that.

    The class also supports common predefined addresses: \l Null, \l
    LocalHost, \l LocalHostIPv6, \l Broadcast, and \l Any.

    \sa QHostInfo, QTcpSocket, QTcpServer, QUdpSocket
*/

/*! \enum QHostAddress::SpecialAddress

    \value Null The null address object. Equivalent to QHostAddress().
    \value LocalHost The IPv4 localhost address. Equivalent to QHostAddress("127.0.0.1").
    \value LocalHostIPv6 The IPv6 localhost address. Equivalent to QHostAddress("::1").
    \value Broadcast The IPv4 broadcast address. Equivalent to QHostAddress("255.255.255.255").
    \value Any The IPv4 any-address. Equivalent to QHostAddress("0.0.0.0").
    \value AnyIPv6 The IPv6 any-address. Equivalent to QHostAddress("::").
*/

/*!  Constructs a host address object with the IP address 0.0.0.0.

    \sa clear()
*/
QHostAddress::QHostAddress()
    : d(new QHostAddressPrivate())
{
}

/*!
    Constructs an IPv4 or IPv6 address based on the string \a address
    (e.g., "127.0.0.1").

    \sa setAddress()
*/
QHostAddress::QHostAddress(const QByteArray &address)
    : d(new QHostAddressPrivate())
{
    setAddress(address);
}

/*!
    \fn QHostAddress::QHostAddress(const sockaddr *sockaddr)

    Constructs an IPv4 or IPv6 address using the address specified by
    the native structure \a sockaddr.

    \sa setAddress()
*/
QHostAddress::QHostAddress(const struct sockaddr *sockaddr)
    : d(new QHostAddressPrivate())
{
    setAddress(sockaddr);
}

/*!
    Constructs a copy of the given \a address.
*/
QHostAddress::QHostAddress(const QHostAddress &address)
    : d(new QHostAddressPrivate(*address.d.data()))
{
}

/*!
    Constructs a QHostAddress object for \a address.
*/
QHostAddress::QHostAddress(SpecialAddress address)
    : d(new QHostAddressPrivate())
{
    switch (address) {
        case QHostAddress::Null: {
            break;
        }
        case QHostAddress::Broadcast: {
            d->protocol = QAbstractSocket::IPv4Protocol;
            d->ipString = QByteArray("255.255.255.255");
            break;
        }
        case QHostAddress::LocalHost: {
            d->protocol = QAbstractSocket::IPv4Protocol;
            d->ipString = QByteArray("127.0.0.1");
            break;
        }
        case QHostAddress::LocalHostIPv6: {
            d->protocol = QAbstractSocket::IPv6Protocol;
            d->ipString = QByteArray("::1");
            break;
        }
        case QHostAddress::Any: {
            d->protocol = QAbstractSocket::IPv4Protocol;
            d->ipString = QByteArray("0.0.0.0");
            break;
        }
        case QHostAddress::AnyIPv6: {
            d->protocol = QAbstractSocket::IPv6Protocol;
            d->ipString = QByteArray("::");
            break;
        }
    }
}

/*!
    Destroys the host address object.
*/
QHostAddress::~QHostAddress()
{
}

/*!
    Assigns another host \a address to this object, and returns a reference
    to this object.
*/
QHostAddress &QHostAddress::operator=(const QHostAddress &address)
{
    *d.data() = *address.d.data();
    return *this;
}

/*!
    Assigns the host address \a address to this object, and returns a
    reference to this object.

    \sa setAddress()
*/
QHostAddress &QHostAddress::operator=(const QByteArray &address)
{
    setAddress(address);
    return *this;
}

/*!
    \fn bool QHostAddress::operator!=(const QHostAddress &other) const
    \since 4.2

    Returns true if this host address is not the same as the \a other
    address given; otherwise returns false.
*/

/*!
    \fn bool QHostAddress::operator!=(SpecialAddress other) const

    Returns true if this host address is not the same as the \a other
    address given; otherwise returns false.
*/

/*!
    Sets the host address to 0.0.0.0.
*/
void QHostAddress::clear()
{
    d->protocol = QAbstractSocket::UnknownNetworkLayerProtocol;
    d->ipString.clear();
    d->scopeId.clear();
}

/*!
    \overload

    Sets the IPv4 or IPv6 address specified by the string
    representation specified by \a address (e.g. "127.0.0.1").
    Returns true and sets the address if the address was successfully
    parsed; otherwise returns false.
*/
bool QHostAddress::setAddress(const QByteArray &address)
{
    // compat
    QByteArray scope;
    QByteArray fixedaddress(address.trimmed());
    if (fixedaddress.contains(':')) {
        const int indexofpercent = fixedaddress.indexOf(addrscopeseparator);
        if (indexofpercent >= 0) {
            scope = fixedaddress.mid(indexofpercent + 1, fixedaddress.size() - indexofpercent - 1);
            fixedaddress = fixedaddress.mid(0, indexofpercent);
        }
    }
    int result = 0;
#ifndef QT_NO_IPV6
    struct in6_addr inaddripv6;
    result = inet_pton(AF_INET6, fixedaddress.constData(), &inaddripv6);
    if (result == 1) {
        d->protocol = QAbstractSocket::IPv6Protocol;
        d->ipString = fixedaddress;
        d->scopeId = scope;
        return true;
    }
#endif
    struct in_addr inaddripv4;
    result = inet_pton(AF_INET, fixedaddress.constData(), &inaddripv4);
    if (result == 1) {
        d->protocol = QAbstractSocket::IPv4Protocol;
        d->ipString = fixedaddress;
        d->scopeId.clear();
        return true;
    }
    clear();
    return false;
}

/*!
    \fn void QHostAddress::setAddress(const sockaddr *sockaddr)
    \overload

    Sets the IPv4 or IPv6 address specified by the native structure \a
    sockaddr.  Returns true and sets the address if the address was
    successfully parsed; otherwise returns false.
*/
void QHostAddress::setAddress(const struct sockaddr *sockaddr)
{
    if (!sockaddr) {
        clear();
        return;
    }
    if (sockaddr->sa_family == AF_INET) {
        QSTACKARRAY(char, ntopbuffer, INET_ADDRSTRLEN + 1);
        if (inet_ntop(AF_INET, &((sockaddr_in *)sockaddr)->sin_addr, ntopbuffer, INET_ADDRSTRLEN) != NULL) {
            d->protocol = QAbstractSocket::IPv4Protocol;
            d->ipString = ntopbuffer;
            d->scopeId.clear();
        } else {
            clear();
        }
        return;
    }
#ifndef QT_NO_IPV6
    if (sockaddr->sa_family == AF_INET6) {
        QSTACKARRAY(char, ntopbuffer, INET6_ADDRSTRLEN + 1);
        const sockaddr_in6* si6 = (sockaddr_in6 *)sockaddr;
        if (inet_ntop(AF_INET6, &(si6->sin6_addr), ntopbuffer, INET6_ADDRSTRLEN) != NULL) {
            d->protocol = QAbstractSocket::IPv6Protocol;
            d->ipString = ntopbuffer;
            d->scopeId.clear();
            if (si6->sin6_scope_id) {
#ifndef QT_NO_IPV6IFNAME
                QSTACKARRAY(char, indexofnamebuffer, IFNAMSIZ);
                if (::if_indextoname(si6->sin6_scope_id, indexofnamebuffer)) {
                    d->scopeId = indexofnamebuffer;
                }
#endif
                if (d->scopeId.isEmpty()) {
                    d->scopeId = QByteArray::number(si6->sin6_scope_id);
                }
            }
        } else {
            clear();
        }
        return;
    }
#endif
    clear();
}

/*!
    Returns the network layer protocol of the host address.
*/
QAbstractSocket::NetworkLayerProtocol QHostAddress::protocol() const
{
    return d->protocol;
}

/*!
    Returns the address as a string.

    For example, if the address is the IPv4 address 127.0.0.1, the
    returned string is "127.0.0.1". For IPv6 the string format will 
    follow the RFC5952 recommendation.
*/
QByteArray QHostAddress::toString() const
{
    if (d->protocol == QAbstractSocket::IPv6Protocol && !d->scopeId.isEmpty()) {
        return d->ipString + addrscopeseparator + d->scopeId;
    }
    return d->ipString;
}

/*!
    \since 4.1

    Returns the scope ID of an IPv6 address. For IPv4 addresses, or if the
    address does not contain a scope ID, an empty QByteArray is returned.

    The IPv6 scope ID specifies the scope of \e reachability for non-global
    IPv6 addresses, limiting the area in which the address can be used. All
    IPv6 addresses are associated with such a reachability scope. The scope ID
    is used to disambiguate addresses that are not guaranteed to be globally
    unique.

    IPv6 specifies the following four levels of reachability:

    \list

    \o Node-local: Addresses that are only used for communicating with
    services on the same interface (e.g., the loopback interface "::1").

    \o Link-local: Addresses that are local to the network interface
    (\e{link}). There is always one link-local address for each IPv6 interface
    on your host. Link-local addresses ("fe80...") are generated from the MAC
    address of the local network adaptor, and are not guaranteed to be unique.

    \o Site-local: Addresses that are local to the site / private network
    (e.g., the company intranet). Site-local addresses ("fec0...")  are
    usually distributed by the site router, and are not guaranteed to be
    unique outside of the local site.

    \o Global: For globally routable addresses, such as public servers on the
    Internet.

    \endlist

    When using a link-local or site-local address for IPv6 connections, you
    must specify the scope ID. The scope ID for a link-local address is
    usually the same as the interface name (e.g., "eth0", "en1") or number
    (e.g., "1", "2").

    \sa setScopeId()
*/
QByteArray QHostAddress::scopeId() const
{
    if (d->protocol == QAbstractSocket::IPv6Protocol) {
        return d->scopeId;
    }
    return QByteArray();
}

/*!
    \since 4.1

    Sets the IPv6 scope ID of the address to \a id. If the address
    protocol is not IPv6, this function does nothing.
*/
void QHostAddress::setScopeId(const QByteArray &id)
{
    if (d->protocol == QAbstractSocket::IPv6Protocol) {
        d->scopeId = id;
    }
}

/*!
    Returns true if this host address is the same as the \a other address
    given; otherwise returns false.
*/
bool QHostAddress::operator==(const QHostAddress &other) const
{
    return (
        d->protocol == other.d->protocol
        && d->ipString == other.d->ipString
        && d->scopeId == other.d->scopeId
    );
}

/*!
    Returns true if this host address is null (INADDR_ANY or in6addr_any).
    The default constructor creates a null address, and that address is
    not valid for any host or interface.
*/
bool QHostAddress::isNull() const
{
    return (d->protocol == QAbstractSocket::UnknownNetworkLayerProtocol);
}

#ifndef QT_NO_DEBUG_STREAM
QDebug operator<<(QDebug d, const QHostAddress &address)
{
    d.maybeSpace() << "QHostAddress(" << address.toString() << ')';
    return d.space();
}
#endif

uint qHash(const QHostAddress &key)
{
    return qHash(key.toString());
}

#ifndef QT_NO_DATASTREAM

/*! \relates QHostAddress

    Writes host address \a address to the stream \a out and returns a reference
    to the stream.

    \sa {Serializing Qt Data Types}
*/
QDataStream &operator<<(QDataStream &out, const QHostAddress &address)
{
    out << qint8(address.protocol());
    switch (address.protocol()) {
        case QAbstractSocket::UnknownNetworkLayerProtocol: {
            break;
        }
        case QAbstractSocket::IPv4Protocol:
        case QAbstractSocket::IPv6Protocol: {
            out << address.toString();
            break;
        }
    }
    return out;
}

/*! \relates QHostAddress

    Reads a host address into \a address from the stream \a in and returns a
    reference to the stream.

    \sa {Serializing Qt Data Types}
*/
QDataStream &operator>>(QDataStream &in, QHostAddress &address)
{
    qint8 prot;
    in >> prot;
    switch (QAbstractSocket::NetworkLayerProtocol(prot)) {
        case QAbstractSocket::UnknownNetworkLayerProtocol: {
            address.clear();
            break;
        }
        case QAbstractSocket::IPv4Protocol: {
            QByteArray ipv4;
            in >> ipv4;
            address.setAddress(ipv4);
            break;
        }
        case QAbstractSocket::IPv6Protocol: {
            QByteArray ipv6;
            in >> ipv6;
            address.setAddress(ipv6);
            break;
        }
        default: {
            address.clear();
            in.setStatus(QDataStream::ReadCorruptData);
        }
    }
    return in;
}
#endif //QT_NO_DATASTREAM

QT_END_NAMESPACE


