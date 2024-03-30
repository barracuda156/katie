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

#include "qnetworkinterface.h"
#include "qnetworkinterface_p.h"

#include "qdebug.h"
#include "qendian.h"
#include "qcorecommon_p.h"

#ifndef QT_NO_NETWORKINTERFACE

QT_BEGIN_NAMESPACE

/*!
    \class QNetworkAddressEntry
    \brief The QNetworkAddressEntry class stores one IP address
    supported by a network interface, along with its associated
    netmask and broadcast address.

    \since 4.2
    \reentrant
    \ingroup network

    Each network interface can contain zero or more IP addresses, which
    in turn can be associated with a netmask and/or a broadcast
    address (depending on support from the operating system).

    This class represents one such group.
*/

/*!
    Constructs an empty QNetworkAddressEntry object.
*/
QNetworkAddressEntry::QNetworkAddressEntry()
    : d(new QNetworkAddressEntryPrivate())
{
}

/*!
    Constructs a QNetworkAddressEntry object that is a copy of the
    object \a other.
*/
QNetworkAddressEntry::QNetworkAddressEntry(const QNetworkAddressEntry &other)
    : d(new QNetworkAddressEntryPrivate())
{
    d->address = other.d->address;
    d->netmask = other.d->netmask;
    d->broadcast = other.d->broadcast;
}

/*!
    Makes a copy of the QNetworkAddressEntry object \a other.
*/
QNetworkAddressEntry &QNetworkAddressEntry::operator=(const QNetworkAddressEntry &other)
{
    d->address = other.d->address;
    d->netmask = other.d->netmask;
    d->broadcast = other.d->broadcast;
    return *this;
}

/*!
    Destroys this QNetworkAddressEntry object.
*/
QNetworkAddressEntry::~QNetworkAddressEntry()
{
    delete d;
}

/*!
    Returns true if this network address entry is the same as \a
    other.
*/
bool QNetworkAddressEntry::operator==(const QNetworkAddressEntry &other) const
{
    return (
        d->address == other.d->address &&
        d->netmask == other.d->netmask &&
        d->broadcast == other.d->broadcast
    );
}

/*!
    \fn bool QNetworkAddressEntry::operator!=(const QNetworkAddressEntry &other) const

    Returns true if this network address entry is different from \a
    other.
*/

/*!
    This function returns one IPv4 or IPv6 address found, that was
    found in a network interface.
*/
QHostAddress QNetworkAddressEntry::ip() const
{
    return d->address;
}

/*!
    Returns the netmask associated with the IP address. The
    netmask is expressed in the form of an IP address, such as
    255.255.0.0.

    For IPv6 addresses, the prefix length is converted to an address
    where the number of bits set to 1 is equal to the prefix
    length. For a prefix length of 64 bits (the most common value),
    the netmask will be expressed as a QHostAddress holding the
    address FFFF:FFFF:FFFF:FFFF::

    \sa prefixLength()
*/
QHostAddress QNetworkAddressEntry::netmask() const
{
    return d->netmask;
}

/*!
    Returns the broadcast address associated with the IPv4
    address and netmask. It can usually be derived from those two by
    setting to 1 the bits of the IP address where the netmask contains
    a 0. (In other words, by bitwise-OR'ing the IP address with the
    inverse of the netmask)

    This member is always empty for IPv6 addresses, since the concept
    of broadcast has been abandoned in that system in favor of
    multicast. In particular, the group of hosts corresponding to all
    the nodes in the local network can be reached by the "all-nodes"
    special multicast group (address FF02::1).
*/
QHostAddress QNetworkAddressEntry::broadcast() const
{
    return d->broadcast;
}

/*!
    \class QNetworkInterface
    \brief The QNetworkInterface class provides a listing of the host's IP
    addresses and network interfaces.

    \since 4.2
    \reentrant
    \ingroup network

    QNetworkInterface represents one network interface attached to the
    host where the program is being run. Each network interface may
    contain zero or more IP addresses, each of which is optionally
    associated with a netmask and/or a broadcast address. The list of
    such trios can be obtained with addressEntries(). Alternatively,
    when the netmask or the broadcast addresses aren't necessary, use
    the allAddresses() convenience function to obtain just the IP
    addresses.

    QNetworkInterface also reports the interface's hardware address with
    hardwareAddress().

    Not all operating systems support reporting all features. Only the
    IPv4 addresses are guaranteed to be listed by this class in all
    platforms. In particular, IPv6 address listing is only supported
    on Linux and the BSDs, hardware address only on Linux.

    \sa QNetworkAddressEntry
*/

/*!
    \enum QNetworkInterface::InterfaceFlag
    Specifies the flags associated with this network interface. The
    possible values are:

    \value IsUp                 the network interface is active
    \value IsRunning            the network interface has resources
                                allocated
    \value CanBroadcast         the network interface works in
                                broadcast mode
    \value IsLoopBack           the network interface is a loopback
                                interface: that is, it's a virtual
                                interface whose destination is the
                                host computer itself
    \value IsPointToPoint       the network interface is a
                                point-to-point interface: that is,
                                there is one, single other address
                                that can be directly reached by it.
    \value CanMulticast         the network interface supports
                                multicasting

    Note that one network interface cannot be both broadcast-based and
    point-to-point.
*/

/*!
    Constructs an empty network interface object.
*/
QNetworkInterface::QNetworkInterface()
    : d(nullptr)
{
}

/*!
    Frees the resources associated with the QNetworkInterface object.
*/
QNetworkInterface::~QNetworkInterface()
{
}

/*!
    Creates a copy of the QNetworkInterface object contained in \a
    other.
*/
QNetworkInterface::QNetworkInterface(const QNetworkInterface &other)
    : d(other.d)
{
}

