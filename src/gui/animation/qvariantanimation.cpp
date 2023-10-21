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
** $QT_END_LICENSE$
**
****************************************************************************/

#include "qvariantanimation.h"
#include "qvariantanimation_p.h"
#include "qrect.h"
#include "qline.h"
#include "qcolor.h"
#include "qvector2d.h"
#include "qvector3d.h"
#include "qvector4d.h"

#ifndef QT_NO_ANIMATION

QT_BEGIN_NAMESPACE

/*!
    \class QVariantAnimation
    \ingroup animation
    \brief The QVariantAnimation class provides an abstract base class for animations.
    \since 4.6

    This class is part of \l{The Animation Framework}. It serves as a
    base class for property and item animations, with functions for
    shared functionality.

    QVariantAnimation cannot be used directly as it is an abstract
    class; it has a pure virtual method called updateCurrentValue().
    The class performs interpolation over
    \l{QVariant}s, but leaves using the interpolated values to its
    subclasses. Currently, Katie provides QPropertyAnimation, which
    animates Katie \l{Katie's Property System}{properties}. See the
    QPropertyAnimation class description if you wish to animate such
    properties.

    You can then set start and end values for the property by calling
    setStartValue() and setEndValue(), and finally call start() to
    start the animation. QVariantAnimation will interpolate the
    property of the target object and emit valueChanged(). To react to
    a change in the current value you have to reimplement the
    updateCurrentValue() virtual function.

    It is also possible to set values at specified steps situated
    between the start and end value. The interpolation will then
    touch these points at the specified steps. Note that the start and
    end values are defined as the key values at 0.0 and 1.0.

    There are two ways to affect how QVariantAnimation interpolates
    the values. You can set an easing curve by calling
    setEasingCurve(), and configure the duration by calling
    setDuration(). You can change how the QVariants are interpolated
    by creating a subclass of QVariantAnimation, and reimplementing
    the virtual interpolated() function.

    Subclassing QVariantAnimation can be an alternative if you have
    \l{QVariant}s that you do not wish to declare as Katie properties.
    Note, however, that you in most cases will be better off declaring
    your QVariant as a property.

    Not all QVariant types are supported. Below is a list of currently
    supported QVariant types:

    \list
        \o \l{QMetaType::}{Int}
        \o \l{QMetaType::}{UInt}
        \o \l{QMetaType::}{Double}
        \o \l{QMetaType::}{Float}
        \o \l{QMetaType::}{QLine}
        \o \l{QMetaType::}{QLineF}
        \o \l{QMetaType::}{QPoint}
        \o \l{QMetaType::}{QPointF}
        \o \l{QMetaType::}{QSize}
        \o \l{QMetaType::}{QSizeF}
        \o \l{QMetaType::}{QRect}
        \o \l{QMetaType::}{QRectF}
        \o \l{QMetaType::}{QColor}
        \o \l{QMetaType::}{QVector2D}
        \o \l{QMetaType::}{QVector3D}
        \o \l{QMetaType::}{QVector4D}
    \endlist

    If you need to interpolate other variant types, including custom
    types, you have to implement interpolation for these yourself.
    To do this you should reimplement interpolated(), which returns
    interpolation values for the value being interpolated.

    \sa QPropertyAnimation, QAbstractAnimation, {The Animation Framework}
*/

/*!
    \fn void QVariantAnimation::valueChanged(const QVariant &value)

    QVariantAnimation emits this signal whenever the current \a value changes.

    \sa currentValue, startValue, endValue
*/

/*!
    \fn void QVariantAnimation::updateCurrentValue(const QVariant &value) = 0;

    This pure virtual function is called every time the animation's current
    value changes. The \a value argument is the new current value.

    \sa currentValue
*/

static bool animationValueLessThan(const QVariantAnimation::KeyValue &p1, const QVariantAnimation::KeyValue &p2)
{
    return p1.first < p2.first;
}

static QVariant defaultInterpolator(const void *, const void *, qreal)
{
    return QVariant();
}

// this should make the interpolation faster
template<typename T>
inline T _q_interpolate(const T &f, const T &t, qreal progress)
{
    return T(f + (t - f) * progress);
}

template<typename T >
inline QVariant _q_interpolateVariant(const T &from, const T &to, qreal progress)
{
    return _q_interpolate(from, to, progress);
}

