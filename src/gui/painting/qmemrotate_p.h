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

#ifndef QMEMROTATE_P_H
#define QMEMROTATE_P_H

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

#include "qdrawhelper_p.h"

QT_BEGIN_NAMESPACE

#define QT_ROTATION_CACHEDREAD 1
#define QT_ROTATION_CACHEDWRITE 2

#define QT_ROTATION_ALGORITHM QT_ROTATION_CACHEDREAD


template <class T>
static inline void qt_memrotate90_template(const T *src,
                                           int srcWidth, int srcHeight, int srcStride,
                                           T *dest, int dstStride)
{
#if QT_ROTATION_ALGORITHM == QT_ROTATION_CACHEDREAD
    const char *s = reinterpret_cast<const char*>(src);
    char *d = reinterpret_cast<char*>(dest);
    for (int y = 0; y < srcHeight; ++y) {
        for (int x = srcWidth - 1; x >= 0; --x) {
            T *destline = reinterpret_cast<T*>(d + (srcWidth - x - 1) * dstStride);
            destline[y] = src[x];
        }
        s += srcStride;
        src = reinterpret_cast<const T*>(s);
    }
#elif QT_ROTATION_ALGORITHM == QT_ROTATION_CACHEDWRITE
    for (int x = srcWidth - 1; x >= 0; --x) {
        T *d = dest + (srcWidth - x - 1) * dstStride;
        for (int y = 0; y < srcHeight; ++y) {
            *d++ = src[y * srcStride + x];
        }
    }
#endif
}

template <class T>
static inline void qt_memrotate270_template(const T *src,
                                            int srcWidth, int srcHeight, int srcStride,
                                            T *dest, int dstStride)
{
#if QT_ROTATION_ALGORITHM == QT_ROTATION_CACHEDREAD
    const char *s = reinterpret_cast<const char*>(src);
    char *d = reinterpret_cast<char*>(dest);
    s += (srcHeight - 1) * srcStride;
    for (int y = srcHeight - 1; y >= 0; --y) {
        src = reinterpret_cast<const T*>(s);
        for (int x = 0; x < srcWidth; ++x) {
            T *destline = reinterpret_cast<T*>(d + x * dstStride);
            destline[srcHeight - y - 1] = src[x];
        }
        s -= srcStride;
    }
#elif QT_ROTATION_ALGORITHM == QT_ROTATION_CACHEDWRITE
    for (int x = 0; x < srcWidth; ++x) {
        T *d = dest + x * dstStride;
        for (int y = srcHeight - 1; y >= 0; --y) {
            *d++ = src[y * srcStride + x];
        }
    }
#endif
}

#define QT_IMPL_MEMROTATE(TYPE)                        \
static inline void qt_memrotate90(const TYPE *src, int w, int h, int sstride,  \
                    TYPE *dest, int dstride)                                   \
{                                                                   \
    qt_memrotate90_template(src, w, h, sstride, dest, dstride);     \
}                                                                   \
static inline void qt_memrotate270(const TYPE *src, int w, int h, int sstride, \
                     TYPE *dest, int dstride)                                  \
{                                                                   \
    qt_memrotate270_template(src, w, h, sstride, dest, dstride);    \
}

QT_IMPL_MEMROTATE(quint16)
QT_IMPL_MEMROTATE(quint32)

#undef QT_IMPL_MEMROTATE

QT_END_NAMESPACE

#endif // QMEMROTATE_P_H
