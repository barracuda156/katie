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
#include "qbackingstore_p.h"
#include "qdebug.h"
#include "qevent.h"
#include "qapplication.h"
#include "qpaintengine.h"
#include "qgraphicsproxywidget.h"
#include "qthread.h"
#include "qelapsedtimer.h"
#include "qwidget_p.h"
#include "qwindowsurface_p.h"
#include "qapplication_p.h"
#include "qstdcontainers_p.h"
#include "qpaintengine_raster_p.h"

QT_BEGIN_NAMESPACE

/**
 * Flushes the contents of the \a windowSurface into the screen area of \a widget.
 * \a tlwOffset is the position of the top level widget relative to the window surface.
 * \a region is the region to be updated in \a widget coordinates.
 */
static inline void qt_flush(QWidget *widget, const QRegion &region, QWindowSurface *windowSurface,
                            QWidget *tlw, const QPoint &tlwOffset)
{
    Q_ASSERT(widget);
    Q_ASSERT(!region.isEmpty());
    Q_ASSERT(windowSurface);
    Q_ASSERT(tlw);

    //The performance hit by doing this should be negligible. However, be aware that
    //using this FPS when you have > 1 windowsurface can give you inaccurate FPS
    static const bool fpsDebug = qgetenv("QT_DEBUG_FPS").toInt();
    if (fpsDebug) {
        static QElapsedTimer time;
        static int frames = 0;

        frames++;

        if (!time.isValid()) {
            time.start();
        }

        if (time.elapsed() > 5000) {
            double fps = double(frames * 1000) /time.restart();
            fprintf(stderr,"FPS: %.1f\n",fps);
            frames = 0;
        }
    }
    if (widget != tlw) {
        windowSurface->flush(widget, region, tlwOffset + widget->mapTo(tlw, QPoint()));
    } else {
        windowSurface->flush(widget, region, tlwOffset);
    }
}

#ifndef QT_NO_DEBUG
bool QWidgetBackingStore::flushPaint(QWidget *widget, const QRegion &rgn)
{
    if (!widget)
        return false;

    if (widget->testAttribute(Qt::WA_WState_InPaintEvent)) {
        static int flushPaintEvent = qgetenv("QT_FLUSH_PAINT_EVENT").toInt();
        if (!flushPaintEvent)
            return false;
    } else {
        static int flushPaint = qgetenv("QT_FLUSH_PAINT").toInt();
        if (!flushPaint)
            return false;
    }

    return true;
}

void QWidgetBackingStore::unflushPaint(QWidget *widget, const QRegion &rgn)
{
    if (widget->d_func()->paintOnScreen() || rgn.isEmpty())
        return;

    QWidget *tlw = widget->window();
    QTLWExtra *tlwExtra = tlw->d_func()->maybeTopData();
    if (!tlwExtra)
        return;

    const QPoint offset = widget->mapTo(tlw, QPoint());
    qt_flush(widget, rgn, tlwExtra->backingStore->windowSurface, tlw, offset);
}
#endif // QT_NO_DEBUG

/*
    Moves the whole rect by (dx, dy) in widget's coordinate system.
    Doesn't generate any updates.
*/
bool QWidgetBackingStore::bltRect(const QRect &rect, int dx, int dy, QWidget *widget)
{
    const QPoint pos(tlwOffset + widget->mapTo(tlw, rect.topLeft()));
    const QRect tlwRect(QRect(pos, rect.size()));
    if (dirty.intersects(tlwRect))
        return false; // We don't want to scroll junk.
    return windowSurface->scroll(tlwRect, dx, dy);
}

