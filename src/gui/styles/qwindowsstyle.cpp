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

#include "qwindowsstyle.h"
#include "qwindowsstyle_p.h"

#if !defined(QT_NO_STYLE_WINDOWS)

#include "qapplication.h"
#include "qbitmap.h"
#include "qdrawutil.h" // for now
#include "qevent.h"
#include "qmenu.h"
#include "qmenubar.h"
#include "qmenubar_p.h"
#include "qpaintengine.h"
#include "qpainter.h"
#include "qprogressbar.h"
#include "qrubberband.h"
#include "qstyleoption.h"
#include "qtabbar.h"
#include "qwidget.h"
#include "qdebug.h"
#include "qmainwindow.h"
#include "qfile.h"
#include "qtextstream.h"
#include "qpixmapcache.h"
#include "qwizard.h"
#include "qlistview.h"
#include "qmath.h"
#include "qstylehelper_p.h"
#include "qguicommon_p.h"
#include "qguiimages_p.h"

#include <limits.h>

QT_BEGIN_NAMESPACE

enum QSliderDirection { SlUp, SlDown, SlLeft, SlRight };

/*
    \internal
*/
QWindowsStylePrivate::QWindowsStylePrivate()
    : animationFps(10), animateTimer(0), animateStep(0)
{
    startTime.start();
}

void QWindowsStylePrivate::startAnimation(QObject *o, QProgressBar *bar)
{
    if (!animatedProgressBars.contains(bar)) {
        animatedProgressBars << bar;
        if (!animateTimer) {
            Q_ASSERT(animationFps > 0);
            animateTimer = o->startTimer(1000 / animationFps);
        }
    }
}

void QWindowsStylePrivate::stopAnimation(QObject *o, QProgressBar *bar)
{
    animatedProgressBars.removeAll(bar);
    if (animatedProgressBars.isEmpty() && animateTimer) {
        o->killTimer(animateTimer);
        animateTimer = 0;
    }
}

/*!
    \reimp
*/
void QWindowsStyle::timerEvent(QTimerEvent *event)
{
#ifndef QT_NO_PROGRESSBAR
    Q_D(QWindowsStyle);
    if (event->timerId() == d->animateTimer) {
        Q_ASSERT(d->animationFps> 0);
        d->animateStep = d->startTime.elapsed() / (1000 / d->animationFps);
        foreach (QProgressBar *bar, d->animatedProgressBars)
            bar->update();
        event->ignore();
    }
#endif // QT_NO_PROGRESSBAR
}

/*!
    \reimp
*/
bool QWindowsStyle::eventFilter(QObject *o, QEvent *e)
{
    if (!o->isWidgetType())
        return QObject::eventFilter(o, e);

    Q_D(QWindowsStyle);
    switch(e->type()) {
#ifndef QT_NO_PROGRESSBAR
    case QEvent::StyleChange:
    case QEvent::Paint:
    case QEvent::Show:
        if (QProgressBar *bar = qobject_cast<QProgressBar *>(o)) {
            // Animation by timer for progress bars that have their min and
            // max values the same
            if (bar->minimum() == bar->maximum())
                d->startAnimation(this, bar);
            else
                d->stopAnimation(this, bar);
        }
        break;
    case QEvent::Destroy:
    case QEvent::Hide:
        // Do static_cast because there is no type info when getting
        // the destroy event. We know that it is a QProgressBar, since
        // we only install a widget event filter for QScrollBars.
        d->stopAnimation(this, static_cast<QProgressBar *>(o));
        break;
#endif // QT_NO_PROGRESSBAR
    default:
        break;
    }
    return QCommonStyle::eventFilter(o, e);
}

/*!
    \class QWindowsStyle
    \brief The QWindowsStyle class provides a Microsoft Windows-like look and feel.

    \ingroup appearance

    \img qwindowsstyle.png
    \sa QCleanlooksStyle
*/

/*!
    Constructs a QWindowsStyle object.
*/
QWindowsStyle::QWindowsStyle() : QCommonStyle(*new QWindowsStylePrivate)
{
    setObjectName(QLatin1String("Windows"));
}

/*!
    \internal

    Constructs a QWindowsStyle object.
*/
QWindowsStyle::QWindowsStyle(QWindowsStylePrivate &dd) : QCommonStyle(dd)
{
    setObjectName(QLatin1String("Windows"));
}

/*! \reimp */
void QWindowsStyle::polish(QApplication *app)
{
    Q_D(QWindowsStyle);

    QCommonStyle::polish(app);

    d->activeCaptionColor = app->palette().highlight().color();
    d->activeGradientCaptionColor = app->palette().highlight() .color();
    d->inactiveCaptionColor = app->palette().dark().color();
    d->inactiveGradientCaptionColor = app->palette().dark().color();
    d->inactiveCaptionText = app->palette().background().color();
}

/*! \reimp */
void QWindowsStyle::unpolish(QApplication *app)
{
    QCommonStyle::unpolish(app);
    app->removeEventFilter(this);
}

/*! \reimp */
void QWindowsStyle::polish(QWidget *widget)
{
    QCommonStyle::polish(widget);
#ifndef QT_NO_PROGRESSBAR
    if (qobject_cast<QProgressBar *>(widget))
        widget->installEventFilter(this);
#endif
}

/*! \reimp */
void QWindowsStyle::unpolish(QWidget *widget)
{
    QCommonStyle::unpolish(widget);
#ifndef QT_NO_PROGRESSBAR
    if (QProgressBar *bar=qobject_cast<QProgressBar *>(widget)) {
        Q_D(QWindowsStyle);
        widget->removeEventFilter(this);
        d->stopAnimation(this, bar);
    }
#endif
}

/*!
  \reimp
*/
void QWindowsStyle::polish(QPalette &pal)
{
    QCommonStyle::polish(pal);
}

/*!
  \reimp
*/
int QWindowsStyle::pixelMetric(PixelMetric pm, const QStyleOption *opt, const QWidget *widget) const
{
    int ret;

    switch (pm) {
    case PM_ButtonDefaultIndicator:
    case PM_ButtonShiftHorizontal:
    case PM_ButtonShiftVertical:
    case PM_MenuHMargin:
    case PM_MenuVMargin:
        ret = 1;
        break;
#ifndef QT_NO_TABBAR
    case PM_TabBarTabShiftHorizontal:
        ret = 0;
        break;
    case PM_TabBarTabShiftVertical:
        ret = 2;
        break;
#endif
    case PM_MaximumDragDistance:
        ret = 60;
        break;

#ifndef QT_NO_SLIDER
    case PM_SliderLength:
        ret = 11;
        break;

        // Returns the number of pixels to use for the business part of the
        // slider (i.e., the non-tickmark portion). The remaining space is shared
        // equally between the tickmark regions.
    case PM_SliderControlThickness:
        if (const QStyleOptionSlider *sl = qstyleoption_cast<const QStyleOptionSlider *>(opt)) {
            int space = (sl->orientation == Qt::Horizontal) ? sl->rect.height() : sl->rect.width();
            int ticks = sl->tickPosition;
            int n = 0;
            if (ticks & QSlider::TicksAbove)
                ++n;
            if (ticks & QSlider::TicksBelow)
                ++n;
            if (!n) {
                ret = space;
                break;
            }

            int thick = 6;        // Magic constant to get 5 + 16 + 5
            if (ticks != QSlider::TicksBothSides && ticks != QSlider::NoTicks)
                thick += proxy()->pixelMetric(PM_SliderLength, sl, widget) / 4;

            space -= thick;
            if (space > 0)
                thick += (space * 2) / (n + 2);
            ret = thick;
        } else {
            ret = 0;
        }
        break;
#endif // QT_NO_SLIDER

#ifndef QT_NO_MENU
    case PM_MenuBarHMargin:
        ret = 0;
        break;

    case PM_MenuBarVMargin:
        ret = 0;
        break;

    case PM_MenuBarPanelWidth:
        ret = 0;
        break;

    case PM_SmallIconSize:
        ret = 16;
        break;

    case PM_LargeIconSize:
        ret = 32;
        break;

    case PM_IconViewIconSize:
        ret = proxy()->pixelMetric(PM_LargeIconSize, opt, widget);
        break;

    case PM_DockWidgetTitleMargin:
        ret = 2;
        break;
    case PM_DockWidgetTitleBarButtonMargin:
    case PM_DockWidgetFrameWidth:
        ret = 4;
        break;
#endif // QT_NO_MENU

    case PM_SplitterWidth:
        ret = qMax(4, QApplication::globalStrut().width());
        break;

    case PM_ToolBarItemMargin:
        ret = 1;
        break;
    case PM_ToolBarItemSpacing:
        ret = 0;
        break;
    case PM_ToolBarHandleExtent:
        ret = 10;
        break;
    default:
        ret = QCommonStyle::pixelMetric(pm, opt, widget);
        break;
    }

    return ret;
}

/*!
 \reimp
 */
QPixmap QWindowsStyle::standardPixmap(StandardPixmap standardpixmap, const QStyleOption *opt,
                                      const QWidget *widget) const
{
    switch (standardpixmap) {
    case SP_TitleBarMenuButton: {
        QPixmap pix;
        pix.loadFromData(
            windows_qt_windows_menu_png,
            windows_qt_windows_menu_png_len,
            qt_images_format
        );
        return pix;
    }
    case SP_TitleBarShadeButton: {
        QPixmap pix;
        pix.loadFromData(
            windows_qt_windows_shade_png,
            windows_qt_windows_shade_png_len,
            qt_images_format
        );
        return pix;
    }
    case SP_TitleBarUnshadeButton: {
        QPixmap pix;
        pix.loadFromData(
            windows_qt_windows_unshade_png,
            windows_qt_windows_unshade_png_len,
            qt_images_format
        );
        return pix;
    }
    case SP_TitleBarNormalButton: {
        QPixmap pix;
        pix.loadFromData(
            windows_qt_windows_normalizeup_png,
            windows_qt_windows_normalizeup_png_len,
            qt_images_format
        );
        return pix;
    }
    case SP_TitleBarMinButton: {
        QPixmap pix;
        pix.loadFromData(
            windows_qt_windows_minimize_png,
            windows_qt_windows_minimize_png_len,
            qt_images_format
        );
        return pix;
    }
    case SP_TitleBarMaxButton: {
        QPixmap pix;
        pix.loadFromData(
            windows_qt_windows_maximize_png,
            windows_qt_windows_maximize_png_len,
            qt_images_format
        );
        return pix;
    }
    case SP_TitleBarCloseButton: {
        QPixmap pix;
        pix.loadFromData(
            windows_qt_windows_close_png,
            windows_qt_windows_close_png_len,
            qt_images_format
        );
        return pix;
    }
    case SP_TitleBarContextHelpButton: {
        QPixmap pix;
        pix.loadFromData(
            windows_qt_windows_help_png,
            windows_qt_windows_help_png_len,
            qt_images_format
        );
        return pix;
    }
    case SP_DockWidgetCloseButton: {
        QPixmap pix;
        pix.loadFromData(
            windows_qt_windows_dock_widget_close_png,
            windows_qt_windows_dock_widget_close_png_len,
            qt_images_format
        );
        return pix;
    }
    case SP_MessageBoxInformation: {
        QPixmap pix;
        pix.loadFromData(
            windows_qt_information_png,
            windows_qt_information_png_len,
            qt_images_format
        );
        return pix;
    }
    case SP_MessageBoxWarning: {
        QPixmap pix;
        pix.loadFromData(
            windows_qt_warning_png,
            windows_qt_warning_png_len,
            qt_images_format
        );
        return pix;
    }
    case SP_MessageBoxCritical: {
        QPixmap pix;
        pix.loadFromData(
            windows_qt_critical_png,
            windows_qt_critical_png_len,
            qt_images_format
        );
        return pix;
    }
    case SP_MessageBoxQuestion: {
        QPixmap pix;
        pix.loadFromData(
            windows_qt_question_png,
            windows_qt_question_png_len,
            qt_images_format
        );
        return pix;
    }
    default:
        break;
    }
    return QCommonStyle::standardPixmap(standardpixmap, opt, widget);
}

