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


#include <qcoreapplication.h>
#include <QtTest/QtTest>
#include <qhostaddress.h>
#include <qplatformdefs.h>
#include <qdebug.h>
#include <qhash.h>
#include <qbytearray.h>
#include <qdatastream.h>
#include "qnet_unix_p.h"

#include <netinet/in.h>
#include <arpa/inet.h>

//TESTED_CLASS=
//TESTED_FILES=

class tst_QHostAddress : public QObject
{
    Q_OBJECT

public:
    tst_QHostAddress();
    virtual ~tst_QHostAddress();


public slots:
    void init();
    void cleanup();
private slots:
    void constructor_QByteArray_data();
    void constructor_QByteArray();
    void setAddress_QByteArray_data();
    void setAddress_QByteArray();
    void specialAddresses_data();
    void specialAddresses();
    void compare_data();
    void compare();
    void assignment();
    void scopeId();
    void hashKey();
    void streaming_data();
    void streaming();
};

QT_BEGIN_NAMESPACE
namespace QTest {
    template<>
    char *toString(const QHostAddress &addr)
    {
        if (addr.protocol() == QAbstractSocket::UnknownNetworkLayerProtocol)
            return qstrdup("<invalid>");
        return qstrdup(addr.toString());
    }
}
QT_END_NAMESPACE

tst_QHostAddress::tst_QHostAddress()
{
}

tst_QHostAddress::~tst_QHostAddress()
{
}

Q_DECLARE_METATYPE(QHostAddress)

void tst_QHostAddress::init()
{
    qRegisterMetaType<QHostAddress>("QHostAddress");
}

void tst_QHostAddress::cleanup()
{
    // No cleanup is required.
}

void tst_QHostAddress::constructor_QByteArray_data()
{
    setAddress_QByteArray_data();
}

void tst_QHostAddress::constructor_QByteArray()
{
    QFETCH(QByteArray, address);
    QFETCH(bool, ok);
    QFETCH(QByteArray, resAddr);
    QFETCH(int, protocol);

    QHostAddress hostAddr(address);

    if (address == "0.0.0.0" || address == "::") {
        QVERIFY(ok);
    } else {
        QVERIFY(hostAddr.isNull() != ok);
    }

    if (ok)
        QTEST(hostAddr.toString(), resAddr);

    if ( protocol == 4 ) {
        QVERIFY( hostAddr.protocol() == QAbstractSocket::IPv4Protocol || hostAddr.protocol() == QAbstractSocket::UnknownNetworkLayerProtocol );
        QVERIFY( hostAddr.protocol() != QAbstractSocket::IPv6Protocol );
    } else if ( protocol == 6 ) {
        QVERIFY( hostAddr.protocol() != QAbstractSocket::IPv4Protocol && hostAddr.protocol() != QAbstractSocket::UnknownNetworkLayerProtocol );
        QVERIFY( hostAddr.protocol() == QAbstractSocket::IPv6Protocol );
    } else {
        QVERIFY( hostAddr.isNull() );
        QVERIFY( hostAddr.protocol() == QAbstractSocket::UnknownNetworkLayerProtocol );
    }
}

