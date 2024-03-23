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

// #define QHOSTINFO_DEBUG

#include "qplatformdefs.h"

#include "qhostinfo_p.h"
#include "qiodevice.h"
#include "qbytearray.h"
#include "qurl.h"
#include "qfile.h"
#include "qnet_unix_p.h"
#include "qcorecommon_p.h"

#include <sys/types.h>
#include <netdb.h>
#include <arpa/inet.h>
#include <netinet/in.h>

QT_BEGIN_NAMESPACE

#if defined(QHOSTINFO_DEBUG)
void dumpHostResult(const QString &hostName, const QHostInfo &results)
{
    if (results.error() != QHostInfo::NoError) {
        qDebug("QHostInfoPrivate::fromName(): error #%d %s",
               int(results.error()), results.errorString().toLatin1().constData());
    } else {
        QString tmp;
        QList<QHostAddress> addresses = results.addresses();
        for (int i = 0; i < addresses.count(); ++i) {
            if (i != 0) tmp += ", ";
            tmp += addresses.at(i).toString();
        }
        qDebug("QHostInfoPrivate::fromName(): found %i entries for \"%s\": {%s}",
               addresses.count(), hostName.toLatin1().constData(),
               tmp.toLatin1().constData());
    }
}
#endif

QHostInfo QHostInfoPrivate::fromName(const QString &hostName)
{
    QHostInfo results;
    results.d->err = QHostInfo::NoError;
    results.d->errorStr = QCoreApplication::translate("QHostInfo", "Unknown error");

    QHostAddress address;
    if (address.setAddress(hostName)) {
#if defined(QHOSTINFO_DEBUG)
        qDebug("QHostInfoPrivate::fromName(%s) looking up address...",
               hostName.toLatin1().constData());
#endif

        // Reverse lookup
        sockaddr_in sa4;
#ifndef QT_NO_IPV6
        sockaddr_in6 sa6;
#endif
        sockaddr *sa = 0;
        QT_SOCKLEN_T saSize = 0;
        if (address.protocol() == QAbstractSocket::IPv4Protocol) {
            sa = (sockaddr *)&sa4;
            saSize = sizeof(sa4);
            ::memset(&sa4, 0, sizeof(sa4));
            sa4.sin_family = AF_INET;
            sa4.sin_addr.s_addr = htonl(address.toIPv4Address());
        }
#ifndef QT_NO_IPV6
        else {
            sa = (sockaddr *)&sa6;
            saSize = sizeof(sa6);
            ::memset(&sa6, 0, sizeof(sa6));
            sa6.sin6_family = AF_INET6;
            ::memcpy(sa6.sin6_addr.s6_addr, address.toIPv6Address().c, sizeof(sa6.sin6_addr.s6_addr));
        }
#endif

        QSTACKARRAY(char, hbuf, NI_MAXHOST);
        int result = (sa ? ::getnameinfo(sa, saSize, hbuf, sizeof(hbuf), NULL, 0, NI_NAMEREQD) : EAI_NONAME);
        // try again without NI_NAMEREQD, that may work for who knows what reason
        if (result != 0) {
#if defined(QHOSTINFO_DEBUG)
            qDebug("QHostInfoPrivate::fromName(%s) getnameinfo retry without NI_NAMEREQD due to %d",
                   hostName.toLatin1().constData(), result);
#endif
            result = (sa ? ::getnameinfo(sa, saSize, hbuf, sizeof(hbuf), NULL, 0, 0) : EAI_NONAME);
        }
#if defined(QHOSTINFO_DEBUG)
        qDebug("QHostInfoPrivate::fromName(%s) getnameinfo returned %d",
               hostName.toLatin1().constData(), result);
#endif
        if (result == 0) {
            results.d->hostName = QString::fromLatin1(hbuf);
        } else if (result == EAI_NONAME || result == EAI_FAIL
#ifdef EAI_NODATA
               // EAI_NODATA is deprecated in RFC 3493
               || result == EAI_NODATA
#endif
               ) {
            results.d->err = QHostInfo::HostNotFound;
            results.d->errorStr = QCoreApplication::translate("QHostInfo", "Host not found");
        } else {
            results.d->err = QHostInfo::UnknownError;
            results.d->errorStr = QString::fromLocal8Bit(::gai_strerror(result));
        }

        if (results.hostName().isEmpty())
            results.d->hostName = address.toString();
        results.d->addrs.append(address);
#if defined(QHOSTINFO_DEBUG)
        dumpHostResult(hostName, results);
#endif
        return results;
    }

    // IDN support
#if defined(QHOSTINFO_DEBUG)
    qDebug("QHostInfoPrivate::fromName(%s) looking up name...",
           hostName.toLatin1().constData());
#endif
    QByteArray aceHostname = QUrl::toAce(hostName);
    results.d->hostName = hostName;
    if (aceHostname.isEmpty()) {
        results.d->err = QHostInfo::HostNotFound;
        if (hostName.isEmpty()) {
            results.d->errorStr = QCoreApplication::translate("QHostInfo", "No host name given");
        } else {
            results.d->errorStr = QCoreApplication::translate("QHostInfo", "Invalid hostname");
        }
#if defined(QHOSTINFO_DEBUG)
        dumpHostResult(hostName, results);
#endif
        return results;
    }

    // Call getaddrinfo, and place all IPv4 addresses at the start and
    // the IPv6 addresses at the end of the address list in results.
    addrinfo *res = 0;
    struct addrinfo hints;
    ::memset(&hints, 0, sizeof(hints));
    hints.ai_family = PF_UNSPEC;
#ifdef AI_ADDRCONFIG
    hints.ai_flags = AI_ADDRCONFIG;
#endif

    int result = ::getaddrinfo(aceHostname.constData(), 0, &hints, &res);
# ifdef AI_ADDRCONFIG
    if (result == EAI_BADFLAGS) {
        // if the lookup failed with AI_ADDRCONFIG set, try again without it
        hints.ai_flags = 0;
        result = ::getaddrinfo(aceHostname.constData(), 0, &hints, &res);
    }
# endif
#if defined(QHOSTINFO_DEBUG)
    qDebug("QHostInfoPrivate::fromName(%s) getaddrinfo returned %d",
           hostName.toLatin1().constData(), result);
#endif

    if (result == 0) {
        addrinfo *node = res;
        QList<QHostAddress> addresses;
        while (node) {
#ifdef QHOSTINFO_DEBUG
                qDebug() << "getaddrinfo node: flags:" << node->ai_flags << "family:" << node->ai_family << "ai_socktype:" << node->ai_socktype << "ai_protocol:" << node->ai_protocol << "ai_addrlen:" << node->ai_addrlen;
#endif
            if (node->ai_family == AF_INET) {
                QHostAddress addr(ntohl(((sockaddr_in *) node->ai_addr)->sin_addr.s_addr));
                if (!addresses.contains(addr))
                    addresses.append(addr);
            }
#ifndef QT_NO_IPV6
            else if (node->ai_family == AF_INET6) {
                sockaddr_in6 *sa6 = (sockaddr_in6 *) node->ai_addr;
                QHostAddress addr(sa6->sin6_addr.s6_addr);
                if (sa6->sin6_scope_id)
                    addr.setScopeId(QString::number(sa6->sin6_scope_id));
                if (!addresses.contains(addr))
                    addresses.append(addr);
            }
#endif
            node = node->ai_next;
        }
        if (addresses.isEmpty() && node == 0) {
            // Reached the end of the list, but no addresses were found; this
            // means the list contains one or more unknown address types.
            results.d->err = QHostInfo::UnknownError;
            results.d->errorStr = QCoreApplication::translate("QHostInfo", "Unknown address type");
        }

        results.d->addrs = addresses;
        ::freeaddrinfo(res);
    } else if (result == EAI_NONAME || result ==  EAI_FAIL
#ifdef EAI_NODATA
                // EAI_NODATA is deprecated in RFC 3493
                || result == EAI_NODATA
#endif
                ) {
        results.d->err = QHostInfo::HostNotFound;
        results.d->errorStr = QCoreApplication::translate("QHostInfo", "Host not found");
    } else {
        results.d->err = QHostInfo::UnknownError;
        results.d->errorStr = QString::fromLocal8Bit(::gai_strerror(result));
    }

#if defined(QHOSTINFO_DEBUG)
    dumpHostResult(hostName, results);
#endif
    return results;
}

