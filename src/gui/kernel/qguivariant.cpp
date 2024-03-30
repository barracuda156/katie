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

#include "qvariant.h"

#include "qbitmap.h"
#include "qbrush.h"
#include "qcolor.h"
#include "qcursor.h"
#include "qdatastream.h"
#include "qdebug.h"
#include "qfont.h"
#include "qicon.h"
#include "qimage.h"
#include "qkeysequence.h"
#include "qtransform.h"
#include "qmatrix.h"
#include "qpalette.h"
#include "qpen.h"
#include "qpixmap.h"
#include "qpolygon.h"
#include "qregion.h"
#include "qsizepolicy.h"
#include "qtextformat.h"
#include "qmatrix4x4.h"
#include "qvector2d.h"
#include "qvector3d.h"
#include "qvector4d.h"
#include "qvariant_p.h"

QT_BEGIN_NAMESPACE


Q_CORE_EXPORT const QVariant::Handler *qcoreVariantHandler();

static bool isNull(const QVariantPrivate *d)
{
    switch(d->type) {
    case QVariant::Bitmap:
        return v_cast<QBitmap>(d)->isNull();
    case QVariant::Region:
        return v_cast<QRegion>(d)->isEmpty();
    case QVariant::Polygon:
        return v_cast<QPolygon>(d)->isEmpty();
    case QVariant::Pixmap:
        return v_cast<QPixmap>(d)->isNull();
    case QVariant::Image:
        return v_cast<QImage>(d)->isNull();
#ifndef QT_NO_ICON
    case QVariant::Icon:
        return v_cast<QIcon>(d)->isNull();
#endif
    case QVariant::Matrix:
    case QVariant::TextFormat:
    case QVariant::TextLength:
    case QVariant::Cursor:
    case QVariant::StringList:
    case QVariant::Font:
    case QVariant::Brush:
    case QVariant::Color:
    case QVariant::Palette:
    case QVariant::SizePolicy:
#ifndef QT_NO_SHORTCUT
    case QVariant::KeySequence:
#endif
    case QVariant::Pen:
#ifndef QT_NO_MATRIX4X4
    case QVariant::Matrix4x4:
#endif
        break;
#ifndef QT_NO_VECTOR2D
    case QVariant::Vector2D:
        return v_cast<QVector2D>(d)->isNull();
#endif
#ifndef QT_NO_VECTOR3D
    case QVariant::Vector3D:
        return v_cast<QVector3D>(d)->isNull();
#endif
#ifndef QT_NO_VECTOR4D
    case QVariant::Vector4D:
        return v_cast<QVector4D>(d)->isNull();
#endif
    default:
        return qcoreVariantHandler()->isNull(d);
    }
    return d->is_null;
}