/*! \reimp */
int QWindowsStyle::styleHint(StyleHint hint, const QStyleOption *opt, const QWidget *widget,
                             QStyleHintReturn *returnData) const
{
    int ret = 0;

    switch (hint) {
    case SH_EtchDisabledText:
    case SH_Slider_SnapToValue:
    case SH_PrintDialog_RightAlignButtons:
    case SH_FontDialog_SelectAssociatedText:
    case SH_Menu_AllowActiveAndDisabled:
    case SH_MenuBar_AltKeyNavigation:
    case SH_MenuBar_MouseTracking:
    case SH_Menu_MouseTracking:
    case SH_ComboBox_ListMouseTracking:
    case SH_ScrollBar_StopMouseOverSlider:
    case SH_MainWindow_SpaceBelowMenuBar:
        ret = 1;
        break;
    case SH_ItemView_ShowDecorationSelected:
#ifndef QT_NO_LISTVIEW
        if (qobject_cast<const QListView*>(widget))
            ret = 1;
#endif
        break;
    case SH_ItemView_ChangeHighlightOnFocus:
        ret = 1;
        break;
    case SH_ToolBox_SelectedPageTitleBold:
        ret = 0;
        break;

#ifndef QT_NO_RUBBERBAND
    case SH_RubberBand_Mask:
        if (const QStyleOptionRubberBand *rbOpt = qstyleoption_cast<const QStyleOptionRubberBand *>(opt)) {
            ret = 0;
            if (rbOpt->shape == QRubberBand::Rectangle) {
                ret = true;
                if(QStyleHintReturnMask *mask = qstyleoption_cast<QStyleHintReturnMask*>(returnData)) {
                    mask->region = opt->rect;
                    int size = 1;
                    if (widget && widget->isWindow())
                        size = 4;
                    mask->region -= opt->rect.adjusted(size, size, -size, -size);
                }
            }
        }
        break;
#endif // QT_NO_RUBBERBAND
    case SH_LineEdit_PasswordCharacter:
        ret = '*';
        break;
    case SH_ItemView_ArrowKeysNavigateIntoChildren:
        ret = true;
        break;
    case SH_DialogButtonBox_ButtonsHaveIcons:
        ret = 0;
        break;
    default:
        ret = QCommonStyle::styleHint(hint, opt, widget, returnData);
        break;
    }
    return ret;
}

