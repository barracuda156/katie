/****************************************************************************
**
** Copyright (C) 2015 The Qt Company Ltd.
** Contact: http://www.qt.io/licensing/
**
** This file is part of the QML Shaders plugin of the Qt Toolkit.
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

#include <QApplication>
#include <QtTest/qtest.h>
#include <QtDeclarative>
#include "../../../../src/imports/shaders/shadereffectitem.h"
#include "../../../../src/imports/shaders/shadereffectsource.h"
//#include "../../../src/shadereffect.h"

class BenchmarkItem : public QDeclarativeItem
{
    Q_OBJECT

public:
    BenchmarkItem( QDeclarativeItem * parent = 0 ) : QDeclarativeItem(parent)
        , m_frameCount(0)
    {
        setFlag(QGraphicsItem::ItemHasNoContents, false);
    }

    void paint (QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget) {
        QDeclarativeItem::paint(painter, option, widget);
        if (timer.restart() > 7) m_frameCount++;
    }

    int frameCount() { return m_frameCount; }

private:
    int m_frameCount;
    QTime timer;
};

int main(int argc, char *argv[])
{
    QApplication app(argc, argv);

    QDeclarativeView view;
    view.setViewportUpdateMode(QGraphicsView::FullViewportUpdate);
    view.setAttribute(Qt::WA_OpaquePaintEvent);
    view.setAttribute(Qt::WA_NoSystemBackground);
    view.setResizeMode(QDeclarativeView::SizeViewToRootObject);

    qmlRegisterType<ShaderEffectItem>("Qt.labs.shaders", 1, 0, "ShaderEffectItem");
    qmlRegisterType<ShaderEffectSource>("Qt.labs.shaders", 1, 0, "ShaderEffectSource");

    QGLFormat format = QGLFormat::defaultFormat();
    format.setSampleBuffers(false);
    format.setSwapInterval(1);

    QGLWidget* glWidget = new QGLWidget(format);
    glWidget->setAutoFillBackground(false);
    view.setViewport(glWidget);
    view.show();

    view.setSource(QUrl::fromLocalFile("TestWater.qml"));
    BenchmarkItem *benchmarkItem;

    qDebug() << "Sea Water benchmark:";
    benchmarkItem = new BenchmarkItem(dynamic_cast<QDeclarativeItem *>(view.rootObject()));
    QTest::qWait(5000);
    qDebug() << "Rendered " << benchmarkItem->frameCount() << " frames in 5 seconds";
    qDebug() << "Average " << benchmarkItem->frameCount() / 5.0 << " frames per second";

    qDebug() << "Gaussian drop shadow benchmark:";
    view.setSource(QUrl::fromLocalFile("TestGaussianDropShadow.qml"));
    benchmarkItem = new BenchmarkItem(dynamic_cast<QDeclarativeItem *>(view.rootObject()));
    QTest::qWait(5000);
    qDebug() << "Rendered " << benchmarkItem->frameCount() << " frames in 5 seconds";
    qDebug() << "Average " << benchmarkItem->frameCount() / 5.0 << " frames per second";
}

#include "moc_tst_performance.cpp"
