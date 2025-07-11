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

#include "qstyleoption.h"
#include "qpainter.h"
#include "qpixmapcache.h"
#include "qstyle_p.h"
#include "qmath.h"
#include "qpen.h"
#include "qstylehelper_p.h"
#include "qguicommon_p.h"

QT_BEGIN_NAMESPACE

namespace QStyleHelper {

QByteArray uniqueName(const QByteArray &key, const QStyleOption *option, const QSize &size)
{
    const QStyleOptionComplex *complexOption = qstyleoption_cast<const QStyleOptionComplex *>(option);
    QByteArray tmp = key + qHexString(
        "_%d_%d_%d_%lld_%d_%d",
        static_cast<int>(option->state),
        static_cast<int>(option->direction),
        static_cast<int>(complexOption ? int(complexOption->activeSubControls) : 0u),
        option->palette.cacheKey(),
        size.width(),
        size.height()
    );

#ifndef QT_NO_SPINBOX
    if (const QStyleOptionSpinBox *spinBox = qstyleoption_cast<const QStyleOptionSpinBox *>(option)) {
        tmp += '_' + static_cast<char>(spinBox->buttonSymbols);
        tmp += '_' + static_cast<char>(spinBox->stepEnabled);
        tmp += '_' + static_cast<char>(spinBox->frame ? '1' : '0');
    }
#endif // QT_NO_SPINBOX
    return tmp;
}

#ifndef QT_NO_DIAL

int calcBigLineSize(int radius)
{
    int bigLineSize = radius / 6;
    if (bigLineSize < 4)
        bigLineSize = 4;
    if (bigLineSize > radius / 2)
        bigLineSize = radius / 2;
    return bigLineSize;
}

static QPointF calcRadialPos(const QStyleOptionSlider *dial, qreal offset)
{
    const int width = dial->rect.width();
    const int height = dial->rect.height();
    const int r = qMin(width, height) / 2;
    const int currentSliderPosition = dial->upsideDown ? dial->sliderPosition : (dial->maximum - dial->sliderPosition);
    qreal a = 0;
    if (dial->maximum == dial->minimum)
        a = M_PI_2;
    else if (dial->dialWrapping)
        a = M_PI * 3 / 2 - (currentSliderPosition - dial->minimum) * 2 * M_PI
            / (dial->maximum - dial->minimum);
    else
        a = (M_PI * 8 - (currentSliderPosition - dial->minimum) * 10 * M_PI
            / (dial->maximum - dial->minimum)) / 6;
    qreal xc = width / 2.0;
    qreal yc = height / 2.0;
    qreal len = r - QStyleHelper::calcBigLineSize(r) - 3;
    qreal back = offset * len;
    return QPointF(xc + back * qCos(a), yc - back * qSin(a));
}

qreal angle(const QPointF &p1, const QPointF &p2)
{
    static const qreal rad_factor = 180 / M_PI;
    qreal _angle = 0;

    if (p1.x() == p2.x()) {
        if (p1.y() < p2.y())
            _angle = 270;
        else
            _angle = 90;
    } else  {
        qreal x1, x2, y1, y2;

        if (p1.x() <= p2.x()) {
            x1 = p1.x(); y1 = p1.y();
            x2 = p2.x(); y2 = p2.y();
        } else {
            x2 = p1.x(); y2 = p1.y();
            x1 = p2.x(); y1 = p2.y();
        }

        qreal m = -(y2 - y1) / (x2 - x1);
        _angle = qAtan(m) *  rad_factor;

        if (p1.x() < p2.x())
            _angle = 180 - _angle;
        else
            _angle = -_angle;
    }
    return _angle;
}

QPolygonF calcLines(const QStyleOptionSlider *dial)
{
    QPolygonF poly;
    int width = dial->rect.width();
    int height = dial->rect.height();
    qreal r = qMin(width, height) / 2;
    int bigLineSize = calcBigLineSize(int(r));

    qreal xc = width / 2 + 0.5;
    qreal yc = height / 2 + 0.5;
    const int ns = dial->tickInterval;
    if (!ns) // Invalid values may be set by Katie Designer.
        return poly;
    int notches = (dial->maximum + ns - 1 - dial->minimum) / ns;
    if (notches <= 0)
        return poly;
    if (dial->maximum < dial->minimum || dial->maximum - dial->minimum > 1000) {
        int maximum = dial->minimum + 1000;
        notches = (maximum + ns - 1 - dial->minimum) / ns;
    }

    poly.resize(2 + 2 * notches);
    int smallLineSize = bigLineSize / 2;
    for (int i = 0; i <= notches; ++i) {
        qreal angle = dial->dialWrapping ? M_PI * 3 / 2 - i * 2 * M_PI / notches
                  : (M_PI * 8 - i * 10 * M_PI / notches) / 6;
        qreal s = qSin(angle);
        qreal c = qCos(angle);
        if (i == 0 || (((ns * i) % (dial->pageStep ? dial->pageStep : 1)) == 0)) {
            poly[2 * i] = QPointF(xc + (r - bigLineSize) * c,
                                  yc - (r - bigLineSize) * s);
            poly[2 * i + 1] = QPointF(xc + r * c, yc - r * s);
        } else {
            poly[2 * i] = QPointF(xc + (r - 1 - smallLineSize) * c,
                                  yc - (r - 1 - smallLineSize) * s);
            poly[2 * i + 1] = QPointF(xc + (r - 1) * c, yc -(r - 1) * s);
        }
    }
    return poly;
}

// This will draw a nice and shiny QDial for us. We don't want
// all the shinyness in QWindowsStyle, hence we place it here

void drawDial(const QStyleOptionSlider *option, QPainter *painter)
{
    QColor buttonColor = option->palette.button().color();
    const int width = option->rect.width();
    const int height = option->rect.height();
    const bool enabled = option->state & QStyle::State_Enabled;
    qreal r = qMin(width, height) / 2;
    r -= r/50;
    const qreal penSize = r/20.0;

    painter->save();

    // Draw notches
    if (option->subControls & QStyle::SC_DialTickmarks) {
        painter->setPen(option->palette.dark().color().darker(120));
        painter->drawLines(QStyleHelper::calcLines(option));
    }

    // Cache dial background
    BEGIN_STYLE_PIXMAPCACHE(QByteArray("qdial"));

    const qreal d_ = r / 6;
    const qreal dx = option->rect.x() + d_ + (width - 2 * r) / 2 + 1;
    const qreal dy = option->rect.y() + d_ + (height - 2 * r) / 2 + 1;

    QRectF br = QRectF(dx + 0.5, dy + 0.5,
                       int(r * 2 - 2 * d_ - 2),
                       int(r * 2 - 2 * d_ - 2));
    buttonColor.setHsv(buttonColor .hue(),
                       qMin(140, buttonColor .saturation()),
                       qMax(180, buttonColor.value()));

    if (enabled) {
        // Drop shadow
        qreal shadowSize = qMax(1.0, penSize/2.0);
        QRectF shadowRect= br.adjusted(-2*shadowSize, -2*shadowSize,
                                       2*shadowSize, 2*shadowSize);
        QRadialGradient shadowGradient(shadowRect.center().x(),
                                       shadowRect.center().y(), shadowRect.width()/2.0,
                                       shadowRect.center().x(), shadowRect.center().y());
        shadowGradient.setColorAt(qreal(0.91), QColor(0, 0, 0, 40));
        shadowGradient.setColorAt(qreal(1.0), Qt::transparent);
        p->setBrush(shadowGradient);
        p->setPen(Qt::NoPen);
        p->translate(shadowSize, shadowSize);
        p->drawEllipse(shadowRect);
        p->translate(-shadowSize, -shadowSize);

        // Main gradient
        QRadialGradient gradient(br.center().x() - br.width()/3, dy,
                                 br.width()*1.3, br.center().x(),
                                 br.center().y() - br.height()/2);
        gradient.setColorAt(0, buttonColor.lighter(110));
        gradient.setColorAt(qreal(0.5), buttonColor);
        gradient.setColorAt(qreal(0.501), buttonColor.darker(102));
        gradient.setColorAt(1, buttonColor.darker(115));
        p->setBrush(gradient);
    } else {
        p->setBrush(Qt::NoBrush);
    }

    p->setPen(QPen(buttonColor.darker(280)));
    p->drawEllipse(br);
    p->setBrush(Qt::NoBrush);
    p->setPen(buttonColor.lighter(110));
    p->drawEllipse(br.adjusted(1, 1, -1, -1));

    if (option->state & QStyle::State_HasFocus) {
        QColor highlight = option->palette.highlight().color();
        highlight.setHsv(highlight.hue(),
                         qMin(160, highlight.saturation()),
                         qMax(230, highlight.value()));
        highlight.setAlpha(127);
        p->setPen(QPen(highlight, 2.0));
        p->setBrush(Qt::NoBrush);
        p->drawEllipse(br.adjusted(-1, -1, 1, 1));
    }

    END_STYLE_PIXMAPCACHE

    QPointF dp = calcRadialPos(option, qreal(0.70));
    buttonColor = buttonColor.lighter(104);
    buttonColor.setAlphaF(qreal(0.8));
    const qreal ds = r/qreal(7.0);
    QRectF dialRect(dp.x() - ds, dp.y() - ds, 2*ds, 2*ds);
    QRadialGradient dialGradient(dialRect.center().x() + dialRect.width()/2,
                                 dialRect.center().y() + dialRect.width(),
                                 dialRect.width()*2,
                                 dialRect.center().x(), dialRect.center().y());
    dialGradient.setColorAt(1, buttonColor.darker(140));
    dialGradient.setColorAt(qreal(0.4), buttonColor.darker(120));
    dialGradient.setColorAt(0, buttonColor.darker(110));
    if (penSize > 3.0) {
        painter->setPen(QPen(QColor(0, 0, 0, 25), penSize));
        painter->drawLine(calcRadialPos(option, qreal(0.90)), calcRadialPos(option, qreal(0.96)));
    }

    painter->setBrush(dialGradient);
    painter->setPen(QColor(255, 255, 255, 150));
    painter->drawEllipse(dialRect.adjusted(-1, -1, 1, 1));
    painter->setPen(QColor(0, 0, 0, 80));
    painter->drawEllipse(dialRect);
    painter->restore();
}
#endif //QT_NO_DIAL

}
QT_END_NAMESPACE




