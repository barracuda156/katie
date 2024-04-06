/****************************************************************************
**
** Copyright (C) 2023 Ivailo Monev
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

#include "qdatetime_p.h"
#include "qdatetimeedit_p.h"
#include "qdatetimeedit.h"
#include "qwidgetaction.h"
#include "qlineedit.h"

#ifndef QT_NO_DATETIMEEDIT

QT_BEGIN_NAMESPACE

static QTime getTime(const int value)
{
    return QDateTime::fromTime_t(value).time();
}

static int getTime_t(const QTime &value)
{
    return QDateTime(QDATETIMEEDIT_DATE_INITIAL, value).toTime_t();
}

static QValidator::State validateTime(const QLocale &locale, QString &input, int &pos)
{
    const QTime time = locale.toTime(input, QLocale::ShortFormat);
    if (!time.isValid()) {
        // try to auto-correct "1:53:25" to "10:53:25" when two digits have been selected ala
        // "fixup", the fixup could be done based on the cursor position but meh
        QString inputwithzero = input;
        inputwithzero.insert(pos, locale.zeroDigit());
        const QTime timewithzero = locale.toTime(inputwithzero, QLocale::ShortFormat);
        if (timewithzero.isValid()) {
            input = inputwithzero;
            pos += 1;
            return QValidator::Acceptable;
        }
        return QValidator::Invalid;
    }
    return QValidator::Acceptable;
}

QTimeValidator::QTimeValidator(QObject *parent)
    : QValidator(parent)
{
}

QValidator::State QTimeValidator::validate(QString &input, int &pos) const
{
    return validateTime(locale(), input, pos);
}

QTimeBox::QTimeBox(QDateTimeEdit *parent)
    : QSpinBox(parent),
    m_datetimeedit(parent),
    m_validator(nullptr)
{
    m_validator = new QTimeValidator(this);
    updateLocale(m_datetimeedit->locale());
}

void QTimeBox::updateLocale(const QLocale &locale)
{
    const QString timeformat = locale.timeFormat(QLocale::ShortFormat);
    if (!timeformat.contains(QLatin1String("ss"))) {
        setSingleStep(60);
    } else {
        setSingleStep(1);
    }
    m_validator->setLocale(locale);
    lineEdit()->setValidator(m_validator);
}

QValidator::State QTimeBox::validate(QString &input, int &pos) const
{
    return validateTime(m_datetimeedit->locale(), input, pos);
}

int QTimeBox::valueFromText(const QString &text) const
{
    return getTime_t(m_datetimeedit->locale().toTime(text, QLocale::ShortFormat));
}

QString QTimeBox::textFromValue(int value) const
{
    return m_datetimeedit->locale().toString(getTime(value), QLocale::ShortFormat);
}

/*!
  \internal
  Constructs a QDateTimeEditPrivate object
*/
QDateTimeEditPrivate::QDateTimeEditPrivate()
    : QWidgetPrivate(),
    calendarwidget(nullptr),
    m_showdate(true),
    m_showtime(true),
    m_layout(nullptr),
    m_timebox(nullptr),
    m_datebutton(nullptr),
    m_datemenu(nullptr),
    m_dateaction(nullptr)
{
}

void QDateTimeEditPrivate::init(const QDateTime &datetime, const bool showdate, const bool showtime)
{
    Q_Q(QDateTimeEdit);
    m_showdate = showdate;
    m_showtime = showtime;
    m_layout = new QHBoxLayout(q);
    m_timebox = new QTimeBox(q);
    m_layout->addWidget(m_timebox);
    m_datebutton = new QToolButton(q);
    m_datebutton->setToolButtonStyle(Qt::ToolButtonTextBesideIcon);
    m_datebutton->setSizePolicy(QSizePolicy::Minimum, QSizePolicy::Minimum);
    m_datebutton->setIcon(QIcon::fromTheme("x-office-calendar", QIcon::fromTheme("text-calendar")));
    m_layout->addWidget(m_datebutton);
    q->setLayout(m_layout);

    minimumdate = QDATETIMEEDIT_DATETIME_MIN;
    maximumdate = QDATETIMEEDIT_DATETIME_MAX;

    if (m_showdate) {
        m_datemenu = new QMenu(m_datebutton);
        m_dateaction = new QWidgetAction(m_datemenu);
        m_datemenu->addAction(m_dateaction);
        m_datebutton->setMenu(m_datemenu);
        setCalendar(new QCalendarWidget(m_datemenu));
        q->connect(m_datebutton, SIGNAL(pressed()), q, SLOT(_q_selectDate()));
    }

    updateWidgets(datetime);

    q->connect(m_timebox, SIGNAL(valueChanged(int)), q, SLOT(_q_timeChanged()));
}

