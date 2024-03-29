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

#ifndef QWINDOWSURFACE_P_H
#define QWINDOWSURFACE_P_H

//
//  W A R N I N G
//  -------------
//
// This file is not part of the Katie API.  It exists for the convenience
// of other Qt classes.  This header file may change from version to
// version without notice, or even be removed.
//
// We mean it.
//

#include <QtGui/qwidget.h>

QT_BEGIN_NAMESPACE

class QPaintDevice;
class QRegion;
class QRect;
class QPoint;
class QWindowSurfacePrivate;

class QWindowSurface
{
public:
    QWindowSurface(QWidget *windows);
    ~QWindowSurface();

    QWidget *window() const;

    QPaintDevice *paintDevice();

    // 'widget' can be a child widget, in which case 'region' is in child widget coordinates and
    // offset is the (child) widget's offset in relation to the window surface. On QWS, 'offset'
    // can be larger than just the offset from the top-level widget as there may also be window
    // decorations which are painted into the window surface.
    void flush(QWidget *widget, const QRegion &region, const QPoint &offset);
    void setGeometry(const QRect &rect);
    QRect geometry() const;

    bool scroll(const QRegion &area, int dx, int dy);

    void beginPaint(const QRegion &region);
    void endPaint();

    QPoint offset(const QWidget *widget) const;
    inline QRect rect(const QWidget *widget) const;

private:
    QWindowSurfacePrivate *d_ptr;
};

inline QRect QWindowSurface::rect(const QWidget *widget) const
{
    return widget->rect().translated(offset(widget));
}

QT_END_NAMESPACE

#endif // QWINDOWSURFACE_P_H
