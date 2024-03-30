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


#include <QtTest/QtTest>

#include <qcoreapplication.h>
#include <qnetworkinterface.h>
#include <qtcpsocket.h>
#include "../network-settings.h"

// TESTED_FILES=qnetworkinterface.cpp qnetworkinterface.h qnetworkinterface_unix.cpp

#ifndef QT_NO_NETWORKINTERFACE

class tst_QNetworkInterface : public QObject
{
    Q_OBJECT

private slots:
    void dump();
    void loopbackIPv4();
    void loopbackIPv6();
    void localAddress();
    void interfaceFromXXX();
    void copyInvalidInterface();
};

void tst_QNetworkInterface::dump()
{
    // This is for manual testing:
    QList<QNetworkInterface> allInterfaces = QNetworkInterface::allInterfaces();
    foreach (const QNetworkInterface &i, allInterfaces) {
        QString flags;
        if (i.flags() & QNetworkInterface::IsUp) flags += "Up,";
        if (i.flags() & QNetworkInterface::IsRunning) flags += "Running,";
        if (i.flags() & QNetworkInterface::CanBroadcast) flags += "Broadcast,";
        if (i.flags() & QNetworkInterface::IsLoopBack) flags += "Loopback,";
        if (i.flags() & QNetworkInterface::IsPointToPoint) flags += "PointToPoint,";
        if (i.flags() & QNetworkInterface::CanMulticast) flags += "Multicast,";
        flags.chop(1);          // drop last comma

        qDebug() << "Interface:     " << i.name();
        QVERIFY(i.isValid());

        qDebug() << "    index:     " << i.index();
        qDebug() << "    flags:     " << qPrintable(flags);
        qDebug() << "    hw address:" << qPrintable(i.hardwareAddress());

        int count = 0;
        foreach (const QNetworkAddressEntry &e, i.addressEntries()) {
            QDebug s = qDebug();
            s.nospace() <<    "    address "
                        << "  " << count++;
            s.nospace() << ": " << e.ip().toString();
            if (!e.netmask().isNull())
                s.nospace() << " netmask " << e.netmask().toString();
            if (!e.broadcast().isNull())
                s.nospace() << " broadcast " << e.broadcast().toString();
        }
    }
}

void tst_QNetworkInterface::loopbackIPv4()
{
    QList<QHostAddress> all = QNetworkInterface::allAddresses();
    QVERIFY(all.contains(QHostAddress(QHostAddress::LocalHost)));
}

void tst_QNetworkInterface::loopbackIPv6()
{
    QList<QHostAddress> all = QNetworkInterface::allAddresses();

    bool loopbackfound = false;
    bool anyIPv6 = false;
    foreach (QHostAddress addr, all)
        if (addr == QHostAddress::LocalHostIPv6) {
            loopbackfound = true;
            anyIPv6 = true;
            break;
        } else if (addr.protocol() == QAbstractSocket::IPv6Protocol)
            anyIPv6 = true;

    QVERIFY(!anyIPv6 || loopbackfound);
}

void tst_QNetworkInterface::localAddress()
{
    QTcpSocket socket;
    socket.connectToHost(QtNetworkSettings::serverName(), 80);
    QVERIFY(socket.waitForConnected(5000));

    QHostAddress local = socket.localAddress();

    // make Apache happy on fluke
    socket.write("GET / HTTP/1.0\r\n\r\n");
    socket.waitForBytesWritten(1000);
    socket.close();

    // test that we can find the address that QTcpSocket reported
    QList<QHostAddress> all = QNetworkInterface::allAddresses();
    QVERIFY(all.contains(local));
}

void tst_QNetworkInterface::interfaceFromXXX()
{
    QList<QNetworkInterface> allInterfaces = QNetworkInterface::allInterfaces();

    foreach (QNetworkInterface iface, allInterfaces) {
        QVERIFY(QNetworkInterface::interfaceFromName(iface.name()).isValid());
        foreach (QNetworkAddressEntry entry, iface.addressEntries()) {
            QVERIFY(!entry.ip().isNull());

            if (!entry.netmask().isNull()) {
                QCOMPARE(entry.netmask().protocol(), entry.ip().protocol());
            }

            if (!entry.broadcast().isNull())
                QCOMPARE(entry.broadcast().protocol(), entry.ip().protocol());
        }
    }
}

void tst_QNetworkInterface::copyInvalidInterface()
{
    // Force a copy of an interfaces that isn't likely to exist
    QNetworkInterface i = QNetworkInterface::interfaceFromName("plopp");
    QVERIFY(!i.isValid());

    QCOMPARE(i.index(), 0);
    QVERIFY(i.name().isEmpty());
    QVERIFY(i.hardwareAddress().isEmpty());
    QCOMPARE(int(i.flags()), 0);
    QVERIFY(i.addressEntries().isEmpty());
}

QTEST_MAIN(tst_QNetworkInterface)

#include "moc_tst_qnetworkinterface.cpp"

#else // QT_NO_NETWORKINTERFACE

QTEST_NOOP_MAIN

#endif // QT_NO_NETWORKINTERFACE
