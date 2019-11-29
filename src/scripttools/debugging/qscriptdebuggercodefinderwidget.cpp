/****************************************************************************
**
** Copyright (C) 2015 The Qt Company Ltd.
** Copyright (C) 2016-2019 Ivailo Monev
**
** This file is part of the QtSCriptTools module of the Katie Toolkit.
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

#include "qscriptdebuggercodefinderwidget_p.h"
#include "qscriptdebuggercodefinderwidgetinterface_p_p.h"

#include <QtGui/qboxlayout.h>
#include <QtGui/qlineedit.h>
#include <QtGui/qcheckbox.h>
#include <QtGui/qlabel.h>
#include <QtGui/qtoolbutton.h>
#include <QtGui/qevent.h>
#include <QtCore/qdebug.h>

QT_BEGIN_NAMESPACE

class QScriptDebuggerCodeFinderWidgetPrivate
    : public QScriptDebuggerCodeFinderWidgetInterfacePrivate
{
    Q_DECLARE_PUBLIC(QScriptDebuggerCodeFinderWidget)
public:
    QScriptDebuggerCodeFinderWidgetPrivate();
    ~QScriptDebuggerCodeFinderWidgetPrivate();

    // private slots
    void _q_updateButtons();
    void _q_onTextChanged(const QString &);
    void _q_next();
    void _q_previous();

    int findOptions() const;

    QLineEdit *editFind;
    QCheckBox *checkCase;
    QLabel *labelWrapped;
    QToolButton *toolNext;
    QToolButton *toolClose;
    QToolButton *toolPrevious;
    QCheckBox *checkWholeWords;
};

QScriptDebuggerCodeFinderWidgetPrivate::QScriptDebuggerCodeFinderWidgetPrivate()
{
}

QScriptDebuggerCodeFinderWidgetPrivate::~QScriptDebuggerCodeFinderWidgetPrivate()
{
}

void QScriptDebuggerCodeFinderWidgetPrivate::_q_updateButtons()
{
    if (editFind->text().isEmpty()) {
        toolPrevious->setEnabled(false);
        toolNext->setEnabled(false);
    } else {
        toolPrevious->setEnabled(true);
        toolNext->setEnabled(true);
    }
}

int QScriptDebuggerCodeFinderWidgetPrivate::findOptions() const
{
    int flags = 0;
    if (checkCase->isChecked())
        flags |= QTextDocument::FindCaseSensitively;
    if (checkWholeWords->isChecked())
        flags |= QTextDocument::FindWholeWords;
    return flags;
}

void QScriptDebuggerCodeFinderWidgetPrivate::_q_onTextChanged(const QString &text)
{
    emit q_func()->findRequest(text, findOptions() | 0x100);
}

void QScriptDebuggerCodeFinderWidgetPrivate::_q_next()
{
    emit q_func()->findRequest(editFind->text(), findOptions());
}

void QScriptDebuggerCodeFinderWidgetPrivate::_q_previous()
{
    emit q_func()->findRequest(editFind->text(), findOptions() | QTextDocument::FindBackward);
}

QScriptDebuggerCodeFinderWidget::QScriptDebuggerCodeFinderWidget(QWidget *parent)
    : QScriptDebuggerCodeFinderWidgetInterface(
        *new QScriptDebuggerCodeFinderWidgetPrivate, parent, 0)
{
    Q_D(QScriptDebuggerCodeFinderWidget);
    QHBoxLayout *hboxLayout = new QHBoxLayout(this);
    hboxLayout->setSpacing(6);
    hboxLayout->setMargin(0);

    d->toolClose = new QToolButton(this);
    d->toolClose->setIcon(QIcon(QLatin1String(":/qt/scripttools/debugging/images/win/closetab.png")));
    d->toolClose->setAutoRaise(true);
    d->toolClose->setText(tr("Close"));
    hboxLayout->addWidget(d->toolClose);

    d->editFind = new QLineEdit(this);
    d->editFind->setMinimumSize(QSize(150, 0));
    connect(d->editFind, SIGNAL(textChanged(QString)),
            this, SLOT(_q_updateButtons()));
    connect(d->editFind, SIGNAL(returnPressed()),
            this, SLOT(_q_next()));
    hboxLayout->addWidget(d->editFind);

    d->toolPrevious = new QToolButton(this);
    d->toolPrevious->setAutoRaise(true);
    d->toolPrevious->setText(tr("Previous"));
    d->toolPrevious->setToolButtonStyle(Qt::ToolButtonTextBesideIcon);
    d->toolPrevious->setIcon(QIcon(QLatin1String(":/qt/scripttools/debugging/images/win/previous.png")));
    hboxLayout->addWidget(d->toolPrevious);

    d->toolNext = new QToolButton(this);
    d->toolNext->setAutoRaise(true);
    d->toolNext->setText(tr("Next"));
    d->toolNext->setToolButtonStyle(Qt::ToolButtonTextBesideIcon);
    d->toolNext->setIcon(QIcon(QLatin1String(":/qt/scripttools/debugging/images/win/next.png")));
    hboxLayout->addWidget(d->toolNext);

    d->checkCase = new QCheckBox(tr("Case Sensitive"), this);
    hboxLayout->addWidget(d->checkCase);

    d->checkWholeWords = new QCheckBox(tr("Whole words"), this);
    hboxLayout->addWidget(d->checkWholeWords);

    d->labelWrapped = new QLabel(this);
    d->labelWrapped->setMinimumSize(QSize(0, 20));
    d->labelWrapped->setMaximumSize(QSize(115, 20));
    d->labelWrapped->setTextFormat(Qt::RichText);
    d->labelWrapped->setScaledContents(true);
    d->labelWrapped->setAlignment(Qt::AlignLeading|Qt::AlignLeft|Qt::AlignVCenter);
    d->labelWrapped->setText(tr("<img src=\":/qt/scripttools/debugging/images/wrap.png\">&nbsp;Search wrapped"));
    hboxLayout->addWidget(d->labelWrapped);

    QSpacerItem *spacerItem = new QSpacerItem(20, 20, QSizePolicy::Expanding, QSizePolicy::Minimum);
    hboxLayout->addItem(spacerItem);
    setMinimumWidth(minimumSizeHint().width());
    d->labelWrapped->hide();

    d->_q_updateButtons();

    setFocusProxy(d->editFind);
    QObject::connect(d->toolClose, SIGNAL(clicked()), this, SLOT(hide()));
    QObject::connect(d->editFind, SIGNAL(textChanged(QString)),
                     this, SLOT(_q_onTextChanged(QString)));
    QObject::connect(d->toolNext, SIGNAL(clicked()), this, SLOT(_q_next()));
    QObject::connect(d->toolPrevious, SIGNAL(clicked()), this, SLOT(_q_previous()));
}

QScriptDebuggerCodeFinderWidget::~QScriptDebuggerCodeFinderWidget()
{
}

int QScriptDebuggerCodeFinderWidget::findOptions() const
{
    Q_D(const QScriptDebuggerCodeFinderWidget);
    return d->findOptions();
}

QString QScriptDebuggerCodeFinderWidget::text() const
{
    Q_D(const QScriptDebuggerCodeFinderWidget);
    return d->editFind->text();
}

void QScriptDebuggerCodeFinderWidget::setText(const QString &text)
{
    Q_D(const QScriptDebuggerCodeFinderWidget);
    d->editFind->setText(text);
}

void QScriptDebuggerCodeFinderWidget::setOK(bool ok)
{
    Q_D(QScriptDebuggerCodeFinderWidget);
    QPalette p = d->editFind->palette();
    QColor c;
    if (ok)
        c = Qt::white;
    else
        c = QColor(255, 102, 102);
    p.setColor(QPalette::Active, QPalette::Base, c);
    d->editFind->setPalette(p);
    if (!ok)
        d->labelWrapped->hide();
}

void QScriptDebuggerCodeFinderWidget::setWrapped(bool wrapped)
{
    Q_D(QScriptDebuggerCodeFinderWidget);
    d->labelWrapped->setVisible(wrapped);
}

void QScriptDebuggerCodeFinderWidget::keyPressEvent(QKeyEvent *e)
{
    if (e->key() == Qt::Key_Escape)
        hide();
    else
        QScriptDebuggerCodeFinderWidgetInterface::keyPressEvent(e);
}

QT_END_NAMESPACE

#include "moc_qscriptdebuggercodefinderwidget_p.h"