/*! \reimp */
void QWindowsStyle::drawPrimitive(PrimitiveElement pe, const QStyleOption *opt, QPainter *p,
                                  const QWidget *w) const
{
    // Used to restore across fallthrough cases. Currently only used in PE_IndicatorCheckBox
    bool doRestore = false;

    switch (pe) {
#ifndef QT_NO_TOOLBAR
    case PE_IndicatorToolBarSeparator: {
        QRect rect = opt->rect;
        const int margin = 2;
        QPen oldPen = p->pen();
        if(opt->state & State_Horizontal){
            const int offset = rect.width()/2;
            p->setPen(QPen(opt->palette.dark().color()));
            p->drawLine(rect.bottomLeft().x() + offset,
                        rect.bottomLeft().y() - margin,
                        rect.topLeft().x() + offset,
                        rect.topLeft().y() + margin);
            p->setPen(QPen(opt->palette.light().color()));
            p->drawLine(rect.bottomLeft().x() + offset + 1,
                        rect.bottomLeft().y() - margin,
                        rect.topLeft().x() + offset + 1,
                        rect.topLeft().y() + margin);
        } else{ //Draw vertical separator
            const int offset = rect.height()/2;
            p->setPen(QPen(opt->palette.dark().color()));
            p->drawLine(rect.topLeft().x() + margin ,
                        rect.topLeft().y() + offset,
                        rect.topRight().x() - margin,
                        rect.topRight().y() + offset);
            p->setPen(QPen(opt->palette.light().color()));
            p->drawLine(rect.topLeft().x() + margin ,
                        rect.topLeft().y() + offset + 1,
                        rect.topRight().x() - margin,
                        rect.topRight().y() + offset + 1);
        }
        p->setPen(oldPen);
        break;
    }
    case PE_IndicatorToolBarHandle: {
        p->save();
        p->translate(opt->rect.x(), opt->rect.y());
        if (opt->state & State_Horizontal) {
            int x = opt->rect.width() / 2 - 4;
            if (opt->direction == Qt::RightToLeft)
                x -= 2;
            if (opt->rect.height() > 4) {
                qDrawShadePanel(p, x, 2, 3, opt->rect.height() - 4,
                                opt->palette, false, 1, 0);
                qDrawShadePanel(p, x + 3, 2, 3, opt->rect.height() - 4,
                                opt->palette, false, 1, 0);
            }
        } else {
            if (opt->rect.width() > 4) {
                int y = opt->rect.height() / 2 - 4;
                qDrawShadePanel(p, 2, y, opt->rect.width() - 4, 3,
                                opt->palette, false, 1, 0);
                qDrawShadePanel(p, 2, y + 3, opt->rect.width() - 4, 3,
                                opt->palette, false, 1, 0);
            }
        }
        p->restore();
        break;
    }
#endif // QT_NO_TOOLBAR
    case PE_FrameButtonTool:
    case PE_PanelButtonTool: {
        QPen oldPen = p->pen();
#ifndef QT_NO_DOCKWIDGET
        if (w && w->inherits("QDockWidgetTitleButton")) {
           if (const QWidget *dw = w->parentWidget())
                if (dw->isWindow()){
                    qDrawWinButton(p, opt->rect.adjusted(1, 1, 0, 0), opt->palette, opt->state & (State_Sunken | State_On),
                           &opt->palette.button());

                    return;
                }
        }
#endif // QT_NO_DOCKWIDGET
        QBrush fill;
        bool stippled;
        bool panel = (pe == PE_PanelButtonTool);
        if ((!(opt->state & State_Sunken ))
            && (!(opt->state & State_Enabled)
                || !(opt->state & State_MouseOver && opt->state & State_AutoRaise))
            && (opt->state & State_On)) {
            fill = QBrush(opt->palette.light().color(), Qt::Dense4Pattern);
            stippled = true;
        } else {
            fill = opt->palette.brush(QPalette::Button);
            stippled = false;
        }

        if (opt->state & (State_Raised | State_Sunken | State_On)) {
            if (opt->state & State_AutoRaise) {
                if(opt->state & (State_Enabled | State_Sunken | State_On)){
                    if (panel)
                        qDrawShadePanel(p, opt->rect, opt->palette,
                                        opt->state & (State_Sunken | State_On), 1, &fill);
                    else
                        qDrawShadeRect(p, opt->rect, opt->palette,
                                       opt->state & (State_Sunken | State_On), 1);
                }
                if (stippled) {
                    p->setPen(opt->palette.button().color());
                    p->drawRect(opt->rect.adjusted(1,1,-2,-2));
                }
            } else {
                qDrawWinButton(p, opt->rect, opt->palette,
                               opt->state & (State_Sunken | State_On), panel ? &fill : 0);
            }
        } else {
            p->fillRect(opt->rect, fill);
        }
        p->setPen(oldPen);
        break;
    }
    case PE_PanelButtonCommand: {
        if (const QStyleOptionButton *btn = qstyleoption_cast<const QStyleOptionButton *>(opt)) {
            QBrush fill;
            State flags = opt->state;
            QPalette pal = opt->palette;
            QRect r = opt->rect;
            if (! (flags & State_Sunken) && (flags & State_On))
                fill = QBrush(pal.light().color(), Qt::Dense4Pattern);
            else
                fill = pal.brush(QPalette::Button);

            if (btn->features & QStyleOptionButton::DefaultButton && flags & State_Sunken) {
                p->setPen(pal.dark().color());
                p->setBrush(fill);
                p->drawRect(r.adjusted(0, 0, -1, -1));
            } else if (flags & (State_Raised | State_Sunken | State_On | State_Sunken)) {
                qDrawWinButton(p, r, pal, flags & (State_Sunken | State_On),
                               &fill);
            } else {
                p->fillRect(r, fill);
            }
        }
        break;
    }
    case PE_FrameDefaultButton: {
        QPen oldPen = p->pen();
        p->setPen(opt->palette.shadow().color());
        QRect rect = opt->rect;
        rect.adjust(0, 0, -1, -1);
        p->drawRect(rect);
        p->setPen(oldPen);
        break;
    }
    case PE_IndicatorArrowUp:
    case PE_IndicatorArrowDown:
    case PE_IndicatorArrowRight:
    case PE_IndicatorArrowLeft: {
        if (opt->rect.width() <= 1 || opt->rect.height() <= 1)
            break;
        QRect r = opt->rect;
        int size = qMin(r.height(), r.width());
        QPixmap pixmap;
        QByteArray pixmapName = QStyleHelper::uniqueName(QByteArray("qt_arrow") + metaObject()->className(), opt, QSize(size, size))
                                                      + '_' + static_cast<char>(pe);
        if (!QPixmapCache::find(pixmapName, pixmap)) {
            int border = size/5;
            int sqsize = 2*(size/2);
            QImage image(sqsize, sqsize, QImage::Format_ARGB32_Premultiplied);
            image.fill(0);
            QPainter imagePainter(&image);

            QPolygon a;
            switch (pe) {
                case PE_IndicatorArrowUp:
                    a.setPoints(3, border, sqsize/2,  sqsize/2, border,  sqsize - border, sqsize/2);
                    break;
                case PE_IndicatorArrowDown:
                    a.setPoints(3, border, sqsize/2,  sqsize/2, sqsize - border,  sqsize - border, sqsize/2);
                    break;
                case PE_IndicatorArrowRight:
                    a.setPoints(3, sqsize - border, sqsize/2,  sqsize/2, border,  sqsize/2, sqsize - border);
                    break;
                case PE_IndicatorArrowLeft:
                    a.setPoints(3, border, sqsize/2,  sqsize/2, border,  sqsize/2, sqsize - border);
                    break;
                default:
                    break;
            }

            int bsx = 0;
            int bsy = 0;

            if (opt->state & State_Sunken) {
                bsx = proxy()->pixelMetric(PM_ButtonShiftHorizontal, opt, w);
                bsy = proxy()->pixelMetric(PM_ButtonShiftVertical, opt, w);
            }

            QRect bounds = a.boundingRect();
            int sx = sqsize / 2 - bounds.center().x() - 1;
            int sy = sqsize / 2 - bounds.center().y() - 1;
            imagePainter.translate(sx + bsx, sy + bsy);
            imagePainter.setPen(opt->palette.buttonText().color());
            imagePainter.setBrush(opt->palette.buttonText());

            if (!(opt->state & State_Enabled)) {
                imagePainter.translate(1, 1);
                imagePainter.setBrush(opt->palette.light().color());
                imagePainter.setPen(opt->palette.light().color());
                imagePainter.drawPolygon(a);
                imagePainter.translate(-1, -1);
                imagePainter.setBrush(opt->palette.mid().color());
                imagePainter.setPen(opt->palette.mid().color());
            }

            imagePainter.drawPolygon(a);
            imagePainter.end();
            pixmap = QPixmap::fromImage(image);
            QPixmapCache::insert(pixmapName, pixmap);
        }
        int xOffset = r.x() + (r.width() - size)/2;
        int yOffset = r.y() + (r.height() - size)/2;
        p->drawPixmap(xOffset, yOffset, pixmap);
        break;
    }
    case PE_IndicatorCheckBox: {
        QBrush fill;
        if (opt->state & State_NoChange)
            fill = QBrush(opt->palette.base().color(), Qt::Dense4Pattern);
        else if (opt->state & State_Sunken)
            fill = opt->palette.button();
        else if (opt->state & State_Enabled)
            fill = opt->palette.base();
        else
            fill = opt->palette.background();
        p->save();
        doRestore = true;
        qDrawWinPanel(p, opt->rect, opt->palette, true, &fill);
        if (opt->state & State_NoChange)
            p->setPen(opt->palette.dark().color());
        else
            p->setPen(opt->palette.text().color());
    } // Fall through!
    case PE_IndicatorViewItemCheck: {
        if (!doRestore) {
            p->save();
            doRestore = true;
        }
        // XXX: specialize? (git blame)
        if (pe == PE_IndicatorViewItemCheck) {
            const QStyleOptionViewItem *itemViewOpt = qstyleoption_cast<const QStyleOptionViewItem *>(opt);
            p->setPen(itemViewOpt
                      && itemViewOpt->showDecorationSelected
                      && opt->state & State_Selected
                        ? opt->palette.highlightedText().color()
                        : opt->palette.text().color());
            if (opt->state & State_NoChange)
                p->setBrush(opt->palette.brush(QPalette::Button));
            p->drawRect(opt->rect.x() + 1, opt->rect.y() + 1, 11, 11);
        }
        if (!(opt->state & State_Off)) {
            QLineF lines[7];
            int i, xx, yy;
            xx = opt->rect.x() + 3;
            yy = opt->rect.y() + 5;
            for (i = 0; i < 3; ++i) {
                lines[i] = QLineF(xx, yy, xx, yy + 2);
                ++xx;
                ++yy;
            }
            yy -= 2;
            for (i = 3; i < 7; ++i) {
                lines[i] = QLineF(xx, yy, xx, yy + 2);
                ++xx;
                --yy;
            }
            p->drawLines(lines, 7);
        }
        if (doRestore)
            p->restore();
        break;
    }
    case PE_FrameFocusRect: {
        if (const QStyleOptionFocusRect *fropt = qstyleoption_cast<const QStyleOptionFocusRect *>(opt)) {
            if (!(fropt->state & State_KeyboardFocusChange) && !proxy()->styleHint(SH_UnderlineShortcut, opt))
                return;
            p->save();
            p->setBackgroundMode(Qt::TransparentMode);
            QColor bg_col = fropt->backgroundColor;
            if (!bg_col.isValid())
                bg_col = p->background().color();
            // Create an "XOR" color.
            QColor patternCol((bg_col.red() ^ 0xff) & 0xff,
                              (bg_col.green() ^ 0xff) & 0xff,
                              (bg_col.blue() ^ 0xff) & 0xff);
            p->setBrush(QBrush(patternCol, Qt::Dense4Pattern));
            p->setBrushOrigin(opt->rect.topLeft());
            p->setPen(Qt::NoPen);
            p->drawRect(opt->rect.left(), opt->rect.top(), opt->rect.width(), 1);    // Top
            p->drawRect(opt->rect.left(), opt->rect.bottom(), opt->rect.width(), 1); // Bottom
            p->drawRect(opt->rect.left(), opt->rect.top(), 1, opt->rect.height());   // Left
            p->drawRect(opt->rect.right(), opt->rect.top(), 1, opt->rect.height());  // Right
            p->restore();
        }
        break;
    }
    case PE_IndicatorRadioButton: {
#define PTSARRLEN(x) sizeof(x)/(sizeof(QPoint))
        static const QPoint pts1[] = {              // dark lines
            QPoint(1, 9), QPoint(1, 8), QPoint(0, 7), QPoint(0, 4), QPoint(1, 3), QPoint(1, 2),
            QPoint(2, 1), QPoint(3, 1), QPoint(4, 0), QPoint(7, 0), QPoint(8, 1), QPoint(9, 1)
        };
        static const QPoint pts2[] = {              // black lines
            QPoint(2, 8), QPoint(1, 7), QPoint(1, 4), QPoint(2, 3), QPoint(2, 2), QPoint(3, 2),
            QPoint(4, 1), QPoint(7, 1), QPoint(8, 2), QPoint(9, 2)
        };
        static const QPoint pts3[] = {              // background lines
            QPoint(2, 9), QPoint(3, 9), QPoint(4, 10), QPoint(7, 10), QPoint(8, 9), QPoint(9, 9),
            QPoint(9, 8), QPoint(10, 7), QPoint(10, 4), QPoint(9, 3)
        };
        static const QPoint pts4[] = {              // white lines
            QPoint(2, 10), QPoint(3, 10), QPoint(4, 11), QPoint(7, 11), QPoint(8, 10),
            QPoint(9, 10), QPoint(10, 9), QPoint(10, 8), QPoint(11, 7), QPoint(11, 4),
            QPoint(10, 3), QPoint(10, 2)
        };
        static const QPoint pts5[] = {              // inner fill
            QPoint(4, 2), QPoint(7, 2), QPoint(9, 4), QPoint(9, 7), QPoint(7, 9), QPoint(4, 9),
            QPoint(2, 7), QPoint(2, 4)
        };

        // make sure the indicator is square
        QRect ir = opt->rect;

        if (opt->rect.width() < opt->rect.height()) {
            ir.setTop(opt->rect.top() + (opt->rect.height() - opt->rect.width()) / 2);
            ir.setHeight(opt->rect.width());
        } else if (opt->rect.height() < opt->rect.width()) {
            ir.setLeft(opt->rect.left() + (opt->rect.width() - opt->rect.height()) / 2);
            ir.setWidth(opt->rect.height());
        }

        p->save();
        bool down = opt->state & State_Sunken;
        bool enabled = opt->state & State_Enabled;
        bool on = opt->state & State_On;

        //center when rect is larger than indicator size
        int xOffset = 0;
        int yOffset = 0;
        int indicatorWidth = proxy()->pixelMetric(PM_ExclusiveIndicatorWidth);
        int indicatorHeight = proxy()->pixelMetric(PM_ExclusiveIndicatorWidth);
        if (ir.width() > indicatorWidth)
            xOffset += (ir.width() - indicatorWidth)/2;
        if (ir.height() > indicatorHeight)
            yOffset += (ir.height() - indicatorHeight)/2;
        p->translate(xOffset, yOffset);

        p->translate(ir.x(), ir.y());

        p->setPen(opt->palette.dark().color());
        p->drawPolyline(pts1, PTSARRLEN(pts1));

        p->setPen(opt->palette.shadow().color());
        p->drawPolyline(pts2, PTSARRLEN(pts2));

        p->setPen(opt->palette.midlight().color());
        p->drawPolyline(pts3, PTSARRLEN(pts3));

        p->setPen(opt->palette.light().color());
        p->drawPolyline(pts4, PTSARRLEN(pts4));

        QColor fillColor = (down || !enabled)
                            ? opt->palette.button().color()
                            : opt->palette.base().color();
        p->setPen(fillColor);
        p->setBrush(fillColor) ;
        p->drawPolygon(pts5, PTSARRLEN(pts5));

        p->translate(-ir.x(), -ir.y()); // restore translate

        if (on) {
            p->setPen(Qt::NoPen);
            p->setBrush(opt->palette.text());
            p->drawRect(ir.x() + 5, ir.y() + 4, 2, 4);
            p->drawRect(ir.x() + 4, ir.y() + 5, 4, 2);
        }
        p->restore();
        break;
    }
#ifndef QT_NO_FRAME
    case PE_Frame:
    case PE_FrameMenu: {
        if (const QStyleOptionFrame *frame = qstyleoption_cast<const QStyleOptionFrame *>(opt)) {
            if (frame->lineWidth == 2 || pe == PE_Frame) {
                QPalette popupPal = frame->palette;
                if (pe == PE_FrameMenu) {
                    popupPal.setColor(QPalette::Light, frame->palette.background().color());
                    popupPal.setColor(QPalette::Midlight, frame->palette.light().color());
                }
                if (pe == PE_Frame && (frame->state & State_Raised))
                    qDrawWinButton(p, frame->rect, popupPal, frame->state & State_Sunken);
                else if (pe == PE_Frame && (frame->state & State_Sunken))
                {
                    popupPal.setColor(QPalette::Midlight, frame->palette.background().color());
                    qDrawWinPanel(p, frame->rect, popupPal, frame->state & State_Sunken);
                }
                else
                    qDrawWinPanel(p, frame->rect, popupPal, frame->state & State_Sunken);
            } else {
                QCommonStyle::drawPrimitive(pe, opt, p, w);
            }
        } else {
            QPalette popupPal = opt->palette;
            popupPal.setColor(QPalette::Light, opt->palette.background().color());
            popupPal.setColor(QPalette::Midlight, opt->palette.light().color());
            qDrawWinPanel(p, opt->rect, popupPal, opt->state & State_Sunken);
        }
        break;
    }
#endif // QT_NO_FRAME
    case PE_IndicatorBranch: {
        // This is _way_ too similar to the common style.
        static const int decoration_size = 9;
        int mid_h = opt->rect.x() + opt->rect.width() / 2;
        int mid_v = opt->rect.y() + opt->rect.height() / 2;
        int bef_h = mid_h;
        int bef_v = mid_v;
        int aft_h = mid_h;
        int aft_v = mid_v;
        if (opt->state & State_Children) {
            int delta = decoration_size / 2;
            bef_h -= delta;
            bef_v -= delta;
            aft_h += delta;
            aft_v += delta;
            p->drawLine(bef_h + 2, bef_v + 4, bef_h + 6, bef_v + 4);
            if (!(opt->state & State_Open))
                p->drawLine(bef_h + 4, bef_v + 2, bef_h + 4, bef_v + 6);
            QPen oldPen = p->pen();
            p->setPen(opt->palette.dark().color());
            p->drawRect(bef_h, bef_v, decoration_size - 1, decoration_size - 1);
            p->setPen(oldPen);
        }
        QBrush brush(opt->palette.dark().color(), Qt::Dense4Pattern);
        if (opt->state & State_Item) {
            if (opt->direction == Qt::RightToLeft)
                p->fillRect(opt->rect.left(), mid_v, bef_h - opt->rect.left(), 1, brush);
            else
                p->fillRect(aft_h, mid_v, opt->rect.right() - aft_h + 1, 1, brush);
        }
        if (opt->state & State_Sibling)
            p->fillRect(mid_h, aft_v, 1, opt->rect.bottom() - aft_v + 1, brush);
        if (opt->state & (State_Open | State_Children | State_Item | State_Sibling))
            p->fillRect(mid_h, opt->rect.y(), 1, bef_v - opt->rect.y(), brush);
        break;
    }
    case PE_FrameButtonBevel:
    case PE_PanelButtonBevel: {
        QBrush fill;
        bool panel = pe != PE_FrameButtonBevel;
        p->setBrushOrigin(opt->rect.topLeft());
        if (!(opt->state & State_Sunken) && (opt->state & State_On))
            fill = QBrush(opt->palette.light().color(), Qt::Dense4Pattern);
        else
            fill = opt->palette.brush(QPalette::Button);

        if (opt->state & (State_Raised | State_On | State_Sunken)) {
            qDrawWinButton(p, opt->rect, opt->palette, opt->state & (State_Sunken | State_On),
                           panel ? &fill : 0);
        } else {
            if (panel)
                p->fillRect(opt->rect, fill);
            else
                p->drawRect(opt->rect);
        }
        break;
    }
    case PE_FrameWindow: {
        QPalette popupPal = opt->palette;
        popupPal.setColor(QPalette::Light, opt->palette.background().color());
        popupPal.setColor(QPalette::Midlight, opt->palette.light().color());
        qDrawWinPanel(p, opt->rect, popupPal, opt->state & State_Sunken);
        break;
    }
#ifndef QT_NO_DOCKWIDGET
    case PE_IndicatorDockWidgetResizeHandle: {
        QPen oldPen = p->pen();
        p->setPen(opt->palette.light().color());
        if (opt->state & State_Horizontal) {
            p->drawLine(opt->rect.left(),          opt->rect.top(),
                        opt->rect.right(), opt->rect.top());
            p->setPen(opt->palette.dark().color());
            p->drawLine(opt->rect.left(),          opt->rect.bottom() - 1,
                        opt->rect.right(), opt->rect.bottom() - 1);
            p->setPen(opt->palette.shadow().color());
            p->drawLine(opt->rect.left(),          opt->rect.bottom(),
                        opt->rect.right(), opt->rect.bottom());
        } else {
            p->drawLine(opt->rect.left(), opt->rect.top(),
                        opt->rect.left(), opt->rect.bottom());
            p->setPen(opt->palette.dark().color());
            p->drawLine(opt->rect.right() - 1, opt->rect.top(),
                        opt->rect.right() - 1, opt->rect.bottom());
            p->setPen(opt->palette.shadow().color());
            p->drawLine(opt->rect.right(), opt->rect.top(),
                        opt->rect.right(), opt->rect.bottom());
        }
        p->setPen(oldPen);
        break;
    }
    case PE_FrameDockWidget: {
        if (qstyleoption_cast<const QStyleOptionFrame *>(opt)) {
            proxy()->drawPrimitive(QStyle::PE_FrameWindow, opt, p, w);
        }
        break;
    }
#endif // QT_NO_DOCKWIDGET
    case PE_FrameStatusBarItem: {
        qDrawShadePanel(p, opt->rect, opt->palette, true, 1, 0);
        break;
    }
#ifndef QT_NO_PROGRESSBAR
    case PE_IndicatorProgressChunk: {
        bool vertical = false, inverted = false;
        if (const QStyleOptionProgressBar *pb = qstyleoption_cast<const QStyleOptionProgressBar *>(opt)) {
            vertical = (pb->orientation == Qt::Vertical);
            inverted = pb->invertedAppearance;
        }

        int space = 2;
        int chunksize = proxy()->pixelMetric(PM_ProgressBarChunkWidth, opt, w) - space;
        if (!vertical) {
            if (opt->rect.width() <= chunksize)
                space = 0;

            if (inverted)
                p->fillRect(opt->rect.x() + space, opt->rect.y(), opt->rect.width() - space, opt->rect.height(),
                        opt->palette.brush(QPalette::Highlight));
            else
                p->fillRect(opt->rect.x(), opt->rect.y(), opt->rect.width() - space, opt->rect.height(),
                            opt->palette.brush(QPalette::Highlight));
        } else {
            if (opt->rect.height() <= chunksize)
                space = 0;

            if (inverted)
                p->fillRect(opt->rect.x(), opt->rect.y(), opt->rect.width(), opt->rect.height() - space,
                        opt->palette.brush(QPalette::Highlight));
            else
                p->fillRect(opt->rect.x(), opt->rect.y() + space, opt->rect.width(), opt->rect.height() - space,
                            opt->palette.brush(QPalette::Highlight));
        }
        break;
    }
#endif // QT_NO_PROGRESSBAR

    case PE_FrameTabWidget: {
        qDrawWinButton(p, opt->rect, opt->palette, false, 0);
        break;
    }
    default:
        QCommonStyle::drawPrimitive(pe, opt, p, w);
    }
}