void tst_QHostAddress::setAddress_QByteArray_data()
{
    QTest::addColumn<QByteArray>("address");
    QTest::addColumn<bool>("ok");
    QTest::addColumn<QByteArray>("resAddr");
    QTest::addColumn<int>("protocol"); // 4: IPv4, 6: IPv6, other: undefined

    // next fill it with data
    QTest::newRow("ip4_00")  << QByteArray("127.0.0.1") << (bool)true << QByteArray("127.0.0.1") << 4;
    QTest::newRow("ip4_01")  << QByteArray("255.3.2.1") << (bool)true << QByteArray("255.3.2.1") << 4;
    QTest::newRow("ip4_03")  << QByteArray(" 255.3.2.1") << (bool)true << QByteArray("255.3.2.1") << 4;
    QTest::newRow("ip4_04")  << QByteArray("255.3.2.1\r ") << (bool)true << QByteArray("255.3.2.1") << 4;
    QTest::newRow("ip4_05")  << QByteArray("0.0.0.0") << (bool)true << QByteArray("0.0.0.0") << 4;

    // for the format of IPv6 addresses see also RFC 5952
    QTest::newRow("ip6_00")  << QByteArray("FEDC:BA98:7654:3210:FEDC:BA98:7654:3210") << (bool)true << QByteArray("FEDC:BA98:7654:3210:FEDC:BA98:7654:3210") << 6;
    QTest::newRow("ip6_01")  << QByteArray("1080:0000:0000:0000:0008:0800:200C:417A") << (bool)true << QByteArray("1080::8:800:200C:417A") << 6;
    QTest::newRow("ip6_02")  << QByteArray("1080:0:0:0:8:800:200C:417A") << (bool)true << QByteArray("1080::8:800:200C:417A") << 6;
    QTest::newRow("ip6_03")  << QByteArray("1080::8:800:200C:417A") << (bool)true << QByteArray("1080::8:800:200C:417A") << 6;
    QTest::newRow("ip6_04")  << QByteArray("FF01::43") << (bool)true << QByteArray("FF01::43") << 6;
    QTest::newRow("ip6_05")  << QByteArray("::1") << (bool)true << QByteArray("::1") << 6;
    QTest::newRow("ip6_06")  << QByteArray("1::") << (bool)true << QByteArray("1::") << 6;
    QTest::newRow("ip6_07")  << QByteArray("::") << (bool)true << QByteArray("::") << 6;
    QTest::newRow("ip6_08")  << QByteArray("0:0:0:0:0:0:13.1.68.3") << (bool)true << QByteArray("::D01:4403") << 6;
    QTest::newRow("ip6_09")  << QByteArray("::13.1.68.3") << (bool)true <<  QByteArray("::D01:4403") << 6;
    QTest::newRow("ip6_10")  << QByteArray("0:0:0:0:0:FFFF:129.144.52.38") << (bool)true << QByteArray("::FFFF:8190:3426") << 6;
    QTest::newRow("ip6_11")  << QByteArray("::FFFF:129.144.52.38") << (bool)true << QByteArray("::FFFF:8190:3426") << 6;
    QTest::newRow("ip6_12")  << QByteArray("1::FFFF:129.144.52.38") << (bool)true << QByteArray("1::FFFF:8190:3426") << 6;
    QTest::newRow("ip6_13")  << QByteArray("A:B::D:E") << (bool)true << QByteArray("A:B::D:E") << 6;
    QTest::newRow("ip6_14")  << QByteArray("1080:0:1:0:8:800:200C:417A") << (bool)true << QByteArray("1080:0:1:0:8:800:200C:417A") << 6;
    QTest::newRow("ip6_15")  << QByteArray("1080:0:1:0:8:800:200C:0") << (bool)true << QByteArray("1080:0:1:0:8:800:200C:0") << 6;
    QTest::newRow("ip6_16")  << QByteArray("1080:0:1:0:8:800:0:0") << (bool)true << QByteArray("1080:0:1:0:8:800::") << 6;
    QTest::newRow("ip6_17")  << QByteArray("1080:0:0:0:8:800:0:0") << (bool)true << QByteArray("1080::8:800:0:0") << 6;
    QTest::newRow("ip6_18")  << QByteArray("0:1:1:1:8:800:0:0") << (bool)true << QByteArray("0:1:1:1:8:800::") << 6;
    QTest::newRow("ip6_19")  << QByteArray("0:1:1:1:8:800:0:1") << (bool)true << QByteArray("0:1:1:1:8:800:0:1") << 6;

    QTest::newRow("error_00")  << QByteArray("foobarcom") << (bool)false << QByteArray() << 0;
    QTest::newRow("error_01")  << QByteArray("foo.bar.com") << (bool)false << QByteArray() << 0;
    QTest::newRow("error_02")  << QByteArray("") << (bool)false << QByteArray() << 0;
    QTest::newRow("error_03")  << QByteArray() << (bool)false << QByteArray() << 0;
    QTest::newRow("error_04")  << QByteArray(" \t\r") << (bool)false << QByteArray() << 0;

    QTest::newRow("error_ip4_00")  << QByteArray("256.9.9.9") << (bool)false << QByteArray() << 0;
    QTest::newRow("error_ip4_01")  << QByteArray("-1.9.9.9") << (bool)false << QByteArray() << 0;
    QTest::newRow("error_ip4_02")  << QByteArray("123.0.0") << (bool)false << QByteArray() << 0;
    QTest::newRow("error_ip4_03")  << QByteArray("123.0.0.0.0") << (bool)false << QByteArray() << 0;
    QTest::newRow("error_ip4_04")  << QByteArray("255.2 3.2.1") << (bool)false << QByteArray() << 0;

    QTest::newRow("error_ip6_00")  << QByteArray(":") << (bool)false << QByteArray() << 0;
    QTest::newRow("error_ip6_01")  << QByteArray(":::") << (bool)false << QByteArray() << 0;
    QTest::newRow("error_ip6_02")  << QByteArray("::AAAA:") << (bool)false << QByteArray() << 0;
    QTest::newRow("error_ip6_03")  << QByteArray(":AAAA::") << (bool)false << QByteArray() << 0;
    QTest::newRow("error_ip6_04")  << QByteArray("FFFF:::129.144.52.38") << (bool)false << QByteArray() << 0;
    QTest::newRow("error_ip6_05")  << QByteArray("FEDC:BA98:7654:3210:FEDC:BA98:7654:3210:1234") << (bool)false << QByteArray() << 0;
    QTest::newRow("error_ip6_06")  << QByteArray("129.144.52.38::") << (bool)false << QByteArray() << 0;
    QTest::newRow("error_ip6_07")  << QByteArray("::129.144.52.38:129.144.52.38") << (bool)false << QByteArray() << 0;
    QTest::newRow("error_ip6_08")  << QByteArray(":::129.144.52.38") << (bool)false << QByteArray() << 0;
    QTest::newRow("error_ip6_09")  << QByteArray("1FEDC:BA98:7654:3210:FEDC:BA98:7654:3210") << (bool)false << QByteArray() << 0;
    QTest::newRow("error_ip6_10")  << QByteArray("::FFFFFFFF") << (bool)false << QByteArray() << 0;
    QTest::newRow("error_ip6_11")  << QByteArray("::EFGH") << (bool)false << QByteArray() << 0;
    QTest::newRow("error_ip6_12")  << QByteArray("ABCD:ABCD:ABCD") << (bool)false << QByteArray() << 0;
    QTest::newRow("error_ip6_13")  << QByteArray("::ABCD:ABCD::") << (bool)false << QByteArray() << 0;
    QTest::newRow("error_ip6_14")  << QByteArray("1::2::3") << (bool)false << QByteArray() << 0;
    QTest::newRow("error_ip6_15")  << QByteArray("1:2:::") << (bool)false << QByteArray() << 0;
    QTest::newRow("error_ip6_16")  << QByteArray(":::1:2") << (bool)false << QByteArray() << 0;
    QTest::newRow("error_ip6_17")  << QByteArray("1:::2") << (bool)false << QByteArray() << 0;
    QTest::newRow("error_ip6_18")  << QByteArray("FEDC::7654:3210:FEDC:BA98::3210") << (bool)false << QByteArray() << 0;
    QTest::newRow("error_ip6_19")  << QByteArray("ABCD:ABCD:ABCD:1.2.3.4") << (bool)false << QByteArray() << 0;
    QTest::newRow("error_ip6_20")  << QByteArray("ABCD::ABCD::ABCD:1.2.3.4") << (bool)false << QByteArray() << 0;

}