void QDateTimeEditPrivate::updateWidgets(const QDateTime &datetime)
{
    Q_ASSERT(m_showdate || m_showtime);
    if (m_showdate) {
        calendarwidget->setMinimumDate(minimumdate.date());
        calendarwidget->setMaximumDate(maximumdate.date());
        m_datebutton->show();
    } else {
        m_datebutton->hide();
    }
    if (m_showtime) {
        m_timebox->setMinimum(getTime_t(minimumdate.time()));
        m_timebox->setMaximum(getTime_t(maximumdate.time()));
        m_timebox->show();
    } else {
        m_timebox->hide();
    }

    if (m_showdate) {
        const QDate curdate = datetime.date();
        calendarwidget->setSelectedDate(curdate);
        updateButton(curdate);
    }
    if (m_showtime) {
        m_timebox->setValue(getTime_t(datetime.time()));
        m_datebutton->setToolButtonStyle(Qt::ToolButtonIconOnly);
    } else {
        m_datebutton->setToolButtonStyle(Qt::ToolButtonTextBesideIcon);
    }
}

void QDateTimeEditPrivate::updateButton(const QDate &date)
{
    m_datebutton->setText(calendarwidget->locale().toString(date));
}

void QDateTimeEditPrivate::updateLocale(const QLocale &locale)
{
    m_timebox->updateLocale(locale);
    if (calendarwidget) {
        calendarwidget->setLocale(locale);
    }
}

void QDateTimeEditPrivate::setCalendar(QCalendarWidget *calendar)
{
    Q_Q(QDateTimeEdit);
    if (!m_showdate) {
        return;
    }
    if (calendarwidget) {
        q->disconnect(calendarwidget, 0, q, 0);
    }
    calendarwidget = calendar;
    m_datemenu->removeAction(m_dateaction);
    m_dateaction->setDefaultWidget(calendarwidget);
    m_datemenu->addAction(m_dateaction);
    q->connect(calendarwidget, SIGNAL(selectionChanged()), q, SLOT(_q_dateChanged()));
}

QDateTime QDateTimeEditPrivate::currentDateTime() const
{
    return QDateTime(
        calendarwidget ? calendarwidget->selectedDate() : QDATETIMEEDIT_DATE_INITIAL,
        getTime(m_timebox->value())
    );
}

void QDateTimeEditPrivate::_q_dateChanged()
{
    Q_Q(QDateTimeEdit);
    Q_ASSERT(m_showdate);
    const QDateTime curdatetime = currentDateTime();
    const QDate curdate = curdatetime.date();
    m_datemenu->hide();
    updateButton(curdate);
    emit q->dateTimeChanged(curdatetime);
    emit q->dateChanged(curdate);
}

void QDateTimeEditPrivate::_q_timeChanged()
{
    Q_Q(QDateTimeEdit);
    Q_ASSERT(m_showtime);
    const QDateTime curdatetime = currentDateTime();
    emit q->dateTimeChanged(curdatetime);
    emit q->timeChanged(curdatetime.time());
}

void QDateTimeEditPrivate::_q_selectDate()
{
    m_datebutton->showMenu();
}

/*!
  \class QDateTimeEdit
  \brief The QDateTimeEdit class provides a widget for editing dates and times.

  \ingroup basicwidgets


  QDateTimeEdit allows the user to edit dates by using the keyboard or
  the arrow keys to increase and decrease date and time values.

  The minimum value for QDateTimeEdit is 14 September 1752,
  and 2 January 4713BC for QDate. You can change this by calling
  setDateTimeRange(), setDateRange(), and setTimeRange().

  \sa QDateEdit, QTimeEdit, QDate, QTime
*/

/*!
  \fn void QDateTimeEdit::dateTimeChanged(const QDateTime &datetime)

  This signal is emitted whenever the date or time is changed. The
  new date and time is passed in \a datetime.
*/