static bool compare(const QVariantPrivate *a, const QVariantPrivate *b)
{
    Q_ASSERT(a->type == b->type);
    switch(a->type) {
    case QVariant::Cursor:
#ifndef QT_NO_CURSOR
        return v_cast<QCursor>(a)->shape() == v_cast<QCursor>(b)->shape();
#endif
    case QVariant::Bitmap:
        return *v_cast<QBitmap>(a) == *v_cast<QBitmap>(b);
    case QVariant::Polygon:
        return *v_cast<QPolygon>(a) == *v_cast<QPolygon>(b);
    case QVariant::Region:
        return *v_cast<QRegion>(a) == *v_cast<QRegion>(b);
    case QVariant::Font:
        return *v_cast<QFont>(a) == *v_cast<QFont>(b);
    case QVariant::Pixmap:
        return *v_cast<QPixmap>(a) == *v_cast<QPixmap>(b);
    case QVariant::Image:
        return *v_cast<QImage>(a) == *v_cast<QImage>(b);
    case QVariant::Brush:
        return *v_cast<QBrush>(a) == *v_cast<QBrush>(b);
    case QVariant::Color:
        return *v_cast<QColor>(a) == *v_cast<QColor>(b);
    case QVariant::Palette:
        return *v_cast<QPalette>(a) == *v_cast<QPalette>(b);
#ifndef QT_NO_ICON
    case QVariant::Icon:
        return v_cast<QIcon>(a)->cacheKey() == v_cast<QIcon>(b)->cacheKey();
#endif
    case QVariant::Matrix:
        return *v_cast<QMatrix>(a) == *v_cast<QMatrix>(b);
    case QVariant::Transform:
        return *v_cast<QTransform>(a) == *v_cast<QTransform>(b);
    case QVariant::TextFormat:
        return *v_cast<QTextFormat>(a) == *v_cast<QTextFormat>(b);
    case QVariant::TextLength:
        return *v_cast<QTextLength>(a) == *v_cast<QTextLength>(b);
    case QVariant::SizePolicy:
        return *v_cast<QSizePolicy>(a) == *v_cast<QSizePolicy>(b);
#ifndef QT_NO_SHORTCUT
    case QVariant::KeySequence:
        return *v_cast<QKeySequence>(a) == *v_cast<QKeySequence>(b);
#endif
    case QVariant::Pen:
        return *v_cast<QPen>(a) == *v_cast<QPen>(b);
#ifndef QT_NO_MATRIX4X4
    case QVariant::Matrix4x4:
        return *v_cast<QMatrix4x4>(a) == *v_cast<QMatrix4x4>(b);
#endif
#ifndef QT_NO_VECTOR2D
    case QVariant::Vector2D:
        return *v_cast<QVector2D>(a) == *v_cast<QVector2D>(b);
#endif
#ifndef QT_NO_VECTOR3D
    case QVariant::Vector3D:
        return *v_cast<QVector3D>(a) == *v_cast<QVector3D>(b);
#endif
#ifndef QT_NO_VECTOR4D
    case QVariant::Vector4D:
        return *v_cast<QVector4D>(a) == *v_cast<QVector4D>(b);
#endif
    default:
        break;
    }
    return qcoreVariantHandler()->compare(a, b);
}

static bool convert(const QVariantPrivate *d, int t,
                 void *result, bool *ok)
{
    if (!d) {
        if (ok) {
            *ok = false;
        }
        return false;
    }

    switch (t) {
    case QVariant::ByteArray: {
        if (d->type == QVariant::Color) {
            *static_cast<QByteArray *>(result) = v_cast<QColor>(d)->name().toLatin1();
            return true;
        }
        break;
    }
    case QVariant::String: {
        QString *str = static_cast<QString *>(result);
        switch (d->type) {
#ifndef QT_NO_SHORTCUT
        case QVariant::KeySequence:
            *str = v_cast<QKeySequence>(d)->toString();
            return true;
#endif
        case QVariant::Font:
            *str = v_cast<QFont>(d)->toString();
            return true;
        case QVariant::Color:
            *str = v_cast<QColor>(d)->name();
            return true;
        default:
            break;
        }
        break;
    }
    case QVariant::Pixmap: {
        if (d->type == QVariant::Image) {
            *static_cast<QPixmap *>(result) = QPixmap::fromImage(*v_cast<QImage>(d));
            return true;
        } else if (d->type == QVariant::Bitmap) {
            *static_cast<QPixmap *>(result) = *v_cast<QBitmap>(d);
            return true;
        } else if (d->type == QVariant::Brush) {
            if (v_cast<QBrush>(d)->style() == Qt::TexturePattern) {
                *static_cast<QPixmap *>(result) = v_cast<QBrush>(d)->texture();
                return true;
            }
        }
        break;
    }
    case QVariant::Image: {
        if (d->type == QVariant::Pixmap) {
            *static_cast<QImage *>(result) = v_cast<QPixmap>(d)->toImage();
            return true;
        } else if (d->type == QVariant::Bitmap) {
            *static_cast<QImage *>(result) = v_cast<QBitmap>(d)->toImage();
            return true;
        }
        break;
    }
    case QVariant::Bitmap: {
        if (d->type == QVariant::Pixmap) {
            *static_cast<QBitmap *>(result) = *v_cast<QPixmap>(d);
            return true;
        } else if (d->type == QVariant::Image) {
            *static_cast<QBitmap *>(result) = QBitmap::fromImage(*v_cast<QImage>(d));
            return true;
        }
        break;
    }
#ifndef QT_NO_SHORTCUT
    case QVariant::Int: {
        if (d->type == QVariant::KeySequence) {
            *static_cast<int *>(result) = (int)(*(v_cast<QKeySequence>(d)));
            return true;
        }
        break;
    }
#endif
    case QVariant::Font: {
        if (d->type == QVariant::String) {
            QFont *f = static_cast<QFont *>(result);
            f->fromString(*v_cast<QString>(d));
            return true;
        }
        break;
    }
    case QVariant::Color: {
        if (d->type == QVariant::String) {
            static_cast<QColor *>(result)->setNamedColor(*v_cast<QString>(d));
            return static_cast<QColor *>(result)->isValid();
        } else if (d->type == QVariant::ByteArray) {
            static_cast<QColor *>(result)->setNamedColor(QString::fromLatin1(
                                *v_cast<QByteArray>(d)));
            return static_cast<QColor *>(result)->isValid();
        } else if (d->type == QVariant::Brush) {
            if (v_cast<QBrush>(d)->style() == Qt::SolidPattern) {
                *static_cast<QColor *>(result) = v_cast<QBrush>(d)->color();
                return true;
            }
        }
        break;
    }
    case QVariant::Brush: {
        if (d->type == QVariant::Color) {
            *static_cast<QBrush *>(result) = QBrush(*v_cast<QColor>(d));
            return true;
        } else if (d->type == QVariant::Pixmap) {
            *static_cast<QBrush *>(result) = QBrush(*v_cast<QPixmap>(d));
            return true;
        }
        break;
    }
#ifndef QT_NO_SHORTCUT
    case QVariant::KeySequence: {
        QKeySequence *seq = static_cast<QKeySequence *>(result);
        if (d->type == QVariant::String) {
            *seq = QKeySequence(*v_cast<QString>(d));
            return true;
        } else if (d->type == QVariant::Int) {
            *seq = QKeySequence(*v_cast<int>(d));
            return true;
        }
        break;
    }
#endif
    default:
        break;
    }
    return qcoreVariantHandler()->convert(d, t, result, ok);
}

