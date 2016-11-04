/****************************************************************************
**
** Copyright (C) 2015 The Qt Company Ltd.
** Contact: http://www.qt.io/licensing/
**
** This file is part of the QtDeclarative module of the Qt Toolkit.
**
** $QT_BEGIN_LICENSE:LGPL$
** Commercial License Usage
** Licensees holding valid commercial Qt licenses may use this file in
** accordance with the commercial license agreement provided with the
** Software or, alternatively, in accordance with the terms contained in
** a written agreement between you and The Qt Company. For licensing terms
** and conditions see http://www.qt.io/terms-conditions. For further
** information use the contact form at http://www.qt.io/contact-us.
**
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

#ifndef QDECLARATIVESMOOTHEDANIMATION_H
#define QDECLARATIVESMOOTHEDANIMATION_H

#include <qdeclarative.h>
#include "qdeclarativeanimation_p.h"

#include <QtCore/qobject.h>

QT_BEGIN_HEADER

QT_BEGIN_NAMESPACE


class QDeclarativeProperty;
class QDeclarativeSmoothedAnimationPrivate;
class Q_AUTOTEST_EXPORT QDeclarativeSmoothedAnimation : public QDeclarativeNumberAnimation
{
    Q_OBJECT
    Q_DECLARE_PRIVATE(QDeclarativeSmoothedAnimation)
    Q_ENUMS(ReversingMode)

    Q_PROPERTY(qreal velocity READ velocity WRITE setVelocity NOTIFY velocityChanged)
    Q_PROPERTY(ReversingMode reversingMode READ reversingMode WRITE setReversingMode NOTIFY reversingModeChanged)
    Q_PROPERTY(qreal maximumEasingTime READ maximumEasingTime WRITE setMaximumEasingTime NOTIFY maximumEasingTimeChanged)

public:
    enum ReversingMode { Eased, Immediate, Sync };

    QDeclarativeSmoothedAnimation(QObject *parent = Q_NULLPTR);
    ~QDeclarativeSmoothedAnimation();

    ReversingMode reversingMode() const;
    void setReversingMode(ReversingMode);

    virtual int duration() const;
    virtual void setDuration(int);

    qreal velocity() const;
    void setVelocity(qreal);

    int maximumEasingTime() const;
    void setMaximumEasingTime(int);

    virtual void transition(QDeclarativeStateActions &actions,
                            QDeclarativeProperties &modified,
                            TransitionDirection direction);
    QAbstractAnimation* qtAnimation();

Q_SIGNALS:
    void velocityChanged();
    void reversingModeChanged();
    void maximumEasingTimeChanged();
};

QT_END_NAMESPACE

QML_DECLARE_TYPE(QDeclarativeSmoothedAnimation)

QT_END_HEADER

#endif // QDECLARATIVESMOOTHEDANIMATION_H