void tst_QHostAddress::setAddress_QByteArray()
{
    QFETCH(QByteArray, address);
    QFETCH(bool, ok);
    QFETCH(QByteArray, resAddr);
    QFETCH(int, protocol);

    QHostAddress hostAddr;
    QVERIFY(hostAddr.setAddress(address) == ok);

    if (ok)
        QTEST(hostAddr.toString(), resAddr);

    if ( protocol == 4 ) {
        QVERIFY( hostAddr.protocol() == QAbstractSocket::IPv4Protocol || hostAddr.protocol() == QAbstractSocket::UnknownNetworkLayerProtocol );
        QVERIFY( hostAddr.protocol() != QAbstractSocket::IPv6Protocol );
    } else if ( protocol == 6 ) {
        QVERIFY( hostAddr.protocol() != QAbstractSocket::IPv4Protocol && hostAddr.protocol() != QAbstractSocket::UnknownNetworkLayerProtocol );
        QVERIFY( hostAddr.protocol() == QAbstractSocket::IPv6Protocol );
    } else {
        QVERIFY( hostAddr.isNull() );
        QVERIFY( hostAddr.protocol() == QAbstractSocket::UnknownNetworkLayerProtocol );
    }
}

void tst_QHostAddress::specialAddresses_data()
{
    QTest::addColumn<QByteArray>("text");
    QTest::addColumn<int>("address");
    QTest::addColumn<bool>("result");

    QTest::newRow("localhost_1") << QByteArray("127.0.0.1") << (int)QHostAddress::LocalHost << true;
    QTest::newRow("localhost_2") << QByteArray("127.0.0.2") << (int)QHostAddress::LocalHost << false;
    QTest::newRow("localhost_3") << QByteArray("127.0.0.2") << (int)QHostAddress::LocalHostIPv6 << false;

    QTest::newRow("localhost_ipv6_4") << QByteArray("::1") << (int)QHostAddress::LocalHostIPv6 << true;
    QTest::newRow("localhost_ipv6_5") << QByteArray("::2") << (int)QHostAddress::LocalHostIPv6 << false;
    QTest::newRow("localhost_ipv6_6") << QByteArray("::1") << (int)QHostAddress::LocalHost << false;

    QTest::newRow("null_1") << QByteArray("") << (int)QHostAddress::Null << true;
    QTest::newRow("null_2") << QByteArray("bjarne") << (int)QHostAddress::Null << true;
    
    QTest::newRow("compare_from_null") << QByteArray("") << (int)QHostAddress::Broadcast << false;

    QTest::newRow("broadcast_1") << QByteArray("255.255.255.255") << (int)QHostAddress::Any << false;
    QTest::newRow("broadcast_2") << QByteArray("255.255.255.255") << (int)QHostAddress::Broadcast << true;

    QTest::newRow("any_ipv6") << QByteArray("::") << (int)QHostAddress::AnyIPv6 << true;
    QTest::newRow("any_ipv4") << QByteArray("0.0.0.0") << (int)QHostAddress::Any << true;
}