/*! \reimp */
void QWindowsStyle::drawControl(ControlElement ce, const QStyleOption *opt, QPainter *p,
                                const QWidget *widget) const
{
    switch (ce) {
#ifndef QT_NO_RUBBERBAND
    case CE_RubberBand: {
        if (qstyleoption_cast<const QStyleOptionRubberBand *>(opt)) {
            // ### workaround for slow general painter path
            QPixmap tiledPixmap(16, 16);
            QPainter pixmapPainter(&tiledPixmap);
            pixmapPainter.setPen(Qt::NoPen);
            pixmapPainter.setBrush(Qt::Dense4Pattern);
            pixmapPainter.setBackground(Qt::white);
            pixmapPainter.setBackgroundMode(Qt::OpaqueMode);
            pixmapPainter.drawRect(0, 0, tiledPixmap.width(), tiledPixmap.height());
            pixmapPainter.end();
            tiledPixmap = QPixmap::fromImage(tiledPixmap.toImage());
            p->save();
            QRect r = opt->rect;
            QStyleHintReturnMask mask;
            if (proxy()->styleHint(QStyle::SH_RubberBand_Mask, opt, widget, &mask))
                p->setClipRegion(mask.region);
            p->drawTiledPixmap(r.x(), r.y(), r.width(), r.height(), tiledPixmap);
            p->restore();
            return;
        }
        break;
    }
#endif // QT_NO_RUBBERBAND
#if !defined(QT_NO_MENU) && !defined(QT_NO_MAINWINDOW)
    case CE_MenuBarEmptyArea: {
        if (widget && qobject_cast<const QMainWindow *>(widget->parentWidget())) {
            p->fillRect(opt->rect, opt->palette.button());
            QPen oldPen = p->pen();
            p->setPen(QPen(opt->palette.dark().color()));
            p->drawLine(opt->rect.bottomLeft(), opt->rect.bottomRight());
            p->setPen(oldPen);
        }
        break;
    }
#endif
#ifndef QT_NO_MENU
    case CE_MenuItem: {
        if (const QStyleOptionMenuItem *menuitem = qstyleoption_cast<const QStyleOptionMenuItem *>(opt)) {
            int x, y, w, h;
            menuitem->rect.getRect(&x, &y, &w, &h);
            int tab = menuitem->tabWidth;
            bool dis = !(menuitem->state & State_Enabled);
            bool checked = menuitem->checkType != QStyleOptionMenuItem::NotCheckable
                            ? menuitem->checked : false;
            bool act = menuitem->state & State_Selected;

            // windows always has a check column, regardless whether we have an icon or not
            int checkcol = qMax<int>(menuitem->maxIconWidth, QWindowsStylePrivate::windowsCheckMarkWidth);

            QBrush fill = menuitem->palette.brush(act ? QPalette::Highlight : QPalette::Button);
            p->fillRect(menuitem->rect.adjusted(0, 0, -1, 0), fill);

            if (menuitem->menuItemType == QStyleOptionMenuItem::Separator){
                int yoff = y-1 + h / 2;
                p->setPen(menuitem->palette.dark().color());
                p->drawLine(x + 2, yoff, x + w - 4, yoff);
                p->setPen(menuitem->palette.light().color());
                p->drawLine(x + 2, yoff + 1, x + w - 4, yoff + 1);
                return;
            }

            QRect vCheckRect = visualRect(opt->direction, menuitem->rect, QRect(menuitem->rect.x(), menuitem->rect.y(), checkcol, menuitem->rect.height()));
            if (!menuitem->icon.isNull() && checked) {
                if (act) {
                    qDrawShadePanel(p, vCheckRect,
                                    menuitem->palette, true, 1,
                                    &menuitem->palette.brush(QPalette::Button));
                } else {
                    QBrush fill(menuitem->palette.light().color(), Qt::Dense4Pattern);
                    qDrawShadePanel(p, vCheckRect, menuitem->palette, true, 1, &fill);
                }
            } else if (!act) {
                p->fillRect(vCheckRect, menuitem->palette.brush(QPalette::Button));
            }

            // On Windows Style, if we have a checkable item and an icon we
            // draw the icon recessed to indicate an item is checked. If we
            // have no icon, we draw a checkmark instead.
            if (!menuitem->icon.isNull()) {
                QIcon::Mode mode = dis ? QIcon::Disabled : QIcon::Normal;
                if (act && !dis)
                    mode = QIcon::Active;
                QPixmap pixmap;
                if (checked)
                    pixmap = menuitem->icon.pixmap(proxy()->pixelMetric(PM_SmallIconSize, opt, widget), mode, QIcon::On);
                else
                    pixmap = menuitem->icon.pixmap(proxy()->pixelMetric(PM_SmallIconSize, opt, widget), mode);
                int pixw = pixmap.width();
                int pixh = pixmap.height();
                if (act && !dis && !checked)
                    qDrawShadePanel(p, vCheckRect,  menuitem->palette, false, 1,
                                    &menuitem->palette.brush(QPalette::Button));
                QRect pmr(0, 0, pixw, pixh);
                pmr.moveCenter(vCheckRect.center());
                p->setPen(menuitem->palette.text().color());
                p->drawPixmap(pmr.topLeft(), pixmap);
            } else if (checked) {
                QStyleOptionMenuItem newMi = *menuitem;
                newMi.state = State_None;
                if (!dis)
                    newMi.state |= State_Enabled;
                if (act)
                    newMi.state |= State_On;
                newMi.rect = visualRect(opt->direction, menuitem->rect, QRect(menuitem->rect.x() + QWindowsStylePrivate::windowsItemFrame,
                                                                              menuitem->rect.y() + QWindowsStylePrivate::windowsItemFrame,
                                                                              checkcol - 2 * QWindowsStylePrivate::windowsItemFrame,
                                                                              menuitem->rect.height() - 2 * QWindowsStylePrivate::windowsItemFrame));
                proxy()->drawPrimitive(PE_IndicatorMenuCheckMark, &newMi, p, widget);
            }
            p->setPen(act ? menuitem->palette.highlightedText().color() : menuitem->palette.buttonText().color());

            QColor discol;
            if (dis) {
                discol = menuitem->palette.text().color();
                p->setPen(discol);
            }

            int xm = int(QWindowsStylePrivate::windowsItemFrame) + checkcol + int(QWindowsStylePrivate::windowsItemHMargin);
            int xpos = menuitem->rect.x() + xm;
            QRect textRect(xpos, y + QWindowsStylePrivate::windowsItemVMargin,
                           w - xm - QWindowsStylePrivate::windowsRightBorder - tab + 1, h - 2 * QWindowsStylePrivate::windowsItemVMargin);
            QRect vTextRect = visualRect(opt->direction, menuitem->rect, textRect);
            QString s = menuitem->text;
            if (!s.isEmpty()) {                     // draw text
                p->save();
                int t = s.indexOf(QLatin1Char('\t'));
                int text_flags = Qt::AlignVCenter | Qt::TextShowMnemonic | Qt::TextDontClip | Qt::TextSingleLine;
                if (!proxy()->styleHint(SH_UnderlineShortcut, menuitem, widget))
                    text_flags |= Qt::TextHideMnemonic;
                text_flags |= Qt::AlignLeft;
                if (t >= 0) {
                    QRect vShortcutRect = visualRect(opt->direction, menuitem->rect,
                        QRect(textRect.topRight(), QPoint(menuitem->rect.right(), textRect.bottom())));
                    if (dis && !act && proxy()->styleHint(SH_EtchDisabledText, opt, widget)) {
                        p->setPen(menuitem->palette.light().color());
                        p->drawText(vShortcutRect.adjusted(1,1,1,1), text_flags, s.mid(t + 1));
                        p->setPen(discol);
                    }
                    p->drawText(vShortcutRect, text_flags, s.mid(t + 1));
                    s = s.left(t);
                }
                QFont font = menuitem->font;
                if (menuitem->menuItemType == QStyleOptionMenuItem::DefaultItem)
                    font.setBold(true);
                p->setFont(font);
                if (dis && !act && proxy()->styleHint(SH_EtchDisabledText, opt, widget)) {
                    p->setPen(menuitem->palette.light().color());
                    p->drawText(vTextRect.adjusted(1,1,1,1), text_flags, s.left(t));
                    p->setPen(discol);
                }
                p->drawText(vTextRect, text_flags, s.left(t));
                p->restore();
            }
            if (menuitem->menuItemType == QStyleOptionMenuItem::SubMenu) {// draw sub menu arrow
                int dim = (h - 2 * QWindowsStylePrivate::windowsItemFrame) / 2;
                PrimitiveElement arrow;
                arrow = (opt->direction == Qt::RightToLeft) ? PE_IndicatorArrowLeft : PE_IndicatorArrowRight;
                xpos = x + w - QWindowsStylePrivate::windowsArrowHMargin - QWindowsStylePrivate::windowsItemFrame - dim;
                QRect  vSubMenuRect = visualRect(opt->direction, menuitem->rect, QRect(xpos, y + h / 2 - dim / 2, dim, dim));
                QStyleOptionMenuItem newMI = *menuitem;
                newMI.rect = vSubMenuRect;
                newMI.state = dis ? State_None : State_Enabled;
                if (act)
                    newMI.palette.setColor(QPalette::ButtonText,
                                           newMI.palette.highlightedText().color());
                proxy()->drawPrimitive(arrow, &newMI, p, widget);
            }

        }
        break;
    }
#endif // QT_NO_MENU
#ifndef QT_NO_MENUBAR
    case CE_MenuBarItem: {
        if (const QStyleOptionMenuItem *mbi = qstyleoption_cast<const QStyleOptionMenuItem *>(opt)) {
            bool active = mbi->state & State_Selected;
            bool hasFocus = mbi->state & State_HasFocus;
            bool down = mbi->state & State_Sunken;
            QStyleOptionMenuItem newMbi = *mbi;
            p->fillRect(mbi->rect, mbi->palette.brush(QPalette::Button));
            if (active || hasFocus) {
                QBrush b = mbi->palette.brush(QPalette::Button);
                if (active && down)
                    p->setBrushOrigin(p->brushOrigin() + QPoint(1, 1));
                if (active && hasFocus)
                    qDrawShadeRect(p, mbi->rect.x(), mbi->rect.y(), mbi->rect.width(),
                                   mbi->rect.height(), mbi->palette, active && down, 1, 0, &b);
                if (active && down) {
                    newMbi.rect.translate(proxy()->pixelMetric(PM_ButtonShiftHorizontal, mbi, widget),
                                       proxy()->pixelMetric(PM_ButtonShiftVertical, mbi, widget));
                    p->setBrushOrigin(p->brushOrigin() - QPoint(1, 1));
                }
            }
            QCommonStyle::drawControl(ce, &newMbi, p, widget);
        }
        break;
    }
#endif // QT_NO_MENUBAR
#ifndef QT_NO_TABBAR
    case CE_TabBarTabShape: {
        if (const QStyleOptionTab *tab = qstyleoption_cast<const QStyleOptionTab *>(opt)) {
            bool rtlHorTabs = (tab->direction == Qt::RightToLeft
                               && (tab->shape == QTabBar::RoundedNorth
                                   || tab->shape == QTabBar::RoundedSouth));
            bool selected = tab->state & State_Selected;
            bool lastTab = ((!rtlHorTabs && tab->position == QStyleOptionTab::End)
                            || (rtlHorTabs
                                && tab->position == QStyleOptionTab::Beginning));
            bool firstTab = ((!rtlHorTabs
                               && tab->position == QStyleOptionTab::Beginning)
                             || (rtlHorTabs
                                 && tab->position == QStyleOptionTab::End));
            bool onlyOne = tab->position == QStyleOptionTab::OnlyOneTab;
            bool previousSelected =
                ((!rtlHorTabs
                  && tab->selectedPosition == QStyleOptionTab::PreviousIsSelected)
                || (rtlHorTabs
                    && tab->selectedPosition == QStyleOptionTab::NextIsSelected));
            bool nextSelected =
                ((!rtlHorTabs
                  && tab->selectedPosition == QStyleOptionTab::NextIsSelected)
                 || (rtlHorTabs
                     && tab->selectedPosition
                            == QStyleOptionTab::PreviousIsSelected));
            int tabBarAlignment = proxy()->styleHint(SH_TabBar_Alignment, tab, widget);
            bool leftAligned = (!rtlHorTabs && tabBarAlignment == Qt::AlignLeft)
                                || (rtlHorTabs
                                    && tabBarAlignment == Qt::AlignRight);

            bool rightAligned = (!rtlHorTabs && tabBarAlignment == Qt::AlignRight)
                                 || (rtlHorTabs
                                         && tabBarAlignment == Qt::AlignLeft);

            QColor light = tab->palette.light().color();
            QColor dark = tab->palette.dark().color();
            QColor shadow = tab->palette.shadow().color();
            int borderThinkness = proxy()->pixelMetric(PM_TabBarBaseOverlap, tab, widget);
            if (selected)
                borderThinkness /= 2;
            QRect r2(opt->rect);
            int x1 = r2.left();
            int x2 = r2.right();
            int y1 = r2.top();
            int y2 = r2.bottom();
            switch (tab->shape) {
                case QTabBar::RoundedNorth: {
                    if (!selected) {
                        y1 += 2;
                        x1 += onlyOne || firstTab ? borderThinkness : 0;
                        x2 -= onlyOne || lastTab ? borderThinkness : 0;
                    }

                    p->fillRect(QRect(x1 + 1, y1 + 1, (x2 - x1) - 1, (y2 - y1) - 2), tab->palette.background());

                    // Delete border
                    if (selected) {
                        p->fillRect(QRect(x1,y2-1,x2-x1,1), tab->palette.background());
                        p->fillRect(QRect(x1,y2,x2-x1,1), tab->palette.background());
                    }
                    // Left
                    if (firstTab || selected || onlyOne || !previousSelected) {
                        p->setPen(light);
                        p->drawLine(x1, y1 + 2, x1, y2 - ((onlyOne || firstTab) && selected && leftAligned ? 0 : borderThinkness));
                        p->drawPoint(x1 + 1, y1 + 1);
                    }
                    // Top
                    {
                        int beg = x1 + (previousSelected ? 0 : 2);
                        int end = x2 - (nextSelected ? 0 : 2);
                        p->setPen(light);
                        p->drawLine(beg, y1, end, y1);
                    }
                    // Right
                    if (lastTab || selected || onlyOne || !nextSelected) {
                        p->setPen(shadow);
                        p->drawLine(x2, y1 + 2, x2, y2 - ((onlyOne || lastTab) && selected && rightAligned ? 0 : borderThinkness));
                        p->drawPoint(x2 - 1, y1 + 1);
                        p->setPen(dark);
                        p->drawLine(x2 - 1, y1 + 2, x2 - 1, y2 - ((onlyOne || lastTab) && selected && rightAligned ? 0 : borderThinkness));
                    }
                    break;
                }
                case QTabBar::RoundedSouth: {
                    if (!selected) {
                        y2 -= 2;
                        x1 += firstTab ? borderThinkness : 0;
                        x2 -= lastTab ? borderThinkness : 0;
                    }

                    p->fillRect(QRect(x1 + 1, y1 + 2, (x2 - x1) - 1, (y2 - y1) - 1), tab->palette.background());

                    // Delete border
                    if (selected) {
                        p->fillRect(QRect(x1, y1 + 1, (x2 - 1)-x1, 1), tab->palette.background());
                        p->fillRect(QRect(x1, y1, (x2 - 1)-x1, 1), tab->palette.background());
                    }
                    // Left
                    if (firstTab || selected || onlyOne || !previousSelected) {
                        p->setPen(light);
                        p->drawLine(x1, y2 - 2, x1, y1 + ((onlyOne || firstTab) && selected && leftAligned ? 0 : borderThinkness));
                        p->drawPoint(x1 + 1, y2 - 1);
                    }
                    // Bottom
                    {
                        int beg = x1 + (previousSelected ? 0 : 2);
                        int end = x2 - (nextSelected ? 0 : 2);
                        p->setPen(shadow);
                        p->drawLine(beg, y2, end, y2);
                        p->setPen(dark);
                        p->drawLine(beg, y2 - 1, end, y2 - 1);
                    }
                    // Right
                    if (lastTab || selected || onlyOne || !nextSelected) {
                        p->setPen(shadow);
                        p->drawLine(x2, y2 - 2, x2, y1 + ((onlyOne || lastTab) && selected && rightAligned ? 0 : borderThinkness));
                        p->drawPoint(x2 - 1, y2 - 1);
                        p->setPen(dark);
                        p->drawLine(x2 - 1, y2 - 2, x2 - 1, y1 + ((onlyOne || lastTab) && selected && rightAligned ? 0 : borderThinkness));
                    }
                    break;
                }
                case QTabBar::RoundedWest: {
                    if (!selected) {
                        x1 += 2;
                        y1 += firstTab ? borderThinkness : 0;
                        y2 -= lastTab ? borderThinkness : 0;
                    }

                    p->fillRect(QRect(x1 + 1, y1 + 1, (x2 - x1) - 2, (y2 - y1) - 1), tab->palette.background());

                    // Delete border
                    if (selected) {
                        p->fillRect(QRect(x2 - 1, y1, 1, y2-y1), tab->palette.background());
                        p->fillRect(QRect(x2, y1, 1, y2-y1), tab->palette.background());
                    }
                    // Top
                    if (firstTab || selected || onlyOne || !previousSelected) {
                        p->setPen(light);
                        p->drawLine(x1 + 2, y1, x2 - ((onlyOne || firstTab) && selected && leftAligned ? 0 : borderThinkness), y1);
                        p->drawPoint(x1 + 1, y1 + 1);
                    }
                    // Left
                    {
                        int beg = y1 + (previousSelected ? 0 : 2);
                        int end = y2 - (nextSelected ? 0 : 2);
                        p->setPen(light);
                        p->drawLine(x1, beg, x1, end);
                    }
                    // Bottom
                    if (lastTab || selected || onlyOne || !nextSelected) {
                        p->setPen(shadow);
                        p->drawLine(x1 + 3, y2, x2 - ((onlyOne || lastTab) && selected && rightAligned ? 0 : borderThinkness), y2);
                        p->drawPoint(x1 + 2, y2 - 1);
                        p->setPen(dark);
                        p->drawLine(x1 + 3, y2 - 1, x2 - ((onlyOne || lastTab) && selected && rightAligned ? 0 : borderThinkness), y2 - 1);
                        p->drawPoint(x1 + 1, y2 - 1);
                        p->drawPoint(x1 + 2, y2);
                    }
                    break;
                }
                case QTabBar::RoundedEast: {
                    if (!selected) {
                        x2 -= 2;
                        y1 += firstTab ? borderThinkness : 0;
                        y2 -= lastTab ? borderThinkness : 0;
                    }

                    p->fillRect(QRect(x1 + 2, y1 + 1, (x2 - x1) - 1, (y2 - y1) - 1), tab->palette.background());

                    // Delete border
                    if (selected) {
                        p->fillRect(QRect(x1 + 1, y1, 1, (y2 - 1)-y1),tab->palette.background());
                        p->fillRect(QRect(x1, y1, 1, (y2-1)-y1), tab->palette.background());
                    }
                    // Top
                    if (firstTab || selected || onlyOne || !previousSelected) {
                        p->setPen(light);
                        p->drawLine(x2 - 2, y1, x1 + ((onlyOne || firstTab) && selected && leftAligned ? 0 : borderThinkness), y1);
                        p->drawPoint(x2 - 1, y1 + 1);
                    }
                    // Right
                    {
                        int beg = y1 + (previousSelected ? 0 : 2);
                        int end = y2 - (nextSelected ? 0 : 2);
                        p->setPen(shadow);
                        p->drawLine(x2, beg, x2, end);
                        p->setPen(dark);
                        p->drawLine(x2 - 1, beg, x2 - 1, end);
                    }
                    // Bottom
                    if (lastTab || selected || onlyOne || !nextSelected) {
                        p->setPen(shadow);
                        p->drawLine(x2 - 2, y2, x1 + ((onlyOne || lastTab) && selected && rightAligned ? 0 : borderThinkness), y2);
                        p->drawPoint(x2 - 1, y2 - 1);
                        p->setPen(dark);
                        p->drawLine(x2 - 2, y2 - 1, x1 + ((onlyOne || lastTab) && selected && rightAligned ? 0 : borderThinkness), y2 - 1);
                    }
                    break;
                }
                default: {
                    QCommonStyle::drawControl(ce, tab, p, widget);
                    break;
                }
            }
        }
        break;
    }
#endif // QT_NO_TABBAR
    case CE_ToolBoxTabShape: {
        qDrawShadePanel(p, opt->rect, opt->palette,
                        opt->state & (State_Sunken | State_On), 1,
                        &opt->palette.brush(QPalette::Button));
        break;
    }
#ifndef QT_NO_SPLITTER
    case CE_Splitter: {
        p->eraseRect(opt->rect);
        break;
    }
#endif // QT_NO_SPLITTER
#ifndef QT_NO_SCROLLBAR
    case CE_ScrollBarSubLine:
    case CE_ScrollBarAddLine: {
        if ((opt->state & State_Sunken)) {
            p->setPen(opt->palette.dark().color());
            p->setBrush(opt->palette.brush(QPalette::Button));
            p->drawRect(opt->rect.adjusted(0, 0, -1, -1));
        } else {
            QStyleOption buttonOpt = *opt;
            if (!(buttonOpt.state & State_Sunken))
                buttonOpt.state |= State_Raised;
            QPalette pal(opt->palette);
            pal.setColor(QPalette::Button, opt->palette.light().color());
            pal.setColor(QPalette::Light, opt->palette.button().color());
            qDrawWinButton(p, opt->rect, pal, opt->state & (State_Sunken | State_On),
                           &opt->palette.brush(QPalette::Button));
        }
        PrimitiveElement arrow;
        if (opt->state & State_Horizontal) {
            if (ce == CE_ScrollBarAddLine)
                arrow = opt->direction == Qt::LeftToRight ? PE_IndicatorArrowRight : PE_IndicatorArrowLeft;
            else
                arrow = opt->direction == Qt::LeftToRight ? PE_IndicatorArrowLeft : PE_IndicatorArrowRight;
        } else {
            if (ce == CE_ScrollBarAddLine)
                arrow = PE_IndicatorArrowDown;
            else
                arrow = PE_IndicatorArrowUp;
        }
        QStyleOption arrowOpt = *opt;
        arrowOpt.rect = opt->rect.adjusted(4, 4, -4, -4);
        proxy()->drawPrimitive(arrow, &arrowOpt, p, widget);
        break;
    }
    case CE_ScrollBarAddPage:
    case CE_ScrollBarSubPage: {
        QBrush br;
        QBrush bg = p->background();
        Qt::BGMode bg_mode = p->backgroundMode();
        p->setPen(Qt::NoPen);
        p->setBackgroundMode(Qt::OpaqueMode);

        if (opt->state & State_Sunken) {
            br = QBrush(opt->palette.shadow().color(), Qt::Dense4Pattern);
            p->setBackground(opt->palette.dark().color());
            p->setBrush(br);
        } else {
            QPixmap pm = opt->palette.brush(QPalette::Light).texture();
            br = !pm.isNull() ? QBrush(pm) : QBrush(opt->palette.light().color(), Qt::Dense4Pattern);
            p->setBackground(opt->palette.background().color());
            p->setBrush(br);
        }
        p->drawRect(opt->rect);
        p->setBackground(bg);
        p->setBackgroundMode(bg_mode);
        break;
    }
    case CE_ScrollBarSlider: {
        if (!(opt->state & State_Enabled)) {
            QPixmap pm = opt->palette.brush(QPalette::Light).texture();
            QBrush br = !pm.isNull() ? QBrush(pm) : QBrush(opt->palette.light().color(), Qt::Dense4Pattern);
            p->setPen(Qt::NoPen);
            p->setBrush(br);
            p->setBackgroundMode(Qt::OpaqueMode);
            p->drawRect(opt->rect);
        } else {
            QStyleOptionButton buttonOpt;
            buttonOpt.QStyleOption::operator=(*opt);
            buttonOpt.state = State_Enabled | State_Raised;

            QPalette pal(opt->palette);
            pal.setColor(QPalette::Button, opt->palette.light().color());
            pal.setColor(QPalette::Light, opt->palette.button().color());
            qDrawWinButton(p, opt->rect, pal, false, &opt->palette.brush(QPalette::Button));
        }
        break;
    }
#endif // QT_NO_SCROLLBAR
    case CE_HeaderSection: {
        QBrush fill;
        if (opt->state & State_On)
            fill = QBrush(opt->palette.light().color(), Qt::Dense4Pattern);
        else
            fill = opt->palette.brush(QPalette::Button);

        if (opt->state & (State_Raised | State_Sunken)) {
            qDrawWinButton(p, opt->rect, opt->palette, opt->state & State_Sunken, &fill);
        } else {
            p->fillRect(opt->rect, fill);
        }
        break;
    }
#ifndef QT_NO_TOOLBAR
    case CE_ToolBar: {
        if (const QStyleOptionToolBar *toolbar = qstyleoption_cast<const QStyleOptionToolBar *>(opt)) {
            QRect rect = opt->rect;

            bool paintLeftBorder = true;
            bool paintRightBorder = true;
            bool paintBottomBorder = true;

            switch (toolbar->toolBarArea){
            case Qt::BottomToolBarArea: {
                switch(toolbar->positionOfLine) {
                case QStyleOptionToolBar::Beginning:
                case QStyleOptionToolBar::OnlyOne:
                    paintBottomBorder = false;
                default:
                    break;
                }
                break;
            }
            case Qt::TopToolBarArea: {
                switch(toolbar->positionWithinLine){
                case QStyleOptionToolBar::Beginning:
                    paintLeftBorder = false;
                    break;
                case QStyleOptionToolBar::End:
                    paintRightBorder = false;
                    break;
                case QStyleOptionToolBar::OnlyOne:
                    paintRightBorder = false;
                    paintLeftBorder = false;
                default:
                    break;
                }
                if(opt->direction == Qt::RightToLeft){ //reverse layout changes the order of Beginning/end
                    bool tmp = paintLeftBorder;
                    paintRightBorder=paintLeftBorder;
                    paintLeftBorder=tmp;
                }
                break;
            }
            case Qt::RightToolBarArea: {
                switch (toolbar->positionOfLine){
                case QStyleOptionToolBar::Beginning:
                case QStyleOptionToolBar::OnlyOne:
                    paintRightBorder = false;
                    break;
                default:
                    break;
                }
                break;
            }
            case Qt::LeftToolBarArea: {
                switch (toolbar->positionOfLine){
                case QStyleOptionToolBar::Beginning:
                case QStyleOptionToolBar::OnlyOne:
                    paintLeftBorder = false;
                    break;
                default:
                    break;
                }
                break;
            }
            default:
                break;
            }


            //draw top border
            p->setPen(QPen(opt->palette.light().color()));
            p->drawLine(rect.topLeft().x(),
                        rect.topLeft().y(),
                        rect.topRight().x(),
                        rect.topRight().y());

            if (paintLeftBorder){
                p->setPen(QPen(opt->palette.light().color()));
                p->drawLine(rect.topLeft().x(),
                            rect.topLeft().y(),
                            rect.bottomLeft().x(),
                            rect.bottomLeft().y());
            }

            if (paintRightBorder){
                p->setPen(QPen(opt->palette.dark().color()));
                p->drawLine(rect.topRight().x(),
                            rect.topRight().y(),
                            rect.bottomRight().x(),
                            rect.bottomRight().y());
            }

            if (paintBottomBorder){
                p->setPen(QPen(opt->palette.dark().color()));
                p->drawLine(rect.bottomLeft().x(),
                            rect.bottomLeft().y(),
                            rect.bottomRight().x(),
                            rect.bottomRight().y());
            }
        }
        break;
    }
#endif // QT_NO_TOOLBAR
#ifndef QT_NO_PROGRESSBAR
    case CE_ProgressBarContents: {
        if (const QStyleOptionProgressBar *pb = qstyleoption_cast<const QStyleOptionProgressBar *>(opt)) {
            QRect rect = pb->rect;
            if (!rect.isValid())
                return;

            // Get extra style options if version 2
            bool vertical = (pb->orientation == Qt::Vertical);
            bool inverted = pb->invertedAppearance;

            QMatrix m;
            if (vertical) {
                rect = QRect(rect.y(), rect.x(), rect.height(), rect.width()); // flip width and height
                m.rotate(90);
                m.translate(0, -(rect.height() + rect.y()*2));
            }
            QPalette pal2 = pb->palette;
            // Correct the highlight color if it is the same as the background
            if (pal2.highlight() == pal2.background())
                pal2.setColor(QPalette::Highlight, pb->palette.color(QPalette::Active,
                                                                     QPalette::Highlight));
            bool reverse = ((!vertical && (pb->direction == Qt::RightToLeft)) || vertical);
            if (inverted)
                reverse = !reverse;
            int w = rect.width();
            if (pb->minimum == 0 && pb->maximum == 0) {
                Q_D(const QWindowsStyle);
                const int unit_width = proxy()->pixelMetric(PM_ProgressBarChunkWidth, pb, widget);
                QStyleOptionProgressBar pbBits = *pb;
                Q_ASSERT(unit_width >0);

                pbBits.rect = rect;
                pbBits.palette = pal2;

                int chunkCount = w / unit_width + 1;
                int step = d->animateStep%chunkCount;
                int chunksInRow = 5;
                int myY = pbBits.rect.y();
                int myHeight = pbBits.rect.height();
                int chunksToDraw = chunksInRow;

                if(step > chunkCount - 5)chunksToDraw = (chunkCount - step);
                p->save();
                p->setClipRect(m.mapRect(QRectF(rect)).toRect());

                int x0 = reverse ? rect.left() + rect.width() - unit_width*(step) - unit_width  : rect.left() + unit_width * step;
                int x = 0;

                for (int i = 0; i < chunksToDraw ; ++i) {
                    pbBits.rect.setRect(x0 + x, myY, unit_width, myHeight);
                    pbBits.rect = m.mapRect(QRectF(pbBits.rect)).toRect();
                    proxy()->drawPrimitive(PE_IndicatorProgressChunk, &pbBits, p, widget);
                    x += reverse ? -unit_width : unit_width;
                }
                //Draw wrap-around chunks
                if( step > chunkCount-5){
                    x0 = reverse ? rect.left() + rect.width() - unit_width : rect.left() ;
                    x = 0;
                    int chunksToDraw = step - (chunkCount - chunksInRow);
                    for (int i = 0; i < chunksToDraw ; ++i) {
                        pbBits.rect.setRect(x0 + x, myY, unit_width, myHeight);
                        pbBits.rect = m.mapRect(QRectF(pbBits.rect)).toRect();
                        proxy()->drawPrimitive(PE_IndicatorProgressChunk, &pbBits, p, widget);
                        x += reverse ? -unit_width : unit_width;
                    }
                }
                p->restore(); //restore state
            } else {
                QCommonStyle::drawControl(ce, opt, p, widget);
            }
        }
        break;
    }
#endif // QT_NO_PROGRESSBAR

#ifndef QT_NO_DOCKWIDGET
    case CE_DockWidgetTitle: {
        if (const QStyleOptionDockWidget *dwOpt = qstyleoption_cast<const QStyleOptionDockWidget *>(opt)) {
            Q_D(const QWindowsStyle);

            bool verticalTitleBar = dwOpt->verticalTitleBar;

            QRect rect = dwOpt->rect;
            QRect r = rect;

            if (verticalTitleBar) {
                QSize s = r.size();
                s.transpose();
                r.setSize(s);

                p->save();
                p->translate(r.left(), r.top() + r.width());
                p->rotate(-90);
                p->translate(-r.left(), -r.top());
            }

            bool floating = false;
            bool active = dwOpt->state & State_Active;
            QColor inactiveCaptionTextColor = d->inactiveCaptionText;
            if (dwOpt->movable) {
                QColor left, right;

                //Titlebar gradient
                if (widget && widget->isWindow()) {
                    floating = true;
                    if (active) {
                        left = d->activeCaptionColor;
                        right = d->activeGradientCaptionColor;
                    } else {
                        left = d->inactiveCaptionColor;
                        right = d->inactiveGradientCaptionColor;
                    }
                    QBrush fillBrush(left);
                    if (left != right) {
                        QPoint p1(r.x(), r.top() + r.height()/2);
                        QPoint p2(rect.right(), r.top() + r.height()/2);
                        QLinearGradient lg(p1, p2);
                        lg.setColorAt(0, left);
                        lg.setColorAt(1, right);
                        fillBrush = lg;
                    }
                    p->fillRect(r.adjusted(0, 0, 0, -3), fillBrush);
                }
                p->setPen(dwOpt->palette.color(QPalette::Light));
                if (!widget || !widget->isWindow()) {
                    p->drawLine(r.topLeft(), r.topRight());
                    p->setPen(dwOpt->palette.color(QPalette::Dark));
                    p->drawLine(r.bottomLeft(), r.bottomRight());
                }
            }
            if (!dwOpt->title.isEmpty()) {
                QFont oldFont = p->font();
                if (floating) {
                    QFont font = oldFont;
                    font.setBold(true);
                    p->setFont(font);
                }
                QPalette palette = dwOpt->palette;
                palette.setColor(QPalette::Window, inactiveCaptionTextColor);
                QRect titleRect = subElementRect(SE_DockWidgetTitleBarText, opt, widget);
                if (verticalTitleBar) {
                    titleRect = QRect(r.left() + rect.bottom()
                                        - titleRect.bottom(),
                                    r.top() + titleRect.left() - rect.left(),
                                    titleRect.height(), titleRect.width());
                }
                proxy()->drawItemText(p, titleRect,
                            Qt::AlignLeft | Qt::AlignVCenter | Qt::TextShowMnemonic, palette,
                            dwOpt->state & State_Enabled, dwOpt->title,
                            floating ? (active ? QPalette::BrightText : QPalette::Window) : QPalette::WindowText);
                p->setFont(oldFont);
            }
            if (verticalTitleBar)
                p->restore();
        }
        return;
    }
#endif // QT_NO_DOCKWIDGET
    default:
        QCommonStyle::drawControl(ce, opt, p, widget);
    }
}