/*!
    Prepares the window surface to paint a\ toClean region and updates the
    BeginPaintInfo struct accordingly.

    The \a toClean region might be clipped by the window surface.
*/
void QWidgetBackingStore::beginPaint(QRegion &toClean, QWindowSurface *windowSurface,
                                     BeginPaintInfo *returnInfo)
{
    // Always flush repainted areas.
    dirtyOnScreen += toClean;

#ifdef QT_NO_DEBUG
    windowSurface->beginPaint(toClean);
    Q_UNUSED(returnInfo);
#else
    returnInfo->wasFlushed = QWidgetBackingStore::flushPaint(tlw, toClean);
    // Avoid deadlock with QT_FLUSH_PAINT: the server will wait for
    // the BackingStore lock, so if we hold that, the server will
    // never release the Communication lock that we are waiting for in
    // sendSynchronousCommand
    if (!returnInfo->wasFlushed)
        windowSurface->beginPaint(toClean);
#endif
}

void QWidgetBackingStore::endPaint(const QRegion &cleaned, QWindowSurface *windowSurface,
        BeginPaintInfo *beginPaintInfo)
{
#ifndef QT_NO_DEBUG
    if (!beginPaintInfo->wasFlushed)
        windowSurface->endPaint();
    else
        QWidgetBackingStore::unflushPaint(tlw, cleaned);
#else
    Q_UNUSED(beginPaintInfo);
    windowSurface->endPaint();
#endif

    flush();
}

static inline void sendUpdateRequest(QWidget *widget, bool updateImmediately)
{
    if (!widget)
        return;

    if (updateImmediately) {
        QEvent event(QEvent::UpdateRequest);
        QApplication::sendEvent(widget, &event);
    } else {
        QApplication::postEvent(widget, new QEvent(QEvent::UpdateRequest), Qt::LowEventPriority);
    }
}

/*!
    Marks the region of the widget as dirty (if not already marked as dirty) and
    posts an UpdateRequest event to the top-level widget (if not already posted).

    If updateImmediately is true, the event is sent immediately instead of posted.

    If invalidateBuffer is true, all widgets intersecting with the region will be dirty.

    If the widget paints directly on screen, the event is sent to the widget
    instead of the top-level widget, and invalidateBuffer is completely ignored.
*/
void QWidgetBackingStore::markDirty(const QRegion &rgn, QWidget *widget, bool updateImmediately,
                                    bool invalidateBuffer)
{
    Q_ASSERT(tlw->d_func()->extra);
    Q_ASSERT(tlw->d_func()->extra->topextra);
    Q_ASSERT(!tlw->d_func()->extra->topextra->inTopLevelResize);
    Q_ASSERT(widget->isVisible() && widget->updatesEnabled());
    Q_ASSERT(widget->window() == tlw);
    Q_ASSERT(!rgn.isEmpty());

    if (widget->d_func()->paintOnScreen()) {
        if (widget->d_func()->dirty.isEmpty()) {
            widget->d_func()->dirty = rgn;
            sendUpdateRequest(widget, updateImmediately);
            return;
        } else if (qt_region_strictContains(widget->d_func()->dirty, widget->rect())) {
            if (updateImmediately)
                sendUpdateRequest(widget, updateImmediately);
            return; // Already dirty.
        }

        const bool eventAlreadyPosted = !widget->d_func()->dirty.isEmpty();
        widget->d_func()->dirty += rgn;
        if (!eventAlreadyPosted || updateImmediately)
            sendUpdateRequest(widget, updateImmediately);
        return;
    }

    const QPoint offset = widget->mapTo(tlw, QPoint());
    const QRect widgetRect = widget->rect();
    if (qt_region_strictContains(dirty, widgetRect.translated(offset))) {
        if (updateImmediately)
            sendUpdateRequest(tlw, updateImmediately);
        return; // Already dirty.
    }

    if (invalidateBuffer) {
        const bool eventAlreadyPosted = !dirty.isEmpty();
        dirty += rgn.translated(offset);
        if (!eventAlreadyPosted || updateImmediately)
            sendUpdateRequest(tlw, updateImmediately);
        return;
    }

    if (dirtyWidgets.isEmpty()) {
        addDirtyWidget(widget, rgn);
        sendUpdateRequest(tlw, updateImmediately);
        return;
    }

    if (widget->d_func()->inDirtyList) {
        if (!qt_region_strictContains(widget->d_func()->dirty, widgetRect)) {
            widget->d_func()->dirty += rgn;
        }
    } else {
        addDirtyWidget(widget, rgn);
    }

    if (updateImmediately)
        sendUpdateRequest(tlw, updateImmediately);
}

