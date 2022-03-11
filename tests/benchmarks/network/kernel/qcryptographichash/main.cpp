/****************************************************************************
**
** Copyright (C) 2019 Ivailo Monev
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

#include <qdebug.h>
#include <qcryptographichash.h>
#include <qtest.h>

QT_USE_NAMESPACE

static const QByteArray lorem = QByteArray("Lorem ipsum dolor sit amet, consectetuer adipiscing elit, sed diam nonummy nibh euismod tincidunt ut laoreet dolore magna aliquam erat volutpat. Ut wisi enim ad minim veniam, quis nostrud exerci tation ullamcorper suscipit lobortis nisl ut aliquip ex ea commodo consequat. Duis autem vel eum iriure dolor in hendrerit in vulputate velit esse molestie consequat, vel illum dolore eu feugiat nulla facilisis at vero eros et accumsan et iusto odio dignissim qui blandit praesent luptatum zzril delenit augue duis dolore te feugait nulla facilisi.");

Q_DECLARE_METATYPE(QCryptographicHash::Algorithm)

static QString algorithmToString(const QCryptographicHash::Algorithm algorithm)
{
    switch (algorithm) {
        case QCryptographicHash::Md5: {
            return QString::fromLatin1("Md5");
        }
        case QCryptographicHash::Sha1: {
            return QString::fromLatin1("Sha1");
        }
        case QCryptographicHash::Sha256: {
            return QString::fromLatin1("Sha256");
        }
        case QCryptographicHash::Sha512: {
            return QString::fromLatin1("Sha512");
        }
        case QCryptographicHash::BLAKE2b: {
            return QString::fromLatin1("BLAKE2b");
        }
        case QCryptographicHash::BLAKE2s: {
            return QString::fromLatin1("BLAKE2s");
        }
    }
    Q_ASSERT(false);
    return QString();
}

class tst_qcryptographichash : public QObject
{
    Q_OBJECT
private slots:
    void append_data();
    void append();
    void append_once_data();
    void append_once();
    void statichash_data();
    void statichash();
    void algorithms_data();
    void algorithms();
};

void tst_qcryptographichash::append_data()
{
    QTest::addColumn<int>("size");
    QTest::addColumn<QCryptographicHash::Algorithm>("algorithm");
    for (int i = QCryptographicHash::Md5; i < (QCryptographicHash::BLAKE2s + 1); i++) {
        const QCryptographicHash::Algorithm algorithm = static_cast<QCryptographicHash::Algorithm>(i);
        QTest::newRow(QString::fromLatin1("10 (%1)").arg(algorithmToString(algorithm)).toAscii())  << int(10) << algorithm;
        QTest::newRow(QString::fromLatin1("100 (%1)").arg(algorithmToString(algorithm)).toAscii()) << int(100) << algorithm;
        QTest::newRow(QString::fromLatin1("250 (%1)").arg(algorithmToString(algorithm)).toAscii()) << int(250) << algorithm;
        QTest::newRow(QString::fromLatin1("500 (%1)").arg(algorithmToString(algorithm)).toAscii()) << int(500) << algorithm;
    }
}

void tst_qcryptographichash::append()
{
    QFETCH(int, size);
    QFETCH(QCryptographicHash::Algorithm, algorithm);

    const int chunksize = lorem.size() / size;
    QVERIFY(chunksize > 0);
    QList<QByteArray> chunks;
    int chunkposition = 0;
    for (int i = 0; i < chunksize; i++) {
        chunks.append(lorem.mid(chunkposition, chunksize));
        chunkposition += chunksize;
    }

    QBENCHMARK {
        QCryptographicHash hash(algorithm);
        for (int i = 0; i < chunks.size(); i++) {
            hash.addData(chunks.at(i));
        }
        QVERIFY(!hash.result().isEmpty());
    }
}

void tst_qcryptographichash::append_once_data()
{
    QTest::addColumn<QCryptographicHash::Algorithm>("algorithm");
    QTest::newRow("Md5")    << QCryptographicHash::Md5;
    QTest::newRow("Sha1")   << QCryptographicHash::Sha1;
    QTest::newRow("Sha256") << QCryptographicHash::Sha256;
    QTest::newRow("Sha512") << QCryptographicHash::Sha512;
    QTest::newRow("BLAKE2b") << QCryptographicHash::BLAKE2b;
    QTest::newRow("BLAKE2s") << QCryptographicHash::BLAKE2s;
}

void tst_qcryptographichash::append_once()
{
    QFETCH(QCryptographicHash::Algorithm, algorithm);

    QBENCHMARK {
        QCryptographicHash hash(algorithm);
        hash.addData(lorem);
        QVERIFY(!hash.result().isEmpty());
    }
}

void tst_qcryptographichash::statichash_data()
{
    append_once_data();
}

void tst_qcryptographichash::statichash()
{
    QFETCH(QCryptographicHash::Algorithm, algorithm);

    QBENCHMARK {
        QByteArray hash = QCryptographicHash::hash(lorem, algorithm);
        QVERIFY(!hash.isEmpty());
    }
}

void tst_qcryptographichash::algorithms_data()
{
    statichash_data();
}

void tst_qcryptographichash::algorithms()
{
    QFETCH(QCryptographicHash::Algorithm, algorithm);

    QBENCHMARK {
        QByteArray hash = QCryptographicHash::hash(lorem, algorithm);
        QVERIFY(!hash.isEmpty());
    }
}

QTEST_MAIN(tst_qcryptographichash)

#include "moc_main.cpp"