/*!
  \fn void QDateTimeEdit::timeChanged(const QTime &time)

  This signal is emitted whenever the time is changed. The new time
  is passed in \a time.
*/

/*!
  \fn void QDateTimeEdit::dateChanged(const QDate &date)

  This signal is emitted whenever the date is changed. The new date
  is passed in \a date.
*/

/*!
  Constructs an empty date time editor with a \a parent.
*/
QDateTimeEdit::QDateTimeEdit(QWidget *parent)
    : QWidget(*new QDateTimeEditPrivate(), parent, 0)
{
    Q_D(QDateTimeEdit);
    d->init(QDateTime(QDATETIMEEDIT_DATE_INITIAL, QDATETIMEEDIT_TIME_MIN), true, true);
}

/*!
  Constructs an empty date time editor with a \a parent. The value
  is set to \a datetime.
*/
QDateTimeEdit::QDateTimeEdit(const QDateTime &datetime, QWidget *parent)
    : QWidget(*new QDateTimeEditPrivate(), parent, 0)
{
    Q_D(QDateTimeEdit);
    d->init(datetime, true, true);
}

/*!
  \fn QDateTimeEdit::QDateTimeEdit(const QDate &date, QWidget *parent)

  Constructs an empty date time editor with a \a parent.
  The value is set to \a date.
*/
QDateTimeEdit::QDateTimeEdit(const QDate &date, QWidget *parent)
    : QWidget(*new QDateTimeEditPrivate(), parent, 0)
{
    Q_D(QDateTimeEdit);
    d->init(QDateTime(date, QDATETIMEEDIT_TIME_MIN), true, false);
}

/*!
  \fn QDateTimeEdit::QDateTimeEdit(const QTime &time, QWidget *parent)

  Constructs an empty date time editor with a \a parent.
  The value is set to \a time.
*/
QDateTimeEdit::QDateTimeEdit(const QTime &time, QWidget *parent)
    : QWidget(*new QDateTimeEditPrivate(), parent, 0)
{
    Q_D(QDateTimeEdit);
    d->init(QDateTime(QDATETIMEEDIT_DATE_INITIAL, time), false, true);
}

/*!
  \property QDateTimeEdit::dateTime
  \brief the QDateTime that is set in the QDateTimeEdit

  When setting this property the timespec of the QDateTimeEdit remains the same
  and the timespec of the new QDateTime is ignored.

  By default, this property contains a date that refers to January 1,
  2000 and a time of 00:00:00 and 0 milliseconds.

  \sa date, time
*/
QDateTime QDateTimeEdit::dateTime() const
{
    Q_D(const QDateTimeEdit);
    return d->currentDateTime();
}

void QDateTimeEdit::setDateTime(const QDateTime &datetime)
{
    Q_D(QDateTimeEdit);
    if (datetime.isValid()) {
        if (datetime < d->minimumdate || datetime > d->maximumdate) {
            qWarning("QDateTimeEdit::setDateTime: date/time is out of range");
            return;
        }
        d->updateWidgets(datetime);
    }
}

/*!
  \property QDateTimeEdit::date
  \brief the QDate that is set in the widget

  By default, this property contains a date that refers to January 1, 2000.

  \sa time, dateTime
*/

/*!
    Returns the date of the date time edit.
*/
QDate QDateTimeEdit::date() const
{
    return dateTime().date();
}

void QDateTimeEdit::setDate(const QDate &date)
{
    if (date.isValid()) {
        setDateTime(QDateTime(date, time()));
    }
}

/*!
  \property QDateTimeEdit::time
  \brief the QTime that is set in the widget

  By default, this property contains a time of 00:00:00 and 0 milliseconds.

  \sa date, dateTime
*/

/*!
    Returns the time of the date time edit.
*/
QTime QDateTimeEdit::time() const
{
    return dateTime().time();
}

void QDateTimeEdit::setTime(const QTime &time)
{
    if (time.isValid()) {
        setDateTime(QDateTime(date(), time));
    }
}