template<> Q_INLINE_TEMPLATE QRect _q_interpolate(const QRect &f, const QRect &t, qreal progress)
{
    return QRect(_q_interpolate(f.left(), t.left(), progress),
                 _q_interpolate(f.top(), t.top(), progress),
                 _q_interpolate(f.right(), t.right(), progress),
                 _q_interpolate(f.bottom(), t.bottom(), progress));
}

template<> Q_INLINE_TEMPLATE QRectF _q_interpolate(const QRectF &f, const QRectF &t, qreal progress)
{
    return QRectF(_q_interpolate(f.x(), t.x(), progress), _q_interpolate(f.y(), t.y(), progress),
                  _q_interpolate(f.width(), t.width(), progress), _q_interpolate(f.height(), t.height(), progress));
}

template<> Q_INLINE_TEMPLATE QLine _q_interpolate(const QLine &f, const QLine &t, qreal progress)
{
    return QLine( _q_interpolate(f.p1(), t.p1(), progress), _q_interpolate(f.p2(), t.p2(), progress));
}

template<> Q_INLINE_TEMPLATE QLineF _q_interpolate(const QLineF &f, const QLineF &t, qreal progress)
{
    return QLineF( _q_interpolate(f.p1(), t.p1(), progress), _q_interpolate(f.p2(), t.p2(), progress));
}

template<> Q_INLINE_TEMPLATE QColor _q_interpolate(const QColor &f,const QColor &t, qreal progress)
{
    return QColor(qBound(0,_q_interpolate(f.red(), t.red(), progress),255),
                  qBound(0,_q_interpolate(f.green(), t.green(), progress),255),
                  qBound(0,_q_interpolate(f.blue(), t.blue(), progress),255),
                  qBound(0,_q_interpolate(f.alpha(), t.alpha(), progress),255));
}

static QVariantAnimation::Interpolator getInterpolator(int interpolationType)
{
    switch(QMetaType::Type(interpolationType)) {
        case QMetaType::Int:
            return reinterpret_cast<QVariantAnimation::Interpolator>(_q_interpolateVariant<int>);
        case QMetaType::UInt:
            return reinterpret_cast<QVariantAnimation::Interpolator>(_q_interpolateVariant<uint>);
        case QMetaType::Double:
            return reinterpret_cast<QVariantAnimation::Interpolator>(_q_interpolateVariant<double>);
        case QMetaType::Float:
            return reinterpret_cast<QVariantAnimation::Interpolator>(_q_interpolateVariant<float>);
        case QMetaType::QLine:
            return reinterpret_cast<QVariantAnimation::Interpolator>(_q_interpolateVariant<QLine>);
        case QMetaType::QLineF:
            return reinterpret_cast<QVariantAnimation::Interpolator>(_q_interpolateVariant<QLineF>);
        case QMetaType::QPoint:
            return reinterpret_cast<QVariantAnimation::Interpolator>(_q_interpolateVariant<QPoint>);
        case QMetaType::QPointF:
            return reinterpret_cast<QVariantAnimation::Interpolator>(_q_interpolateVariant<QPointF>);
        case QMetaType::QSize:
            return reinterpret_cast<QVariantAnimation::Interpolator>(_q_interpolateVariant<QSize>);
        case QMetaType::QSizeF:
            return reinterpret_cast<QVariantAnimation::Interpolator>(_q_interpolateVariant<QSizeF>);
        case QMetaType::QRect:
            return reinterpret_cast<QVariantAnimation::Interpolator>(_q_interpolateVariant<QRect>);
        case QMetaType::QRectF:
            return reinterpret_cast<QVariantAnimation::Interpolator>(_q_interpolateVariant<QRectF>);
        case QMetaType::QColor:
            return reinterpret_cast<QVariantAnimation::Interpolator>(_q_interpolateVariant<QColor>);
#ifndef QT_NO_VECTOR2D
        case QMetaType::QVector2D:
            return reinterpret_cast<QVariantAnimation::Interpolator>(_q_interpolateVariant<QVector2D>);
#endif // QT_NO_VECTOR2D
#ifndef QT_NO_VECTOR3D
        case QMetaType::QVector3D:
            return reinterpret_cast<QVariantAnimation::Interpolator>(_q_interpolateVariant<QVector3D>);
#endif // QT_NO_VECTOR3D
#ifndef QT_NO_VECTOR4D
        case QMetaType::QVector4D:
            return reinterpret_cast<QVariantAnimation::Interpolator>(_q_interpolateVariant<QVector4D>);
#endif // QT_NO_VECTOR4D
        default:
            return nullptr; //this type is not handled
    }
}