#if !defined(QT_NO_DEBUG_STREAM) && !defined(Q_BROKEN_DEBUG_STREAM)
static void streamDebug(QDebug dbg, const QVariant &v)
{
    switch(v.type()) {
    case QVariant::Cursor:
#ifndef QT_NO_CURSOR
//        dbg.nospace() << qvariant_cast<QCursor>(v); //FIXME
#endif
        break;
    case QVariant::Bitmap:
//        dbg.nospace() << qvariant_cast<QBitmap>(v); //FIXME
        break;
    case QVariant::Polygon:
        dbg.nospace() << qvariant_cast<QPolygon>(v);
        break;
    case QVariant::Region:
        dbg.nospace() << qvariant_cast<QRegion>(v);
        break;
    case QVariant::Font:
        dbg.nospace() << qvariant_cast<QFont>(v);
        break;
    case QVariant::Matrix:
        dbg.nospace() << qvariant_cast<QMatrix>(v);
        break;
    case QVariant::Transform:
        dbg.nospace() << qvariant_cast<QTransform>(v);
        break;
    case QVariant::Pixmap:
//        dbg.nospace() << qvariant_cast<QPixmap>(v); //FIXME
        break;
    case QVariant::Image:
//        dbg.nospace() << qvariant_cast<QImage>(v); //FIXME
        break;
    case QVariant::Brush:
        dbg.nospace() << qvariant_cast<QBrush>(v);
        break;
    case QVariant::Color:
        dbg.nospace() << qvariant_cast<QColor>(v);
        break;
    case QVariant::Palette:
//        dbg.nospace() << qvariant_cast<QPalette>(v); //FIXME
        break;
#ifndef QT_NO_ICON
    case QVariant::Icon:
//        dbg.nospace() << qvariant_cast<QIcon>(v); // FIXME
        break;
#endif
    case QVariant::SizePolicy:
//        dbg.nospace() << qvariant_cast<QSizePolicy>(v); //FIXME
        break;
#ifndef QT_NO_SHORTCUT
    case QVariant::KeySequence:
        dbg.nospace() << qvariant_cast<QKeySequence>(v);
        break;
#endif
    case QVariant::Pen:
        dbg.nospace() << qvariant_cast<QPen>(v);
        break;
#ifndef QT_NO_MATRIX4X4
    case QVariant::Matrix4x4:
        dbg.nospace() << qvariant_cast<QMatrix4x4>(v);
        break;
#endif
#ifndef QT_NO_VECTOR2D
    case QVariant::Vector2D:
        dbg.nospace() << qvariant_cast<QVector2D>(v);
        break;
#endif
#ifndef QT_NO_VECTOR3D
    case QVariant::Vector3D:
        dbg.nospace() << qvariant_cast<QVector3D>(v);
        break;
#endif
#ifndef QT_NO_VECTOR4D
    case QVariant::Vector4D:
        dbg.nospace() << qvariant_cast<QVector4D>(v);
        break;
#endif
    default:
        qcoreVariantHandler()->debugStream(dbg, v);
        break;
    }
}
#endif

