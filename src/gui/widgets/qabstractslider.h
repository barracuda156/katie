/****************************************************************************
**
** Copyright (C) 2015 The Qt Company Ltd.
** Copyright (C) 2016-2019 Ivailo Monev
**
** This file is part of the QtGui module of the Katie Toolkit.
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

#ifndef QABSTRACTSLIDER_H
#define QABSTRACTSLIDER_H

#include <QtGui/qwidget.h>

QT_BEGIN_HEADER

QT_BEGIN_NAMESPACE


class QAbstractSliderPrivate;

class Q_GUI_EXPORT QAbstractSlider : public QWidget
{
    Q_OBJECT

    Q_PROPERTY(int minimum READ minimum WRITE setMinimum)
    Q_PROPERTY(int maximum READ maximum WRITE setMaximum)
    Q_PROPERTY(int singleStep READ singleStep WRITE setSingleStep)
    Q_PROPERTY(int pageStep READ pageStep WRITE setPageStep)
    Q_PROPERTY(int value READ value WRITE setValue NOTIFY valueChanged USER true)
    Q_PROPERTY(int sliderPosition READ sliderPosition WRITE setSliderPosition NOTIFY sliderMoved)
    Q_PROPERTY(bool tracking READ hasTracking WRITE setTracking)
    Q_PROPERTY(Qt::Orientation orientation READ orientation WRITE setOrientation)
    Q_PROPERTY(bool invertedAppearance READ invertedAppearance WRITE setInvertedAppearance)
    Q_PROPERTY(bool invertedControls READ invertedControls WRITE setInvertedControls)
    Q_PROPERTY(bool sliderDown READ isSliderDown WRITE setSliderDown DESIGNABLE false)

public:
    explicit QAbstractSlider(QWidget *parent=0);
    ~QAbstractSlider();

    Qt::Orientation orientation() const;

    void setMinimum(int);
    int minimum() const;

    void setMaximum(int);
    int maximum() const;

    void setRange(int min, int max);

    void setSingleStep(int);
    int singleStep() const;

    void setPageStep(int);
    int pageStep() const;

    void setTracking(bool enable);
    bool hasTracking() const;

    void setSliderDown(bool);
    bool isSliderDown() const;

    void setSliderPosition(int);
    int sliderPosition() const;

    void setInvertedAppearance(bool);
    bool invertedAppearance() const;

    void setInvertedControls(bool);
    bool invertedControls() const;

    enum SliderAction {
        SliderNoAction,
        SliderSingleStepAdd,
        SliderSingleStepSub,
        SliderPageStepAdd,
        SliderPageStepSub,
        SliderToMinimum,
        SliderToMaximum,
        SliderMove
    };

    int value() const;

    void triggerAction(SliderAction action);

public Q_SLOTS:
    void setValue(int);
    void setOrientation(Qt::Orientation);

Q_SIGNALS:
    void valueChanged(int value);

    void sliderPressed();
    void sliderMoved(int position);
    void sliderReleased();

    void rangeChanged(int min, int max);

    void actionTriggered(int action);

protected:
    bool event(QEvent *e);

    void setRepeatAction(SliderAction action, int thresholdTime = 500, int repeatTime = 50);
    SliderAction repeatAction() const;

    enum SliderChange {
        SliderRangeChange,
        SliderOrientationChange,
        SliderStepsChange,
        SliderValueChange
    };
    virtual void sliderChange(SliderChange change);

    void keyPressEvent(QKeyEvent *ev);
    void timerEvent(QTimerEvent *);
#ifndef QT_NO_WHEELEVENT
    void wheelEvent(QWheelEvent *e);
#endif
    void changeEvent(QEvent *e);


protected:
    QAbstractSlider(QAbstractSliderPrivate &dd, QWidget *parent=0);

private:
    Q_DISABLE_COPY(QAbstractSlider)
    Q_DECLARE_PRIVATE(QAbstractSlider)
};

QT_END_NAMESPACE

QT_END_HEADER

#endif // QABSTRACTSLIDER_H