QVariantAnimationPrivate::QVariantAnimationPrivate()
    : duration(250), interpolator(&defaultInterpolator)
{
}

void QVariantAnimationPrivate::convertValues(int t)
{
    // this ensures that all the keyValues are of type t
    for (int i = 0; i < keyValues.count(); ++i) {
        QVariantAnimation::KeyValue &pair = keyValues[i];
        pair.second.convert(static_cast<QVariant::Type>(t));
    }
    // we also need update to the current interval if needed
    currentInterval.start.second.convert(static_cast<QVariant::Type>(t));
    currentInterval.end.second.convert(static_cast<QVariant::Type>(t));

    // ... and the interpolator
    updateInterpolator();
}

void QVariantAnimationPrivate::updateInterpolator()
{
    int type = currentInterval.start.second.userType();
    if (type == currentInterval.end.second.userType()) {
        interpolator = getInterpolator(type);
    } else {
        interpolator = 0;
    }

    // we make sure that the interpolator is always set to something
    if (!interpolator) {
        interpolator = &defaultInterpolator;
    }
}

/*!
    \internal
    The goal of this function is to update the currentInterval member. As a consequence, we also
    need to update the currentValue.
    Set \a force to true to always recalculate the interval.
*/
void QVariantAnimationPrivate::recalculateCurrentInterval(bool force/*=false*/)
{
    // can't interpolate if we don't have at least 2 values
    if ((keyValues.count() + (defaultStartEndValue.isValid() ? 1 : 0)) < 2) {
        return;
    }

    const qreal endProgress = (direction == QAbstractAnimation::Forward) ? qreal(1) : qreal(0);
    const qreal progress = easing.valueForProgress(((duration == 0) ? endProgress : qreal(currentTime) / qreal(duration)));

    //0 and 1 are still the boundaries
    if (force || (currentInterval.start.first > 0 && progress < currentInterval.start.first)
        || (currentInterval.end.first < 1 && progress > currentInterval.end.first)) {
        //let's update currentInterval
        QVariantAnimation::KeyValues::const_iterator it = qLowerBound(keyValues.constBegin(),
                                                                      keyValues.constEnd(),
                                                                      qMakePair(progress, QVariant()),
                                                                      animationValueLessThan);
        if (it == keyValues.constBegin()) {
            //the item pointed to by it is the start element in the range    
            if (it->first == 0 && keyValues.count() > 1) {
                currentInterval.start = *it;
                currentInterval.end = *(it+1);
            } else {
                currentInterval.start = qMakePair(qreal(0), defaultStartEndValue);
                currentInterval.end = *it;
            }
        } else if (it == keyValues.constEnd()) {
            --it; //position the iterator on the last item
            if (it->first == 1 && keyValues.count() > 1) {
                //we have an end value (item with progress = 1)
                currentInterval.start = *(it-1);
                currentInterval.end = *it;
            } else {
                //we use the default end value here
                currentInterval.start = *it;
                currentInterval.end = qMakePair(qreal(1), defaultStartEndValue);
            }
        } else {
            currentInterval.start = *(it-1);
            currentInterval.end = *it;
        }

        // update all the values of the currentInterval
        updateInterpolator();
    }
    setCurrentValueForProgress(progress);
}

void QVariantAnimationPrivate::setCurrentValueForProgress(const qreal progress)
{
    Q_Q(QVariantAnimation);

    const qreal startProgress = currentInterval.start.first;
    const qreal endProgress = currentInterval.end.first;
    const qreal localProgress = (progress - startProgress) / (endProgress - startProgress);

    QVariant ret = q->interpolated(currentInterval.start.second,
                                   currentInterval.end.second,
                                   localProgress);
    qSwap(currentValue, ret);
    q->updateCurrentValue(currentValue);
    static QAtomicInt changedSignalIndex(0);
    if (!changedSignalIndex) {
        // we keep the mask so that we emit valueChanged only when needed (for performance reasons)
        changedSignalIndex.testAndSetRelaxed(0, signalIndex("valueChanged(QVariant)"));
    }
    if (isSignalConnected(changedSignalIndex) && currentValue != ret) {
        // the value has changed
        emit q->valueChanged(currentValue);
    }
}

