/****************************************************************************
**
** Copyright (C) 2015 The Qt Company Ltd.
** Copyright (C) 2016-2019 Ivailo Monev
**
** This file is part of the tools applications of the Katie Toolkit.
**
** $QT_BEGIN_LICENSE:LGPL$
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

#ifndef GRADIENTMANAGER_H
#define GRADIENTMANAGER_H

#include <QtCore/QObject>
#include <QtCore/QMap>
#include <QtCore/QSize>
#include <QtXml/qdom.h>
#include <QtGui/qbrush.h>

QT_BEGIN_NAMESPACE

class QGradient;
class QPixmap;
class QColor;

class QtGradientManager : public QObject
{
    Q_OBJECT
public:
    QtGradientManager(QObject *parent = Q_NULLPTR);

    QMap<QString, QGradient> gradients() const;

    QString uniqueId(const QString &id) const;

public slots:

    QString addGradient(const QString &id, const QGradient &gradient);
    void renameGradient(const QString &id, const QString &newId);
    void changeGradient(const QString &id, const QGradient &newGradient);
    void removeGradient(const QString &id);

    //utils
    void clear();

signals:

    void gradientAdded(const QString &id, const QGradient &gradient);
    void gradientRenamed(const QString &id, const QString &newId);
    void gradientChanged(const QString &id, const QGradient &newGradient);
    void gradientRemoved(const QString &id);

private:

    QMap<QString, QGradient> m_idToGradient;
};

QT_END_NAMESPACE

#endif
