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

#ifndef QDATABUFFER_P_H
#define QDATABUFFER_P_H

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

#include "QtCore/qglobal.h"

QT_BEGIN_NAMESPACE

template <typename Type> class QDataBuffer
{
public:
    QDataBuffer(int res)
        : capacity(res),
        siz(0),
        buffer(nullptr)
    {
        if (res)
            buffer = (Type*) malloc(capacity * sizeof(Type));
    }

    ~QDataBuffer()
    {
        if (buffer)
            free(buffer);
    }

    inline void reset() { siz = 0; }

    inline bool isEmpty() const { return siz==0; }

    inline int size() const { return siz; }
    inline Type *data() const { return buffer; }

    inline Type &at(int i) { Q_ASSERT(i >= 0 && i < siz); return buffer[i]; }
    inline const Type &at(int i) const { Q_ASSERT(i >= 0 && i < siz); return buffer[i]; }
    inline Type &last() { Q_ASSERT(!isEmpty()); return buffer[siz-1]; }
    inline const Type &last() const { Q_ASSERT(!isEmpty()); return buffer[siz-1]; }
    inline Type &first() { Q_ASSERT(!isEmpty()); return buffer[0]; }
    inline const Type &first() const { Q_ASSERT(!isEmpty()); return buffer[0]; }

    inline void add(const Type &t) {
        reserve(siz + 1);
        buffer[siz] = t;
        ++siz;
    }

    inline void resize(int size) {
        reserve(size);
        siz = size;
    }

    inline void reserve(int size) {
        if (size > capacity) {
            if (capacity == 0)
                capacity = 1;
            while (capacity < size)
                capacity *= 2;
            buffer = (Type*) realloc(buffer, capacity * sizeof(Type));
        }
    }

    inline void shrink(int size) {
        capacity = size;
        if (size)
            buffer = (Type*) realloc(buffer, capacity * sizeof(Type));
        else {
            free(buffer);
            buffer = 0;
        }
    }

    inline void swap(QDataBuffer<Type> &other) {
        qSwap(capacity, other.capacity);
        qSwap(siz, other.siz);
        qSwap(buffer, other.buffer);
    }

private:
    int capacity;
    int siz;
    Type *buffer;
};

QT_END_NAMESPACE

#endif // QDATABUFFER_P_H
