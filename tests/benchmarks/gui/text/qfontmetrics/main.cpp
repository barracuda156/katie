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

#include <QObject>
#include <QFont>
#include <QFontMetrics>

#include <qtest.h>

//this test benchmarks the once-off (per font configuration) cost
//associated with using QFontMetrics
class tst_QFontMetrics : public QObject
{
    Q_OBJECT
public:
    tst_QFontMetrics() {}
private slots:
    void fontmetrics_create();
    void fontmetrics_create_once_loaded();

    void fontmetrics_height();
    void fontmetrics_height_once_loaded();

private:
    void testQFontMetrics(const QFontMetrics &fm);
};

void tst_QFontMetrics::testQFontMetrics( const QFontMetrics &fm )
{
    QBENCHMARK {
      int fontHeight = fm.height();
      Q_UNUSED(fontHeight);
    }
}

void tst_QFontMetrics::fontmetrics_create()
{
    QBENCHMARK {
      QFont boldfont = QApplication::font();
      boldfont.setBold( true );
      boldfont.setPointSize(boldfont.pointSize() * 1.5 );
      QFontMetrics bfm( boldfont );
    }
}

void tst_QFontMetrics::fontmetrics_create_once_loaded()
{
    QBENCHMARK {
      QFont boldfont = QApplication::font();
      boldfont.setBold( true );
      boldfont.setPointSize(boldfont.pointSize() * 1.5 );
      QFontMetrics bfm( boldfont );
    }
}

void tst_QFontMetrics::fontmetrics_height()
{
    QFont boldfont = QApplication::font();
    boldfont.setBold( true );
    boldfont.setPointSize(boldfont.pointSize() * 1.5 );
    QFontMetrics bfm( boldfont );

    QBENCHMARK { testQFontMetrics(bfm); }
}

void tst_QFontMetrics::fontmetrics_height_once_loaded()
{
    QFont boldfont = QApplication::font();
    boldfont.setBold( true );
    boldfont.setPointSize(boldfont.pointSize() * 1.5 );
    QFontMetrics bfm( boldfont );
    QBENCHMARK { testQFontMetrics(bfm); }
}

QTEST_MAIN(tst_QFontMetrics)

#include "moc_main.cpp"