const QVariant::Handler qt_gui_variant_handler = {
    isNull,
    compare,
    convert,
#if !defined(QT_NO_DEBUG_STREAM) && !defined(Q_BROKEN_DEBUG_STREAM)
    streamDebug
#else
    0
#endif
};

struct QMetaTypeGuiHelper
{
    QMetaType::Constructor constr;
    QMetaType::Destructor destr;
#ifndef QT_NO_DATASTREAM
    QMetaType::SaveOperator saveOp;
    QMetaType::LoadOperator loadOp;
#endif
};

extern Q_CORE_EXPORT const QMetaTypeGuiHelper *qMetaTypeGuiHelper;


#ifdef QT_NO_DATASTREAM
#  define Q_DECL_METATYPE_HELPER(TYPE) \
     typedef void *(*QConstruct##TYPE)(const TYPE *); \
     static const QConstruct##TYPE qConstruct##TYPE = qMetaTypeConstructHelper<TYPE>; \
     typedef void (*QDestruct##TYPE)(TYPE *); \
     static const QDestruct##TYPE qDestruct##TYPE = qMetaTypeDeleteHelper<TYPE>;
#else
#  define Q_DECL_METATYPE_HELPER(TYPE) \
     typedef void *(*QConstruct##TYPE)(const TYPE *); \
     static const QConstruct##TYPE qConstruct##TYPE = qMetaTypeConstructHelper<TYPE>; \
     typedef void (*QDestruct##TYPE)(TYPE *); \
     static const QDestruct##TYPE qDestruct##TYPE = qMetaTypeDeleteHelper<TYPE>; \
     typedef void (*QSave##TYPE)(QDataStream &, const TYPE *); \
     static const QSave##TYPE qSave##TYPE = qMetaTypeSaveHelper<TYPE>; \
     typedef void (*QLoad##TYPE)(QDataStream &, TYPE *); \
     static const QLoad##TYPE qLoad##TYPE = qMetaTypeLoadHelper<TYPE>;
#endif

Q_DECL_METATYPE_HELPER(QFont)
Q_DECL_METATYPE_HELPER(QPixmap)
Q_DECL_METATYPE_HELPER(QBrush)
Q_DECL_METATYPE_HELPER(QColor)
Q_DECL_METATYPE_HELPER(QPalette)
#ifndef QT_NO_ICON
Q_DECL_METATYPE_HELPER(QIcon)
#endif
Q_DECL_METATYPE_HELPER(QImage)
Q_DECL_METATYPE_HELPER(QPolygon)
Q_DECL_METATYPE_HELPER(QRegion)
Q_DECL_METATYPE_HELPER(QBitmap)
#ifndef QT_NO_CURSOR
Q_DECL_METATYPE_HELPER(QCursor)
#endif
Q_DECL_METATYPE_HELPER(QSizePolicy)
#ifndef QT_NO_SHORTCUT
Q_DECL_METATYPE_HELPER(QKeySequence)
#endif
Q_DECL_METATYPE_HELPER(QPen)
Q_DECL_METATYPE_HELPER(QTextLength)
Q_DECL_METATYPE_HELPER(QTextFormat)
Q_DECL_METATYPE_HELPER(QMatrix)
Q_DECL_METATYPE_HELPER(QTransform)
#ifndef QT_NO_MATRIX4X4
Q_DECL_METATYPE_HELPER(QMatrix4x4)
#endif
#ifndef QT_NO_VECTOR2D
Q_DECL_METATYPE_HELPER(QVector2D)
#endif
#ifndef QT_NO_VECTOR3D
Q_DECL_METATYPE_HELPER(QVector3D)
#endif
#ifndef QT_NO_VECTOR4D
Q_DECL_METATYPE_HELPER(QVector4D)
#endif

#ifdef QT_NO_DATASTREAM
#  define Q_IMPL_METATYPE_HELPER(TYPE) \
     { reinterpret_cast<QMetaType::Constructor>(qConstruct##TYPE), \
       reinterpret_cast<QMetaType::Destructor>(qDestruct##TYPE) }
#else
#  define Q_IMPL_METATYPE_HELPER(TYPE) \
     { reinterpret_cast<QMetaType::Constructor>(qConstruct##TYPE), \
       reinterpret_cast<QMetaType::Destructor>(qDestruct##TYPE), \
       reinterpret_cast<QMetaType::SaveOperator>(qSave##TYPE), \
       reinterpret_cast<QMetaType::LoadOperator>(qLoad##TYPE) \
     }
#endif

static const QMetaTypeGuiHelper qVariantGuiHelper[] = {
    Q_IMPL_METATYPE_HELPER(QFont),
    Q_IMPL_METATYPE_HELPER(QPixmap),
    Q_IMPL_METATYPE_HELPER(QBrush),
    Q_IMPL_METATYPE_HELPER(QColor),
    Q_IMPL_METATYPE_HELPER(QPalette),
#ifdef QT_NO_ICON
    {0, 0, 0, 0},
#else
    Q_IMPL_METATYPE_HELPER(QIcon),
#endif
    Q_IMPL_METATYPE_HELPER(QImage),
    Q_IMPL_METATYPE_HELPER(QPolygon),
    Q_IMPL_METATYPE_HELPER(QRegion),
    Q_IMPL_METATYPE_HELPER(QBitmap),
#ifdef QT_NO_CURSOR
    {0, 0, 0, 0},
#else
    Q_IMPL_METATYPE_HELPER(QCursor),
#endif
    Q_IMPL_METATYPE_HELPER(QSizePolicy),
#ifdef QT_NO_SHORTCUT
    {0, 0, 0, 0},
#else
    Q_IMPL_METATYPE_HELPER(QKeySequence),
#endif
    Q_IMPL_METATYPE_HELPER(QPen),
    Q_IMPL_METATYPE_HELPER(QTextLength),
    Q_IMPL_METATYPE_HELPER(QTextFormat),
    Q_IMPL_METATYPE_HELPER(QMatrix),
    Q_IMPL_METATYPE_HELPER(QTransform),
#ifndef QT_NO_MATRIX4X4
    Q_IMPL_METATYPE_HELPER(QMatrix4x4),
#else
    {0, 0, 0, 0},
#endif
#ifndef QT_NO_VECTOR2D
    Q_IMPL_METATYPE_HELPER(QVector2D),
#else
    {0, 0, 0, 0},
#endif
#ifndef QT_NO_VECTOR3D
    Q_IMPL_METATYPE_HELPER(QVector3D),
#else
    {0, 0, 0, 0},
#endif
#ifndef QT_NO_VECTOR4D
    Q_IMPL_METATYPE_HELPER(QVector4D),
#else
    {0, 0, 0, 0},
#endif
};

static const QVariant::Handler *qt_guivariant_last_handler = 0;
int qRegisterGuiVariant()
{
    qt_guivariant_last_handler = QVariant::handler;
    QVariant::handler = &qt_gui_variant_handler;
    qMetaTypeGuiHelper = qVariantGuiHelper;
    return 1;
}
Q_CONSTRUCTOR_FUNCTION(qRegisterGuiVariant)

int qUnregisterGuiVariant()
{
    QVariant::handler = qt_guivariant_last_handler;
    qMetaTypeGuiHelper = 0;
    return 1;
}
Q_DESTRUCTOR_FUNCTION(qUnregisterGuiVariant)

QT_END_NAMESPACE


