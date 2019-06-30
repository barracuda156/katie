/****************************************************************************
**
** Copyright (C) 2015 The Qt Company Ltd.
** Copyright (C) 2016-2019 Ivailo Monev
**
** This file is part of the test suite of the Katie Toolkit.
**
** $QT_BEGIN_LICENSE:LGPL$
** Commercial License Usage
** Licensees holding valid commercial Qt licenses may use this file in
** accordance with the commercial license agreement provided with the
** Software or, alternatively, in accordance with the terms contained in
** a written agreement between you and The Qt Company. For licensing terms
** and conditions see http://www.qt.io/terms-conditions. For further
** information use the contact form at http://www.qt.io/contact-us.
**
** GNU Lesser General Public License Usage
** Alternatively, this file may be used under the terms of the GNU Lesser
** General Public License version 2.1 or version 3 as published by the Free
** Software Foundation and appearing in the file LICENSE.LGPLv21 and
** LICENSE.LGPLv3 included in the packaging of this file. Please review the
** following information to ensure the GNU Lesser General Public License
** requirements will be met: https://www.gnu.org/licenses/lgpl.html and
** http://www.gnu.org/licenses/old-licenses/lgpl-2.1.html.
**
** As a special exception, The Qt Company gives you certain additional
** rights. These rights are described in The Qt Company LGPL Exception
** version 1.1, included in the file LGPL_EXCEPTION.txt in this package.
**
** GNU General Public License Usage
** Alternatively, this file may be used under the terms of the GNU
** General Public License version 3.0 as published by the Free Software
** Foundation and appearing in the file LICENSE.GPL included in the
** packaging of this file.  Please review the following information to
** ensure the GNU General Public License version 3.0 requirements will be
** met: http://www.gnu.org/copyleft/gpl.html.
**
** $QT_END_LICENSE$
**
****************************************************************************/

#include <qdebug.h>
#include <qcryptographichash.h>
#include <qtest.h>

QT_USE_NAMESPACE

static const QByteArray lorem = QByteArray("Lorem ipsum dolor sit amet, consectetuer adipiscing elit, sed diam nonummy nibh euismod tincidunt ut laoreet dolore magna aliquam erat volutpat. Ut wisi enim ad minim veniam, quis nostrud exerci tation ullamcorper suscipit lobortis nisl ut aliquip ex ea commodo consequat. Duis autem vel eum iriure dolor in hendrerit in vulputate velit esse molestie consequat, vel illum dolore eu feugiat nulla facilisis at vero eros et accumsan et iusto odio dignissim qui blandit praesent luptatum zzril delenit augue duis dolore te feugait nulla facilisi.");

class tst_qcryptographichash : public QObject
{
    Q_OBJECT
private slots:
    void append_data();
    void append();
    void append_once();
    void hash();
};

void tst_qcryptographichash::append_data()
{
    QTest::addColumn<int>("size");
    QTest::newRow("1")         << int(1);
    QTest::newRow("10")        << int(10);
    QTest::newRow("100")       << int(100);
}

void tst_qcryptographichash::append()
{
    QFETCH(int, size);

    const int chunksize = lorem.size() / size;
    QVERIFY(chunksize > 0);
    QList<QByteArray> chunks;
    int chunkposition = 0;
    for (int i = 0; i < chunksize; i++) {
        chunks.append(lorem.mid(chunkposition, chunksize));
        chunkposition += chunksize;
    }

    QBENCHMARK {
        QCryptographicHash hash(QCryptographicHash::Sha512);
        for (int i = 0; i < chunks.size(); i++) {
            hash.addData(chunks.at(i));
        }
        QVERIFY(!hash.result().isEmpty());
    }
}

void tst_qcryptographichash::append_once()
{
    QBENCHMARK {
        QCryptographicHash hash(QCryptographicHash::Sha512);
        hash.addData(lorem);
        QVERIFY(!hash.result().isEmpty());
    }
}

void tst_qcryptographichash::hash() {
    QBENCHMARK {
        QByteArray hash = QCryptographicHash::hash(lorem, QCryptographicHash::Sha512);
        QVERIFY(!hash.isEmpty());
    }
}

QTEST_MAIN(tst_qcryptographichash)

#include "moc_main.cpp"
