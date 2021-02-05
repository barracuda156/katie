/****************************************************************************
**
** Copyright (C) 2015 The Qt Company Ltd.
** Copyright (C) 2016 Ivailo Monev
**
** This file is part of the QtCore module of the Katie Toolkit.
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

#ifndef QCONTAINERFWD_H
#define QCONTAINERFWD_H

#include <QtCore/qglobal.h>


QT_BEGIN_NAMESPACE

template <class Key, class T> class QCache;
template <class Key, class T> class QHash;
template <class T> class QLinkedList;
template <class T> class QList;
template <class Key, class T> class QMap;
template <class Key, class T> class QMultiHash;
template <class Key, class T> class QMultiMap;
template <class T1, class T2> struct QPair;
template <class T> class QQueue;
template <class T> class QSet;
template <class T> class QStack;
template<class T, int Prealloc = 256> class QVarLengthArray;
template <class T> class QVector;

QT_END_NAMESPACE


#endif // QCONTAINERFWD_H
