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

#include "qplatformdefs.h"
#include "qitemeditorfactory.h"
#include "qitemeditorfactory_p.h"

#ifndef QT_NO_ITEMVIEWS

#include "qcombobox.h"
#include "qdatetimeedit.h"
#include "qfontcombobox.h"
#include "qlabel.h"
#include "qlineedit.h"
#include "qspinbox.h"
#include "qapplication.h"
#include "qdebug.h"

#include <limits.h>
#include <float.h>

QT_BEGIN_NAMESPACE

static std::unique_ptr<QItemEditorFactory> q_default_factory(nullptr);

/*!
    \class QItemEditorFactory
    \brief The QItemEditorFactory class provides widgets for editing item data
    in views and delegates.
    \since 4.2
    \ingroup model-view

    When editing data in an item view, editors are created and displayed by a
    delegate. QItemDelegate, which is the delegate by default installed on
    Katie's item views, uses a QItemEditorFactory to create editors for it.
    A default unique instance provided by QItemEditorFactory is used by all
    item delegates.

    A factory has specialized editors for particular QVariant data type
    (All Katie models store their data in \l{QVariant}s).

    \section1 Standard Editing Widgets

    The standard factory implementation provides editors for a variety of data
    types. These are created whenever a delegate needs to provide an editor for
    data supplied by a model. The following table shows the relationship between
    types and the standard editors provided.

    \table
    \header \o Type \o Editor Widget
    \row    \o bool \o QComboBox
    \row    \o double \o QDoubleSpinBox
    \row    \o int \o{1,2} QSpinBox
    \row    \o unsigned int
    \row    \o QDate \o QDateEdit
    \row    \o QDateTime \o QDateTimeEdit
    \row    \o QString \o QLineEdit
    \row    \o QTime \o QTimeEdit
    \row    \o QFont \o QFontComboBox
    \endtable

    \sa QItemDelegate, {Model/View Programming}, {Color Editor Factory Example}
*/

/*!
    Constructs a new item editor factory.
*/
QItemEditorFactory::QItemEditorFactory()
{
}

/*!
    Creates an editor widget with the given \a parent for the specified
    \a variant and returns it as a QWidget.
*/
QWidget *QItemEditorFactory::createEditor(const QVariant &variant, QWidget *parent) const
{
    switch (variant.type()) {
#ifndef QT_NO_COMBOBOX
    case QVariant::Bool: {
        QBooleanComboBox *cb = new QBooleanComboBox(parent);
        cb->setFrame(false);
        cb->setValue(variant.toBool());
        return cb;
    }
#endif
#ifndef QT_NO_SPINBOX
    case QVariant::UInt: {
        QSpinBox *sb = new QSpinBox(parent);
        sb->setFrame(false);
        sb->setMaximum(INT_MAX);
        sb->setValue(variant.toUInt());
        return sb;
    }
    case QVariant::Int: {
        QSpinBox *sb = new QSpinBox(parent);
        sb->setFrame(false);
        sb->setMinimum(INT_MIN);
        sb->setMaximum(INT_MAX);
        sb->setValue(variant.toInt());
        return sb;
    }
#endif
#ifndef QT_NO_DATETIMEEDIT
    case QVariant::Date: {
        QDateEdit *ed = new QDateEdit(parent);
        ed->setDate(variant.toDate());
        return ed;
    }
    case QVariant::Time: {
        QTimeEdit *ed = new QTimeEdit(parent);
        ed->setTime(variant.toTime());
        return ed;
    }
    case QVariant::DateTime: {
        QDateTimeEdit *ed = new QDateTimeEdit(parent);
        ed->setDateTime(variant.toDateTime());
        return ed;
    }
#endif
#ifndef QT_NO_SPINBOX
    case QVariant::Double: {
        QDoubleSpinBox *sb = new QDoubleSpinBox(parent);
        sb->setFrame(false);
        sb->setMinimum(-DBL_MAX);
        sb->setMaximum(DBL_MAX);
        sb->setValue(variant.toDouble());
        return sb;
    }
#endif
#ifndef QT_NO_FONTCOMBOBOX
    case QVariant::Font: {
        QFontComboBox *fb = new QFontComboBox(parent);
        fb->setFrame(false);
        fb->setCurrentFont(qvariant_cast<QFont>(variant));
        return fb;
    }
#endif
#ifndef QT_NO_LINEEDIT
    case QVariant::String:
    default: {
        // the default editor is a lineedit
        QExpandingLineEdit *le = new QExpandingLineEdit(parent);
        le->setFrame(le->style()->styleHint(QStyle::SH_ItemView_DrawDelegateFrame, 0, le));
        if (!le->style()->styleHint(QStyle::SH_ItemView_ShowDecorationSelected, 0, le))
            le->setWidgetOwnsGeometry(true);
        le->setText(variant.toString());
        return le;
    }
#else
    default:
        break;
#endif
    }
    return nullptr;
}

