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

#include "qsizepolicy.h"
#include "qvariant.h"

QT_BEGIN_NAMESPACE

/*!
    \class QSizePolicy
    \brief The QSizePolicy class is a layout attribute describing horizontal
    and vertical resizing policy.

    \ingroup geomanagement

    The size policy of a widget is an expression of its willingness to
    be resized in various ways, and affects how the widget is treated
    by the \l{Layout Management}{layout engine}. Each widget returns a
    QSizePolicy that describes the horizontal and vertical resizing
    policy it prefers when being laid out. You can change this for
    a specific widget by changing its QWidget::sizePolicy property.

    QSizePolicy contains two independent QSizePolicy::Policy values
    and two stretch factors; one describes the widgets's horizontal
    size policy, and the other describes its vertical size policy. It
    also contains a flag to indicate whether the height and width of
    its preferred size are related.

    The horizontal and vertical policies can be set in the
    constructor, and altered using the setHorizontalPolicy() and
    setVerticalPolicy() functions. The stretch factors can be set
    using the setHorizontalStretch() and setVerticalStretch()
    functions. The flag indicating whether the widget's
    \l{QWidget::sizeHint()}{sizeHint()} is width-dependent (such as a
    menu bar or a word-wrapping label) can be set using the
    setHeightForWidth() function.

    The current size policies and stretch factors be retrieved using
    the horizontalPolicy(), verticalPolicy(), horizontalStretch() and
    verticalStretch() functions. Alternatively, use the transpose()
    function to swap the horizontal and vertical policies and
    stretches. The hasHeightForWidth() function returns the current
    status of the flag indicating the size hint dependencies.

    Use the expandingDirections() function to determine whether the
    associated widget can make use of more space than its
    \l{QWidget::sizeHint()}{sizeHint()} function indicates, as well as
    find out in which directions it can expand.

    Finally, the QSizePolicy class provides operators comparing this
    size policy to a given policy, as well as a QVariant operator
    storing this QSizePolicy as a QVariant object.

    \sa QSize, QWidget::sizeHint(), QWidget::sizePolicy,
    QLayoutItem::sizeHint()
*/

/*!
    \enum QSizePolicy::PolicyFlag

    These flags are combined together to form the various \l{Policy}
    values:

    \value GrowFlag  The widget can grow beyond its size hint if necessary.
    \value ExpandFlag  The widget should get as much space as possible.
    \value ShrinkFlag  The widget can shrink below its size hint if necessary.
    \value IgnoreFlag  The widget's size hint is ignored. The widget will get
        as much space as possible.

    \sa Policy
*/

/*!
    \enum QSizePolicy::Policy

    This enum describes the various per-dimension sizing types used
    when constructing a QSizePolicy.

    \value Fixed  The QWidget::sizeHint() is the only acceptable
        alternative, so the widget can never grow or shrink (e.g. the
        vertical direction of a push button).

    \value Minimum  The sizeHint() is minimal, and sufficient. The
        widget can be expanded, but there is no advantage to it being
        larger (e.g. the horizontal direction of a push button).
        It cannot be smaller than the size provided by sizeHint().

    \value Maximum  The sizeHint() is a maximum. The widget can be
        shrunk any amount without detriment if other widgets need the
        space (e.g. a separator line).
        It cannot be larger than the size provided by sizeHint().

    \value Preferred  The sizeHint() is best, but the widget can be
        shrunk and still be useful. The widget can be expanded, but there
        is no advantage to it being larger than sizeHint() (the default
        QWidget policy).

    \value Expanding  The sizeHint() is a sensible size, but the
        widget can be shrunk and still be useful. The widget can make use
        of extra space, so it should get as much space as possible (e.g.
        the horizontal direction of a horizontal slider).

    \value MinimumExpanding  The sizeHint() is minimal, and sufficient.
        The widget can make use of extra space, so it should get as much
        space as possible (e.g. the horizontal direction of a horizontal
        slider).

    \value Ignored  The sizeHint() is ignored. The widget will get as
        much space as possible.

    \sa PolicyFlag, setHorizontalPolicy(), setVerticalPolicy()
*/

/*!
    \fn QSizePolicy::QSizePolicy()

    Constructs a QSizePolicy object with \l Fixed as its horizontal
    and vertical policies.

    The policies can be altered using the setHorizontalPolicy() and
    setVerticalPolicy() functions. Use the setHeightForWidth()
    function if the preferred height of the widget is dependent on the
    width of the widget (for example, a QLabel with line wrapping).

    \sa setHorizontalStretch(), setVerticalStretch()
*/

