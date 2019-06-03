/****************************************************************************
**
** Copyright (C) 2015 The Qt Company Ltd.
** Copyright (C) 2016-2019 Ivailo Monev
**
** This file is part of the Qt Designer of the Katie Toolkit.
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

#ifndef QLONGLONGVALIDATOR_H
#define QLONGLONGVALIDATOR_H

#include <QtGui/QValidator>

QT_BEGIN_NAMESPACE

namespace qdesigner_internal {

class QLongLongValidator : public QValidator
{
    Q_OBJECT
    Q_PROPERTY(qlonglong bottom READ bottom WRITE setBottom)
    Q_PROPERTY(qlonglong top READ top WRITE setTop)

public:
    explicit QLongLongValidator(QObject * parent);
    QLongLongValidator(qlonglong bottom, qlonglong top, QObject * parent);
    ~QLongLongValidator();

    QValidator::State validate(QString &, int &) const;

    void setBottom(qlonglong);
    void setTop(qlonglong);
    virtual void setRange(qlonglong bottom, qlonglong top);

    qlonglong bottom() const { return b; }
    qlonglong top() const { return t; }

private:
    Q_DISABLE_COPY(QLongLongValidator)

    qlonglong b;
    qlonglong t;
};

// ----------------------------------------------------------------------------
class QULongLongValidator : public QValidator
{
    Q_OBJECT
    Q_PROPERTY(qulonglong bottom READ bottom WRITE setBottom)
    Q_PROPERTY(qulonglong top READ top WRITE setTop)

public:
    explicit QULongLongValidator(QObject * parent);
    QULongLongValidator(qulonglong bottom, qulonglong top, QObject * parent);
    ~QULongLongValidator();

    QValidator::State validate(QString &, int &) const;

    void setBottom(qulonglong);
    void setTop(qulonglong);
    virtual void setRange(qulonglong bottom, qulonglong top);

    qulonglong bottom() const { return b; }
    qulonglong top() const { return t; }

private:
    Q_DISABLE_COPY(QULongLongValidator)

    qulonglong b;
    qulonglong t;
};

}  // namespace qdesigner_internal

QT_END_NAMESPACE

#endif // QLONGLONGVALIDATOR_H
