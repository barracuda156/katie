/****************************************************************************
**
** Copyright (C) 2015 The Qt Company Ltd.
** Copyright (C) 2016-2019 Ivailo Monev
**
** This file is part of the QtGui module of the Katie Toolkit.
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

#ifndef QGRAPHICSLAYOUT_H
#define QGRAPHICSLAYOUT_H

#include <QtGui/qgraphicslayoutitem.h>

QT_BEGIN_HEADER

#ifndef QT_NO_GRAPHICSVIEW

QT_BEGIN_NAMESPACE

class QGraphicsLayoutPrivate;
class QGraphicsLayoutItem;
class QGraphicsWidget;

class Q_GUI_EXPORT QGraphicsLayout : public QGraphicsLayoutItem
{
public:
    QGraphicsLayout(QGraphicsLayoutItem *parent = 0);
    ~QGraphicsLayout();

    void setContentsMargins(qreal left, qreal top, qreal right, qreal bottom);
    void getContentsMargins(qreal *left, qreal *top, qreal *right, qreal *bottom) const;

    void activate();
    bool isActivated() const;
    virtual void invalidate();
    virtual void updateGeometry();

    virtual void widgetEvent(QEvent *e);

    virtual int count() const = 0;
    virtual QGraphicsLayoutItem *itemAt(int i) const = 0;
    virtual void removeAt(int index) = 0;

    static void setInstantInvalidatePropagation(bool enable);
    static bool instantInvalidatePropagation();
protected:
    QGraphicsLayout(QGraphicsLayoutPrivate &, QGraphicsLayoutItem *);
    void addChildLayoutItem(QGraphicsLayoutItem *layoutItem);

private:
    Q_DISABLE_COPY(QGraphicsLayout)
    Q_DECLARE_PRIVATE(QGraphicsLayout)
    friend class QGraphicsWidget;
};

QT_END_NAMESPACE

Q_DECLARE_INTERFACE(QGraphicsLayout, "com.trolltech.Qt.QGraphicsLayout")

#endif

QT_END_HEADER

#endif