QString QHostInfo::localHostName()
{
    static long size_max = sysconf(_SC_HOST_NAME_MAX);
    if (size_max == -1) {
        size_max = _POSIX_HOST_NAME_MAX;
    }
    QSTACKARRAY(char, gethostbuff, size_max);
    if (Q_LIKELY(::gethostname(gethostbuff, size_max) == 0)) {
        return QString::fromLocal8Bit(gethostbuff);
    }
    return QString();
}

QString QHostInfo::localDomainName()
{
#if defined(QT_HAVE_GETDOMAINNAME)
    // thread-safe
    static long size_max = sysconf(_SC_HOST_NAME_MAX);
    if (size_max == -1) {
        size_max = _POSIX_HOST_NAME_MAX;
    }
    QSTACKARRAY(char, getdomainbuff, size_max);
    if (Q_LIKELY(::getdomainname(getdomainbuff, sizeof(getdomainbuff)) == 0)) {
        if (qstrncmp("(none)", getdomainbuff, 6) == 0) {
            // not set
            return QString();
        }
        return QUrl::fromAce(QByteArray(getdomainbuff));
    }
    return QString();
#else
    // doing it by ourselves
    QFile resolvconf(QLatin1String("/etc/resolv.conf"));
    if (!resolvconf.open(QIODevice::ReadOnly))
        return QString();       // failure

    QString domainName;
    while (!resolvconf.atEnd()) {
        QByteArray line = resolvconf.readLine().trimmed();
        if (line.startsWith("domain "))
            return QUrl::fromAce(line.mid(sizeof "domain " - 1).trimmed());

        // in case there's no "domain" line, fall back to the first "search" entry
        if (domainName.isEmpty() && line.startsWith("search ")) {
            QByteArray searchDomain = line.mid(sizeof "search " - 1).trimmed();
            int pos = searchDomain.indexOf(' ');
            if (pos != -1)
                searchDomain.truncate(pos);
            domainName = QUrl::fromAce(searchDomain);
        }
    }

    // return the fallen-back-to searched domain
    return domainName;
#endif // QT_HAVE_GETDOMAINNAME
}

QT_END_NAMESPACE


