/****************************************************************************
**
** Copyright (C) 2015 The Qt Company Ltd.
** Copyright (C) 2016 Ivailo Monev
**
** This file is part of the QtGui module of the Katie Toolkit.
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

#ifndef QABSTRACTSCROLLAREA_P_H
#define QABSTRACTSCROLLAREA_P_H

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

#include "qframe_p.h"
#include "qabstractscrollarea.h"

QT_BEGIN_NAMESPACE

#ifndef QT_NO_SCROLLAREA

class QScrollBar;
class QAbstractScrollAreaScrollBarContainer;

class QAbstractScrollAreaPrivate: public QFramePrivate
{
    Q_DECLARE_PUBLIC(QAbstractScrollArea)
public:
    QAbstractScrollAreaPrivate();

    void replaceScrollBar(QScrollBar *scrollBar, Qt::Orientation orientation);

    QAbstractScrollAreaScrollBarContainer *scrollBarContainers[Qt::Vertical + 1];
    QScrollBar *hbar, *vbar;
    Qt::ScrollBarPolicy vbarpolicy, hbarpolicy;

    QWidget *viewport;
    QWidget *cornerWidget;
    QRect cornerPaintingRect;
    int left, top, right, bottom; // viewport margin

    int xoffset, yoffset;

    void init();
    void layoutChildren();

    void _q_hslide(int);
    void _q_vslide(int);
    void _q_showOrHideScrollBars();

    virtual QPoint contentsOffset() const;

    inline bool viewportEvent(QEvent *event)
    { return q_func()->viewportEvent(event); }
    QScopedPointer<QObject> viewportFilter;
};

class QAbstractScrollAreaFilter : public QObject
{
    Q_OBJECT
public:
    QAbstractScrollAreaFilter(QAbstractScrollAreaPrivate *p) : d(p)
    { setObjectName(QLatin1String("qt_abstractscrollarea_filter")); }
    bool eventFilter(QObject *o, QEvent *e)
    { return (o == d->viewport ? d->viewportEvent(e) : false); }
private:
    QAbstractScrollAreaPrivate *d;
};

class QBoxLayout;
class QAbstractScrollAreaScrollBarContainer : public QWidget
{
public:
    enum LogicalPosition { LogicalLeft = 1, LogicalRight = 2 };

    QAbstractScrollAreaScrollBarContainer(Qt::Orientation orientation, QWidget *parent);
    void addWidget(QWidget *widget, LogicalPosition position);
    QWidgetList widgets(LogicalPosition position);
    void removeWidget(QWidget *widget);

    QScrollBar *scrollBar;
    QBoxLayout *layout;
private:
    int scrollBarLayoutIndex() const;

    Qt::Orientation orientation;
};

#endif // QT_NO_SCROLLAREA

QT_END_NAMESPACE

#endif // QABSTRACTSCROLLAREA_P_H
