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

#include "qset.h"
#include "qnetworkinterface.h"
#include "qnetworkinterface_p.h"
#include "qalgorithms.h"
#include "qnet_unix_p.h"
#include "qplatformdefs.h"
#include "qcorecommon_p.h"

#ifndef QT_NO_NETWORKINTERFACE

#include <sys/types.h>
#include <sys/socket.h>
#include <net/if.h>
#include <netinet/in.h>
#include <arpa/inet.h>

#ifdef Q_OS_SOLARIS
#  include <sys/sockio.h>
#endif

#ifdef QT_HAVE_GETIFADDRS
#  include <ifaddrs.h>
#endif
#include <sys/ioctl.h>

QT_BEGIN_NAMESPACE

static QString makeHwAddress(const uchar *data)
{
    QSTACKARRAY(char, snprintfbuf, 18);
    ::snprintf(snprintfbuf, sizeof(snprintfbuf),
        "%02hX:%02hX:%02hX:%02hX:%02hX:%02hX",
        ushort(data[0]), ushort(data[1]), ushort(data[2]), ushort(data[3]), ushort(data[4]), ushort(data[5])
    );
    return QString::fromLatin1(snprintfbuf, sizeof(snprintfbuf) - 1);
}

QList<QNetworkInterfacePrivate *> QNetworkInterfacePrivate::scan()
{
    QList<QNetworkInterfacePrivate *> interfaces;

#ifdef QT_HAVE_GETIFADDRS
    struct ifaddrs *iflist = nullptr;
    if (::getifaddrs(&iflist) != 0) {
        return interfaces;
    }

    int socket = qt_safe_socket(AF_INET, SOCK_STREAM, IPPROTO_IP);
    struct ifaddrs *ifiter = nullptr;
    for (ifiter = iflist; ifiter; ifiter = ifiter->ifa_next) {
        QNetworkInterfacePrivate *iface = nullptr;
        const uint ifindex = ::if_nametoindex(ifiter->ifa_name);
        foreach (QNetworkInterfacePrivate *ifaceit, interfaces) {
            if (ifaceit->index == ifindex) {
                iface = ifaceit;
                break;
            }
        }

        if (!iface) {
            iface = new QNetworkInterfacePrivate();
            iface->index = ifindex;
            iface->name = QString::fromLatin1(ifiter->ifa_name);
            iface->flags = 0;
            if (ifiter->ifa_flags & IFF_UP) {
                iface->flags |= QNetworkInterface::IsUp;
            }
            if (ifiter->ifa_flags & IFF_RUNNING) {
                iface->flags |= QNetworkInterface::IsRunning;
            }
            if (ifiter->ifa_flags & IFF_BROADCAST) {
                iface->flags |= QNetworkInterface::CanBroadcast;
            }
            if (ifiter->ifa_flags & IFF_LOOPBACK) {
                iface->flags |= QNetworkInterface::IsLoopBack;
            }
            if (ifiter->ifa_flags & IFF_POINTOPOINT) {
                iface->flags |= QNetworkInterface::IsPointToPoint;
            }
            if (ifiter->ifa_flags & IFF_MULTICAST) {
                iface->flags |= QNetworkInterface::CanMulticast;
            }
            interfaces.append(iface);
        }

        QNetworkAddressEntry entry;
        entry.d->address = QHostAddress(ifiter->ifa_addr);
        if (entry.ip().isNull()) {
            // could not parse the address
            continue;
        }

        entry.d->netmask = QHostAddress(ifiter->ifa_netmask);
        if (iface->flags & QNetworkInterface::CanBroadcast) {
            entry.d->broadcast = QHostAddress(ifiter->ifa_broadaddr);
        }

        iface->addressEntries << entry;

#ifdef SIOCGIFHWADDR // not defined on FreeBSD
        if (socket != -1) {
            struct ifreq req;
            ::memset(&req, 0, sizeof(ifreq));
            ::memcpy(req.ifr_name, ifiter->ifa_name, sizeof(req.ifr_name) - 1);
            // Get the HW address
            if (::ioctl(socket, SIOCGIFHWADDR, &req) >= 0) {
                iface->hardwareAddress = makeHwAddress((uchar *)req.ifr_addr.sa_data);
            }
        }
#endif // SIOCGIFHWADDR
    }

    if (socket != -1) {
        qt_safe_close(socket);
    }
    ::freeifaddrs(iflist);
#endif // QT_HAVE_GETIFADDRS

    return interfaces;
}

QT_END_NAMESPACE

#endif // QT_NO_NETWORKINTERFACE