/*! \reimp */
QRect QWindowsStyle::subElementRect(SubElement sr, const QStyleOption *opt, const QWidget *w) const
{
    QRect r;
    switch (sr) {
    case SE_SliderFocusRect:
    case SE_ToolBoxTabContents:
        r = visualRect(opt->direction, opt->rect, opt->rect);
        break;
    case SE_DockWidgetTitleBarText: {
        r = QCommonStyle::subElementRect(sr, opt, w);
        const QStyleOptionDockWidget *dwOpt = qstyleoption_cast<const QStyleOptionDockWidget*>(opt);
        bool verticalTitleBar = dwOpt == 0 ? false : dwOpt->verticalTitleBar;
        int m = proxy()->pixelMetric(PM_DockWidgetTitleMargin, opt, w);
        if (verticalTitleBar) {
            r.adjust(0, 0, 0, -m);
        } else {
            if (opt->direction == Qt::LeftToRight)
                r.adjust(m, 0, 0, 0);
            else
                r.adjust(0, 0, -m, 0);
        }
        break;
    }
    case SE_ProgressBarContents:
        r = QCommonStyle::subElementRect(SE_ProgressBarGroove, opt, w);
        r.adjust(3, 3, -3, -3);
        break;
    default:
        r = QCommonStyle::subElementRect(sr, opt, w);
    }
    return r;
}


