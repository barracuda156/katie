/****************************************************************************
**
** Copyright (C) 2015 The Qt Company Ltd.
** Copyright (C) 2016-2019 Ivailo Monev
**
** This file is part of the QtTest module of the Katie Toolkit.
**
** $QT_BEGIN_LICENSE:LGPL$
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

#ifndef QTESTTABLE_P_H
#define QTESTTABLE_P_H

//
//  W A R N I N G
//  -------------
//
// This file is not part of the Katie API.  It exists purely as an
// implementation detail.  This header file may change from version to
// version without notice, or even be removed.
//
// We mean it.
//

#include <QtTest/qtest_global.h>

QT_BEGIN_NAMESPACE

class QTestData;
class QTestTablePrivate;

class Q_TEST_EXPORT QTestTable
{
public:
    QTestTable();
    ~QTestTable();

    void addColumn(int elementType, const char *elementName);
    QTestData *newData(const char *tag);

    int elementCount() const;
    int dataCount() const;

    int elementTypeId(int index) const;
    const char *dataTag(int index) const;
    int indexOf(const char *elementName) const;
    bool isEmpty() const;
    QTestData *testData(int index) const;

    static QTestTable *globalTestTable();
    static QTestTable *currentTestTable();
    static void clearGlobalTestTable();

private:
    Q_DISABLE_COPY(QTestTable)

    QTestTablePrivate *d;
};

QT_END_NAMESPACE

#endif
