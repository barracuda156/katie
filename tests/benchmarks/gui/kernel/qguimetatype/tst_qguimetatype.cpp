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

#include <qtest.h>
#include <QtCore/qmetatype.h>

class tst_QGuiMetaType : public QObject
{
    Q_OBJECT

public:
    tst_QGuiMetaType();
    virtual ~tst_QGuiMetaType();

private slots:
    void constructGuiType_data();
    void constructGuiType();
    void constructGuiTypeCopy_data();
    void constructGuiTypeCopy();
};

tst_QGuiMetaType::tst_QGuiMetaType()
{
}

tst_QGuiMetaType::~tst_QGuiMetaType()
{
}

void tst_QGuiMetaType::constructGuiType_data()
{
    QTest::addColumn<int>("typeId");
    for (int i = QMetaType::FirstGuiType; i <= QMetaType::LastGuiType; ++i) {
        QTest::newRow(QMetaType::typeName(i)) << i;
    }
}

// Tests how fast QMetaType can default-construct and destroy a Qt GUI
// type. The purpose of this benchmark is to measure the overhead of
// using type id-based creation compared to creating the type directly
// (i.e. "T *t = new T(); delete t;").
void tst_QGuiMetaType::constructGuiType()
{
    QFETCH(int, typeId);
    QBENCHMARK {
        for (int i = 0; i < 100000; ++i) {
            void *data = QMetaType::construct(typeId, (void *)0);
            QMetaType::destroy(typeId, data);
        }
    }
}

void tst_QGuiMetaType::constructGuiTypeCopy_data()
{
    constructGuiType_data();
}

// Tests how fast QMetaType can copy-construct and destroy a Qt GUI
// type. The purpose of this benchmark is to measure the overhead of
// using type id-based creation compared to creating the type directly
// (i.e. "T *t = new T(other); delete t;").
void tst_QGuiMetaType::constructGuiTypeCopy()
{
    QFETCH(int, typeId);
    QVariant other(typeId, (void *)0);
    const void *copy = other.constData();
    QBENCHMARK {
        for (int i = 0; i < 100000; ++i) {
            void *data = QMetaType::construct(typeId, copy);
            QMetaType::destroy(typeId, data);
        }
    }
}

QTEST_MAIN(tst_QGuiMetaType)

#include "moc_tst_qguimetatype.cpp"