/*!
  \property QDateTimeEdit::minimumDateTime
  \since 4.4

  \brief the minimum datetime of the date time edit

  When setting this property the \l maximumDateTime() is adjusted if
  necessary to ensure that the range remains valid. If the datetime is
  not a valid QDateTime object, this function does nothing.

  By default, this property contains a date that refers to September 14,
  1752 and a time of 00:00:00 and 0 milliseconds.

  \sa maximumDateTime(), minimumTime(), maximumTime(), minimumDate(),
  maximumDate(), setDateTimeRange(), setDateRange(), setTimeRange()
*/
QDateTime QDateTimeEdit::minimumDateTime() const
{
    Q_D(const QDateTimeEdit);
    return d->minimumdate;
}

/*!
  \property QDateTimeEdit::maximumDateTime
  \since 4.4

  \brief the maximum datetime of the date time edit

  When setting this property the \l minimumDateTime() is adjusted if
  necessary to ensure that the range remains valid. If the datetime is
  not a valid QDateTime object, this function does nothing.

  By default, this property contains a date that refers to 31 December,
  7999 and a time of 23:59:59 and 999 milliseconds.

  \sa minimumDateTime(), minimumTime(), maximumTime(), minimumDate(),
  maximumDate(), setDateTimeRange(), setDateRange(), setTimeRange()
*/
QDateTime QDateTimeEdit::maximumDateTime() const
{
    Q_D(const QDateTimeEdit);
    return d->maximumdate;
}

/*!
  Convenience function to set minimum and maximum date time with one
  function call.
  \since 4.4

  If either \a min or \a max are not valid, this function does
  nothing.

  \sa setDateRange(), setTimeRange(), QDateTime::isValid()
*/
void QDateTimeEdit::setDateTimeRange(const QDateTime &min, const QDateTime &max)
{
    Q_D(QDateTimeEdit);
    if (min > max) {
        qWarning("QDateTimeEdit::setDateTimeRange: minimum is greater than maximum");
        return;
    }
    d->minimumdate = min;
    d->maximumdate = max;
    d->updateWidgets(dateTime());
}

/*!
  \property QDateTimeEdit::minimumDate

  \brief the minimum date of the date time edit

  When setting this property the \l maximumDate is adjusted if
  necessary, to ensure that the range remains valid. If the date is
  not a valid QDate object, this function does nothing.

  By default, this property contains a date that refers to September 14, 1752.

  \sa minimumTime(), maximumTime(), setDateRange()
*/
QDate QDateTimeEdit::minimumDate() const
{
    return minimumDateTime().date();
}

/*!
  \property QDateTimeEdit::maximumDate

  \brief the maximum date of the date time edit

  When setting this property the \l minimumDate is adjusted if
  necessary to ensure that the range remains valid. If the date is
  not a valid QDate object, this function does nothing.

  By default, this property contains a date that refers to December 31, 7999.

  \sa minimumDate, minimumTime, maximumTime, setDateRange()
*/
QDate QDateTimeEdit::maximumDate() const
{
    return maximumDateTime().date();
}

/*!
  \property QDateTimeEdit::minimumTime

  \brief the minimum time of the date time edit

  When setting this property the \l maximumTime is adjusted if
  necessary, to ensure that the range remains valid. If the time is
  not a valid QTime object, this function does nothing.

  By default, this property contains a time of 00:00:00 and 0 milliseconds.

  \sa maximumTime, minimumDate, maximumDate, setTimeRange()
*/
QTime QDateTimeEdit::minimumTime() const
{
    return minimumDateTime().time();
}

/*!
  \property QDateTimeEdit::maximumTime

  \brief the maximum time of the date time edit

  When setting this property, the \l minimumTime is adjusted if
  necessary to ensure that the range remains valid. If the time is
  not a valid QTime object, this function does nothing.

  By default, this property contains a time of 23:59:59 and 999 milliseconds.

  \sa minimumTime, minimumDate, maximumDate, setTimeRange()
*/
QTime QDateTimeEdit::maximumTime() const
{
    return maximumDateTime().time();
}

/*!
  Convenience function to set minimum and maximum date with one
  function call.

  If either \a min or \a max are not valid, this function does
  nothing.

  \sa setDateTimeRange(), setTimeRange(), QDate::isValid()
*/
void QDateTimeEdit::setDateRange(const QDate &min, const QDate &max)
{
    if (min.isValid() && max.isValid()) {
        setDateTimeRange(QDateTime(min, minimumTime()), QDateTime(max, maximumTime()));
    }
}