/*!
    This function is equivalent to calling markDirty(QRegion(rect), ...), but
    is more efficient as it eliminates QRegion operations/allocations and can
    use the rect more precisely for additional cut-offs.
*/
void QWidgetBackingStore::markDirty(const QRect &rect, QWidget *widget, bool updateImmediately,
                                    bool invalidateBuffer)
{
    Q_ASSERT(tlw->d_func()->extra);
    Q_ASSERT(tlw->d_func()->extra->topextra);
    Q_ASSERT(!tlw->d_func()->extra->topextra->inTopLevelResize);
    Q_ASSERT(widget->isVisible() && widget->updatesEnabled());
    Q_ASSERT(widget->window() == tlw);
    Q_ASSERT(!rect.isEmpty());

    if (widget->d_func()->paintOnScreen()) {
        if (widget->d_func()->dirty.isEmpty()) {
            widget->d_func()->dirty = QRegion(rect);
            sendUpdateRequest(widget, updateImmediately);
            return;
        } else if (qt_region_strictContains(widget->d_func()->dirty, rect)) {
            if (updateImmediately)
                sendUpdateRequest(widget, updateImmediately);
            return; // Already dirty.
        }

        const bool eventAlreadyPosted = !widget->d_func()->dirty.isEmpty();
        widget->d_func()->dirty += rect;
        if (!eventAlreadyPosted || updateImmediately)
            sendUpdateRequest(widget, updateImmediately);
        return;
    }

    const QRect widgetRect = rect;
    const QRect translatedRect(widgetRect.translated(widget->mapTo(tlw, QPoint())));
    if (qt_region_strictContains(dirty, translatedRect)) {
        if (updateImmediately)
            sendUpdateRequest(tlw, updateImmediately);
        return; // Already dirty
    }

    if (invalidateBuffer) {
        const bool eventAlreadyPosted = !dirty.isEmpty();
        dirty += translatedRect;
        if (!eventAlreadyPosted || updateImmediately)
            sendUpdateRequest(tlw, updateImmediately);
        return;
    }

    if (dirtyWidgets.isEmpty()) {
        addDirtyWidget(widget, rect);
        sendUpdateRequest(tlw, updateImmediately);
        return;
    }

    if (widget->d_func()->inDirtyList) {
        if (!qt_region_strictContains(widget->d_func()->dirty, widgetRect))
            widget->d_func()->dirty += widgetRect;
    } else {
        addDirtyWidget(widget, rect);
    }

    if (updateImmediately)
        sendUpdateRequest(tlw, updateImmediately);
}

/*!
    Marks the \a region of the \a widget as dirty on screen. The \a region will be copied from
    the backing store to the \a widget's native parent next time flush() is called.

    Paint on screen widgets are ignored.
*/
void QWidgetBackingStore::markDirtyOnScreen(const QRegion &region, QWidget *widget, const QPoint &topLevelOffset)
{
    if (!widget || widget->d_func()->paintOnScreen() || region.isEmpty())
        return;

    // Top-level.
    if (widget == tlw) {
        if (!widget->testAttribute(Qt::WA_WState_InPaintEvent))
            dirtyOnScreen += region;
        return;
    }

    // Alien widgets.
    if (!widget->internalWinId() && !widget->isWindow()) {
        QWidget *nativeParent = widget->nativeParentWidget();        // Alien widgets with the top-level as the native parent (common case).
        if (nativeParent == tlw) {
            if (!widget->testAttribute(Qt::WA_WState_InPaintEvent))
                dirtyOnScreen += region.translated(topLevelOffset);
            return;
        }

        // Alien widgets with native parent != tlw.
        QWidgetPrivate *nativeParentPrivate = nativeParent->d_func();
        if (!nativeParentPrivate->needsFlush)
            nativeParentPrivate->needsFlush = new QRegion;
        const QPoint nativeParentOffset = widget->mapTo(nativeParent, QPoint());
        *nativeParentPrivate->needsFlush += region.translated(nativeParentOffset);
        appendDirtyOnScreenWidget(nativeParent);
        return;
    }

    // Native child widgets.
    QWidgetPrivate *widgetPrivate = widget->d_func();
    if (!widgetPrivate->needsFlush)
        widgetPrivate->needsFlush = new QRegion;
    *widgetPrivate->needsFlush += region;
    appendDirtyOnScreenWidget(widget);
}

