/****************************************************************************
**
** Copyright (C) 2015 The Qt Company Ltd.
** Copyright (C) 2016 Ivailo Monev
**
** This file is part of the test suite of the Katie Toolkit.
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

#include <QString>
#include <QHostInfo>
#include <QLocalServer>
#include <QTcpServer>

class QtNetworkSettings
{
public:

    static QString serverLocalName()
    {
        return QString::fromLatin1("w3");
    }
    static QString serverDomainName()
    {
        return QString::fromLatin1("org");
    }
    static QString serverName()
    {
        return serverLocalName() + QLatin1Char('.') + serverDomainName();
    }
    static QString wildcardServerName()
    {
        return QLatin1String("qt-test-server.wildcard.dev.") + serverDomainName();
    }

    static QHostAddress serverIP()
    {
        return QHostInfo::fromName(serverName()).addresses().first();
    }

    static QString serverIPs()
    {
        if (supportsIPv6()) {
            return QLatin1String("104.18.22.19 104.18.23.19 2606:4700::6812:1613 2606:4700::6812:1713");
        }
        return QLatin1String("104.18.23.19");
    }

    static bool compareReplyIMAP(QByteArray const& actual)
    {
        // Server greeting may contain capability, version and server name
        // But spec only requires "* OK" and "\r\n"
        // Match against a prefix and postfix that covers all Cyrus versions
        if (actual.startsWith("* OK ") && actual.endsWith("server ready\r\n")) {
            return true;
        }

        return false;
    }

    static bool compareReplyIMAPSSL(QByteArray const& actual)
    {
        return compareReplyIMAP(actual);
    }

    static bool compareReplyFtp(QByteArray const& actual)
    {
        QList<QByteArray> expected;

        // A few different vsFTPd versions.
        // Feel free to add more as needed
        expected << QByteArray( "220 (vsFTPd 2.0.5)\r\n221 Goodbye.\r\n" );
        expected << QByteArray( "220 (vsFTPd 2.2.2)\r\n221 Goodbye.\r\n" );

        Q_FOREACH (QByteArray const& ba, expected) {
            if (ba == actual) {
                return true;
            }
        }

        return false;
    }

    static bool supportsIPv6() {
        // QLocalServer can poke the address regardless if it is protected as it does not bind to
        // the address
#ifndef QT_NO_LOCALSERVER
        QLocalServer server;
        return server.listen("::1");
#else
        QTcpServer server;
        return server.listen(QHostAddress(QHostAddress::LocalHostIPv6));
#endif
    }

};

class QtNetworkSettingsInitializerCode {
public:
    QtNetworkSettingsInitializerCode() {
        QHostInfo testServerResult = QHostInfo::fromName(QtNetworkSettings::serverName());
        if (testServerResult.error() != QHostInfo::NoError) {
            qWarning() << "Could not lookup" << QtNetworkSettings::serverName();
            qWarning() << "Please configure the test environment!";
            qWarning() << "See /etc/hosts or network-settings.h";
            qFatal("Exiting");
        }
    }
};
QtNetworkSettingsInitializerCode qtNetworkSettingsInitializer;