/*! \reimp */
void QWindowsStyle::drawComplexControl(ComplexControl cc, const QStyleOptionComplex *opt,
                                       QPainter *p, const QWidget *widget) const
{
    switch (cc) {
#ifndef QT_NO_SLIDER
    case CC_Slider:
        if (const QStyleOptionSlider *slider = qstyleoption_cast<const QStyleOptionSlider *>(opt)) {
            int thickness  = proxy()->pixelMetric(PM_SliderControlThickness, slider, widget);
            int len        = proxy()->pixelMetric(PM_SliderLength, slider, widget);
            int ticks = slider->tickPosition;
            QRect groove = proxy()->subControlRect(CC_Slider, slider, SC_SliderGroove, widget);
            QRect handle = proxy()->subControlRect(CC_Slider, slider, SC_SliderHandle, widget);

            if ((slider->subControls & SC_SliderGroove) && groove.isValid()) {
                int mid = thickness / 2;

                if (ticks & QSlider::TicksAbove)
                    mid += len / 8;
                if (ticks & QSlider::TicksBelow)
                    mid -= len / 8;

                p->setPen(slider->palette.shadow().color());
                if (slider->orientation == Qt::Horizontal) {
                    qDrawWinPanel(p, groove.x(), groove.y() + mid - 2,
                                   groove.width(), 4, slider->palette, true);
                    p->drawLine(groove.x() + 1, groove.y() + mid - 1,
                                groove.x() + groove.width() - 3, groove.y() + mid - 1);
                } else {
                    qDrawWinPanel(p, groove.x() + mid - 2, groove.y(),
                                  4, groove.height(), slider->palette, true);
                    p->drawLine(groove.x() + mid - 1, groove.y() + 1,
                                groove.x() + mid - 1, groove.y() + groove.height() - 3);
                }
            }

            if (slider->subControls & SC_SliderTickmarks) {
                QStyleOptionSlider tmpSlider = *slider;
                tmpSlider.subControls = SC_SliderTickmarks;
                QCommonStyle::drawComplexControl(cc, &tmpSlider, p, widget);
            }

            if (slider->subControls & SC_SliderHandle) {
                // 4444440
                // 4333310
                // 4322210
                // 4322210
                // 4322210
                // 4322210
                // *43210*
                // **410**
                // ***0***
                const QColor c0 = slider->palette.shadow().color();
                const QColor c1 = slider->palette.dark().color();
                // const QColor c2 = g.button();
                const QColor c3 = slider->palette.midlight().color();
                const QColor c4 = slider->palette.light().color();
                QBrush handleBrush;

                if (slider->state & State_Enabled) {
                    handleBrush = slider->palette.color(QPalette::Button);
                } else {
                    handleBrush = QBrush(slider->palette.color(QPalette::Button),
                                         Qt::Dense4Pattern);
                }


                int x = handle.x(), y = handle.y(),
                   wi = handle.width(), he = handle.height();

                int x1 = x;
                int x2 = x+wi-1;
                int y1 = y;
                int y2 = y+he-1;

                Qt::Orientation orient = slider->orientation;
                bool tickAbove = slider->tickPosition == QSlider::TicksAbove;
                bool tickBelow = slider->tickPosition == QSlider::TicksBelow;

                if (slider->state & State_HasFocus) {
                    QStyleOptionFocusRect fropt;
                    fropt.QStyleOption::operator=(*slider);
                    fropt.rect = subElementRect(SE_SliderFocusRect, slider, widget);
                    proxy()->drawPrimitive(PE_FrameFocusRect, &fropt, p, widget);
                }

                if ((tickAbove && tickBelow) || (!tickAbove && !tickBelow)) {
                    Qt::BGMode oldMode = p->backgroundMode();
                    p->setBackgroundMode(Qt::OpaqueMode);
                    qDrawWinButton(p, QRect(x, y, wi, he), slider->palette, false,
                                   &handleBrush);
                    p->setBackgroundMode(oldMode);
                    return;
                }

                QSliderDirection dir;

                if (orient == Qt::Horizontal)
                    if (tickAbove)
                        dir = SlUp;
                    else
                        dir = SlDown;
                else
                    if (tickAbove)
                        dir = SlLeft;
                    else
                        dir = SlRight;

                QPolygon a;

                int d = 0;
                switch (dir) {
                case SlUp:
                    y1 = y1 + wi/2;
                    d =  (wi + 1) / 2 - 1;
                    a.setPoints(5, x1,y1, x1,y2, x2,y2, x2,y1, x1+d,y1-d);
                    break;
                case SlDown:
                    y2 = y2 - wi/2;
                    d =  (wi + 1) / 2 - 1;
                    a.setPoints(5, x1,y1, x1,y2, x1+d,y2+d, x2,y2, x2,y1);
                    break;
                case SlLeft:
                    d =  (he + 1) / 2 - 1;
                    x1 = x1 + he/2;
                    a.setPoints(5, x1,y1, x1-d,y1+d, x1,y2, x2,y2, x2,y1);
                    break;
                case SlRight:
                    d =  (he + 1) / 2 - 1;
                    x2 = x2 - he/2;
                    a.setPoints(5, x1,y1, x1,y2, x2,y2, x2+d,y1+d, x2,y1);
                    break;
                }

                QBrush oldBrush = p->brush();
                p->setPen(Qt::NoPen);
                p->setBrush(handleBrush);
                Qt::BGMode oldMode = p->backgroundMode();
                p->setBackgroundMode(Qt::OpaqueMode);
                p->drawRect(x1, y1, x2-x1+1, y2-y1+1);
                p->drawPolygon(a);
                p->setBrush(oldBrush);
                p->setBackgroundMode(oldMode);

                if (dir != SlUp) {
                    p->setPen(c4);
                    p->drawLine(x1, y1, x2, y1);
                    p->setPen(c3);
                    p->drawLine(x1, y1+1, x2, y1+1);
                }
                if (dir != SlLeft) {
                    p->setPen(c3);
                    p->drawLine(x1+1, y1+1, x1+1, y2);
                    p->setPen(c4);
                    p->drawLine(x1, y1, x1, y2);
                }
                if (dir != SlRight) {
                    p->setPen(c0);
                    p->drawLine(x2, y1, x2, y2);
                    p->setPen(c1);
                    p->drawLine(x2-1, y1+1, x2-1, y2-1);
                }
                if (dir != SlDown) {
                    p->setPen(c0);
                    p->drawLine(x1, y2, x2, y2);
                    p->setPen(c1);
                    p->drawLine(x1+1, y2-1, x2-1, y2-1);
                }

                switch (dir) {
                case SlUp:
                    p->setPen(c4);
                    p->drawLine(x1, y1, x1+d, y1-d);
                    p->setPen(c0);
                    d = wi - d - 1;
                    p->drawLine(x2, y1, x2-d, y1-d);
                    d--;
                    p->setPen(c3);
                    p->drawLine(x1+1, y1, x1+1+d, y1-d);
                    p->setPen(c1);
                    p->drawLine(x2-1, y1, x2-1-d, y1-d);
                    break;
                case SlDown:
                    p->setPen(c4);
                    p->drawLine(x1, y2, x1+d, y2+d);
                    p->setPen(c0);
                    d = wi - d - 1;
                    p->drawLine(x2, y2, x2-d, y2+d);
                    d--;
                    p->setPen(c3);
                    p->drawLine(x1+1, y2, x1+1+d, y2+d);
                    p->setPen(c1);
                    p->drawLine(x2-1, y2, x2-1-d, y2+d);
                    break;
                case SlLeft:
                    p->setPen(c4);
                    p->drawLine(x1, y1, x1-d, y1+d);
                    p->setPen(c0);
                    d = he - d - 1;
                    p->drawLine(x1, y2, x1-d, y2-d);
                    d--;
                    p->setPen(c3);
                    p->drawLine(x1, y1+1, x1-d, y1+1+d);
                    p->setPen(c1);
                    p->drawLine(x1, y2-1, x1-d, y2-1-d);
                    break;
                case SlRight:
                    p->setPen(c4);
                    p->drawLine(x2, y1, x2+d, y1+d);
                    p->setPen(c0);
                    d = he - d - 1;
                    p->drawLine(x2, y2, x2+d, y2-d);
                    d--;
                    p->setPen(c3);
                    p->drawLine(x2, y1+1, x2+d, y1+1+d);
                    p->setPen(c1);
                    p->drawLine(x2, y2-1, x2+d, y2-1-d);
                    break;
                }
            }
        }
        break;
#endif // QT_NO_SLIDER
#ifndef QT_NO_SCROLLBAR
    case CC_ScrollBar:
        if (const QStyleOptionSlider *scrollbar = qstyleoption_cast<const QStyleOptionSlider *>(opt)) {
            QStyleOptionSlider newScrollbar = *scrollbar;
            if (scrollbar->minimum == scrollbar->maximum)
                newScrollbar.state &= ~State_Enabled; //do not draw the slider.
            QCommonStyle::drawComplexControl(cc, &newScrollbar, p, widget);
        }
        break;
#endif // QT_NO_SCROLLBAR
#ifndef QT_NO_COMBOBOX
    case CC_ComboBox:
        if (const QStyleOptionComboBox *cmb = qstyleoption_cast<const QStyleOptionComboBox *>(opt)) {
            QBrush editBrush = cmb->palette.brush(QPalette::Base);
            if ((cmb->subControls & SC_ComboBoxFrame)) {
                if (cmb->frame) {
                    QPalette shadePal = opt->palette;
                    shadePal.setColor(QPalette::Midlight, shadePal.button().color());
                    qDrawWinPanel(p, opt->rect, shadePal, true, &editBrush);
                }
                else {
                    p->fillRect(opt->rect, editBrush);
                }
            }
            if (cmb->subControls & SC_ComboBoxArrow) {
                State flags = State_None;

                QRect ar = proxy()->subControlRect(CC_ComboBox, cmb, SC_ComboBoxArrow, widget);
                bool sunkenArrow = cmb->activeSubControls == SC_ComboBoxArrow
                                   && cmb->state & State_Sunken;
                if (sunkenArrow) {
                    p->setPen(cmb->palette.dark().color());
                    p->setBrush(cmb->palette.brush(QPalette::Button));
                    p->drawRect(ar.adjusted(0,0,-1,-1));
                } else {
                    // Make qDrawWinButton use the right colors for drawing the shade of the button
                    QPalette pal(cmb->palette);
                    pal.setColor(QPalette::Button, cmb->palette.light().color());
                    pal.setColor(QPalette::Light, cmb->palette.button().color());
                    qDrawWinButton(p, ar, pal, false,
                                   &cmb->palette.brush(QPalette::Button));
                }

                ar.adjust(2, 2, -2, -2);
                if (opt->state & State_Enabled)
                    flags |= State_Enabled;
                if (opt->state & State_HasFocus)
                    flags |= State_HasFocus;

                if (sunkenArrow)
                    flags |= State_Sunken;
                QStyleOption arrowOpt(0);
                arrowOpt.rect = ar.adjusted(1, 1, -1, -1);
                arrowOpt.palette = cmb->palette;
                arrowOpt.state = flags;
                proxy()->drawPrimitive(PE_IndicatorArrowDown, &arrowOpt, p, widget);
            }

            if (cmb->subControls & SC_ComboBoxEditField) {
                QRect re = proxy()->subControlRect(CC_ComboBox, cmb, SC_ComboBoxEditField, widget);
                if (cmb->state & State_HasFocus && !cmb->editable)
                    p->fillRect(re.x(), re.y(), re.width(), re.height(),
                                cmb->palette.brush(QPalette::Highlight));

                if (cmb->state & State_HasFocus) {
                    p->setPen(cmb->palette.highlightedText().color());
                    p->setBackground(cmb->palette.highlight());

                } else {
                    p->setPen(cmb->palette.text().color());
                    p->setBackground(cmb->palette.background());
                }

                if (cmb->state & State_HasFocus && !cmb->editable) {
                    QStyleOptionFocusRect focus;
                    focus.QStyleOption::operator=(*cmb);
                    focus.rect = subElementRect(SE_ComboBoxFocusRect, cmb, widget);
                    focus.state |= State_FocusAtBorder;
                    focus.backgroundColor = cmb->palette.highlight().color();
                    proxy()->drawPrimitive(PE_FrameFocusRect, &focus, p, widget);
                }
            }
        }
        break;
#endif // QT_NO_COMBOBOX
#ifndef QT_NO_SPINBOX
    case CC_SpinBox:
        if (const QStyleOptionSpinBox *sb = qstyleoption_cast<const QStyleOptionSpinBox *>(opt)) {
            QStyleOptionSpinBox copy = *sb;
            PrimitiveElement pe;
            bool enabled = opt->state & State_Enabled;
            if (sb->frame && (sb->subControls & SC_SpinBoxFrame)) {
                QBrush editBrush = sb->palette.brush(QPalette::Base);
                QRect r = proxy()->subControlRect(CC_SpinBox, sb, SC_SpinBoxFrame, widget);
                QPalette shadePal = sb->palette;
                shadePal.setColor(QPalette::Midlight, shadePal.button().color());
                qDrawWinPanel(p, r, shadePal, true, &editBrush);
            }

            QPalette shadePal(opt->palette);
            shadePal.setColor(QPalette::Button, opt->palette.light().color());
            shadePal.setColor(QPalette::Light, opt->palette.button().color());

            if (sb->subControls & SC_SpinBoxUp) {
                copy.subControls = SC_SpinBoxUp;
                QPalette pal2 = sb->palette;
                if (!(sb->stepEnabled & QAbstractSpinBox::StepUpEnabled)) {
                    pal2.setCurrentColorGroup(QPalette::Disabled);
                    copy.state &= ~State_Enabled;
                }

                copy.palette = pal2;

                if (sb->activeSubControls == SC_SpinBoxUp && (sb->state & State_Sunken)) {
                    copy.state |= State_On;
                    copy.state |= State_Sunken;
                } else {
                    copy.state |= State_Raised;
                    copy.state &= ~State_Sunken;
                }
                pe = (sb->buttonSymbols == QAbstractSpinBox::PlusMinus ? PE_IndicatorSpinPlus
                                                                       : PE_IndicatorSpinUp);

                copy.rect = proxy()->subControlRect(CC_SpinBox, sb, SC_SpinBoxUp, widget);
                qDrawWinButton(p, copy.rect, shadePal, copy.state & (State_Sunken | State_On),
                                &copy.palette.brush(QPalette::Button));
                copy.rect.adjust(4, 1, -5, -1);
                if ((!enabled || !(sb->stepEnabled & QAbstractSpinBox::StepUpEnabled))
                    && proxy()->styleHint(SH_EtchDisabledText, opt, widget) )
                {
                    QStyleOptionSpinBox lightCopy = copy;
                    lightCopy.rect.adjust(1, 1, 1, 1);
                    lightCopy.palette.setBrush(QPalette::ButtonText, copy.palette.light());
                    proxy()->drawPrimitive(pe, &lightCopy, p, widget);
                }
                proxy()->drawPrimitive(pe, &copy, p, widget);
            }

            if (sb->subControls & SC_SpinBoxDown) {
                copy.subControls = SC_SpinBoxDown;
                copy.state = sb->state;
                QPalette pal2 = sb->palette;
                if (!(sb->stepEnabled & QAbstractSpinBox::StepDownEnabled)) {
                    pal2.setCurrentColorGroup(QPalette::Disabled);
                    copy.state &= ~State_Enabled;
                }
                copy.palette = pal2;

                if (sb->activeSubControls == SC_SpinBoxDown && (sb->state & State_Sunken)) {
                    copy.state |= State_On;
                    copy.state |= State_Sunken;
                } else {
                    copy.state |= State_Raised;
                    copy.state &= ~State_Sunken;
                }
                pe = (sb->buttonSymbols == QAbstractSpinBox::PlusMinus ? PE_IndicatorSpinMinus
                                                                       : PE_IndicatorSpinDown);

                copy.rect = proxy()->subControlRect(CC_SpinBox, sb, SC_SpinBoxDown, widget);
                qDrawWinButton(p, copy.rect, shadePal, copy.state & (State_Sunken | State_On),
                                &copy.palette.brush(QPalette::Button));
                copy.rect.adjust(4, 0, -5, -1);
                if ((!enabled || !(sb->stepEnabled & QAbstractSpinBox::StepDownEnabled))
                    && proxy()->styleHint(SH_EtchDisabledText, opt, widget) )
                {
                    QStyleOptionSpinBox lightCopy = copy;
                    lightCopy.rect.adjust(1, 1, 1, 1);
                    lightCopy.palette.setBrush(QPalette::ButtonText, copy.palette.light());
                    proxy()->drawPrimitive(pe, &lightCopy, p, widget);
                }
                proxy()->drawPrimitive(pe, &copy, p, widget);
            }
        }
        break;
#endif // QT_NO_SPINBOX

    default:
        QCommonStyle::drawComplexControl(cc, opt, p, widget);
    }
}