void tst_QHostAddress::specialAddresses()
{
    QFETCH(QByteArray, text);
    QFETCH(int, address);
    QFETCH(bool, result);
    QHostAddress specialaddress = QHostAddress((QHostAddress::SpecialAddress)address);
    QVERIFY((QHostAddress(text) == specialaddress) == result);

    QHostAddress setter;
    setter.setAddress(text);
    if (result) {
        QVERIFY(setter == specialaddress);
    } else {
        QVERIFY(!(specialaddress == setter));
    }
}


void tst_QHostAddress::compare_data()
{
    QTest::addColumn<QHostAddress>("first");
    QTest::addColumn<QHostAddress>("second");
    QTest::addColumn<bool>("result");

    QTest::newRow("1") << QHostAddress() << QHostAddress() << true;
    QTest::newRow("2") << QHostAddress(QHostAddress::Any) << QHostAddress(QHostAddress::Any) << true;
    QTest::newRow("3") << QHostAddress(QHostAddress::AnyIPv6) << QHostAddress(QHostAddress::AnyIPv6) << true;
    QTest::newRow("4") << QHostAddress(QHostAddress::Broadcast) << QHostAddress(QHostAddress::Broadcast) << true;
    QTest::newRow("5") << QHostAddress(QHostAddress::LocalHost) << QHostAddress(QHostAddress::Broadcast) << false;
    QTest::newRow("6") << QHostAddress(QHostAddress::LocalHost) << QHostAddress(QHostAddress::LocalHostIPv6) << false;
    QTest::newRow("7") << QHostAddress() << QHostAddress(QHostAddress::LocalHostIPv6) << false;
}