void QWidgetBackingStore::removeDirtyWidget(QWidget *w)
{
    if (!w)
        return;

    dirtyWidgetsRemoveAll(w);
    dirtyOnScreenWidgetsRemoveAll(w);
    resetWidget(w);

    QWidgetPrivate *wd = w->d_func();
    foreach (QObject *objchild, wd->children) {
        QWidget *child = qobject_cast<QWidget*>(objchild);
        if (child)
            removeDirtyWidget(child);
    }
}

QWidgetBackingStore::QWidgetBackingStore(QWidget *topLevel)
    : tlw(topLevel)
{
    windowSurface = tlw->windowSurface();
    if (!windowSurface)
        windowSurface = topLevel->d_func()->createDefaultWindowSurface();

    // The QWindowSurface constructor will call QWidget::setWindowSurface(),
    // but automatically created surfaces should not be added to the topdata.
    topLevel->d_func()->topData()->windowSurface = 0;
}

QWidgetBackingStore::~QWidgetBackingStore()
{
    for (int c = 0; c < dirtyWidgets.size(); ++c) {
        resetWidget(dirtyWidgets.at(c));
    }

    delete windowSurface;
    windowSurface = 0;
}

//parent's coordinates; move whole rect; update parent and widget
//assume the screen blt has already been done, so we don't need to refresh that part
void QWidgetPrivate::moveRect(const QRect &rect, int dx, int dy)
{
    Q_Q(QWidget);
    if (!q->isVisible() || (dx == 0 && dy == 0))
        return;

    QWidget *tlw = q->window();
    QTLWExtra* x = tlw->d_func()->topData();
    if (x->inTopLevelResize)
        return;

    static const int accelEnv = qgetenv("QT_NO_FAST_MOVE").toInt() == 0;

    QWidget *pw = q->parentWidget();
    QPoint toplevelOffset = pw->mapTo(tlw, QPoint());
    QWidgetPrivate *pd = pw->d_func();
    QRect clipR(pd->clipRect());
    const QRect newRect(rect.translated(dx, dy));
    QRect destRect = rect.intersected(clipR);
    if (destRect.isValid())
        destRect = destRect.translated(dx, dy).intersected(clipR);
    const QRect sourceRect(destRect.translated(-dx, -dy));
    const QRect parentRect(rect & clipR);

    bool accelerateMove = accelEnv && isOpaque
#ifndef QT_NO_GRAPHICSVIEW
                          // No accelerate move for proxy widgets.
                          && !tlw->d_func()->extra->proxyWidget
#endif
                          && !isOverlapped(sourceRect) && !isOverlapped(destRect);

    if (!accelerateMove) {
        QRegion parentR(parentRect);
        if (!extra || !extra->hasMask) {
            parentR -= newRect;
        } else {
            // invalidateBuffer() excludes anything outside the mask
            parentR += newRect & clipR;
        }
        pd->invalidateBuffer(parentR);
        invalidateBuffer((newRect & clipR).translated(-data.crect.topLeft()));
    } else {

        QWidgetBackingStore *wbs = x->backingStore.data();
        QRegion childExpose(newRect & clipR);

        if (sourceRect.isValid() && wbs->bltRect(sourceRect, dx, dy, pw))
            childExpose -= destRect;

        if (!pw->updatesEnabled())
            return;

        const bool childUpdatesEnabled = q->updatesEnabled();
        if (childUpdatesEnabled && !childExpose.isEmpty()) {
            childExpose.translate(-data.crect.topLeft());
            wbs->markDirty(childExpose, q);
            isMoved = true;
        }

        QRegion parentExpose(parentRect);
        parentExpose -= newRect;
        if (extra && extra->hasMask)
            parentExpose += QRegion(newRect) - extra->mask.translated(data.crect.topLeft());

        if (!parentExpose.isEmpty()) {
            wbs->markDirty(parentExpose, pw);
            pd->isMoved = true;
        }

        if (childUpdatesEnabled) {
            QRegion needsFlush(sourceRect);
            needsFlush += destRect;
            wbs->markDirtyOnScreen(needsFlush, pw, toplevelOffset);
        }
    }
}