/*!
    Copies the contents of the QNetworkInterface object contained in \a
    other into this one.
*/
QNetworkInterface &QNetworkInterface::operator=(const QNetworkInterface &other)
{
    d = other.d;
    return *this;
}

/*!
    Returns true if this QNetworkInterface object contains valid
    information about a network interface.
*/
bool QNetworkInterface::isValid() const
{
    return !name().isEmpty();
}

/*!
    \since 4.5
    Returns the interface system index, if known. This is an integer
    assigned by the operating system to identify this interface and it
    generally doesn't change. It matches the scope ID field in IPv6
    addresses.

    If the index isn't known, this function returns 0.
*/
int QNetworkInterface::index() const
{
    return d ? d->index : 0;
}

/*!
    Returns the name of this network interface. On Unix systems, this
    is a string containing the type of the interface and optionally a
    sequence number, such as "eth0", "lo" or "pcn0". On Windows, it's
    an internal ID that cannot be changed by the user.
*/
QString QNetworkInterface::name() const
{
    return d ? d->name : QString();
}

/*!
    Returns the flags associated with this network interface.
*/
QNetworkInterface::InterfaceFlags QNetworkInterface::flags() const
{
    return d ? d->flags : InterfaceFlags(0);
}

/*!
    Returns the low-level hardware address for this interface.

    Implementations should not depend on this function
    returning a valid MAC address.
*/
QString QNetworkInterface::hardwareAddress() const
{
    return d ? d->hardwareAddress : QString();
}

/*!
    Returns the list of IP addresses that this interface possesses
    along with their associated netmasks and broadcast addresses.

    If the netmask or broadcast address information is not necessary,
    you can call the allAddresses() function to obtain just the IP
    addresses.
*/
QList<QNetworkAddressEntry> QNetworkInterface::addressEntries() const
{
    return d ? d->addressEntries : QList<QNetworkAddressEntry>();
}

/*!
    Returns a QNetworkInterface object for the interface named \a
    name. If no such interface exists, this function returns an
    invalid QNetworkInterface object.

    \sa name(), isValid()
*/
QNetworkInterface QNetworkInterface::interfaceFromName(const QString &name)
{
    foreach (const QNetworkInterface &iface, QNetworkInterface::allInterfaces()) {
        if (iface.name() == name) {
            return iface;
        }
    }
    return QNetworkInterface();
}

/*!
    Returns a QNetworkInterface object for the interface whose internal
    ID is \a index. Network interfaces have a unique identifier called
    the "interface index" to distinguish it from other interfaces on
    the system. Often, this value is assigned progressively and
    interfaces being removed and then added again get a different
    value every time.

    This index is also found in the IPv6 address' scope ID field.
*/
QNetworkInterface QNetworkInterface::interfaceFromIndex(int index)
{
    foreach (const QNetworkInterface &iface, QNetworkInterface::allInterfaces()) {
        if (iface.index() == index) {
            return iface;
        }
    }
    return QNetworkInterface();
}

/*!
    Returns a listing of all the network interfaces found on the host
    machine.
*/
QList<QNetworkInterface> QNetworkInterface::allInterfaces()
{
    QList<QNetworkInterface> result;
    foreach (QNetworkInterfacePrivate* priv, QNetworkInterfacePrivate::scan()) {
        QNetworkInterface iface;
        iface.d = QSharedDataPointer<QNetworkInterfacePrivate>(priv);
        result << iface;
    }
    return result;
}

/*!
    This convenience function returns all IP addresses found on the
    host machine. It is equivalent to calling addressEntries() on all the
    objects returned by allInterfaces() to obtain lists of QHostAddress
    objects then calling QHostAddress::ip() on each of these.
*/
QList<QHostAddress> QNetworkInterface::allAddresses()
{
    QList<QHostAddress> result;
    foreach (const QNetworkInterface &iface, QNetworkInterface::allInterfaces()) {
        foreach (const QNetworkAddressEntry &entry, iface.addressEntries()) {
            result.append(entry.ip());
        }
    }
    return result;
}

#ifndef QT_NO_DEBUG_STREAM
static inline QDebug operator<<(QDebug debug, const QNetworkAddressEntry &entry)
{
    debug.nospace() << "(address = " << entry.ip();
    if (!entry.netmask().isNull())
        debug.nospace() << ", netmask = " << entry.netmask();
    if (!entry.broadcast().isNull())
        debug.nospace() << ", broadcast = " << entry.broadcast();
    debug.nospace() << ')';
    return debug.space();
}

QDebug operator<<(QDebug debug, const QNetworkInterface &networkInterface)
{
    debug.nospace() << "QNetworkInterface(name = " << networkInterface.name()
                    << ", hardware address = " << networkInterface.hardwareAddress()
                    << ", flags = ";
    const QNetworkInterface::InterfaceFlags flags = networkInterface.flags();
    if (flags & QNetworkInterface::IsUp)
        debug.nospace() << "IsUp ";
    if (flags & QNetworkInterface::IsRunning)
        debug.nospace() << "IsRunning ";
    if (flags & QNetworkInterface::CanBroadcast)
        debug.nospace() << "CanBroadcast ";
    if (flags & QNetworkInterface::IsLoopBack)
        debug.nospace() << "IsLoopBack ";
    if (flags & QNetworkInterface::IsPointToPoint)
        debug.nospace() << "IsPointToPoint ";
    if (flags & QNetworkInterface::CanMulticast)
        debug.nospace() << "CanMulticast ";
    debug.nospace() << ", entries = " << networkInterface.addressEntries() << ")\n";
    return debug.space();
}
#endif

QT_END_NAMESPACE

#endif // QT_NO_NETWORKINTERFACE