QVariant QVariantAnimationPrivate::valueAt(qreal step) const
{
    QVariantAnimation::KeyValues::const_iterator result =
        qBinaryFind(keyValues.constBegin(), keyValues.constEnd(), qMakePair(step, QVariant()), animationValueLessThan);
    if (result != keyValues.constEnd())
        return result->second;

    return QVariant();
}

void QVariantAnimationPrivate::setValueAt(qreal step, const QVariant &value)
{
    if (Q_UNLIKELY(step < qreal(0.0) || step > qreal(1.0))) {
        qWarning("QVariantAnimation::setValueAt: invalid step = %f", step);
        return;
    }

    QVariantAnimation::KeyValue pair(step, value);

    QVariantAnimation::KeyValues::iterator result =
        qLowerBound(keyValues.begin(), keyValues.end(), pair, animationValueLessThan);
    if (result == keyValues.end() || result->first != step) {
        keyValues.insert(result, pair);
    } else {
        if (value.isValid())
            result->second = value; // replaces the previous value
        else
            keyValues.erase(result); // removes the previous value
    }

    recalculateCurrentInterval(/*force=*/true);
}

void QVariantAnimationPrivate::setDefaultStartEndValue(const QVariant &value)
{
    defaultStartEndValue = value;
    recalculateCurrentInterval(/*force=*/true);
}

/*!
    Construct a QVariantAnimation object. \a parent is passed to QAbstractAnimation's
    constructor.
*/
QVariantAnimation::QVariantAnimation(QObject *parent)
    : QAbstractAnimation(*new QVariantAnimationPrivate, parent)
{
}

/*!
    \internal
*/
QVariantAnimation::QVariantAnimation(QVariantAnimationPrivate &dd, QObject *parent)
    : QAbstractAnimation(dd, parent)
{
}

/*!
    Destroys the animation.
*/
QVariantAnimation::~QVariantAnimation()
{
}

/*!
    \property QVariantAnimation::easingCurve
    \brief the easing curve of the animation

    This property defines the easing curve of the animation. By
    default, a linear easing curve is used, resulting in linear
    interpolation. Other curves are provided, for instance,
    QEasingCurve::InCirc, which provides a circular entry curve.
    Another example is QEasingCurve::InOutElastic, which provides an
    elastic effect on the values of the interpolated variant.

    QVariantAnimation will use the QEasingCurve::valueForProgress() to
    transform the "normalized progress" (currentTime / totalDuration)
    of the animation into the effective progress actually
    used by the animation. It is this effective progress that will be
    the progress when interpolated() is called. Also, the steps in the
    keyValues are referring to this effective progress.

    The easing curve is used with the interpolator, the interpolated()
    virtual function, the animation's duration, and iterationCount, to
    control how the current value changes as the animation progresses.
*/
QEasingCurve QVariantAnimation::easingCurve() const
{
    Q_D(const QVariantAnimation);
    return d->easing;
}

void QVariantAnimation::setEasingCurve(const QEasingCurve &easing)
{
    Q_D(QVariantAnimation);
    d->easing = easing;
    d->recalculateCurrentInterval();
}

/*!
    \internal
    \typedef QVariantAnimation::Interpolator

    This is a typedef for a pointer to a function with the following
    signature:
    \code
    QVariant myInterpolator(const QVariant &from, const QVariant &to, qreal progress);
    \endcode

*/

/*!
    \property QVariantAnimation::duration
    \brief the duration of the animation

    This property describes the duration in milliseconds of the
    animation. The default duration is 250 milliseconds.

    \sa QAbstractAnimation::duration()
*/
int QVariantAnimation::duration() const
{
    Q_D(const QVariantAnimation);
    return d->duration;
}

void QVariantAnimation::setDuration(int msecs)
{
    Q_D(QVariantAnimation);
    if (Q_UNLIKELY(msecs < 0)) {
        qWarning("QVariantAnimation::setDuration: cannot set a negative duration");
        return;
    }
    if (d->duration == msecs) {
        return;
    }
    d->duration = msecs;
    d->recalculateCurrentInterval();
}

/*!
    \property QVariantAnimation::startValue
    \brief the optional start value of the animation

    This property describes the optional start value of the animation. If
    omitted, or if a null QVariant is assigned as the start value, the
    animation will use the current position of the end when the animation
    is started.

    \sa endValue
*/
QVariant QVariantAnimation::startValue() const
{
    return keyValueAt(0);
}