//widget's coordinates; scroll within rect;  only update widget
void QWidgetPrivate::scrollRect(const QRect &rect, int dx, int dy)
{
    Q_Q(QWidget);
    QWidget *tlw = q->window();
    QTLWExtra* x = tlw->d_func()->topData();
    if (x->inTopLevelResize)
        return;

    QWidgetBackingStore *wbs = x->backingStore.data();
    if (!wbs)
        return;

    static const int accelEnv = qgetenv("QT_NO_FAST_SCROLL").toInt() == 0;

    QRect scrollRect = rect & clipRect();
    bool overlapped = isOverlapped(scrollRect.translated(data.crect.topLeft()));
    bool accelerateScroll = accelEnv && isOpaque && !overlapped;

    if (!accelerateScroll) {
        if (overlapped) {
            QRegion region(scrollRect);
            subtractOpaqueSiblings(region);
            invalidateBuffer(region);
        }else {
            invalidateBuffer(scrollRect);
        }
    } else {
        const QPoint toplevelOffset = q->mapTo(tlw, QPoint());
        const QRect destRect = scrollRect.translated(dx, dy) & scrollRect;
        const QRect sourceRect = destRect.translated(-dx, -dy);

        QRegion childExpose(scrollRect);
        if (sourceRect.isValid()) {
            if (wbs->bltRect(sourceRect, dx, dy, q))
                childExpose -= destRect;
        }

        if (inDirtyList) {
            if (rect == q->rect()) {
                dirty.translate(dx, dy);
            } else {
                QRegion dirtyScrollRegion = dirty.intersected(scrollRect);
                if (!dirtyScrollRegion.isEmpty()) {
                    dirty -= dirtyScrollRegion;
                    dirtyScrollRegion.translate(dx, dy);
                    dirty += dirtyScrollRegion;
                }
            }
        }

        if (!q->updatesEnabled())
            return;

        if (!childExpose.isEmpty()) {
            wbs->markDirty(childExpose, q);
            isScrolled = true;
        }

        // Instead of using native scroll-on-screen, we copy from
        // backingstore, giving only one screen update for each
        // scroll, and a solid appearance
        wbs->markDirtyOnScreen(destRect, q, toplevelOffset);
    }
}

static inline bool discardSyncRequest(QWidget *tlw, QTLWExtra *tlwExtra)
{
    if (!tlw || !tlwExtra)
        return true;

#ifdef Q_WS_X11
    // Delay the sync until we get an Expose event from X11 (initial show).
    // Qt::WA_Mapped is set to true, but the actual mapping has not yet occurred.
    // However, we must repaint immediately regardless of the state if someone calls repaint().
    if (tlwExtra->waitingForMapNotify && !tlwExtra->inRepaint)
        return true;
#endif

    if (!tlw->testAttribute(Qt::WA_Mapped))
        return true;

    if (!tlw->isVisible()
#ifndef Q_WS_X11
        // If we're minimized on X11, WA_Mapped will be false and we
        // will return in the case above. Some window managers on X11
        // sends us the PropertyNotify to change the minimized state
        // *AFTER* we've received the expose event, which is baaad.
        || tlw->isMinimized()
#endif
        )
        return true;

    return false;
}

