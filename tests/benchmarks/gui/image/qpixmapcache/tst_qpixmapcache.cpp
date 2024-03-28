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

#include <qtest.h>
#include <QPixmapCache>
#include <qguicommon_p.h>
//TESTED_FILES=

class tst_QPixmapCache : public QObject
{
    Q_OBJECT

public:
    tst_QPixmapCache();
    virtual ~tst_QPixmapCache();

public slots:
    void init();
    void cleanup();

private slots:
    void insert_data();
    void insert();
    void find_data();
    void find();
    void styleUseCaseComplexKey();
    void styleUseCaseComplexKey_data();
};

tst_QPixmapCache::tst_QPixmapCache()
{
}

tst_QPixmapCache::~tst_QPixmapCache()
{
}

void tst_QPixmapCache::init()
{
}

void tst_QPixmapCache::cleanup()
{
}

void tst_QPixmapCache::insert_data()
{
    QTest::addColumn<bool>("cacheType");
    QTest::newRow("QPixmapCache") << true;
    QTest::newRow("QPixmapCache (int API)") << false;
}

QList<QByteArray> keys;

void tst_QPixmapCache::insert()
{
    QFETCH(bool, cacheType);
    QPixmap p;
    if (cacheType) {
        QBENCHMARK {
            for (int i = 0 ; i <= 10000 ; i++)
            {
                QByteArray tmp("my-key-");
                tmp += QByteArray::number(i);
                QPixmapCache::insert(tmp, p);
            }
        }
    } else {
        QBENCHMARK {
            for (int i = 0 ; i <= 10000 ; i++)
                keys.append(QPixmapCache::insert(p));
        }
    }
}

void tst_QPixmapCache::find_data()
{
    QTest::addColumn<bool>("cacheType");
    QTest::newRow("QPixmapCache") << true;
    QTest::newRow("QPixmapCache (int API)") << false;
}

void tst_QPixmapCache::find()
{
    QFETCH(bool, cacheType);
    QPixmap p;
    if (cacheType) {
        QBENCHMARK {
            for (int i = 0 ; i <= 10000 ; i++)
            {
                QByteArray tmp("my-key-");
                tmp += QByteArray::number(i);
                QPixmapCache::find(tmp, p);
            }
        }
    } else {
        QBENCHMARK {
            for (int i = 0 ; i <= 10000 ; i++)
                QPixmapCache::find(keys.at(i), &p);
        }
    }

}

void tst_QPixmapCache::styleUseCaseComplexKey_data()
{
    QTest::addColumn<bool>("cacheType");
    QTest::newRow("QPixmapCache") << true;
    QTest::newRow("QPixmapCache (int API)") << false;
}

struct styleStruct {
    QByteArray key;
    uint state;
    uint direction;
    uint complex;
    uint palette;
    int width;
    int height;
    bool operator==(const styleStruct &str) const
    {
        return  str.state == state && str.direction == direction
                && str.complex == complex && str.palette == palette && str.width == width
                && str.height == height && str.key == key;
    }
};

uint qHash(const styleStruct &myStruct)
{
    return qHash(myStruct.state);
}

void tst_QPixmapCache::styleUseCaseComplexKey()
{
    QFETCH(bool, cacheType);
    QPixmap p;
    if (cacheType) {
        QBENCHMARK {
            for (int i = 0 ; i <= 10000 ; i++)
            {
                QByteArray tmp = qHexString("my-progressbar-%d-%d-%d-%d-%d-%d-%d", i, 5, 3, 0, 358, 100, 200);
                QPixmapCache::insert(tmp, p);
            }

            for (int i = 0 ; i <= 10000 ; i++)
            {
                QByteArray tmp = qHexString("my-progressbar-%d-%d-%d-%d-%d-%d-%d", i, 5, 3, 0, 358, 100, 200);
                QPixmapCache::find(tmp, p);
            }
        }
    } else {
        QHash<styleStruct, QByteArray> hash;
        QBENCHMARK {
            for (int i = 0 ; i <= 10000 ; i++)
            {
                styleStruct myStruct;
                myStruct.key = QByteArray("my-progressbar-") + QByteArray::number(i);
                myStruct.state = 5;
                myStruct.direction = 4;
                myStruct.complex = 3;
                myStruct.palette = 358;
                myStruct.width = 100;
                QByteArray key = QPixmapCache::insert(p);
                hash.insert(myStruct, key);
            }
            for (int i = 0 ; i <= 10000 ; i++)
            {
                styleStruct myStruct;
                myStruct.key = QByteArray("my-progressbar-") + QByteArray::number(i);
                myStruct.state = 5;
                myStruct.direction = 4;
                myStruct.complex = 3;
                myStruct.palette = 358;
                myStruct.width = 100;
                QByteArray key = hash.value(myStruct);
                QPixmapCache::find(key, &p);
            }
        }
    }

}


QTEST_MAIN(tst_QPixmapCache)

#include "moc_tst_qpixmapcache.cpp"