/*!
    \fn QSizePolicy::QSizePolicy(Policy horizontal, Policy vertical)

    Constructs a QSizePolicy object with the given \a horizontal and
    \a vertical policies, and DefaultType as the control type.

    Use setHeightForWidth() if the preferred height of the widget is
    dependent on the width of the widget (for example, a QLabel with
    line wrapping).

    \sa setHorizontalStretch(), setVerticalStretch()
*/

/*!
    \fn QSizePolicy::QSizePolicy(Policy horizontal, Policy vertical, ControlType type)
    \since 4.3

    Constructs a QSizePolicy object with the given \a horizontal and
    \a vertical policies, and the specified control \a type.

    Use setHeightForWidth() if the preferred height of the widget is
    dependent on the width of the widget (for example, a QLabel with
    line wrapping).

    \sa setHorizontalStretch(), setVerticalStretch(), controlType()
*/

/*!
    \fn QSizePolicy::Policy QSizePolicy::horizontalPolicy() const

    Returns the horizontal component of the size policy.

    \sa setHorizontalPolicy(), verticalPolicy(), horizontalStretch()
*/

/*!
    \fn QSizePolicy::Policy QSizePolicy::verticalPolicy() const

    Returns the vertical component of the size policy.

    \sa setVerticalPolicy(), horizontalPolicy(), verticalStretch()
*/

/*!
    \fn void QSizePolicy::setHorizontalPolicy(Policy policy)

    Sets the horizontal component to the given \a policy.

    \sa horizontalPolicy(), setVerticalPolicy(), setHorizontalStretch()
*/

/*!
    \fn void QSizePolicy::setVerticalPolicy(Policy policy)

    Sets the vertical component to the given \a policy.

    \sa verticalPolicy(), setHorizontalPolicy(), setVerticalStretch()
*/

/*!
    \fn Qt::Orientations QSizePolicy::expandingDirections() const

    Returns whether a widget can make use of more space than the
    QWidget::sizeHint() function indicates.

    A value of Qt::Horizontal or Qt::Vertical means that the widget
    can grow horizontally or vertically (i.e., the horizontal or
    vertical policy is \l Expanding or \l MinimumExpanding), whereas
    Qt::Horizontal | Qt::Vertical means that it can grow in both
    dimensions.

    \sa horizontalPolicy(), verticalPolicy()
*/

/*!
    \fn ControlType QSizePolicy::controlType() const
    \since 4.3

    Returns the control type associated with the widget for which
    this size policy applies.
*/

/*!
    \fn void QSizePolicy::setControlType(ControlType type)
    \since 4.3

    Sets the control type associated with the widget for which this
    size policy applies to \a type.

    The control type specifies the type of the widget for which this
    size policy applies. It is used by some styles to insert proper
    spacing between widgets.

    \sa QStyle::layoutSpacing()
*/

/*!
    \fn void QSizePolicy::setHeightForWidth(bool dependent)

    Sets the flag determining whether the widget's preferred height
    depends on its width, to \a dependent.

    \sa hasHeightForWidth(), setWidthForHeight()
*/

/*!
    \fn bool QSizePolicy::hasHeightForWidth() const

    Returns true if the widget's preferred height depends on its
    width; otherwise returns false.

    \sa setHeightForWidth()
*/

/*!
    \fn void QSizePolicy::setWidthForHeight(bool dependent)
    \since 4.8

    Sets the flag determining whether the widget's width
    depends on its height, to \a dependent.

    This is only supported for QGraphicsLayout's subclasses.
    It is not possible to have a layout with both height-for-width
    and width-for-height constraints at the same time.

    \sa hasWidthForHeight(), setHeightForWidth()
*/

/*!
    \fn bool QSizePolicy::hasWidthForHeight() const
    \since 4.8

    Returns true if the widget's width depends on its
    height; otherwise returns false.

    \sa setWidthForHeight()
*/

/*!
    Returns true if this policy is equal to \a other; otherwise
    returns false.

    \sa operator!=()
*/
bool QSizePolicy::operator==(const QSizePolicy &s) const
{
    return (
        horzPolicy == s.horzPolicy &&
        vertPolicy == s.vertPolicy &&
        hfw == s.hfw &&
        wfh == s.wfh &&
        verStretch == s.verStretch &&
        horStretch == s.horStretch &&
        ctype == s.ctype
    );
}