/*!
    Synchronizes the \a exposedRegion of the \a exposedWidget with the backing store.

    If there's nothing to repaint, the area is flushed and painting does not occur;
    otherwise the area is marked as dirty on screen and will be flushed right after
    we are done with all painting.
*/
void QWidgetBackingStore::sync(QWidget *exposedWidget, const QRegion &exposedRegion)
{
    QTLWExtra *tlwExtra = tlw->d_func()->maybeTopData();
    if (discardSyncRequest(tlw, tlwExtra) || tlwExtra->inTopLevelResize)
        return;

    if (!exposedWidget || !exposedWidget->internalWinId() || !exposedWidget->isVisible()
        || !exposedWidget->updatesEnabled() || exposedRegion.isEmpty()) {
        return;
    }

    // Nothing to repaint.
    if (!isDirty()) {
        qt_flush(exposedWidget, exposedRegion, windowSurface, tlw, tlwOffset);
        return;
    }

    if (exposedWidget != tlw)
        markDirtyOnScreen(exposedRegion, exposedWidget, exposedWidget->mapTo(tlw, QPoint()));
    else
        markDirtyOnScreen(exposedRegion, exposedWidget, QPoint());
    sync();
}

/*!
    Synchronizes the backing store, i.e. dirty areas are repainted and flushed.
*/
void QWidgetBackingStore::sync()
{
    QTLWExtra *tlwExtra = tlw->d_func()->maybeTopData();
    if (discardSyncRequest(tlw, tlwExtra)) {
        // If the top-level is minimized, it's not visible on the screen so we can delay the
        // update until it's shown again. In order to do that we must keep the dirty states.
        // These will be cleared when we receive the first expose after showNormal().
        // However, if the widget is not visible (isVisible() returns false), everything will
        // be invalidated once the widget is shown again, so clear all dirty states.
        if (!tlw->isVisible()) {
            dirty = QRegion();
            for (int i = 0; i < dirtyWidgets.size(); ++i)
                resetWidget(dirtyWidgets.at(i));
            dirtyWidgets.clear();
        }
        return;
    }

    const bool updatesDisabled = !tlw->updatesEnabled();
    bool repaintAllWidgets = false;

    const bool inTopLevelResize = tlwExtra->inTopLevelResize;
    const QRect tlwRect(tlw->data->crect);
    const QRect surfaceGeometry(windowSurface->geometry());
    if ((inTopLevelResize || surfaceGeometry.size() != tlwRect.size()) && !updatesDisabled) {
        // Repaint everything.
        dirty = QRegion(0, 0, tlwRect.width(), tlwRect.height());
        for (int i = 0; i < dirtyWidgets.size(); ++i)
            resetWidget(dirtyWidgets.at(i));
        dirtyWidgets.clear();
        repaintAllWidgets = true;
    }

    if (inTopLevelResize || surfaceGeometry != tlwRect)
        windowSurface->setGeometry(tlwRect);

    if (updatesDisabled)
        return;

    // Contains everything that needs repaint.
    QRegion toClean(dirty);

    // Loop through all update() widgets and remove them from the list before they are
    // painted (in case someone calls update() in paintEvent). If the widget is opaque
    // and does not have transparent overlapping siblings, append it to the
    // opaqueNonOverlappedWidgets list and paint it directly without composition.
    QStdVector<QWidget *> opaqueNonOverlappedWidgets;
    for (int i = 0; i < dirtyWidgets.size(); ++i) {
        QWidget *w = dirtyWidgets.at(i);
        QWidgetPrivate *wd = w->d_func();
        if (wd->data.in_destructor)
            continue;

        // Clip with mask() and clipRect().
        wd->dirty &= wd->clipRect();
        wd->clipToEffectiveMask(wd->dirty);

        // Subtract opaque siblings and children.
        bool hasDirtySiblingsAbove = false;
        // We know for sure that the widget isn't overlapped if 'isMoved' is true.
        if (!wd->isMoved)
            wd->subtractOpaqueSiblings(wd->dirty, &hasDirtySiblingsAbove);
        // Scrolled and moved widgets must draw all children.
        if (!wd->isScrolled && !wd->isMoved)
            wd->subtractOpaqueChildren(wd->dirty, w->rect());

        if (wd->dirty.isEmpty()) {
            resetWidget(w);
            continue;
        }

        const QRegion widgetDirty(w != tlw ? wd->dirty.translated(w->mapTo(tlw, QPoint()))
                                           : wd->dirty);
        toClean += widgetDirty;

#ifndef QT_NO_GRAPHICSVIEW
        if (tlw->d_func()->extra->proxyWidget) {
            resetWidget(w);
            continue;
        }
#endif

        if (!hasDirtySiblingsAbove && wd->isOpaque && !dirty.intersects(widgetDirty.boundingRect())) {
            opaqueNonOverlappedWidgets.append(w);
        } else {
            resetWidget(w);
            dirty += widgetDirty;
        }
    }
    dirtyWidgets.clear();

    if (toClean.isEmpty()) {
        // Nothing to repaint. However, we might have newly exposed areas on the
        // screen if this function was called from sync(QWidget *, QRegion)), so
        // we have to make sure those are flushed.
        flush();
        return;
    }

#ifndef QT_NO_GRAPHICSVIEW
    if (tlw->d_func()->extra->proxyWidget) {
        dirty = QRegion();
        foreach (const QRect &rect, toClean.rects()) {
            tlw->d_func()->extra->proxyWidget->update(rect);
        }
        return;
    }
#endif

    BeginPaintInfo beginPaintInfo;
    beginPaint(toClean, windowSurface, &beginPaintInfo);

    const QRegion dirtyCopy(dirty);
    dirty = QRegion();

    // Paint opaque non overlapped widgets.
    for (int i = 0; i < opaqueNonOverlappedWidgets.size(); ++i) {
        QWidget *w = opaqueNonOverlappedWidgets[i];
        QWidgetPrivate *wd = w->d_func();

        int flags = QWidgetPrivate::DrawRecursive;
        // Scrolled and moved widgets must draw all children.
        if (!wd->isScrolled && !wd->isMoved)
            flags |= QWidgetPrivate::DontDrawOpaqueChildren;
        if (w == tlw)
            flags |= QWidgetPrivate::DrawAsRoot;

        QRegion toBePainted(wd->dirty);
        resetWidget(w);

        QPoint offset(tlwOffset);
        if (w != tlw)
            offset += w->mapTo(tlw, QPoint());
        wd->drawWidget(windowSurface->paintDevice(), toBePainted, offset, flags, 0, this);
    }

    // Paint the rest with composition.
    if (repaintAllWidgets || !dirtyCopy.isEmpty()) {
        const int flags = QWidgetPrivate::DrawAsRoot | QWidgetPrivate::DrawRecursive;
        tlw->d_func()->drawWidget(windowSurface->paintDevice(), dirtyCopy, tlwOffset, flags, 0, this);
    }

    endPaint(toClean, windowSurface, &beginPaintInfo);
}

