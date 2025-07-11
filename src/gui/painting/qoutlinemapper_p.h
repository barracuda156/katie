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

#ifndef QOUTLINEMAPPER_P_H
#define QOUTLINEMAPPER_P_H

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

#include "qrect.h"
#include "qtransform.h"
#include "qpainterpath.h"
#include "qrasterdefs_p.h"
#include "qdatabuffer_p.h"
#include "qpaintengineex_p.h"

QT_BEGIN_NAMESPACE

//#define QT_DEBUG_CONVERT

/********************************************************************************
 * class QOutlineMapper
 *
 * Used to map between QPainterPath and the QT_FT_Outline structure used by the
 * freetype scanconvertor.
 *
 * The outline mapper uses a path iterator to get points from the path,
 * so that it is possible to transform the points as they are converted. The
 * callback can be a noop, translate or full-fledged xform. (Tests indicated
 * that using a C callback was low cost).
 */
class QOutlineMapper
{
public:
    QOutlineMapper() :
        m_element_types(0),
        m_elements(0),
        m_elements_dev(0),
        m_points(0),
        m_tags(0),
        m_contours(0),
        m_in_clip_elements(false)
    {
    }

    /*!
      Sets up the matrix to be used for conversion. This also
      sets up the qt_path_iterator function that is used as a callback
      to get points.
    */
    void setMatrix(const QTransform &m)
    {
        m_m11 = m.m11();
        m_m12 = m.m12();
        m_m13 = m.m13();
        m_m21 = m.m21();
        m_m22 = m.m22();
        m_m23 = m.m23();
        m_m33 = m.m33();
        m_dx = m.dx();
        m_dy = m.dy();
        m_txop = m.type();
    }

    void beginOutline(Qt::FillRule fillRule)
    {
#ifdef QT_DEBUG_CONVERT
        printf("QOutlineMapper::beginOutline rule=%d\n", fillRule);
#endif
        m_valid = true;
        m_elements.reset();
        m_elements_dev.reset();
        m_element_types.reset();
        m_points.reset();
        m_tags.reset();
        m_contours.reset();
        m_outline.flags = fillRule == Qt::WindingFill
                          ? QT_FT_OUTLINE_NONE
                          : QT_FT_OUTLINE_EVEN_ODD_FILL;
        m_subpath_start = 0;
    }

    void endOutline();

    void clipElements(const QPointF *points, const QPainterPath::ElementType *types, int count);

    void convertElements(const QPointF *points, const QPainterPath::ElementType *types, int count);

    inline void moveTo(const QPointF &pt) {
#ifdef QT_DEBUG_CONVERT
        printf("QOutlineMapper::moveTo() (%f, %f)\n", pt.x(), pt.y());
#endif
        closeSubpath();
        m_subpath_start = m_elements.size();
        m_elements.add(pt);
        m_element_types.add(QPainterPath::MoveToElement);
    }

    inline void lineTo(const QPointF &pt) {
#ifdef QT_DEBUG_CONVERT
        printf("QOutlineMapper::lineTo() (%f, %f)\n", pt.x(), pt.y());
#endif
        m_elements.add(pt);
        m_element_types.add(QPainterPath::LineToElement);
    }

    inline void curveTo(const QPointF &cp1, const QPointF &cp2, const QPointF &ep) {
#ifdef QT_DEBUG_CONVERT
        printf("QOutlineMapper::curveTo() (%f, %f)\n", ep.x(), ep.y());
#endif
        m_elements.add(cp1);
        m_elements.add(cp2);
        m_elements.add(ep);
        m_element_types.add(QPainterPath::CurveToElement);
        m_element_types.add(QPainterPath::CurveToDataElement);
        m_element_types.add(QPainterPath::CurveToDataElement);
    }

    inline void closeSubpath() {
        int element_count = m_elements.size();
        if (element_count > 0) {
            if (m_elements.at(element_count-1) != m_elements.at(m_subpath_start)) {
#ifdef QT_DEBUG_CONVERT
                printf(" - implicitly closing\n");
#endif
                // Put the object on the stack to avoid the odd case where
                // lineTo reallocs the databuffer and the QPointF & will
                // be invalidated.
                QPointF pt = m_elements.at(m_subpath_start);

                // only do lineTo if we have element_type array...
                if (m_element_types.size())
                    lineTo(pt);
                else
                    m_elements.add(pt);

            }
        }
    }

    QT_FT_Outline *outline() {
        if (m_valid)
            return &m_outline;
        return 0;
    }

    QT_FT_Outline *convertPath(const QPainterPath &path);
    QT_FT_Outline *convertPath(const QVectorPath &path);

    inline const QPainterPath::ElementType *elementTypes() const { return m_element_types.size() == 0 ? 0 : m_element_types.data(); }

public:
    QDataBuffer<QPainterPath::ElementType> m_element_types;
    QDataBuffer<QPointF> m_elements;
    QDataBuffer<QPointF> m_elements_dev;
    QDataBuffer<QT_FT_Vector> m_points;
    QDataBuffer<char> m_tags;
    QDataBuffer<int> m_contours;

    QRect m_clip_rect;

    QRectF controlPointRect; // only valid after endOutline()

    QT_FT_Outline m_outline;
    QTransform::TransformationType m_txop;

    int m_subpath_start;

    // Matrix
    qreal m_m11;
    qreal m_m12;
    qreal m_m13;
    qreal m_m21;
    qreal m_m22;
    qreal m_m23;
    qreal m_m33;
    qreal m_dx;
    qreal m_dy;

    bool m_valid;
    bool m_in_clip_elements;
};

QT_END_NAMESPACE

#endif // QOUTLINEMAPPER_P_H