/*!
    Returns the property name used to access data for the given \a type of data.
*/
QByteArray QItemEditorFactory::valuePropertyName(QVariant::Type type) const
{
    switch (type) {
#ifndef QT_NO_COMBOBOX
    case QVariant::Bool:
        return "value";
#endif
#ifndef QT_NO_SPINBOX
    case QVariant::UInt:
    case QVariant::Int:
    case QVariant::Double:
        return "value";
#endif
#ifndef QT_NO_DATETIMEEDIT
    case QVariant::Date:
        return "date";
    case QVariant::Time:
        return "time";
    case QVariant::DateTime:
        return "dateTime";
#endif
#ifndef QT_NO_FONTCOMBOBOX
    case QVariant::Font:
        return "currentFont";
#endif
    case QVariant::String:
    default:
        // the default editor is a lineedit
        return "text";
    }
}

/*!
    Destroys the item editor factory.
*/
QItemEditorFactory::~QItemEditorFactory()
{
}

/*!
    Returns the default item editor factory.

    \sa setDefaultFactory()
*/
const QItemEditorFactory *QItemEditorFactory::defaultFactory()
{
    if (!q_default_factory) {
        q_default_factory = std::make_unique<QItemEditorFactory>();
    }
    return q_default_factory.get();
}

#ifndef QT_NO_LINEEDIT
QExpandingLineEdit::QExpandingLineEdit(QWidget *parent)
    : QLineEdit(parent), originalWidth(-1), widgetOwnsGeometry(false)
{
    connect(this, SIGNAL(textChanged(QString)), this, SLOT(resizeToContents()));
    updateMinimumWidth();
}

void QExpandingLineEdit::changeEvent(QEvent *e)
{
    switch (e->type())
    {
    case QEvent::FontChange:
    case QEvent::StyleChange:
    case QEvent::ContentsRectChange:
        updateMinimumWidth();
        break;
    default:
        break;
    }

    QLineEdit::changeEvent(e);
}

void QExpandingLineEdit::updateMinimumWidth()
{
    int left, right;
    getTextMargins(&left, 0, &right, 0);
    int width = left + right + 4 /*horizontalMargin in qlineedit.cpp*/;
    getContentsMargins(&left, 0, &right, 0);
    width += left + right;

    QStyleOptionFrame opt;
    initStyleOption(&opt);
    
    int minWidth = style()->sizeFromContents(QStyle::CT_LineEdit, &opt, QSize(width, 0).
                                      expandedTo(QApplication::globalStrut()), this).width();
    setMinimumWidth(minWidth);
}

void QExpandingLineEdit::resizeToContents()
{
    int oldWidth = width();
    if (originalWidth == -1)
        originalWidth = oldWidth;
    if (QWidget *parent = parentWidget()) {
        QPoint position = pos();
        int hintWidth = minimumWidth() + fontMetrics().width(displayText());
        int parentWidth = parent->width();
        int maxWidth = isRightToLeft() ? position.x() + oldWidth : parentWidth - position.x();
        int newWidth = qBound(originalWidth, hintWidth, maxWidth);
        if (widgetOwnsGeometry)
            setMaximumWidth(newWidth);
        if (isRightToLeft())
            move(position.x() - newWidth + oldWidth, position.y());
        resize(newWidth, height());
    }
}
#endif // QT_NO_LINEEDIT

#ifndef QT_NO_COMBOBOX
QBooleanComboBox::QBooleanComboBox(QWidget *parent)
    : QComboBox(parent)
{
    addItem(QComboBox::tr("False"));
    addItem(QComboBox::tr("True"));
}

void QBooleanComboBox::setValue(bool value)
{
    setCurrentIndex(value ? 1 : 0);
}

bool QBooleanComboBox::value() const
{
    return (currentIndex() == 1);
}
#endif // QT_NO_COMBOBOX

QT_END_NAMESPACE

#if !defined(QT_NO_LINEEDIT) || !defined(QT_NO_COMBOBOX)
#include "moc_qitemeditorfactory_p.h"
#endif

#endif // QT_NO_ITEMVIEWS