/*!
    Flushes the contents of the backing store into the top-level widget.
    If the \a widget is non-zero, the content is flushed to the \a widget.
    If the \a surface is non-zero, the content of the \a surface is flushed.
*/
void QWidgetBackingStore::flush(QWidget *widget, QWindowSurface *surface)
{
    if (!dirtyOnScreen.isEmpty()) {
        QWidget *target = widget ? widget : tlw;
        QWindowSurface *source = surface ? surface : windowSurface;
        qt_flush(target, dirtyOnScreen, source, tlw, tlwOffset);
        dirtyOnScreen = QRegion();
    }

    foreach (QWidget *w, dirtyOnScreenWidgets) {
        QWidgetPrivate *wd = w->d_func();
        Q_ASSERT(wd->needsFlush);
        qt_flush(w, *wd->needsFlush, windowSurface, tlw, tlwOffset);
        *wd->needsFlush = QRegion();
    }
    dirtyOnScreenWidgets.clear();
}

static inline bool discardInvalidateBufferRequest(QWidget *widget, QTLWExtra *tlwExtra)
{
    Q_ASSERT(widget);
    if (QApplication::closingDown())
        return true;

    if (!tlwExtra || tlwExtra->inTopLevelResize || !tlwExtra->backingStore)
        return true;

    if (!widget->isVisible() || !widget->updatesEnabled())
        return true;

    return false;
}

