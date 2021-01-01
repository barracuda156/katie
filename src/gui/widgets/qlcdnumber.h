/****************************************************************************
**
** Copyright (C) 2015 The Qt Company Ltd.
** Copyright (C) 2016-2021 Ivailo Monev
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

#ifndef QLCDNUMBER_H
#define QLCDNUMBER_H

#include <QtGui/qframe.h>
#include <QtCore/qbitarray.h>

QT_BEGIN_HEADER

QT_BEGIN_NAMESPACE


#ifndef QT_NO_LCDNUMBER

class QLCDNumberPrivate;
class Q_GUI_EXPORT QLCDNumber : public QFrame // LCD number widget
{
    Q_OBJECT
    Q_ENUMS(Mode SegmentStyle)
    Q_PROPERTY(bool smallDecimalPoint READ smallDecimalPoint WRITE setSmallDecimalPoint)
    Q_PROPERTY(int digitCount READ digitCount WRITE setDigitCount)
    Q_PROPERTY(Mode mode READ mode WRITE setMode)
    Q_PROPERTY(SegmentStyle segmentStyle READ segmentStyle WRITE setSegmentStyle)
    Q_PROPERTY(double value READ value WRITE display)
    Q_PROPERTY(int intValue READ intValue WRITE display)

public:
    explicit QLCDNumber(QWidget* parent = Q_NULLPTR);
    explicit QLCDNumber(uint numDigits, QWidget* parent = Q_NULLPTR);
    ~QLCDNumber();

    enum Mode {
        Hex, Dec, Oct, Bin
    };
    enum SegmentStyle {
        Outline, Filled, Flat
    };

    bool smallDecimalPoint() const;
    int digitCount() const;
    void setDigitCount(int nDigits);

    bool checkOverflow(double num) const;
    bool checkOverflow(int num) const;

    Mode mode() const;
    void setMode(Mode);

    SegmentStyle segmentStyle() const;
    void setSegmentStyle(SegmentStyle);

    double value() const;
    int intValue() const;

    QSize sizeHint() const;

public Q_SLOTS:
    void display(const QString &str);
    void display(int num);
    void display(double num);
    void setHexMode();
    void setDecMode();
    void setOctMode();
    void setBinMode();
    void setSmallDecimalPoint(bool);

Q_SIGNALS:
    void overflow();

protected:
    bool event(QEvent *e);
    void paintEvent(QPaintEvent *);

private:
    Q_DISABLE_COPY(QLCDNumber)
    Q_DECLARE_PRIVATE(QLCDNumber)
};

#endif // QT_NO_LCDNUMBER

QT_END_NAMESPACE

QT_END_HEADER

#endif // QLCDNUMBER_H