/*! \reimp */
QSize QWindowsStyle::sizeFromContents(ContentsType ct, const QStyleOption *opt,
                                      const QSize &csz, const QWidget *widget) const
{
    QSize sz(csz);
    switch (ct) {
    case CT_PushButton:
        if (const QStyleOptionButton *btn = qstyleoption_cast<const QStyleOptionButton *>(opt)) {
            sz = QCommonStyle::sizeFromContents(ct, opt, csz, widget);
            int w = sz.width(),
                h = sz.height();
            int defwidth = 0;
            if (btn->features & QStyleOptionButton::AutoDefaultButton)
                defwidth = 2 * proxy()->pixelMetric(PM_ButtonDefaultIndicator, btn, widget);
            const int minwidth = 75;
            const int minheight = 23;

            if (w < minwidth + defwidth && !btn->text.isEmpty())
                w = minwidth + defwidth;
            if (h < minheight + defwidth)
                h = minheight + defwidth;
            sz = QSize(w, h);
        }
        break;
#ifndef QT_NO_MENU
    case CT_MenuItem:
        if (const QStyleOptionMenuItem *mi = qstyleoption_cast<const QStyleOptionMenuItem *>(opt)) {
            int w = sz.width();
            sz = QCommonStyle::sizeFromContents(ct, opt, csz, widget);

            if (mi->menuItemType == QStyleOptionMenuItem::Separator) {
                sz = QSize(10, QWindowsStylePrivate::windowsSepHeight);
            }
            else if (mi->icon.isNull()) {
                sz.setHeight(sz.height() - 2);
                w -= 6;
            }

            if (mi->menuItemType != QStyleOptionMenuItem::Separator && !mi->icon.isNull()) {
                int iconExtent = proxy()->pixelMetric(PM_SmallIconSize, opt, widget);
                sz.setHeight(qMax(sz.height(),
                                  mi->icon.actualSize(QSize(iconExtent, iconExtent)).height()
                                  + 2 * QWindowsStylePrivate::windowsItemFrame));
            }
            int maxpmw = mi->maxIconWidth;
            int tabSpacing = 20;
            if (mi->text.contains(QLatin1Char('\t')))
                w += tabSpacing;
            else if (mi->menuItemType == QStyleOptionMenuItem::SubMenu)
                w += 2 * QWindowsStylePrivate::windowsArrowHMargin;
            else if (mi->menuItemType == QStyleOptionMenuItem::DefaultItem) {
                // adjust the font and add the difference in size.
                // it would be better if the font could be adjusted in the initStyleOption qmenu func!!
                QFontMetrics fm(mi->font);
                QFont fontBold = mi->font;
                fontBold.setBold(true);
                QFontMetrics fmBold(fontBold);
                w += fmBold.width(mi->text) - fm.width(mi->text);
            }

            int checkcol = qMax<int>(maxpmw, QWindowsStylePrivate::windowsCheckMarkWidth); // Windows always shows a check column
            w += checkcol;
            w += int(QWindowsStylePrivate::windowsRightBorder) + 10;
            sz.setWidth(w);
        }
        break;
#endif // QT_NO_MENU
#ifndef QT_NO_MENUBAR
    case CT_MenuBarItem:
        if (!sz.isEmpty())
            sz += QSize(QWindowsStylePrivate::windowsItemHMargin * 4, QWindowsStylePrivate::windowsItemVMargin * 2);
        break;
#endif
        // Otherwise, fall through
    case CT_ToolButton:
        if (qstyleoption_cast<const QStyleOptionToolButton *>(opt))
            return sz += QSize(7, 6);
        // Otherwise, fall through

    default:
        sz = QCommonStyle::sizeFromContents(ct, opt, csz, widget);
    }
    return sz;
}

QT_END_NAMESPACE

#include "moc_qwindowsstyle.h"

#endif // QT_NO_STYLE_WINDOWS