/*!
    Invalidates the buffer when the widget is resized.
    Static areas are never invalidated unless absolutely needed.
*/
void QWidgetPrivate::invalidateBuffer_resizeHelper(const QPoint &oldPos, const QSize &oldSize)
{
    Q_Q(QWidget);
    Q_ASSERT(!q->isWindow());
    Q_ASSERT(q->parentWidget());

    const bool sizeDecreased = (data.crect.width() < oldSize.width())
                               || (data.crect.height() < oldSize.height());

    const QPoint offset(data.crect.x() - oldPos.x(), data.crect.y() - oldPos.y());
    const bool parentAreaExposed = !offset.isNull() || sizeDecreased;
    const QRect newWidgetRect(q->rect());
    const QRect oldWidgetRect(0, 0, oldSize.width(), oldSize.height());

    // Entire widget needs repaint.
    invalidateBuffer(newWidgetRect);

    if (!parentAreaExposed)
        return;

    // Invalidate newly exposed area of the parent.
    if (extra && extra->hasMask) {
        QRegion parentExpose(extra->mask.translated(oldPos));
        parentExpose &= QRect(oldPos, oldSize);
        q->parentWidget()->d_func()->invalidateBuffer(parentExpose);
    } else {
        q->parentWidget()->d_func()->invalidateBuffer(QRect(oldPos, oldSize));
    }
}

/*!
    Invalidates the \a rgn (in widget's coordinates) of the backing store, i.e.
    all widgets intersecting with the region will be repainted when the backing store
    is synced.
*/
void QWidgetPrivate::invalidateBuffer(const QRegion &rgn)
{
    Q_Q(QWidget);

    QTLWExtra *tlwExtra = q->window()->d_func()->maybeTopData();
    if (discardInvalidateBufferRequest(q, tlwExtra) || rgn.isEmpty())
        return;

    QRegion wrgn(rgn);
    wrgn &= clipRect();
    if (extra && extra->hasMask)
        wrgn &= extra->mask;
    if (wrgn.isEmpty())
        return;

    tlwExtra->backingStore->markDirty(wrgn, q, false, true);
}

/*!
    This function is equivalent to calling invalidateBuffer(QRegion(rect), ...), but
    is more efficient as it eliminates QRegion operations/allocations and can
    use the rect more precisely for additional cut-offs.
*/
void QWidgetPrivate::invalidateBuffer(const QRect &rect)
{
    Q_Q(QWidget);

    QTLWExtra *tlwExtra = q->window()->d_func()->maybeTopData();
    if (discardInvalidateBufferRequest(q, tlwExtra) || rect.isEmpty())
        return;

    QRect wRect(rect);
    wRect &= clipRect();
    if (wRect.isEmpty())
        return;

    if (!extra || !extra->hasMask) {
        tlwExtra->backingStore->markDirty(wRect, q, false, true);
        return;
    }

    QRegion wRgn(extra->mask);
    wRgn &= wRect;
    if (wRgn.isEmpty())
        return;

    tlwExtra->backingStore->markDirty(wRgn, q, false, true);
}

void QWidgetPrivate::repaint_sys(const QRegion &rgn)
{
    if (data.in_destructor)
        return;

    Q_Q(QWidget);

    QRegion toBePainted(rgn & clipRect());
    clipToEffectiveMask(toBePainted);
    if (toBePainted.isEmpty())
        return; // Nothing to repaint.

#ifndef QT_NO_DEBUG
    bool flushed = QWidgetBackingStore::flushPaint(q, toBePainted);
#endif

    drawWidget(q, toBePainted, QPoint(), QWidgetPrivate::DrawAsRoot | QWidgetPrivate::DrawPaintOnScreen, 0);

#ifndef QT_NO_DEBUG
    if (flushed)
        QWidgetBackingStore::unflushPaint(q, toBePainted);
#endif

    if (Q_UNLIKELY(q->paintingActive()))
        qWarning("QWidget::repaint: It is dangerous to leave painters active on a widget outside of the PaintEvent");
}


QT_END_NAMESPACE