void QVariantAnimation::setStartValue(const QVariant &value)
{
    setKeyValueAt(0, value);
}

/*!
    \property QVariantAnimation::endValue
    \brief the end value of the animation

    This property describes the end value of the animation.

    \sa startValue
*/
QVariant QVariantAnimation::endValue() const
{
    return keyValueAt(1);
}

void QVariantAnimation::setEndValue(const QVariant &value)
{
    setKeyValueAt(1, value);
}

/*!
    Returns the key frame value for the given \a step. The given \a step
    must be in the range 0 to 1. If there is no KeyValue for \a step,
    it returns an invalid QVariant.

    \sa keyValues(), setKeyValueAt()
*/
QVariant QVariantAnimation::keyValueAt(qreal step) const
{
    return d_func()->valueAt(step);
}

/*!
    \typedef QVariantAnimation::KeyValue

    This is a typedef for QPair<qreal, QVariant>.
*/

/*!
    \typedef QVariantAnimation::KeyValues

    This is a typedef for QVector<KeyValue>
*/

/*!
    Creates a key frame at the given \a step with the given \a value.
    The given \a step must be in the range 0 to 1.

    \sa setKeyValues(), keyValueAt()
*/
void QVariantAnimation::setKeyValueAt(qreal step, const QVariant &value)
{
    d_func()->setValueAt(step, value);
}

/*!
    Returns the key frames of this animation.

    \sa keyValueAt(), setKeyValues()
*/
QVariantAnimation::KeyValues QVariantAnimation::keyValues() const
{
    return d_func()->keyValues;
}

/*!
    Replaces the current set of key frames with the given \a keyValues.
    the step of the key frames must be in the range 0 to 1.

    \sa keyValues(), keyValueAt()
*/
void QVariantAnimation::setKeyValues(const KeyValues &keyValues)
{
    Q_D(QVariantAnimation);
    d->keyValues = keyValues;
    qSort(d->keyValues.begin(), d->keyValues.end(), animationValueLessThan);
    d->recalculateCurrentInterval(/*force=*/true);
}

/*!
    \property QVariantAnimation::currentValue
    \brief the current value of the animation.

    This property describes the current value; an interpolated value
    between the \l{startValue}{start value} and the \l{endValue}{end
    value}, using the current time for progress. The value itself is
    obtained from interpolated(), which is called repeatedly as the
    animation is running.

    QVariantAnimation calls the virtual updateCurrentValue() function
    when the current value changes. This is particularly useful for
    subclasses that need to track updates. For example,
    QPropertyAnimation uses this function to animate Qt \l{Qt's
    Property System}{properties}.

    \sa startValue, endValue
*/
QVariant QVariantAnimation::currentValue() const
{
    Q_D(const QVariantAnimation);
    if (!d->currentValue.isValid())
        const_cast<QVariantAnimationPrivate*>(d)->recalculateCurrentInterval();
    return d->currentValue;
}

/*!
    \reimp
*/
void QVariantAnimation::updateState(QAbstractAnimation::State newState,
                                    QAbstractAnimation::State oldState)
{
    Q_UNUSED(oldState);
    Q_UNUSED(newState);
}

/*!

    This virtual function returns the linear interpolation between
    variants \a from and \a to, at \a progress, usually a value
    between 0 and 1. You can reimplement this function in a subclass
    of QVariantAnimation to provide your own interpolation algorithm.

    Note that in order for the interpolation to work with a
    QEasingCurve that return a value smaller than 0 or larger than 1
    (such as QEasingCurve::InBack) you should make sure that it can
    extrapolate. If the semantic of the datatype does not allow
    extrapolation this function should handle that gracefully.

    You should call the QVariantAnimation implementation of this
    function if you want your class to handle the types already
    supported by Qt (see class QVariantAnimation description for a
    list of supported types).

    \sa QEasingCurve
 */
QVariant QVariantAnimation::interpolated(const QVariant &from, const QVariant &to, qreal progress) const
{
    return d_func()->interpolator(from.constData(), to.constData(), progress);
}

/*!
    \reimp
 */
void QVariantAnimation::updateCurrentTime(int)
{
    d_func()->recalculateCurrentInterval();
}

QT_END_NAMESPACE

#include "moc_qvariantanimation.h"


#endif //QT_NO_ANIMATION
