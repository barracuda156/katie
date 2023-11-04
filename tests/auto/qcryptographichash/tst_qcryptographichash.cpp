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
#include <QCryptographicHash>
#include <QDebug>

class tst_QCryptographicHash : public QObject
{
    Q_OBJECT
private slots:
    void repeated_result_data();
    void repeated_result();
    void intermediary_result_data();
    void intermediary_result();
    void static_vs_incremental_result_data();
    void static_vs_incremental_result();
    void collision_data();
    void collision();
};

void tst_QCryptographicHash::repeated_result_data()
{
    intermediary_result_data();
}

void tst_QCryptographicHash::repeated_result()
{
    QCryptographicHash hash;

    QFETCH(QByteArray, first);
    hash.addData(first);

    QFETCH(QByteArray, hash_first);
    QByteArray result = hash.result();
    QCOMPARE(result, hash_first);
    QCOMPARE(result, hash.result());

    hash.reset();
    hash.addData(first);
    result = hash.result();
    QCOMPARE(result, hash_first);
    QCOMPARE(result, hash.result());
}

void tst_QCryptographicHash::intermediary_result_data()
{
    QTest::addColumn<QByteArray>("first");
    QTest::addColumn<QByteArray>("second");
    QTest::addColumn<QByteArray>("hash_first");
    QTest::addColumn<QByteArray>("hash_firstsecond");

    QTest::newRow("abc") << QByteArray("abc") << QByteArray("abc")
                         << QByteArray::fromHex("A96FAF705AF16834E6C632B61E964E1F8C19F5792EBF862E22775E3BD96F68BF")
                         << QByteArray::fromHex("B9FE94D346D39B20369242A646A19333757D229E18328270F7B354793BCE8D36");
}

void tst_QCryptographicHash::intermediary_result()
{
    QCryptographicHash hash;

    QFETCH(QByteArray, first);
    hash.addData(first);

    QFETCH(QByteArray, hash_first);
    QByteArray result = hash.result();
    // qDebug() << result.toHex().toUpper();
    QCOMPARE(result, hash_first);

    // don't reset
    QFETCH(QByteArray, second);
    QFETCH(QByteArray, hash_firstsecond);
    hash.addData(second);

    result = hash.result();
    // qDebug() << result.toHex().toUpper();
    QCOMPARE(result, hash_firstsecond);
}

void tst_QCryptographicHash::static_vs_incremental_result_data()
{
    QTest::addColumn<QByteArray>("first");
    QTest::addColumn<QByteArray>("second");
    QTest::addColumn<QByteArray>("third");

    QTest::newRow("123")          << QByteArray("1") << QByteArray("2") << QByteArray("3");
    QTest::newRow("1bar3")        << QByteArray("1") << QByteArray("bar") << QByteArray("3");
    QTest::newRow("12baz")        << QByteArray("1") << QByteArray("2") << QByteArray("baz");
    QTest::newRow("foobarbaz")    << QByteArray("foo") << QByteArray("bar") << QByteArray("baz");
}

void tst_QCryptographicHash::static_vs_incremental_result()
{
    QFETCH(QByteArray, first);
    QFETCH(QByteArray, second);
    QFETCH(QByteArray, third);

    QCryptographicHash incrementalhash;
    incrementalhash.addData(first);
    incrementalhash.addData(second);
    incrementalhash.addData(third);
    const QByteArray incrementalresult = incrementalhash.result();

    const QByteArray staticresult = QCryptographicHash::hash(first + second + third);

    // qDebug() << incrementalresult.toHex() << staticresult.toHex();
    QEXPECT_FAIL("", "This is expected behaviour currently", Continue);
    QCOMPARE(incrementalresult, staticresult);
}

void tst_QCryptographicHash::collision_data()
{
    QTest::addColumn<QByteArray>("data");
    QTest::addColumn<QByteArray>("data2");
    QTest::addColumn<bool>("incremental");

    QTest::newRow("1vs2")        << QByteArray("1") << QByteArray("2") << false;
    QTest::newRow("1vs2")        << QByteArray("1") << QByteArray("2") << true;
    QTest::newRow("122vs222")    << QByteArray("122") << QByteArray("222") << true;
    QTest::newRow("122vs222")    << QByteArray("122") << QByteArray("222") << false;
    QTest::newRow("221vs222")    << QByteArray("221") << QByteArray("222") << true;
    QTest::newRow("221vs222")    << QByteArray("221") << QByteArray("222") << false;
}

void tst_QCryptographicHash::collision()
{
    QFETCH(QByteArray, data);
    QFETCH(QByteArray, data2);
    QFETCH(bool, incremental);

    if (incremental) {
        QCryptographicHash incrementalhash;
        incrementalhash.addData(data);
        const QByteArray incrementalresult = incrementalhash.result();

        QCryptographicHash incrementalhash2;
        incrementalhash2.addData(data2);
        const QByteArray incrementalresult2 = incrementalhash2.result();

        QVERIFY(incrementalresult != incrementalresult2);
    } else {
        const QByteArray staticresult = QCryptographicHash::hash(data);

        const QByteArray staticresult2 = QCryptographicHash::hash(data2);

        QVERIFY(staticresult != staticresult2);
    }
}

QTEST_MAIN(tst_QCryptographicHash)

#include "moc_tst_qcryptographichash.cpp"
