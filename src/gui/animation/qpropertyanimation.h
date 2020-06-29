/****************************************************************************
**
** Copyright (C) 2015 The Qt Company Ltd.
** Copyright (C) 2016-2020 Ivailo Monev
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

#ifndef QPROPERTYANIMATION_H
#define QPROPERTYANIMATION_H

#include <QtGui/qvariantanimation.h>

QT_BEGIN_HEADER

QT_BEGIN_NAMESPACE

#ifndef QT_NO_ANIMATION

class QPropertyAnimationPrivate;
class Q_GUI_EXPORT QPropertyAnimation : public QVariantAnimation
{
    Q_OBJECT
    Q_PROPERTY(QByteArray propertyName READ propertyName WRITE setPropertyName)
    Q_PROPERTY(QObject* targetObject READ targetObject WRITE setTargetObject)

public:
    QPropertyAnimation(QObject *parent = Q_NULLPTR);
    QPropertyAnimation(QObject *target, const QByteArray &propertyName, QObject *parent = Q_NULLPTR);
    ~QPropertyAnimation();

    QObject *targetObject() const;
    void setTargetObject(QObject *target);

    QByteArray propertyName() const;
    void setPropertyName(const QByteArray &propertyName);

protected:
    void updateCurrentValue(const QVariant &value);
    void updateState(QAbstractAnimation::State newState, QAbstractAnimation::State oldState);

private:
    Q_DISABLE_COPY(QPropertyAnimation)
    Q_DECLARE_PRIVATE(QPropertyAnimation)
};

#endif //QT_NO_ANIMATION

QT_END_NAMESPACE

QT_END_HEADER

#endif // QPROPERTYANIMATION_H