void tst_QHostAddress::compare()
{
    QFETCH(QHostAddress, first);
    QFETCH(QHostAddress, second);
    QFETCH(bool, result);

    QCOMPARE(first == second, result);
}

void tst_QHostAddress::assignment()
{
    QHostAddress address;
    address = "127.0.0.1";
    QCOMPARE(address, QHostAddress("127.0.0.1"));

    address = "::1";
    QCOMPARE(address, QHostAddress("::1"));

    QHostAddress addr("4.2.2.1");
    const QByteArray addStr = addr.toString();
    sockaddr_in sockAddr;
    sockAddr.sin_family = AF_INET;
    struct in_addr ia;
    inet_pton(AF_INET, addStr.constData(), &ia);
    sockAddr.sin_addr = ia;
    address.setAddress((sockaddr *)&sockAddr);
    QCOMPARE(address, addr);
}

void tst_QHostAddress::scopeId()
{
    QHostAddress address("fe80::2e0:4cff:fefb:662a%eth0");
    QCOMPARE(address.scopeId(), QByteArray("eth0"));
    QCOMPARE(address.toString().toLower(), QByteArray("fe80::2e0:4cff:fefb:662a%eth0"));

    QHostAddress address2("fe80::2e0:4cff:fefb:662a");
    QCOMPARE(address2.scopeId(), QByteArray());
    address2.setScopeId(QByteArray("en0"));
    QCOMPARE(address2.toString().toLower(), QByteArray("fe80::2e0:4cff:fefb:662a%en0"));

    address2 = address;
    QCOMPARE(address2.scopeId(), QByteArray("eth0"));
    QCOMPARE(address2.toString().toLower(), QByteArray("fe80::2e0:4cff:fefb:662a%eth0"));
}

void tst_QHostAddress::hashKey()
{
    QHash<QHostAddress, QString> hostHash;
    hostHash.insert(QHostAddress(), "ole");
}

void tst_QHostAddress::streaming_data()
{
    QTest::addColumn<QHostAddress>("address");
    QTest::newRow("1") << QHostAddress();
    QTest::newRow("2") << QHostAddress("127.128.129.130");
    QTest::newRow("3") << QHostAddress("1080:0000:0000:0000:0008:0800:200C:417A");
    QTest::newRow("4") << QHostAddress("fe80::2e0:4cff:fefb:662a%eth0");
    QTest::newRow("5") << QHostAddress(QHostAddress::Null);
    QTest::newRow("6") << QHostAddress(QHostAddress::LocalHost);
    QTest::newRow("7") << QHostAddress(QHostAddress::LocalHostIPv6);
    QTest::newRow("8") << QHostAddress(QHostAddress::Broadcast);
    QTest::newRow("9") << QHostAddress(QHostAddress::Any);
    QTest::newRow("10") << QHostAddress(QHostAddress::AnyIPv6);
    QTest::newRow("11") << QHostAddress("foo.bar.com");
}

void tst_QHostAddress::streaming()
{
    QFETCH(QHostAddress, address);
    QByteArray ba;
    QDataStream ds1(&ba, QIODevice::WriteOnly);
    ds1 << address;
    QVERIFY(ds1.status() == QDataStream::Ok);
    QDataStream ds2(&ba, QIODevice::ReadOnly);
    QHostAddress address2;
    ds2 >> address2;
    QVERIFY(ds2.status() == QDataStream::Ok);
    QCOMPARE(address, address2);
}

QTEST_MAIN(tst_QHostAddress)

#include "moc_tst_qhostaddress.cpp"
