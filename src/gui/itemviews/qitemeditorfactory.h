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

#ifndef QITEMEDITORFACTORY_H
#define QITEMEDITORFACTORY_H

#include <QtCore/qmetaobject.h>
#include <QtCore/qbytearray.h>
#include <QtCore/qhash.h>
#include <QtCore/qvariant.h>


QT_BEGIN_NAMESPACE


#ifndef QT_NO_ITEMVIEWS

class QWidget;

class Q_GUI_EXPORT QItemEditorFactory
{
public:
    QItemEditorFactory();
    virtual ~QItemEditorFactory();

    virtual QWidget *createEditor(const QVariant &variant, QWidget *parent) const;
    virtual QByteArray valuePropertyName(QVariant::Type type) const;

    static const QItemEditorFactory* defaultFactory();
};

#endif // QT_NO_ITEMVIEWS

QT_END_NAMESPACE


#endif // QITEMEDITORFACTORY_H