/*!
    \fn bool QSizePolicy::operator!=(const QSizePolicy &other) const

    Returns true if this policy is different from \a other; otherwise
    returns false.

    \sa operator==()
*/

/*!
    \fn int QSizePolicy::horizontalStretch() const

    Returns the horizontal stretch factor of the size policy.

    \sa setHorizontalStretch(), verticalStretch(), horizontalPolicy()
*/

/*!
    \fn int QSizePolicy::verticalStretch() const

    Returns the vertical stretch factor of the size policy.

    \sa setVerticalStretch(), horizontalStretch(), verticalPolicy()
*/

/*!
    \fn void QSizePolicy::setHorizontalStretch(uchar stretchFactor)

    Sets the horizontal stretch factor of the size policy to the given \a
    stretchFactor.

    \sa horizontalStretch(), setVerticalStretch(), setHorizontalPolicy()
*/

/*!
    \fn void QSizePolicy::setVerticalStretch(uchar stretchFactor)

    Sets the vertical stretch factor of the size policy to the given
    \a stretchFactor.

    \sa verticalStretch(), setHorizontalStretch(), setVerticalPolicy()
*/

/*!
    \enum QSizePolicy::ControlType
    \since 4.3

    This enum specifies the different types of widgets in terms of
    layout interaction:

    \value DefaultType  The default type, when none is specified.
    \value ButtonBox  A QDialogButtonBox instance.
    \value CheckBox  A QCheckBox instance.
    \value ComboBox  A QComboBox instance.
    \value Frame  A QFrame instance.
    \value GroupBox  A QGroupBox instance.
    \value Label  A QLabel instance.
    \value Line  A QFrame instance with QFrame::HLine or QFrame::VLine.
    \value LineEdit  A QLineEdit instance.
    \value PushButton  A QPushButton instance.
    \value RadioButton  A QRadioButton instance.
    \value Slider  A QAbstractSlider instance.
    \value SpinBox  A QAbstractSpinBox instance.
    \value TabWidget  A QTabWidget instance.
    \value ToolButton  A QToolButton instance.

    \sa setControlType(), controlType()
*/

/*!
    Returns a QVariant storing this QSizePolicy.
*/
QSizePolicy::operator QVariant() const
{
    return QVariant(QVariant::SizePolicy, this);
}

/*!
    Swaps the horizontal and vertical policies and stretches.
*/
void QSizePolicy::transpose()
{
    Policy hData = horizontalPolicy();
    Policy vData = verticalPolicy();
    int hStretch = horizontalStretch();
    int vStretch = verticalStretch();
    setHorizontalPolicy(vData);
    setVerticalPolicy(hData);
    setHorizontalStretch(vStretch);
    setVerticalStretch(hStretch);
}

#ifndef QT_NO_DATASTREAM
/*!
    \relates QSizePolicy
    \since 4.2

    Writes the size \a policy to the data stream \a stream.

    \sa \link datastreamformat.html Format of the QDataStream operators \endlink
*/
QDataStream &operator<<(QDataStream &stream, const QSizePolicy &policy)
{
    stream << static_cast<quint32>(policy.horzPolicy);
    stream << static_cast<quint32>(policy.vertPolicy);
    stream << policy.hfw;
    stream << policy.wfh;
    stream << policy.verStretch;
    stream << policy.horStretch;
    stream << policy.ctype;
    return stream;
}

/*!
    \relates QSizePolicy
    \since 4.2

    Reads the size \a policy from the data stream \a stream.

    \sa \link datastreamformat.html Format of the QDataStream operators \endlink
*/
QDataStream &operator>>(QDataStream &stream, QSizePolicy &policy)
{
    quint32 horzPolicy = 0;
    quint32 vertPolicy = 0;
    quint32 ctype = 0;
    stream >> horzPolicy;
    stream >> vertPolicy;
    policy.horzPolicy = static_cast<QSizePolicy::Policy>(horzPolicy);
    policy.vertPolicy = static_cast<QSizePolicy::Policy>(vertPolicy);
    stream >> policy.hfw;
    stream >> policy.wfh;
    stream >> policy.verStretch;
    stream >> policy.horStretch;
    stream >> ctype;
    policy.ctype = static_cast<QSizePolicy::ControlType>(ctype);
    return stream;
}
#endif // QT_NO_DATASTREAM

QT_END_NAMESPACE
