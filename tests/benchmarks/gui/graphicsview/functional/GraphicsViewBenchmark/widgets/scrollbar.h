/****************************************************************************
**
** Copyright (C) 2015 The Qt Company Ltd.
** Copyright (C) 2016-2019 Ivailo Monev
**
** This file is part of the examples of the Katie Toolkit.
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

#ifndef SCROLLBAR_H
#define SCROLLBAR_H

#include <QGraphicsWidget>
#include <QPixmap>

class ScrollBarPrivate;

class ScrollBar : public QGraphicsWidget
{
    Q_OBJECT
    Q_DECLARE_PRIVATE(ScrollBar)

public:

    ScrollBar(Qt::Orientation orientation, QGraphicsWidget *parent=0);
    virtual ~ScrollBar();

public:

    bool sliderDown() const;
    qreal sliderPosition() const;
    qreal sliderSize() const;
    void setSliderSize(const qreal s);

signals:

    void sliderPressed();

    void sliderPositionChange(qreal position);

public slots:

    void setSliderPosition(qreal pos);
    void themeChange();

private:

    void paint(QPainter *painter, 
        const QStyleOptionGraphicsItem *option, 
        QWidget *widget);

    QSizeF sizeHint(Qt::SizeHint which, 
        const QSizeF &constraint = QSizeF()) const;

    void mousePressEvent(QGraphicsSceneMouseEvent *event);
    void mouseReleaseEvent(QGraphicsSceneMouseEvent *event);
    void mouseMoveEvent(QGraphicsSceneMouseEvent *event);
    void resizeEvent(QGraphicsSceneResizeEvent *event);

private:
    Q_DISABLE_COPY(ScrollBar)
    ScrollBarPrivate *d_ptr;
};

#endif // SCROLLBAR_H