/*!
  Convenience function to set minimum and maximum time with one
  function call.

  If either \a min or \a max are not valid, this function does
  nothing.

  \sa setDateTimeRange(), setDateRange(), QTime::isValid()
*/
void QDateTimeEdit::setTimeRange(const QTime &min, const QTime &max)
{
    if (min.isValid() && max.isValid()) {
        setDateTimeRange(QDateTime(minimumDate(), min), QDateTime(maximumDate(), max));
    }
}

/*!
  \since 4.4

  \brief Returns the calendar widget for the editor.
 */
QCalendarWidget *QDateTimeEdit::calendarWidget() const
{
    Q_D(const QDateTimeEdit);
    return d->calendarwidget;
}

/*!
  \since 4.4

  Sets the given \a calendarWidget as the widget to be used for the calendar
  pop-up. The editor does not automatically take ownership of the calendar widget.
*/
void QDateTimeEdit::setCalendarWidget(QCalendarWidget *calendarWidget)
{
    Q_D(QDateTimeEdit);
    if (!calendarWidget) {
        qWarning("QDateTimeEdit::setCalendarWidget: null calendar widget");
        return;
    }
    d->setCalendar(calendarWidget);
}

/*!
  \reimp
*/
bool QDateTimeEdit::event(QEvent *event)
{
    Q_D(QDateTimeEdit);
    switch (event->type()) {
        case QEvent::LocaleChange: {
            d->updateLocale(locale());
            break;
        }
        default: {
            break;
        }
    }
    return QWidget::event(event);
}

/*!
  \class QTimeEdit
  \brief The QTimeEdit class provides a widget for editing times based on
  the QDateTimeEdit widget.

  \ingroup basicwidgets


  Many of the properties and functions provided by QTimeEdit are implemented in
  QDateTimeEdit. The following properties are most relevant to users of this
  class:

  \list
  \o \l{QDateTimeEdit::time}{time} holds the date displayed by the widget.
  \o \l{QDateTimeEdit::minimumTime}{minimumTime} defines the minimum (earliest) time
     that can be set by the user.
  \o \l{QDateTimeEdit::maximumTime}{maximumTime} defines the maximum (latest) time
     that can be set by the user.
  \o \l{QDateTimeEdit::displayFormat}{displayFormat} contains a string that is used
     to format the time displayed in the widget.
  \endlist

  \sa QDateEdit, QDateTimeEdit
*/

/*!
  Constructs an empty time editor with a \a parent.
*/
QTimeEdit::QTimeEdit(QWidget *parent)
    : QDateTimeEdit(QDATETIMEEDIT_TIME_MIN, parent)
{
}

/*!
  Constructs an empty time editor with a \a parent. The time is set
  to \a time.
*/
QTimeEdit::QTimeEdit(const QTime &time, QWidget *parent)
    : QDateTimeEdit(time, parent)
{
}


/*!
  \class QDateEdit
  \brief The QDateEdit class provides a widget for editing dates based on
  the QDateTimeEdit widget.

  \ingroup basicwidgets


  Many of the properties and functions provided by QDateEdit are implemented in
  QDateTimeEdit. The following properties are most relevant to users of this
  class:

  \list
  \o \l{QDateTimeEdit::date}{date} holds the date displayed by the widget.
  \o \l{QDateTimeEdit::minimumDate}{minimumDate} defines the minimum (earliest)
     date that can be set by the user.
  \o \l{QDateTimeEdit::maximumDate}{maximumDate} defines the maximum (latest) date
     that can be set by the user.
  \o \l{QDateTimeEdit::displayFormat}{displayFormat} contains a string that is used
     to format the date displayed in the widget.
  \endlist

  \sa QTimeEdit, QDateTimeEdit
*/

/*!
  Constructs an empty date editor with a \a parent.
*/
QDateEdit::QDateEdit(QWidget *parent)
    : QDateTimeEdit(QDate(), parent)
{
}

/*!
  Constructs an empty date editor with a \a parent. The date is set
  to \a date.
*/
QDateEdit::QDateEdit(const QDate &date, QWidget *parent)
    : QDateTimeEdit(date, parent)
{
}

QT_END_NAMESPACE

#include "moc_qdatetimeedit.h"
#include "moc_qdatetimeedit_p.h"

#endif // QT_NO_DATETIMEEDIT
