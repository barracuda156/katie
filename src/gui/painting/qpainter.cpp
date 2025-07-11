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

#include "qdebug.h"
#include "qmath.h"
#include "qbitmap.h"
#include "qimage.h"
#include "qpaintdevice.h"
#include "qpaintengine.h"
#include "qpainter.h"
#include "qpainter_p.h"
#include "qpainterpath.h"
#include "qpixmapcache.h"
#include "qpolygon.h"
#include "qtextlayout.h"
#include "qwidget.h"
#include "qapplication.h"
#include "qstyle.h"
#include "qthread.h"
#include "qpaintengine_p.h"
#include "qpainterpath_p.h"
#include "qwidget_p.h"
#include "qpaintengine_raster_p.h"
#include "qstylehelper_p.h"
#include "qstdcontainers_p.h"
#include "qguicommon_p.h"

QT_BEGIN_NAMESPACE

// #define QT_DEBUG_DRAW

void qt_format_text(const QFont &font, const QRectF &_r, int tf, const QTextOption *option,
                    const QString& str, QRectF *brect, QPainter *painter);
static void drawTextItemDecoration(QPainter *painter, const QPointF &pos, const QFontEngine *fe,
                                   QTextCharFormat::UnderlineStyle underlineStyle,
                                   QTextItem::RenderFlags flags, qreal width,
                                   const QTextCharFormat &charFormat);

static inline QGradient::CoordinateMode coordinateMode(const QBrush &brush)
{
    switch (brush.style()) {
        case Qt::LinearGradientPattern:
        case Qt::RadialGradientPattern:
            return brush.gradient()->coordinateMode();
        default:
            ;
    }
    return QGradient::LogicalMode;
}

#ifndef QT_NO_DEBUG
static bool qt_painter_thread_test(int devType, const char *what, bool extraCondition = false)
{
    switch (devType) {
    case QInternal::Image:
    case QInternal::Pixmap:
    case QInternal::Printer:
        // can be drawn onto these devices safely from any thread
        if (extraCondition)
            break;
    default:
#ifdef Q_WS_X11
        if (QApplication::testAttribute(Qt::AA_X11InitThreads))
            return true;
#endif
        if (Q_UNLIKELY(!extraCondition && QThread::currentThread() != qApp->thread())) {
            qWarning("QPainter: It is not safe to use %s outside the GUI thread", what);
            return false;
        }
        break;
    }
    return true;
}
#endif


QPainterPrivate::~QPainterPrivate()
{
    for (int i=0; i<states.size(); ++i)
        delete states.at(i);

    if (dummyState)
        delete dummyState;

    delete engine;
}


QTransform QPainterPrivate::viewTransform() const
{
    if (state->VxF) {
        qreal scaleW = qreal(state->vw)/qreal(state->ww);
        qreal scaleH = qreal(state->vh)/qreal(state->wh);
        return QTransform(scaleW, 0, 0, scaleH,
                          state->vx - state->wx*scaleW, state->vy - state->wy*scaleH);
    }
    return QTransform();
}


/*
   \internal
   Returns true if using a shared painter; otherwise false.
*/
bool QPainterPrivate::attachPainterPrivate(QPainter *q, QPaintDevice *pdev)
{
    Q_ASSERT(q);
    Q_ASSERT(pdev);

    if (pdev->devType() != QInternal::Widget)
        return false;

    QWidget *widget = static_cast<QWidget *>(pdev);
    Q_ASSERT(widget);

    // Someone either called QPainter::setRedirected in the widget's paint event
    // right before this painter was created (or begin was called) or
    // sent a paint event directly to the widget.
    if (!widget->d_func()->redirectDev)
        return false;

    QPainter *sp = widget->d_func()->sharedPainter();
    if (!sp || !sp->isActive())
        return false;

    if (sp->paintEngine()->paintDevice() != widget->d_func()->redirectDev)
        return false;

    // Check if we're attempting to paint outside a paint event.
    if (Q_UNLIKELY(!widget->testAttribute(Qt::WA_WState_InPaintEvent))) {
        qWarning("QPainter::begin: Widget painting can only begin as a result of a paintEvent");
        return false;
    }

    // Save the current state of the shared painter and assign
    // the current d_ptr to the shared painter's d_ptr.
    sp->save();
    if (!sp->d_ptr->d_ptrs) {
        // Allocate space for 4 d-pointers (enough for up to 4 sub-sequent
        // redirections within the same paintEvent(), which should be enough
        // in 99% of all cases). E.g: A renders B which renders C which renders D.
        sp->d_ptr->d_ptrs_size = 4;
        sp->d_ptr->d_ptrs = (QPainterPrivate **)::malloc(4 * sizeof(QPainterPrivate *));
        Q_CHECK_PTR(sp->d_ptr->d_ptrs);
    } else if (sp->d_ptr->refcount - 1 == uint(sp->d_ptr->d_ptrs_size)) {
        // However, to support corner cases we grow the array dynamically if needed.
        sp->d_ptr->d_ptrs_size <<= 1;
        const int newSize = sp->d_ptr->d_ptrs_size * sizeof(QPainterPrivate *);
        sp->d_ptr->d_ptrs = (QPainterPrivate **)::realloc(sp->d_ptr->d_ptrs, newSize);
        Q_CHECK_PTR(sp->d_ptr->d_ptrs);
    }
    sp->d_ptr->d_ptrs[++sp->d_ptr->refcount - 2] = q->d_ptr.data();
    q->d_ptr.take();
    q->d_ptr.reset(sp->d_ptr.data());

    Q_ASSERT(q->d_ptr->state);

    // Now initialize the painter with correct widget properties.
    q->initFrom(widget);
    QPoint offset;
    widget->d_func()->redirected(&offset);

    // Update system rect.
    q->d_ptr->state->ww = q->d_ptr->state->vw = widget->width();
    q->d_ptr->state->wh = q->d_ptr->state->vh = widget->height();

    // Update matrix.
    if (q->d_ptr->state->WxF) {
        q->d_ptr->state->redirectionMatrix = q->d_ptr->state->matrix;
        q->d_ptr->state->redirectionMatrix.translate(-offset.x(), -offset.y());
        q->d_ptr->state->worldMatrix = QTransform();
        q->d_ptr->state->WxF = false;
    } else {
        q->d_ptr->state->redirectionMatrix = QTransform::fromTranslate(-offset.x(), -offset.y());
    }
    q->d_ptr->updateMatrix();

    QPaintEnginePrivate *enginePrivate = q->d_ptr->engine->d_func();
    if (enginePrivate->currentClipWidget == widget) {
        enginePrivate->systemStateChanged();
        return true;
    }

    // Update system transform and clip.
    enginePrivate->currentClipWidget = widget;
    enginePrivate->setSystemTransform(q->d_ptr->state->matrix);
    return true;
}

void QPainterPrivate::detachPainterPrivate(QPainter *q)
{
    Q_ASSERT(refcount > 1);
    Q_ASSERT(q);

    QPainterPrivate *original = d_ptrs[--refcount - 1];
    if (inDestructor) {
        inDestructor = false;
        if (original)
            original->inDestructor = true;
    } else if (!original) {
        original = new QPainterPrivate(q);
    }

    d_ptrs[refcount - 1] = 0;
    q->restore();
    q->d_ptr.take();
    q->d_ptr.reset(original);
}


void QPainterPrivate::draw_helper(const QPainterPath &originalPath)
{
#ifdef QT_DEBUG_DRAW
    printf("QPainter::drawHelper\n");
#endif

    if (originalPath.isEmpty())
        return;

    if (!extended) {
        drawStretchedGradient(originalPath);
        return;
    }

    Q_Q(QPainter);

    qreal strokeOffsetX = 0, strokeOffsetY = 0;

    QPainterPath path = originalPath * state->matrix;
    QRectF pathBounds = path.boundingRect();
    QRectF strokeBounds;
    bool doStroke = (state->pen.style() != Qt::NoPen);
    if (doStroke) {
        qreal penWidth = state->pen.widthF();
        if (penWidth == 0) {
            strokeOffsetX = 1;
            strokeOffsetY = 1;
        } else {
            // In case of complex xform
            if (state->matrix.type() > QTransform::TxScale) {
                QPainterPathStroker stroker;
                stroker.setWidth(penWidth);
                stroker.setJoinStyle(state->pen.joinStyle());
                stroker.setCapStyle(state->pen.capStyle());
                QPainterPath stroke = stroker.createStroke(originalPath);
                strokeBounds = (stroke * state->matrix).boundingRect();
            } else {
                strokeOffsetX = qAbs(penWidth * state->matrix.m11() / qreal(2.0));
                strokeOffsetY = qAbs(penWidth * state->matrix.m22() / qreal(2.0));
            }
        }
    }

    QRect absPathRect;
    if (!strokeBounds.isEmpty()) {
        absPathRect = strokeBounds.intersected(QRectF(0, 0, device->width(), device->height())).toAlignedRect();
    } else {
        absPathRect = pathBounds.adjusted(-strokeOffsetX, -strokeOffsetY, strokeOffsetX, strokeOffsetY)
            .intersected(QRectF(0, 0, device->width(), device->height())).toAlignedRect();
    }

    if (q->hasClipping()) {
        bool hasPerspectiveTransform = false;
        for (int i = 0; i < state->clipInfo.size(); ++i) {
            const QPainterClipInfo &info = state->clipInfo.at(i);
            if (info.matrix.type() == QTransform::TxProject) {
                hasPerspectiveTransform = true;
                break;
            }
        }
        // avoid mapping QRegions with perspective transforms
        if (!hasPerspectiveTransform) {
            // The trick with txinv and invMatrix is done in order to
            // avoid transforming the clip to logical coordinates, and
            // then back to device coordinates. This is a problem with
            // QRegion/QRect based clips, since they use integer
            // coordinates and converting to/from logical coordinates will
            // lose precision.
            const bool old_txinv = txinv;
            const QTransform old_invMatrix = invMatrix;
            txinv = true;
            invMatrix = QTransform();
            QPainterPath clipPath = q->clipPath();
            QRectF r = clipPath.boundingRect().intersected(absPathRect);
            absPathRect = r.toAlignedRect();
            txinv = old_txinv;
            invMatrix = old_invMatrix;
        }
    }

//     qDebug("\nQPainterPrivate::draw_helper(), x=%d, y=%d, w=%d, h=%d",
//            devMinX, devMinY, device->width(), device->height());
//     qDebug() << " - matrix" << state->matrix;
//     qDebug() << " - originalPath.bounds" << originalPath.boundingRect();
//     qDebug() << " - path.bounds" << path.boundingRect();

    if (absPathRect.width() <= 0 || absPathRect.height() <= 0)
        return;

    QImage image(absPathRect.width(), absPathRect.height(), QImage::Format_ARGB32_Premultiplied);
    image.fill(0);

    QPainter p(&image);

    p.setOpacity(state->opacity);
    p.translate(-absPathRect.x(), -absPathRect.y());
    p.setTransform(state->matrix, true);
    p.setPen(doStroke ? state->pen : QPen(Qt::NoPen));
    p.setBrush(state->brush);
    p.setBackground(state->bgBrush);
    p.setBackgroundMode(state->bgMode);
    p.setBrushOrigin(state->brushOrigin);

    p.setRenderHint(QPainter::Antialiasing, state->renderHints & QPainter::Antialiasing);
    p.setRenderHint(QPainter::TextAntialiasing, state->renderHints & QPainter::TextAntialiasing);
    p.setRenderHint(QPainter::SmoothPixmapTransform,
                    state->renderHints & QPainter::SmoothPixmapTransform);

    p.drawPath(originalPath);

#ifndef QT_NO_DEBUG
    static bool do_fallback_overlay = qgetenv("QT_PAINT_FALLBACK_OVERLAY").size() > 0;
    if (do_fallback_overlay) {
        QImage block(8, 8, QImage::Format_ARGB32_Premultiplied);
        QPainter pt(&block);
        pt.fillRect(0, 0, 8, 8, QColor(196, 0, 196));
        pt.drawLine(0, 0, 8, 8);
        pt.end();
        p.resetTransform();
        p.setCompositionMode(QPainter::CompositionMode_SourceAtop);
        p.setOpacity(0.5);
        p.fillRect(0, 0, image.width(), image.height(), QBrush(block));
    }
#endif

    p.end();

    q->save();
    state->matrix = QTransform();
    if (extended) {
        extended->transformChanged();
    } else {
        state->dirtyFlags |= QPaintEngine::DirtyTransform;
        updateState(state);
    }
    engine->drawImage(absPathRect,
                 image,
                 QRectF(0, 0, absPathRect.width(), absPathRect.height()),
                 Qt::OrderedDither | Qt::OrderedAlphaDither);
    q->restore();
}

static inline QBrush stretchGradientToUserSpace(const QBrush &brush, const QRectF &boundingRect)
{
    Q_ASSERT(brush.style() == Qt::LinearGradientPattern
             || brush.style() == Qt::RadialGradientPattern);

    QTransform gradientToUser(boundingRect.width(), 0, 0, boundingRect.height(),
                              boundingRect.x(), boundingRect.y());

    QGradient g = *brush.gradient();
    g.setCoordinateMode(QGradient::LogicalMode);

    QBrush b(g);
    b.setTransform(gradientToUser * b.transform());
    return b;
}

void QPainterPrivate::drawStretchedGradient(const QPainterPath &path)
{
    Q_Q(QPainter);

    bool changedPen = false;
    bool changedBrush = false;
    bool needsFill = false;

    const QPen pen = state->pen;
    const QBrush brush = state->brush;

    const QGradient::CoordinateMode penMode = coordinateMode(pen.brush());
    const QGradient::CoordinateMode brushMode = coordinateMode(brush);

    QRectF boundingRect;

    // Draw the xformed fill.
    if (brush.style() != Qt::NoBrush) {
        needsFill = true;

        if (brushMode == QGradient::ObjectBoundingMode) {
            Q_ASSERT(engine->hasFeature(QPaintEngine::PatternTransform));
            boundingRect = path.boundingRect();
            q->setBrush(stretchGradientToUserSpace(brush, boundingRect));
            changedBrush = true;
        }
    }

    if (pen.style() != Qt::NoPen) {
        // Draw the xformed outline.
        if (!needsFill && brush.style() != Qt::NoBrush) {
            q->setBrush(Qt::NoBrush);
            changedBrush = true;
        }

        if (penMode == QGradient::ObjectBoundingMode) {
            Q_ASSERT(engine->hasFeature(QPaintEngine::PatternTransform));

            // avoid computing the bounding rect twice
            if (!needsFill || brushMode != QGradient::ObjectBoundingMode)
                boundingRect = path.boundingRect();

            QPen p = pen;
            p.setBrush(stretchGradientToUserSpace(pen.brush(), boundingRect));
            q->setPen(p);
            changedPen = true;
        } else if (changedPen) {
            q->setPen(pen);
            changedPen = false;
        }

        updateState(state);
        engine->drawPath(path);
    } else if (needsFill) {
        if (pen.style() != Qt::NoPen) {
            q->setPen(Qt::NoPen);
            changedPen = true;
        }

        updateState(state);
        engine->drawPath(path);
    }

    if (changedPen)
        q->setPen(pen);
    if (changedBrush)
        q->setBrush(brush);
}


void QPainterPrivate::updateMatrix()
{
    state->matrix = state->WxF ? state->worldMatrix : QTransform();
    if (state->VxF)
        state->matrix *= viewTransform();

    txinv = false;                                // no inverted matrix
    state->matrix *= state->redirectionMatrix;
    if (extended)
        extended->transformChanged();
    else
        state->dirtyFlags |= QPaintEngine::DirtyTransform;

//     printf("VxF=%d, WxF=%d\n", state->VxF, state->WxF);
//     qDebug() << " --- using matrix" << state->matrix << redirection_offset;
}

/*! \internal */
void QPainterPrivate::updateInvMatrix()
{
    Q_ASSERT(txinv == false);
    txinv = true;                                // creating inverted matrix
    invMatrix = state->matrix.inverted();
}

void QPainterPrivate::updateStateImpl(QPainterState *newState)
{
    // ### we might have to call QPainter::begin() here...
    if (!engine->state) {
        engine->state = newState;
        engine->setDirty(QPaintEngine::AllDirty);
    }

    if (engine->state->painter() != newState->painter)
        // ### this could break with clip regions vs paths.
        engine->setDirty(QPaintEngine::AllDirty);

    // Upon restore, revert all changes since last save
    else if (engine->state != newState)
        newState->dirtyFlags |= static_cast<QPainterState *>(engine->state)->changeFlags;

    // We need to store all changes made so that restore can deal with them
    else
        newState->changeFlags |= newState->dirtyFlags;

    // Unset potential dirty background mode
    newState->dirtyFlags &= ~(QPaintEngine::DirtyBackgroundMode
            | QPaintEngine::DirtyBackground);

    engine->state = newState;
    engine->updateState(*newState);
    engine->clearDirty(QPaintEngine::AllDirty);

}

void QPainterPrivate::updateState(QPainterState *newState)
{

    if (!newState) {
        engine->state = newState;

    } else if (newState->state() || engine->state!=newState) {
        bool setNonCosmeticPen = (newState->renderHints & QPainter::NonCosmeticDefaultPen)
                                 && newState->pen.widthF() == 0;
        if (setNonCosmeticPen) {
            // Override the default pen's cosmetic state if the
            // NonCosmeticDefaultPen render hint is used.
            QPen oldPen = newState->pen;
            newState->pen.setWidth(1);
            newState->pen.setCosmetic(false);
            newState->dirtyFlags |= QPaintEngine::DirtyPen;

            updateStateImpl(newState);

            // Restore the state pen back to its default to preserve visible
            // state.
            newState->pen = oldPen;
        } else {
            updateStateImpl(newState);
        }
    }
}


/*!
    \class QPainter
    \brief The QPainter class performs low-level painting on widgets and
    other paint devices.

    \ingroup painting

    \reentrant

    QPainter provides highly optimized functions to do most of the
    drawing GUI programs require. It can draw everything from simple
    lines to complex shapes like pies and chords. It can also draw
    aligned text and pixmaps. Normally, it draws in a "natural"
    coordinate system, but it can also do view and world
    transformation. QPainter can operate on any object that inherits
    the QPaintDevice class.

    The common use of QPainter is inside a widget's paint event:
    Construct and customize (e.g. set the pen or the brush) the
    painter. Then draw. Remember to destroy the QPainter object after
    drawing. For example:

    \snippet doc/src/snippets/code/src_gui_painting_qpainter.cpp 0

    The core functionality of QPainter is drawing, but the class also
    provide several functions that allows you to customize QPainter's
    settings and its rendering quality, and others that enable
    clipping. In addition you can control how different shapes are
    merged together by specifying the painter's composition mode.

    The isActive() function indicates whether the painter is active. A
    painter is activated by the begin() function and the constructor
    that takes a QPaintDevice argument. The end() function, and the
    destructor, deactivates it.

    Together with the QPaintDevice and QPaintEngine classes, QPainter
    form the basis for Qt's paint system. QPainter is the class used
    to perform drawing operations. QPaintDevice represents a device
    that can be painted on using a QPainter. QPaintEngine provides the
    interface that the painter uses to draw onto different types of
    devices. If the painter is active, device() returns the paint
    device on which the painter paints, and paintEngine() returns the
    paint engine that the painter is currently operating on. For more
    information, see the \l {Paint System}.

    Sometimes it is desirable to make someone else paint on an unusual
    QPaintDevice. QPainter supports a static function to do this,
    setRedirected().

    \warning When the paintdevice is a widget, QPainter can only be
    used inside a paintEvent() function or in a function called by
    paintEvent().

    \tableofcontents

    \section1 Settings

    There are several settings that you can customize to make QPainter
    draw according to your preferences:

    \list

    \o font() is the font used for drawing text. If the painter
        isActive(), you can retrieve information about the currently set
        font, and its metrics, using the fontInfo() and fontMetrics()
        functions respectively.

    \o brush() defines the color or pattern that is used for filling
       shapes.

    \o pen() defines the color or stipple that is used for drawing
       lines or boundaries.

    \o backgroundMode() defines whether there is a background() or
       not, i.e it is either Qt::OpaqueMode or Qt::TransparentMode.

    \o background() only applies when backgroundMode() is \l
       Qt::OpaqueMode and pen() is a stipple. In that case, it
       describes the color of the background pixels in the stipple.

    \o brushOrigin() defines the origin of the tiled brushes, normally
       the origin of widget's background.

    \o viewport(), window(), worldTransform() make up the painter's coordinate
        transformation system. For more information, see the \l
        {Coordinate Transformations} section and the \l {Coordinate
        System} documentation.

    \o hasClipping() tells whether the painter clips at all. (The paint
       device clips, too.) If the painter clips, it clips to clipRegion().

    \o layoutDirection() defines the layout direction used by the
       painter when drawing text.

    \o worldMatrixEnabled() tells whether world transformation is enabled.

    \o viewTransformEnabled() tells whether view transformation is
        enabled.

    \endlist

    Note that some of these settings mirror settings in some paint
    devices, e.g.  QWidget::font(). The QPainter::begin() function (or
    equivalently the QPainter constructor) copies these attributes
    from the paint device.

    You can at any time save the QPainter's state by calling the
    save() function which saves all the available settings on an
    internal stack. The restore() function pops them back.

    \section1 Drawing

    QPainter provides functions to draw most primitives: drawPoint(),
    drawPoints(), drawLine(), drawRect(), drawRoundedRect(),
    drawEllipse(), drawArc(), drawPie(), drawChord(), drawPolyline(),
    drawPolygon(), drawConvexPolygon() and drawCubicBezier().  The two
    convenience functions, drawRects() and drawLines(), draw the given
    number of rectangles or lines in the given array of \l
    {QRect}{QRects} or \l {QLine}{QLines} using the current pen and
    brush.

    The QPainter class also provides the fillRect() function which
    fills the given QRect, with the given QBrush, and the eraseRect()
    function that erases the area inside the given rectangle.

    All of these functions have both integer and floating point
    versions.

    \table 100%
    \row
    \o \inlineimage qpainter-basicdrawing.png
    \o
    \bold {Basic Drawing Example}

    The \l {painting/basicdrawing}{Basic Drawing} example shows how to
    display basic graphics primitives in a variety of styles using the
    QPainter class.

    \endtable

    If you need to draw a complex shape, especially if you need to do
    so repeatedly, consider creating a QPainterPath and drawing it
    using drawPath().

    \table 100%
    \row
    \o
    \bold {Painter Paths example}

    The QPainterPath class provides a container for painting
    operations, enabling graphical shapes to be constructed and
    reused.

    The \l {painting/painterpaths}{Painter Paths} example shows how
    painter paths can be used to build complex shapes for rendering.

    \o \inlineimage qpainter-painterpaths.png
    \endtable

    QPainter also provides the fillPath() function which fills the
    given QPainterPath with the given QBrush, and the strokePath()
    function that draws the outline of the given path (i.e. strokes
    the path).

    See also the \l {demos/deform}{Vector Deformation} demo which
    shows how to use advanced vector techniques to draw text using a
    QPainterPath, the \l {demos/gradients}{Gradients} demo which shows
    the different types of gradients that are available in Qt, and the \l
    {demos/pathstroke}{Path Stroking} demo which shows Qt's built-in
    dash patterns and shows how custom patterns can be used to extend
    the range of available patterns.

    \table
    \header
    \o \l {demos/deform}{Vector Deformation}
    \o \l {demos/gradients}{Gradients}
    \o \l {demos/pathstroke}{Path Stroking}
    \row
    \o \inlineimage qpainter-vectordeformation.png
    \o \inlineimage qpainter-gradients.png
    \o \inlineimage qpainter-pathstroking.png
    \endtable


    There are functions to draw pixmaps/images, namely drawPixmap(),
    drawImage() and drawTiledPixmap(). Both drawPixmap() and drawImage()
    produce the same result, except that drawPixmap() is faster
    on-screen while drawImage() may be faster on a QPrinter or other
    devices.

    Text drawing is done using drawText(). When you need
    fine-grained positioning, boundingRect() tells you where a given
    drawText() command will draw.

    \section1 Rendering Quality

    To get the optimal rendering result using QPainter, you should use
    the platform independent QImage as paint device; i.e. using QImage
    will ensure that the result has an identical pixel representation
    on any platform.

    The QPainter class also provides a means of controlling the
    rendering quality through its RenderHint enum and the support for
    floating point precision: All the functions for drawing primitives
    has a floating point version. These are often used in combination
    with the \l {RenderHint}{QPainter::Antialiasing} render hint.

    \table 100%
    \row
    \o \inlineimage qpainter-concentriccircles.png
    \o
    \bold {Concentric Circles Example}

    The \l {painting/concentriccircles}{Concentric Circles} example
    shows the improved rendering quality that can be obtained using
    floating point precision and anti-aliasing when drawing custom
    widgets.

    The application's main window displays several widgets which are
    drawn using the various combinations of precision and
    anti-aliasing.

    \endtable

    The RenderHint enum specifies flags to QPainter that may or may
    not be respected by any given engine.  \l
    {RenderHint}{QPainter::Antialiasing} indicates that the engine
    should antialias edges of primitives if possible, \l
    {RenderHint}{QPainter::TextAntialiasing} indicates that the engine
    should antialias text if possible, and the \l
    {RenderHint}{QPainter::SmoothPixmapTransform} indicates that the
    engine should use a smooth pixmap transformation algorithm.

    The renderHints() function returns a flag that specifies the
    rendering hints that are set for this painter.  Use the
    setRenderHint() function to set or clear the currently set
    RenderHints.

    \section1 Coordinate Transformations

    Normally, the QPainter operates on the device's own coordinate
    system (usually pixels), but QPainter has good support for
    coordinate transformations.

    \table
    \header
    \o  nop \o rotate() \o scale() \o translate()
    \row
    \o \inlineimage qpainter-clock.png
    \o \inlineimage qpainter-rotation.png
    \o \inlineimage qpainter-scale.png
    \o \inlineimage qpainter-translation.png
    \endtable

    The most commonly used transformations are scaling, rotation,
    translation and shearing. Use the scale() function to scale the
    coordinate system by a given offset, the rotate() function to
    rotate it clockwise and translate() to translate it (i.e. adding a
    given offset to the points). You can also twist the coordinate
    system around the origin using the shear() function. See the \l
    {demos/affine}{Affine Transformations} demo for a visualization of
    a sheared coordinate system.

    See also the \l {painting/transformations}{Transformations}
    example which shows how transformations influence the way that
    QPainter renders graphics primitives. In particular it shows how
    the order of transformations affects the result.

    \table 100%
    \row
    \o
    \bold {Affine Transformations Demo}

    The \l {demos/affine}{Affine Transformations} demo show Qt's
    ability to perform affine transformations on painting
    operations. The demo also allows the user to experiment with the
    transformation operations and see the results immediately.

    \o \inlineimage qpainter-affinetransformations.png
    \endtable

    All the tranformation operations operate on the transformation
    worldTransform(). A matrix transforms a point in the plane to another
    point. For more information about the transformation matrix, see
    the \l {Coordinate System} and QTransform documentation.

    The setWorldTransform() function can replace or add to the currently
    set worldTransform(). The resetTransform() function resets any
    transformations that were made using translate(), scale(),
    shear(), rotate(), setWorldTransform(), setViewport() and setWindow()
    functions. The deviceTransform() returns the matrix that transforms
    from logical coordinates to device coordinates of the platform
    dependent paint device. The latter function is only needed when
    using platform painting commands on the platform dependent handle,
    and the platform does not do transformations nativly.

    When drawing with QPainter, we specify points using logical
    coordinates which then are converted into the physical coordinates
    of the paint device. The mapping of the logical coordinates to the
    physical coordinates are handled by QPainter's combinedTransform(), a
    combination of viewport() and window() and worldTransform(). The
    viewport() represents the physical coordinates specifying an
    arbitrary rectangle, the window() describes the same rectangle in
    logical coordinates, and the worldTransform() is identical with the
    transformation matrix.

    See also \l {Coordinate System}

    \section1 Clipping

    QPainter can clip any drawing operation to a rectangle, a region,
    or a vector path. The current clip is available using the
    functions clipRegion() and clipPath(). Whether paths or regions are
    preferred (faster) depends on the underlying paintEngine(). For
    example, the QImage paint engine prefers paths while the X11 paint
    engine prefers regions. Setting a clip is done in the painters
    logical coordinates.

    After QPainter's clipping, the paint device may also clip. For
    example, most widgets clip away the pixels used by child widgets,
    and most printers clip away an area near the edges of the paper.
    This additional clipping is not reflected by the return value of
    clipRegion() or hasClipping().

    \section1 Composition Modes
    \target Composition Modes

    QPainter provides the CompositionMode enum which defines the
    Porter-Duff rules for digital image compositing; it describes a
    model for combining the pixels in one image, the source, with the
    pixels in another image, the destination.

    The two most common forms of composition are \l
    {QPainter::CompositionMode}{Source} and \l
    {QPainter::CompositionMode}{SourceOver}.  \l
    {QPainter::CompositionMode}{Source} is used to draw opaque objects
    onto a paint device. In this mode, each pixel in the source
    replaces the corresponding pixel in the destination. In \l
    {QPainter::CompositionMode}{SourceOver} composition mode, the
    source object is transparent and is drawn on top of the
    destination.

    Note that composition transformation operates pixelwise. For that
    reason, there is a difference between using the graphic primitive
    itself and its bounding rectangle: The bounding rect contains
    pixels with alpha == 0 (i.e the pixels surrounding the
    primitive). These pixels will overwrite the other image's pixels,
    affectively clearing those, while the primitive only overwrites
    its own area.

    \table 100%
    \row
    \o \inlineimage qpainter-compositiondemo.png

    \o
    \bold {Composition Modes Demo}

    The \l {demos/composition}{Composition Modes} demo, available in
    Qt's demo directory, allows you to experiment with the various
    composition modes and see the results immediately.

    \endtable

    \section1 Limitations
    \target Limitations

    If you are using coordinates with Qt's raster-based paint engine, it is
    important to note that, while coordinates greater than +/- 2\sup 15 can
    be used, any painting performed with coordinates outside this range is not
    guaranteed to be shown; the drawing may be clipped. This is due to the
    use of \c{short int} in the implementation.

    The outlines generated by Qt's stroker are only an approximation when dealing
    with curved shapes. It is in most cases impossible to represent the outline of
    a bezier curve segment using another bezier curve segment, and so Qt approximates
    the curve outlines by using several smaller curves. For performance reasons there
    is a limit to how many curves Qt uses for these outlines, and thus when using
    large pen widths or scales the outline error increases. To generate outlines with
    smaller errors it is possible to use the QPainterPathStroker class, which has the
    setCurveThreshold member function which let's the user specify the error tolerance.
    Another workaround is to convert the paths to polygons first and then draw the
    polygons instead.

    \section1 Performance

    QPainter is a rich framework that allows developers to do a great
    variety of graphical operations, such as gradients, composition
    modes and vector graphics. And QPainter can do this across a
    variety of different hardware and software stacks. Naturally the
    underlying combination of hardware and software has some
    implications for performance, and ensuring that every single
    operation is fast in combination with all the various combinations
    of composition modes, brushes, clipping, transformation, etc, is
    close to an impossible task because of the number of
    permutations. As a compromise we have selected a subset of the
    QPainter API and backends, where performance is guaranteed to be as
    good as we can sensibly get it for the given combination of
    hardware and software.

    The backends we focus on as high-performance engines are:

    \list

    \o Raster - This backend implements all rendering in pure software
    and is always used to render into QImages. For optimal performance
    only use the format types QImage::Format_ARGB32_Premultiplied,
    QImage::Format_RGB32 or QImage::Format_RGB16. Any other format,
    including QImage::Format_ARGB32, has significantly worse
    performance. This engine is also used by default.

    \endlist

    These operations are:

    \list

    \o Simple transformations, meaning translation and scaling, plus
    0, 90, 180, 270 degree rotations.

    \o \c drawPixmap() in combination with simple transformations and
    opacity with non-smooth transformation mode
    (\c QPainter::SmoothPixmapTransform not enabled as a render hint).

    \o Rectangle fills with solid color, two-color linear gradients
    and simple transforms.

    \o Rectangular clipping with simple transformations and intersect
    clip.

    \o Composition Modes \c QPainter::CompositionMode_Source and
    QPainter::CompositionMode_SourceOver

    \o Rounded rectangle filling using solid color and two-color
    linear gradients fills.

    \o 3x3 patched pixmaps, via qDrawBorderPixmap.

    \endlist

    This list gives an indication of which features to safely use in
    an application where performance is critical. For certain setups,
    other operations may be fast too, but before making extensive use
    of them, it is recommended to benchmark and verify them on the
    system where the software will run in the end. There are also
    cases where expensive operations are ok to use, for instance when
    the result is cached in a QPixmap.

    \sa QPaintDevice, QPaintEngine, {QtSvg Module}, {Basic Drawing Example},
        {Drawing Utility Functions}
*/

/*!
    \enum QPainter::RenderHint

    Renderhints are used to specify flags to QPainter that may or
    may not be respected by any given engine.

    \value Antialiasing Indicates that the engine should antialias
    edges of primitives if possible.

    \value TextAntialiasing Indicates that the engine should antialias
    text if possible. To forcibly disable antialiasing for text, do not
    use this hint. Instead, set QFont::PreferNoHinting on your font's
    hint preferences.

    \value SmoothPixmapTransform Indicates that the engine should use
    a smooth pixmap transformation algorithm (such as bilinear) rather
    than nearest neighbor.

    \value NonCosmeticDefaultPen The engine should interpret pens with a width
    of 0 (which otherwise enables QPen::isCosmetic()) as being a non-cosmetic
    pen with a width of 1.

    \sa renderHints(), setRenderHint(), {QPainter#Rendering
    Quality}{Rendering Quality}, {Concentric Circles Example}

*/

/*!
    Constructs a painter.

    \sa begin(), end()
*/

QPainter::QPainter()
    : d_ptr(new QPainterPrivate(this))
{
}

/*!
    \fn QPainter::QPainter(QPaintDevice *device)

    Constructs a painter that begins painting the paint \a device
    immediately.

    This constructor is convenient for short-lived painters, e.g. in a
    QWidget::paintEvent() and should be used only once. The
    constructor calls begin() for you and the QPainter destructor
    automatically calls end().

    Here's an example using begin() and end():
    \snippet doc/src/snippets/code/src_gui_painting_qpainter.cpp 1

    The same example using this constructor:
    \snippet doc/src/snippets/code/src_gui_painting_qpainter.cpp 2

    Since the constructor cannot provide feedback when the initialization
    of the painter failed you should rather use begin() and end() to paint
    on external devices, e.g. printers.

    \sa begin(), end()
*/

QPainter::QPainter(QPaintDevice *pd)
    : d_ptr(0)
{
    Q_ASSERT(pd != 0);
    if (!QPainterPrivate::attachPainterPrivate(this, pd)) {
        d_ptr.reset(new QPainterPrivate(this));
        begin(pd);
    }
    Q_ASSERT(d_ptr);
}

/*!
    Destroys the painter.
*/
QPainter::~QPainter()
{
    d_ptr->inDestructor = true;
    if (isActive()) {
        end();
    } else if (d_ptr->refcount > 1) {
        d_ptr->detachPainterPrivate(this);
    }
    if (d_ptr) {
        // Make sure we haven't messed things up.
        Q_ASSERT(d_ptr->inDestructor);
        d_ptr->inDestructor = false;
        Q_ASSERT(d_ptr->refcount == 1);
        if (d_ptr->d_ptrs)
            free(d_ptr->d_ptrs);
    }
}

/*!
    Returns the paint device on which this painter is currently
    painting, or 0 if the painter is not active.

    \sa isActive()
*/

QPaintDevice *QPainter::device() const
{
    Q_D(const QPainter);
    if (isActive() && d->engine->d_func()->currentClipWidget)
        return d->engine->d_func()->currentClipWidget;
    return d->original_device;
}

/*!
    Returns true if begin() has been called and end() has not yet been
    called; otherwise returns false.

    \sa begin(), QPaintDevice::paintingActive()
*/

bool QPainter::isActive() const
{
    Q_D(const QPainter);
    return d->engine;
}

/*!
    Initializes the painters pen, background and font to the same as
    the given \a widget. This function is called automatically when the
    painter is opened on a QWidget.

    \sa begin(), {QPainter#Settings}{Settings}
*/
void QPainter::initFrom(const QWidget *widget)
{
    Q_ASSERT_X(widget, "QPainter::initFrom(const QWidget *widget)", "Widget cannot be 0");
    Q_D(QPainter);
    if (Q_UNLIKELY(!d->engine)) {
        qWarning("QPainter::initFrom: Painter not active, aborted");
        return;
    }

    const QPalette &pal = widget->palette();
    d->state->pen = QPen(pal.brush(widget->foregroundRole()), 0);
    d->state->bgBrush = pal.brush(widget->backgroundRole());
    d->state->deviceFont = QFont(widget->font(), const_cast<QWidget*> (widget));
    d->state->font = d->state->deviceFont;
    if (d->extended) {
        d->extended->penChanged();
    } else if (d->engine) {
        d->engine->setDirty(QPaintEngine::DirtyPen);
        d->engine->setDirty(QPaintEngine::DirtyBrush);
        d->engine->setDirty(QPaintEngine::DirtyFont);
    }
}


/*!
    Saves the current painter state (pushes the state onto a stack). A
    save() must be followed by a corresponding restore(); the end()
    function unwinds the stack.

    \sa restore()
*/

void QPainter::save()
{
#ifdef QT_DEBUG_DRAW
    printf("QPainter::save()\n");
#endif
    Q_D(QPainter);
    if (Q_UNLIKELY(!d->engine)) {
        qWarning("QPainter::save: Painter not active");
        return;
    }

    if (d->extended) {
        d->state = d->extended->createState(d->states.back());
        d->extended->setState(d->state);
    } else {
        d->updateState(d->state);
        d->state = new QPainterState(d->states.back());
        d->engine->state = d->state;
    }
    d->states.push_back(d->state);
}

/*!
    Restores the current painter state (pops a saved state off the
    stack).

    \sa save()
*/

void QPainter::restore()
{
#ifdef QT_DEBUG_DRAW
    printf("QPainter::restore()\n");
#endif
    Q_D(QPainter);
    if (Q_UNLIKELY(d->states.size()<=1)) {
        qWarning("QPainter::restore: Unbalanced save/restore");
        return;
    } else if (Q_UNLIKELY(!d->engine)) {
        qWarning("QPainter::restore: Painter not active");
        return;
    }

    QPainterState *tmp = d->state;
    d->states.pop_back();
    d->state = d->states.back();
    d->txinv = false;

    if (d->extended) {
        d->extended->setState(d->state);
        delete tmp;
        return;
    }

    // trigger clip update if the clip path/region has changed since
    // last save
    if (!d->state->clipInfo.isEmpty()
        && (tmp->changeFlags & (QPaintEngine::DirtyClipRegion | QPaintEngine::DirtyClipPath))) {
        // reuse the tmp state to avoid any extra allocs...
        tmp->dirtyFlags = QPaintEngine::DirtyClipPath;
        tmp->clipOperation = Qt::NoClip;
        tmp->clipPath = QPainterPath();
        d->engine->updateState(*tmp);
        // replay the list of clip states,
        for (int i=0; i<d->state->clipInfo.size(); ++i) {
            const QPainterClipInfo &info = d->state->clipInfo.at(i);
            tmp->matrix = info.matrix;
            tmp->matrix *= d->state->redirectionMatrix;
            tmp->clipOperation = info.operation;
            if (info.clipType == QPainterClipInfo::RectClip) {
                tmp->dirtyFlags = QPaintEngine::DirtyClipRegion | QPaintEngine::DirtyTransform;
                tmp->clipRegion = info.rect;
            } else if (info.clipType == QPainterClipInfo::RegionClip) {
                tmp->dirtyFlags = QPaintEngine::DirtyClipRegion | QPaintEngine::DirtyTransform;
                tmp->clipRegion = info.region;
            } else { // clipType == QPainterClipInfo::PathClip
                tmp->dirtyFlags = QPaintEngine::DirtyClipPath | QPaintEngine::DirtyTransform;
                tmp->clipPath = info.path;
            }
            d->engine->updateState(*tmp);
        }


        //Since we've updated the clip region anyway, pretend that the clip path hasn't changed:
        d->state->dirtyFlags &= ~(QPaintEngine::DirtyClipPath | QPaintEngine::DirtyClipRegion);
        tmp->changeFlags &= ~(QPaintEngine::DirtyClipPath | QPaintEngine::DirtyClipRegion);
        tmp->changeFlags |= QPaintEngine::DirtyTransform;
    }

    d->updateState(d->state);
    delete tmp;
}


/*!

    \fn bool QPainter::begin(QPaintDevice *device)

    Begins painting the paint \a device and returns true if
    successful; otherwise returns false.

    Notice that all painter settings (setPen(), setBrush() etc.) are reset
    to default values when begin() is called.

    The errors that can occur are serious problems, such as these:

    \snippet doc/src/snippets/code/src_gui_painting_qpainter.cpp 3

    Note that most of the time, you can use one of the constructors
    instead of begin(), and that end() is automatically done at
    destruction.

    \warning A paint device can only be painted by one painter at a
    time.

    \sa end(), QPainter()
*/

static inline void qt_cleanup_painter_state(QPainterPrivate *d)
{
    d->states.clear();
    delete d->state;
    d->state = 0;
    d->engine = 0;
    d->device = 0;
}

bool QPainter::begin(QPaintDevice *pd)
{
    Q_ASSERT(pd);

    if (Q_UNLIKELY(pd->painters > 0)) {
        qWarning("QPainter::begin: A paint device can only be painted by one painter at a time.");
        return false;
    }

    if (Q_UNLIKELY(d_ptr->engine)) {
        qWarning("QPainter::begin: Painter already active");
        return false;
    }

    if (QPainterPrivate::attachPainterPrivate(this, pd))
        return true;

    Q_D(QPainter);

    d->original_device = pd;
    QPaintDevice *rpd = 0;

    QPoint redirectionOffset;
    // We know for sure that redirection is broken when the widget is inside
    // its paint event, so it's safe to use our hard-coded redirection. However,
    // there IS one particular case we still need to support, and that's
    // when people call QPainter::setRedirected in the widget's paint event right
    // before any painter is created (or QPainter::begin is called). In that
    // particular case our hard-coded redirection is restored and the redirection
    // is retrieved from QPainter::redirected (as before).
    if (pd->devType() == QInternal::Widget)
        rpd = static_cast<QWidget *>(pd)->d_func()->redirected(&redirectionOffset);

    if (rpd)
        pd = rpd;

#ifdef QT_DEBUG_DRAW
    printf("QPainter::begin(), device=%p, type=%d\n", pd, pd->devType());
#endif

    if (pd->devType() == QInternal::Pixmap)
        static_cast<QPixmap *>(pd)->detach();
    else if (pd->devType() == QInternal::Image)
        static_cast<QImage *>(pd)->detach();

    d->engine = pd->paintEngine();

    if (Q_UNLIKELY(!d->engine)) {
        qWarning("QPainter::begin: Paint device returned engine == 0, type: %d", pd->devType());
        return false;
    }

    d->device = pd;

    d->extended = d->engine->isExtended() ? static_cast<QPaintEngineEx *>(d->engine) : nullptr;

    // Setup new state...
    Q_ASSERT(!d->state);
    d->state = d->extended ? d->extended->createState(0) : new QPainterState;
    d->state->painter = this;
    d->states.push_back(d->state);

    d->state->redirectionMatrix.translate(-redirectionOffset.x(), -redirectionOffset.y());
    d->state->brushOrigin = QPointF();

    // Slip a painter state into the engine before we do any other operations
    if (d->extended)
        d->extended->setState(d->state);
    else
        d->engine->state = d->state;

    switch (pd->devType()) {
        case QInternal::Widget:
        {
            const QWidget *widget = static_cast<const QWidget *>(pd);
            Q_ASSERT(widget);

            const bool inPaintEvent = widget->testAttribute(Qt::WA_WState_InPaintEvent);
            if(Q_UNLIKELY(!inPaintEvent)) {
                qWarning("QPainter::begin: Widget painting can only begin as a "
                         "result of a paintEvent");
                qt_cleanup_painter_state(d);
                return false;
            }
            break;
        }
        case QInternal::Pixmap:
        {
            QPixmap *pm = static_cast<QPixmap *>(pd);
            Q_ASSERT(pm);
            if (Q_UNLIKELY(pm->isNull())) {
                qWarning("QPainter::begin: Cannot paint on a null pixmap");
                qt_cleanup_painter_state(d);
                return false;
            }

            if (pm->depth() == 1) {
                d->state->pen = QPen(Qt::color1);
                d->state->brush = QBrush(Qt::color0);
            }
            break;
        }
        case QInternal::Image:
        {
            QImage *img = static_cast<QImage *>(pd);
            Q_ASSERT(img);
            if (Q_UNLIKELY(img->isNull())) {
                qWarning("QPainter::begin: Cannot paint on a null image");
                qt_cleanup_painter_state(d);
                return false;
            }
            if (img->depth() == 1) {
                d->state->pen = QPen(Qt::color1);
                d->state->brush = QBrush(Qt::color0);
            }
            break;
        }
        default:
            break;
    }
    if (d->state->ww == 0) // For compat with 3.x painter defaults
        d->state->ww = d->state->wh = d->state->vw = d->state->vh = 1024;

    d->engine->setPaintDevice(pd);

    bool begun = d->engine->begin(pd);
    if (Q_UNLIKELY(!begun)) {
        qWarning("QPainter::begin(): Returned false");
        if (d->engine->isActive()) {
            end();
        } else {
            qt_cleanup_painter_state(d);
        }
        return false;
    } else {
        d->engine->setActive(begun);
    }

    // Copy painter properties from original paint device,
    // required for QPixmap::grabWidget()
    if (d->original_device->devType() == QInternal::Widget) {
        QWidget *widget = static_cast<QWidget *>(d->original_device);
        initFrom(widget);
    } else {
        d->state->layoutDirection = Qt::LayoutDirectionAuto;
        // make sure we have a font compatible with the paintdevice
        d->state->deviceFont = d->state->font = QFont(d->state->deviceFont, device());
    }

    QRect systemRect = d->engine->systemRect();
    if (!systemRect.isEmpty()) {
        d->state->ww = d->state->vw = systemRect.width();
        d->state->wh = d->state->vh = systemRect.height();
    } else {
        d->state->ww = d->state->vw = pd->metric(QPaintDevice::PdmWidth);
        d->state->wh = d->state->vh = pd->metric(QPaintDevice::PdmHeight);
    }

    Q_ASSERT(d->engine->isActive());

    if (!d->state->redirectionMatrix.isIdentity())
        d->updateMatrix();

    Q_ASSERT(d->engine->isActive());
    d->state->renderHints = QPainter::TextAntialiasing;
    ++d->device->painters;

    return true;
}

/*!
    Ends painting. Any resources used while painting are released. You
    don't normally need to call this since it is called by the
    destructor.

    Returns true if the painter is no longer active; otherwise returns false.

    \sa begin(), isActive()
*/

bool QPainter::end()
{
#ifdef QT_DEBUG_DRAW
    printf("QPainter::end()\n");
#endif
    Q_D(QPainter);

    if (Q_UNLIKELY(!d->engine)) {
        qWarning("QPainter::end: Painter not active, aborted");
        qt_cleanup_painter_state(d);
        return false;
    }

    if (d->refcount > 1) {
        d->detachPainterPrivate(this);
        return true;
    }

    bool ended = true;

    if (d->engine->isActive()) {
        ended = d->engine->end();
        d->updateState(0);

        --d->device->painters;
        if (d->device->painters == 0) {
            d->engine->setPaintDevice(0);
            d->engine->setActive(false);
        }
    }

    if (Q_UNLIKELY(d->states.size() > 1)) {
        qWarning("QPainter::end: Painter ended with %d saved states",
                 d->states.size());
    }

    if (d->extended) {
        d->extended = nullptr;
    }

    qt_cleanup_painter_state(d);

    return ended;
}


/*!
    Returns the paint engine that the painter is currently operating
    on if the painter is active; otherwise 0.

    \sa isActive()
*/
QPaintEngine *QPainter::paintEngine() const
{
    Q_D(const QPainter);
    return d->engine;
}

/*!
    Returns the font metrics for the painter if the painter is
    active. Otherwise, the return value is undefined.

    \sa font(), isActive(), {QPainter#Settings}{Settings}
*/

QFontMetrics QPainter::fontMetrics() const
{
    Q_D(const QPainter);
    if (Q_UNLIKELY(!d->engine)) {
        qWarning("QPainter::fontMetrics: Painter not active");
        return QFontMetrics(QFont());
    }
    return QFontMetrics(d->state->font);
}

/*!
    \since 4.2

    Returns the opacity of the painter. The default value is
    1.
*/

qreal QPainter::opacity() const
{
    Q_D(const QPainter);
    if (Q_UNLIKELY(!d->engine)) {
        qWarning("QPainter::opacity: Painter not active");
        return 1.0;
    }
    return d->state->opacity;
}

/*!
    \since 4.2

    Sets the opacity of the painter to \a opacity. The value should
    be in the range 0.0 to 1.0, where 0.0 is fully transparent and
    1.0 is fully opaque.

    Opacity set on the painter will apply to all drawing operations
    individually.
*/

void QPainter::setOpacity(qreal opacity)
{
    Q_D(QPainter);

    if (Q_UNLIKELY(!d->engine)) {
        qWarning("QPainter::setOpacity: Painter not active");
        return;
    }

    opacity = qMin(qreal(1), qMax(qreal(0), opacity));

    if (opacity == d->state->opacity)
        return;

    d->state->opacity = opacity;

    if (d->extended)
        d->extended->opacityChanged();
    else
        d->state->dirtyFlags |= QPaintEngine::DirtyOpacity;
}


/*!
    Returns the currently set brush origin.

    \sa setBrushOrigin(), {QPainter#Settings}{Settings}
*/

QPoint QPainter::brushOrigin() const
{
    Q_D(const QPainter);
    if (Q_UNLIKELY(!d->engine)) {
        qWarning("QPainter::brushOrigin: Painter not active");
        return QPoint();
    }
    return d->state->brushOrigin.toPoint();
}

/*!
    \fn void QPainter::setBrushOrigin(const QPointF &position)

    Sets the brush origin to \a position.

    The brush origin specifies the (0, 0) coordinate of the painter's
    brush.

    Note that while the brushOrigin() was necessary to adopt the
    parent's background for a widget in Qt 3, this is no longer the
    case since the Qt 4 painter doesn't paint the background unless
    you explicitly tell it to do so by setting the widget's \l
    {QWidget::autoFillBackground}{autoFillBackground} property to
    true.

    \sa brushOrigin(), {QPainter#Settings}{Settings}
*/

void QPainter::setBrushOrigin(const QPointF &p)
{
    Q_D(QPainter);
#ifdef QT_DEBUG_DRAW
    printf("QPainter::setBrushOrigin(), (%.2f,%.2f)\n", p.x(), p.y());
#endif

    if (Q_UNLIKELY(!d->engine)) {
        qWarning("QPainter::setBrushOrigin: Painter not active");
        return;
    }

    d->state->brushOrigin = p;

    if (d->extended) {
        d->extended->brushOriginChanged();
        return;
    }

    d->state->dirtyFlags |= QPaintEngine::DirtyBrushOrigin;
}

/*!
    \fn void QPainter::setBrushOrigin(const QPoint &position)
    \overload

    Sets the brush's origin to the given \a position.
*/

/*!
    \fn void QPainter::setBrushOrigin(int x, int y)

    \overload

    Sets the brush's origin to point (\a x, \a y).
*/

/*!
    \enum QPainter::CompositionMode

    Defines the modes supported for digital image compositing.
    Composition modes are used to specify how the pixels in one image,
    the source, are merged with the pixel in another image, the
    destination.

     \image qpainter-compositionmode1.png
     \image qpainter-compositionmode2.png

    The most common type is SourceOver (often referred to as just
    alpha blending) where the source pixel is blended on top of the
    destination pixel in such a way that the alpha component of the
    source defines the translucency of the pixel.

    When the paint device is a QImage, the image format must be set to
    \l {QImage::Format}{Format_ARGB32Premultiplied} or
    \l {QImage::Format}{Format_ARGB32} for the composition modes to have
    any effect. For performance the premultiplied version is the preferred
    format.

    When a composition mode is set it applies to all painting
    operator, pens, brushes, gradients and pixmap/image drawing.

    \value CompositionMode_SourceOver This is the default mode. The
    alpha of the source is used to blend the pixel on top of the
    destination.

    \value CompositionMode_DestinationOver The alpha of the
    destination is used to blend it on top of the source pixels. This
    mode is the inverse of CompositionMode_SourceOver.

    \value CompositionMode_Clear The pixels in the destination are
    cleared (set to fully transparent) independent of the source.

    \value CompositionMode_Source The output is the source
    pixel. (This means a basic copy operation and is identical to
    SourceOver when the source pixel is opaque).

    \value CompositionMode_Destination The output is the destination
    pixel. This means that the blending has no effect. This mode is
    the inverse of CompositionMode_Source.

    \value CompositionMode_SourceIn The output is the source, where
    the alpha is reduced by that of the destination.

    \value CompositionMode_DestinationIn The output is the
    destination, where the alpha is reduced by that of the
    source. This mode is the inverse of CompositionMode_SourceIn.

    \value CompositionMode_SourceOut The output is the source, where
    the alpha is reduced by the inverse of destination.

    \value CompositionMode_DestinationOut The output is the
    destination, where the alpha is reduced by the inverse of the
    source. This mode is the inverse of CompositionMode_SourceOut.

    \value CompositionMode_SourceAtop The source pixel is blended on
    top of the destination, with the alpha of the source pixel reduced
    by the alpha of the destination pixel.

    \value CompositionMode_DestinationAtop The destination pixel is
    blended on top of the source, with the alpha of the destination
    pixel is reduced by the alpha of the destination pixel. This mode
    is the inverse of CompositionMode_SourceAtop.

    \value CompositionMode_Xor The source, whose alpha is reduced with
    the inverse of the destination alpha, is merged with the
    destination, whose alpha is reduced by the inverse of the source
    alpha. CompositionMode_Xor is not the same as the bitwise Xor.

    \value CompositionMode_Plus Both the alpha and color of the source
    and destination pixels are added together.

    \value CompositionMode_Multiply The output is the source color
    multiplied by the destination. Multiplying a color with white
    leaves the color unchanged, while multiplying a color
    with black produces black.

    \value CompositionMode_Screen The source and destination colors
    are inverted and then multiplied. Screening a color with white
    produces white, whereas screening a color with black leaves the
    color unchanged.

    \value CompositionMode_Overlay Multiplies or screens the colors
    depending on the destination color. The destination color is mixed
    with the source color to reflect the lightness or darkness of the
    destination.

    \value CompositionMode_Darken The darker of the source and
    destination colors is selected.

    \value CompositionMode_Lighten The lighter of the source and
    destination colors is selected.

    \value CompositionMode_ColorDodge The destination color is
    brightened to reflect the source color. A black source color
    leaves the destination color unchanged.

    \value CompositionMode_ColorBurn The destination color is darkened
    to reflect the source color. A white source color leaves the
    destination color unchanged.

    \value CompositionMode_HardLight Multiplies or screens the colors
    depending on the source color. A light source color will lighten
    the destination color, whereas a dark source color will darken the
    destination color.

    \value CompositionMode_SoftLight Darkens or lightens the colors
    depending on the source color. Similar to
    CompositionMode_HardLight.

    \value CompositionMode_Difference Subtracts the darker of the
    colors from the lighter.  Painting with white inverts the
    destination color, whereas painting with black leaves the
    destination color unchanged.

    \value CompositionMode_Exclusion Similar to
    CompositionMode_Difference, but with a lower contrast. Painting
    with white inverts the destination color, whereas painting with
    black leaves the destination color unchanged.

    \sa compositionMode(), setCompositionMode(), {QPainter#Composition
    Modes}{Composition Modes}, {Image Composition Example}
*/

/*!
    Sets the composition mode to the given \a mode.

    \warning Only a QPainter operating on a QImage fully supports all
    composition modes.

    \sa compositionMode()
*/
void QPainter::setCompositionMode(CompositionMode mode)
{
    Q_D(QPainter);
    if (Q_UNLIKELY(!d->engine)) {
        qWarning("QPainter::setCompositionMode: Painter not active");
        return;
    }
    if (d->state->composition_mode == mode)
        return;
    if (d->extended) {
        d->state->composition_mode = mode;
        d->extended->compositionModeChanged();
        return;
    }

    if (mode >= QPainter::CompositionMode_Plus) {
        if (Q_UNLIKELY(!d->engine->hasFeature(QPaintEngine::BlendModes))) {
            qWarning("QPainter::setCompositionMode: "
                     "Blend modes not supported on device");
            return;
        }
    } else if (!d->engine->hasFeature(QPaintEngine::PorterDuff)) {
        if (Q_UNLIKELY(mode != CompositionMode_Source && mode != CompositionMode_SourceOver)) {
            qWarning("QPainter::setCompositionMode: "
                     "PorterDuff modes not supported on device");
            return;
        }
    }

    d->state->composition_mode = mode;
    d->state->dirtyFlags |= QPaintEngine::DirtyCompositionMode;
}

/*!
  Returns the current composition mode.

  \sa CompositionMode, setCompositionMode()
*/
QPainter::CompositionMode QPainter::compositionMode() const
{
    Q_D(const QPainter);
    if (Q_UNLIKELY(!d->engine)) {
        qWarning("QPainter::compositionMode: Painter not active");
        return QPainter::CompositionMode_SourceOver;
    }
    return d->state->composition_mode;
}

/*!
    Returns the current background brush.

    \sa setBackground(), {QPainter#Settings}{Settings}
*/

const QBrush &QPainter::background() const
{
    Q_D(const QPainter);
    if (Q_UNLIKELY(!d->engine)) {
        qWarning("QPainter::background: Painter not active");
        return d->fakeState()->brush;
    }
    return d->state->bgBrush;
}


/*!
    Returns true if clipping has been set; otherwise returns false.

    \sa setClipping(), {QPainter#Clipping}{Clipping}
*/

bool QPainter::hasClipping() const
{
    Q_D(const QPainter);
    if (Q_UNLIKELY(!d->engine)) {
        qWarning("QPainter::hasClipping: Painter not active");
        return false;
    }
    return d->state->clipEnabled && d->state->clipOperation != Qt::NoClip;
}


/*!
    Enables clipping if  \a enable is true, or disables clipping if  \a
    enable is false.

    \sa hasClipping(), {QPainter#Clipping}{Clipping}
*/

void QPainter::setClipping(bool enable)
{
    Q_D(QPainter);
#ifdef QT_DEBUG_DRAW
    printf("QPainter::setClipping(), enable=%s, was=%s\n",
           enable ? "on" : "off",
           hasClipping() ? "on" : "off");
#endif
    if (Q_UNLIKELY(!d->engine)) {
        qWarning("QPainter::setClipping: Painter not active, state will be reset by begin");
        return;
    }

    if (hasClipping() == enable)
        return;

    // we can't enable clipping if we don't have a clip
    if (enable
        && (d->state->clipInfo.isEmpty() || d->state->clipInfo.last().operation == Qt::NoClip))
        return;
    d->state->clipEnabled = enable;

    if (d->extended) {
        d->extended->clipEnabledChanged();
        return;
    }

    d->state->dirtyFlags |= QPaintEngine::DirtyClipEnabled;
    d->updateState(d->state);
}


/*!
    Returns the currently set clip region. Note that the clip region
    is given in logical coordinates.

    \warning QPainter does not store the combined clip explicitly as
    this is handled by the underlying QPaintEngine, so the path is
    recreated on demand and transformed to the current logical
    coordinate system. This is potentially an expensive operation.

    \sa setClipRegion(), clipPath(), setClipping()
*/

QRegion QPainter::clipRegion() const
{
    Q_D(const QPainter);
    if (Q_UNLIKELY(!d->engine)) {
        qWarning("QPainter::clipRegion: Painter not active");
        return QRegion();
    }

    QRegion region;
    bool lastWasNothing = true;

    if (!d->txinv)
        const_cast<QPainter *>(this)->d_ptr->updateInvMatrix();

    // ### Falcon: Use QPainterPath
    for (int i=0; i<d->state->clipInfo.size(); ++i) {
        const QPainterClipInfo &info = d->state->clipInfo.at(i);
        switch (info.clipType) {

        case QPainterClipInfo::RegionClip: {
            QTransform matrix = (info.matrix * d->invMatrix);
            if (lastWasNothing) {
                region = info.region * matrix;
                lastWasNothing = false;
                continue;
            }
            if (info.operation == Qt::IntersectClip)
                region &= info.region * matrix;
            else if (info.operation == Qt::UniteClip)
                region |= info.region * matrix;
            else if (info.operation == Qt::NoClip) {
                lastWasNothing = true;
                region = QRegion();
            } else
                region = info.region * matrix;
            break;
        }

        case QPainterClipInfo::PathClip: {
            QTransform matrix = (info.matrix * d->invMatrix);
            if (lastWasNothing) {
                region = QRegion((info.path * matrix).toFillPolygon().toPolygon(),
                                 info.path.fillRule());
                lastWasNothing = false;
                continue;
            }
            if (info.operation == Qt::IntersectClip) {
                region &= QRegion((info.path * matrix).toFillPolygon().toPolygon(),
                                  info.path.fillRule());
            } else if (info.operation == Qt::UniteClip) {
                region |= QRegion((info.path * matrix).toFillPolygon().toPolygon(),
                                  info.path.fillRule());
            } else if (info.operation == Qt::NoClip) {
                lastWasNothing = true;
                region = QRegion();
            } else {
                region = QRegion((info.path * matrix).toFillPolygon().toPolygon(),
                                 info.path.fillRule());
            }
            break;
        }

        case QPainterClipInfo::RectClip: {
            QTransform matrix = (info.matrix * d->invMatrix);
            if (lastWasNothing) {
                region = QRegion(info.rect) * matrix;
                lastWasNothing = false;
                continue;
            }
            if (info.operation == Qt::IntersectClip) {
                // Use rect intersection if possible.
                if (matrix.type() <= QTransform::TxScale)
                    region &= matrix.mapRect(info.rect);
                else
                    region &= matrix.map(QRegion(info.rect));
            } else if (info.operation == Qt::UniteClip) {
                region |= QRegion(info.rect) * matrix;
            } else if (info.operation == Qt::NoClip) {
                lastWasNothing = true;
                region = QRegion();
            } else {
                region = QRegion(info.rect) * matrix;
            }
            break;
        }

        case QPainterClipInfo::RectFClip: {
            QTransform matrix = (info.matrix * d->invMatrix);
            if (lastWasNothing) {
                region = QRegion(info.rectf.toRect()) * matrix;
                lastWasNothing = false;
                continue;
            }
            if (info.operation == Qt::IntersectClip) {
                // Use rect intersection if possible.
                if (matrix.type() <= QTransform::TxScale)
                    region &= matrix.mapRect(info.rectf.toRect());
                else
                    region &= matrix.map(QRegion(info.rectf.toRect()));
            } else if (info.operation == Qt::UniteClip) {
                region |= QRegion(info.rectf.toRect()) * matrix;
            } else if (info.operation == Qt::NoClip) {
                lastWasNothing = true;
                region = QRegion();
            } else {
                region = QRegion(info.rectf.toRect()) * matrix;
            }
            break;
        }
        }
    }

    return region;
}

extern Q_AUTOTEST_EXPORT QPainterPath qt_regionToPath(const QRegion &region);

/*!
    Returns the currently clip as a path. Note that the clip path is
    given in logical coordinates.

    \warning QPainter does not store the combined clip explicitly as
    this is handled by the underlying QPaintEngine, so the path is
    recreated on demand and transformed to the current logical
    coordinate system. This is potentially an expensive operation.

    \sa setClipPath(), clipRegion(), setClipping()
*/
QPainterPath QPainter::clipPath() const
{
    Q_D(const QPainter);

    // ### Since we do not support path intersections and path unions yet,
    // we just use clipRegion() here...
    if (Q_UNLIKELY(!d->engine)) {
        qWarning("QPainter::clipPath: Painter not active");
        return QPainterPath();
    }

    // No clip, return empty
    if (d->state->clipInfo.size() == 0) {
        return QPainterPath();
    } else {

        // Update inverse matrix, used below.
        if (!d->txinv)
            const_cast<QPainter *>(this)->d_ptr->updateInvMatrix();

        // For the simple case avoid conversion.
        if (d->state->clipInfo.size() == 1
            && d->state->clipInfo.at(0).clipType == QPainterClipInfo::PathClip) {
            QTransform matrix = (d->state->clipInfo.at(0).matrix * d->invMatrix);
            return d->state->clipInfo.at(0).path * matrix;

        } else if (d->state->clipInfo.size() == 1
                   && d->state->clipInfo.at(0).clipType == QPainterClipInfo::RectClip) {
            QTransform matrix = (d->state->clipInfo.at(0).matrix * d->invMatrix);
            QPainterPath path;
            path.addRect(d->state->clipInfo.at(0).rect);
            return path * matrix;
        } else {
            // Fallback to clipRegion() for now, since we don't have isect/unite for paths
            return qt_regionToPath(clipRegion());
        }
    }
}

/*!
    Returns the bounding rectangle of the current clip if there is a clip;
    otherwise returns an empty rectangle. Note that the clip region is
    given in logical coordinates.

    The bounding rectangle is not guaranteed to be tight.

    \sa setClipRect(), setClipPath(), setClipRegion()

    \since 4.8
 */

QRectF QPainter::clipBoundingRect() const
{
    Q_D(const QPainter);

    if (Q_UNLIKELY(!d->engine)) {
        qWarning("QPainter::clipBoundingRect: Painter not active");
        return QRectF();
    }

    // Accumulate the bounding box in device space. This is not 100%
    // precise, but it fits within the guarantee and it is reasonably
    // fast.
    QRectF bounds;
    for (int i=0; i<d->state->clipInfo.size(); ++i) {
         QRectF r;
         const QPainterClipInfo &info = d->state->clipInfo.at(i);

         if (info.clipType == QPainterClipInfo::RectClip)
             r = info.rect;
         else if (info.clipType == QPainterClipInfo::RectFClip)
             r = info.rectf;
         else if (info.clipType == QPainterClipInfo::RegionClip)
             r = info.region.boundingRect();
         else
             r = info.path.boundingRect();

         r = info.matrix.mapRect(r);

         if (i == 0)
             bounds = r;
         else if (info.operation == Qt::IntersectClip)
             bounds &= r;
         else if (info.operation == Qt::UniteClip)
             bounds |= r;
    }


    // Map the rectangle back into logical space using the inverse
    // matrix.
    if (!d->txinv)
        const_cast<QPainter *>(this)->d_ptr->updateInvMatrix();

    return d->invMatrix.mapRect(bounds);
}

/*!
    \fn void QPainter::setClipRect(const QRectF &rectangle, Qt::ClipOperation operation)

    Enables clipping, and sets the clip region to the given \a
    rectangle using the given clip \a operation. The default operation
    is to replace the current clip rectangle.

    Note that the clip rectangle is specified in logical (painter)
    coordinates.

    \sa clipRegion(), setClipping(), {QPainter#Clipping}{Clipping}
*/
void QPainter::setClipRect(const QRectF &rect, Qt::ClipOperation op)
{
    Q_D(QPainter);

    if (d->extended) {
        if ((!d->state->clipEnabled && op != Qt::NoClip) || (d->state->clipOperation == Qt::NoClip && op == Qt::UniteClip))
            op = Qt::ReplaceClip;

        if (Q_UNLIKELY(!d->engine)) {
            qWarning("QPainter::setClipRect: Painter not active");
            return;
        }
        qreal right = rect.x() + rect.width();
        qreal bottom = rect.y() + rect.height();
        qreal pts[] = { rect.x(), rect.y(),
                        right, rect.y(),
                        right, bottom,
                        rect.x(), bottom };
        QVectorPath vp(pts, 4, 0, QVectorPath::RectangleHint);
        d->state->clipEnabled = true;
        d->extended->clip(vp, op);
        if (op == Qt::ReplaceClip || op == Qt::NoClip)
            d->state->clipInfo.clear();
        d->state->clipInfo << QPainterClipInfo(rect, op, d->state->matrix);
        d->state->clipOperation = op;
        return;
    }

    if (qreal(int(rect.top())) == rect.top()
        && qreal(int(rect.bottom())) == rect.bottom()
        && qreal(int(rect.left())) == rect.left()
        && qreal(int(rect.right())) == rect.right())
    {
        setClipRect(rect.toRect(), op);
        return;
    }

    if (rect.isEmpty()) {
        setClipRegion(QRegion(), op);
        return;
    }

    QPainterPath path;
    path.addRect(rect);
    setClipPath(path, op);
}

/*!
    \fn void QPainter::setClipRect(const QRect &rectangle, Qt::ClipOperation operation)
    \overload

    Enables clipping, and sets the clip region to the given \a rectangle using the given
    clip \a operation.
*/
void QPainter::setClipRect(const QRect &rect, Qt::ClipOperation op)
{
    Q_D(QPainter);

    if (Q_UNLIKELY(!d->engine)) {
        qWarning("QPainter::setClipRect: Painter not active");
        return;
    }

    if ((!d->state->clipEnabled && op != Qt::NoClip) || (d->state->clipOperation == Qt::NoClip && op == Qt::UniteClip))
        op = Qt::ReplaceClip;

    if (d->extended) {
        d->state->clipEnabled = true;
        d->extended->clip(rect, op);
        if (op == Qt::ReplaceClip || op == Qt::NoClip)
            d->state->clipInfo.clear();
        d->state->clipInfo << QPainterClipInfo(rect, op, d->state->matrix);
        d->state->clipOperation = op;
        return;
    }

    if (d->state->clipOperation == Qt::NoClip && op == Qt::IntersectClip)
        op = Qt::ReplaceClip;

    d->state->clipRegion = rect;
    d->state->clipOperation = op;
    if (op == Qt::NoClip || op == Qt::ReplaceClip)
        d->state->clipInfo.clear();
    d->state->clipInfo << QPainterClipInfo(rect, op, d->state->matrix);
    d->state->clipEnabled = true;
    d->state->dirtyFlags |= QPaintEngine::DirtyClipRegion | QPaintEngine::DirtyClipEnabled;
    d->updateState(d->state);
}

/*!
    \fn void QPainter::setClipRect(int x, int y, int width, int height, Qt::ClipOperation operation)

    Enables clipping, and sets the clip region to the rectangle beginning at (\a x, \a y)
    with the given \a width and \a height.
*/

/*!
    \fn void QPainter::setClipRegion(const QRegion &region, Qt::ClipOperation operation)

    Sets the clip region to the given \a region using the specified clip
    \a operation. The default clip operation is to replace the current
    clip region.

    Note that the clip region is given in logical coordinates.

    \sa clipRegion(), setClipRect(), {QPainter#Clipping}{Clipping}
*/
void QPainter::setClipRegion(const QRegion &r, Qt::ClipOperation op)
{
    Q_D(QPainter);
#ifdef QT_DEBUG_DRAW
    QRect rect = r.boundingRect();
    printf("QPainter::setClipRegion(), size=%d, [%d,%d,%d,%d]\n",
           r.rects().size(), rect.x(), rect.y(), rect.width(), rect.height());
#endif
    if (Q_UNLIKELY(!d->engine)) {
        qWarning("QPainter::setClipRegion: Painter not active");
        return;
    }

    if ((!d->state->clipEnabled && op != Qt::NoClip) || (d->state->clipOperation == Qt::NoClip && op == Qt::UniteClip))
        op = Qt::ReplaceClip;

    if (d->extended) {
        d->state->clipEnabled = true;
        d->extended->clip(r, op);
        if (op == Qt::NoClip || op == Qt::ReplaceClip)
            d->state->clipInfo.clear();
        d->state->clipInfo << QPainterClipInfo(r, op, d->state->matrix);
        d->state->clipOperation = op;
        return;
    }

    if (d->state->clipOperation == Qt::NoClip && op == Qt::IntersectClip)
        op = Qt::ReplaceClip;

    d->state->clipRegion = r;
    d->state->clipOperation = op;
    if (op == Qt::NoClip || op == Qt::ReplaceClip)
        d->state->clipInfo.clear();
    d->state->clipInfo << QPainterClipInfo(r, op, d->state->matrix);
    d->state->clipEnabled = true;
    d->state->dirtyFlags |= QPaintEngine::DirtyClipRegion | QPaintEngine::DirtyClipEnabled;
    d->updateState(d->state);
}

/*!
    \since 4.2
    \obsolete

    Sets the transformation matrix to \a matrix and enables transformations.

    \note It is advisable to use setWorldTransform() instead of this function to
    preserve the properties of perspective transformations.

    If \a combine is true, then \a matrix is combined with the current
    transformation matrix; otherwise \a matrix replaces the current
    transformation matrix.

    If \a matrix is the identity matrix and \a combine is false, this
    function calls setWorldMatrixEnabled(false). (The identity matrix is the
    matrix where QMatrix::m11() and QMatrix::m22() are 1.0 and the
    rest are 0.0.)

    The following functions can transform the coordinate system without using
    a QMatrix:
    \list
    \i translate()
    \i scale()
    \i shear()
    \i rotate()
    \endlist

    They operate on the painter's worldMatrix() and are implemented like this:

    \snippet doc/src/snippets/code/src_gui_painting_qpainter.cpp 4

    For more information about the coordinate system, transformations
    and window-viewport conversion, see \l {Coordinate System}.

    \sa setWorldTransform(), QTransform
*/

void QPainter::setWorldMatrix(const QMatrix &matrix, bool combine)
{
    setWorldTransform(QTransform(matrix), combine);
}

/*!
    \since 4.2
    \obsolete

    Returns the world transformation matrix.

    It is advisable to use worldTransform() because worldMatrix() does not
    preserve the properties of perspective transformations.

    \sa {QPainter#Coordinate Transformations}{Coordinate Transformations},
    {Coordinate System}
*/

const QMatrix &QPainter::worldMatrix() const
{
    Q_D(const QPainter);
    if (Q_UNLIKELY(!d->engine)) {
        qWarning("QPainter::worldMatrix: Painter not active");
        return d->fakeState()->transform.toAffine();
    }
    return d->state->worldMatrix.toAffine();
}

/*!
    \obsolete

    Use setWorldTransform() instead.

    \sa setWorldTransform()
*/

void QPainter::setMatrix(const QMatrix &matrix, bool combine)
{
    setWorldTransform(QTransform(matrix), combine);
}

/*!
    \obsolete

    Use worldTransform() instead.

    \sa worldTransform()
*/

const QMatrix &QPainter::matrix() const
{
    return worldMatrix();
}


/*!
    \since 4.2
    \obsolete

    Returns the transformation matrix combining the current
    window/viewport and world transformation.

    It is advisable to use combinedTransform() instead of this
    function to preserve the properties of perspective transformations.

    \sa setWorldTransform(), setWindow(), setViewport()
*/
QMatrix QPainter::combinedMatrix() const
{
    return combinedTransform().toAffine();
}


/*!
    \obsolete

    Returns the matrix that transforms from logical coordinates to
    device coordinates of the platform dependent paint device.

    \note It is advisable to use deviceTransform() instead of this
    function to preserve the properties of perspective transformations.

    This function is \e only needed when using platform painting
    commands on the platform dependent handle (Qt::HANDLE), and the
    platform does not do transformations nativly.

    The QPaintEngine::PaintEngineFeature enum can be queried to
    determine whether the platform performs the transformations or
    not.

    \sa worldMatrix(), QPaintEngine::hasFeature(),
*/
const QMatrix &QPainter::deviceMatrix() const
{
    Q_D(const QPainter);
    if (Q_UNLIKELY(!d->engine)) {
        qWarning("QPainter::deviceMatrix: Painter not active");
        return d->fakeState()->transform.toAffine();
    }
    return d->state->matrix.toAffine();
}

/*!
    \obsolete

    Resets any transformations that were made using translate(), scale(),
    shear(), rotate(), setWorldMatrix(), setViewport() and
    setWindow().

    It is advisable to use resetTransform() instead of this function
    to preserve the properties of perspective transformations.

    \sa {QPainter#Coordinate Transformations}{Coordinate
    Transformations}
*/

void QPainter::resetMatrix()
{
    resetTransform();
}


/*!
    \since 4.2

    Enables transformations if \a enable is true, or disables
    transformations if \a enable is false. The world transformation
    matrix is not changed.

    \sa worldMatrixEnabled(), worldTransform(), {QPainter#Coordinate
    Transformations}{Coordinate Transformations}
*/

void QPainter::setWorldMatrixEnabled(bool enable)
{
    Q_D(QPainter);
#ifdef QT_DEBUG_DRAW
    printf("QPainter::setMatrixEnabled(), enable=%d\n", enable);
#endif

    if (Q_UNLIKELY(!d->engine)) {
        qWarning("QPainter::setMatrixEnabled: Painter not active");
        return;
    }
    if (enable == d->state->WxF)
        return;

    d->state->WxF = enable;
    d->updateMatrix();
}

/*!
    \since 4.2

    Returns true if world transformation is enabled; otherwise returns
    false.

    \sa setWorldMatrixEnabled(), worldTransform(), {Coordinate System}
*/

bool QPainter::worldMatrixEnabled() const
{
    Q_D(const QPainter);
    if (Q_UNLIKELY(!d->engine)) {
        qWarning("QPainter::worldMatrixEnabled: Painter not active");
        return false;
    }
    return d->state->WxF;
}

/*!
    \obsolete

    Use setWorldMatrixEnabled() instead.

    \sa setWorldMatrixEnabled()
*/

void QPainter::setMatrixEnabled(bool enable)
{
    setWorldMatrixEnabled(enable);
}

/*!
    \obsolete

    Use worldMatrixEnabled() instead

    \sa worldMatrixEnabled()
*/

bool QPainter::matrixEnabled() const
{
    return worldMatrixEnabled();
}

/*!
    Scales the coordinate system by (\a{sx}, \a{sy}).

    \sa setWorldTransform() {QPainter#Coordinate Transformations}{Coordinate
    Transformations}
*/

void QPainter::scale(qreal sx, qreal sy)
{
#ifdef QT_DEBUG_DRAW
    printf("QPainter::scale(), sx=%f, sy=%f\n", sx, sy);
#endif
    Q_D(QPainter);
    if (Q_UNLIKELY(!d->engine)) {
        qWarning("QPainter::scale: Painter not active");
        return;
    }

    d->state->worldMatrix.scale(sx,sy);
    d->state->WxF = true;
    d->updateMatrix();
}

/*!
    Shears the coordinate system by (\a{sh}, \a{sv}).

    \sa setWorldTransform(), {QPainter#Coordinate Transformations}{Coordinate
    Transformations}
*/

void QPainter::shear(qreal sh, qreal sv)
{
#ifdef QT_DEBUG_DRAW
    printf("QPainter::shear(), sh=%f, sv=%f\n", sh, sv);
#endif
    Q_D(QPainter);
    if (Q_UNLIKELY(!d->engine)) {
        qWarning("QPainter::shear: Painter not active");
        return;
    }

    d->state->worldMatrix.shear(sh, sv);
    d->state->WxF = true;
    d->updateMatrix();
}

/*!
    \fn void QPainter::rotate(qreal angle)

    Rotates the coordinate system the given \a angle clockwise.

    \sa setWorldTransform(), {QPainter#Coordinate Transformations}{Coordinate
    Transformations}
*/

void QPainter::rotate(qreal a)
{
#ifdef QT_DEBUG_DRAW
    printf("QPainter::rotate(), angle=%f\n", a);
#endif
    Q_D(QPainter);
    if (Q_UNLIKELY(!d->engine)) {
        qWarning("QPainter::rotate: Painter not active");
        return;
    }

    d->state->worldMatrix.rotate(a);
    d->state->WxF = true;
    d->updateMatrix();
}

/*!
    Translates the coordinate system by the given \a offset; i.e. the
    given \a offset is added to points.

    \sa setWorldTransform(), {QPainter#Coordinate Transformations}{Coordinate
    Transformations}
*/
void QPainter::translate(const QPointF &offset)
{
    qreal dx = offset.x();
    qreal dy = offset.y();
#ifdef QT_DEBUG_DRAW
    printf("QPainter::translate(), dx=%f, dy=%f\n", dx, dy);
#endif
    Q_D(QPainter);
    if (Q_UNLIKELY(!d->engine)) {
        qWarning("QPainter::translate: Painter not active");
        return;
    }

    d->state->worldMatrix.translate(dx, dy);
    d->state->WxF = true;
    d->updateMatrix();
}

/*!
    \fn void QPainter::translate(const QPoint &offset)
    \overload

    Translates the coordinate system by the given \a offset.
*/

/*!
    \fn void QPainter::translate(qreal dx, qreal dy)
    \overload

    Translates the coordinate system by the vector (\a dx, \a dy).
*/

/*!
    \fn void QPainter::setClipPath(const QPainterPath &path, Qt::ClipOperation operation)

    Enables clipping, and sets the clip path for the painter to the
    given \a path, with the clip \a operation.

    Note that the clip path is specified in logical (painter)
    coordinates.

    \sa clipPath(), clipRegion(), {QPainter#Clipping}{Clipping}

*/
void QPainter::setClipPath(const QPainterPath &path, Qt::ClipOperation op)
{
#ifdef QT_DEBUG_DRAW
    QRectF b = path.boundingRect();
    printf("QPainter::setClipPath(), size=%d, op=%d, bounds=[%.2f,%.2f,%.2f,%.2f]\n",
           path.elementCount(), op, b.x(), b.y(), b.width(), b.height());
#endif
    Q_D(QPainter);

    if (Q_UNLIKELY(!d->engine)) {
        qWarning("QPainter::setClipPath: Painter not active");
        return;
    }

    if ((!d->state->clipEnabled && op != Qt::NoClip) || (d->state->clipOperation == Qt::NoClip && op == Qt::UniteClip))
        op = Qt::ReplaceClip;

    if (d->extended) {
        d->state->clipEnabled = true;
        d->extended->clip(path, op);
        if (op == Qt::NoClip || op == Qt::ReplaceClip)
            d->state->clipInfo.clear();
        d->state->clipInfo << QPainterClipInfo(path, op, d->state->matrix);
        d->state->clipOperation = op;
        return;
    }

    if (d->state->clipOperation == Qt::NoClip && op == Qt::IntersectClip)
        op = Qt::ReplaceClip;

    d->state->clipPath = path;
    d->state->clipOperation = op;
    if (op == Qt::NoClip || op == Qt::ReplaceClip)
        d->state->clipInfo.clear();
    d->state->clipInfo << QPainterClipInfo(path, op, d->state->matrix);
    d->state->clipEnabled = true;
    d->state->dirtyFlags |= QPaintEngine::DirtyClipPath | QPaintEngine::DirtyClipEnabled;
    d->updateState(d->state);
}

/*!
    Draws the outline (strokes) the path \a path with the pen specified
    by \a pen

    \sa fillPath(), {QPainter#Drawing}{Drawing}
*/
void QPainter::strokePath(const QPainterPath &path, const QPen &pen)
{
    Q_D(QPainter);

    if (Q_UNLIKELY(!d->engine)) {
        qWarning("QPainter::strokePath: Painter not active");
        return;
    }

    if (path.isEmpty())
        return;

    if (d->extended) {
        const QGradient *g = pen.brush().gradient();
        if (!g || g->coordinateMode() == QGradient::LogicalMode) {
            d->extended->stroke(qtVectorPathForPath(path), pen);
            return;
        }
    }

    QBrush oldBrush = d->state->brush;
    QPen oldPen = d->state->pen;

    setPen(pen);
    setBrush(Qt::NoBrush);

    drawPath(path);

    // Reset old state
    setPen(oldPen);
    setBrush(oldBrush);
}

/*!
    Fills the given \a path using the given \a brush. The outline is
    not drawn.

    Alternatively, you can specify a QColor instead of a QBrush; the
    QBrush constructor (taking a QColor argument) will automatically
    create a solid pattern brush.

    \sa drawPath()
*/
void QPainter::fillPath(const QPainterPath &path, const QBrush &brush)
{
    Q_D(QPainter);

    if (Q_UNLIKELY(!d->engine)) {
        qWarning("QPainter::fillPath: Painter not active");
        return;
    }

    if (path.isEmpty())
        return;

    if (d->extended) {
        const QGradient *g = brush.gradient();
        if (!g || g->coordinateMode() == QGradient::LogicalMode) {
            d->extended->fill(qtVectorPathForPath(path), brush);
            return;
        }
    }

    QBrush oldBrush = d->state->brush;
    QPen oldPen = d->state->pen;

    setPen(Qt::NoPen);
    setBrush(brush);

    drawPath(path);

    // Reset old state
    setPen(oldPen);
    setBrush(oldBrush);
}

/*!
    Draws the given painter \a path using the current pen for outline
    and the current brush for filling.

    \table 100%
    \row
    \o \inlineimage qpainter-path.png
    \o
    \snippet doc/src/snippets/code/src_gui_painting_qpainter.cpp 5
    \endtable

    \sa {painting/painterpaths}{the Painter Paths
    example},{demos/deform}{the Vector Deformation demo}
*/
void QPainter::drawPath(const QPainterPath &path)
{
#ifdef QT_DEBUG_DRAW
    QRectF pathBounds = path.boundingRect();
    printf("QPainter::drawPath(), size=%d, [%.2f,%.2f,%.2f,%.2f]\n",
           path.elementCount(),
           pathBounds.x(), pathBounds.y(), pathBounds.width(), pathBounds.height());
#endif

    Q_D(QPainter);

    if (Q_UNLIKELY(!d->engine)) {
        qWarning("QPainter::drawPath: Painter not active");
        return;
    }

    if (d->extended) {
        d->extended->drawPath(path);
        return;
    }
    d->updateState(d->state);

    if (d->engine->hasFeature(QPaintEngine::PainterPaths)) {
        d->engine->drawPath(path);
    } else {
        d->draw_helper(path);
    }
}

/*!
    \fn void QPainter::drawLine(const QLineF &line)

    Draws a line defined by \a line.

    \table 100%
    \row
    \o \inlineimage qpainter-line.png
    \o
    \snippet doc/src/snippets/code/src_gui_painting_qpainter.cpp 6
    \endtable

    \sa drawLines(), drawPolyline(), {Coordinate System}
*/

/*!
    \fn void QPainter::drawLine(const QLine &line)
    \overload

    Draws a line defined by \a line.
*/

/*!
    \fn void QPainter::drawLine(const QPoint &p1, const QPoint &p2)
    \overload

    Draws a line from \a p1 to \a p2.
*/

/*!
    \fn void QPainter::drawLine(const QPointF &p1, const QPointF &p2)
    \overload

    Draws a line from \a p1 to \a p2.
*/

/*!
    \fn void QPainter::drawLine(int x1, int y1, int x2, int y2)
    \overload

    Draws a line from (\a x1, \a y1) to (\a x2, \a y2) and sets the
    current pen position to (\a x2, \a y2).
*/

/*!
    \fn void QPainter::drawRect(const QRectF &rectangle)

    Draws the current \a rectangle with the current pen and brush.

    A filled rectangle has a size of \a{rectangle}.size(). A stroked
    rectangle has a size of \a{rectangle}.size() plus the pen width.

    \table 100%
    \row
    \o \inlineimage qpainter-rectangle.png
    \o
    \snippet doc/src/snippets/code/src_gui_painting_qpainter.cpp 7
    \endtable

    \sa drawRects(), drawPolygon(), {Coordinate System}
*/

/*!
    \fn void QPainter::drawRect(const QRect &rectangle)

    \overload

    Draws the current \a rectangle with the current pen and brush.
*/

/*!
    \fn void QPainter::drawRect(int x, int y, int width, int height)

    \overload

    Draws a rectangle with upper left corner at (\a{x}, \a{y}) and
    with the given \a width and \a height.
*/

/*!
    \fn void QPainter::drawRects(const QRectF *rectangles, int rectCount)

    Draws the first \a rectCount of the given \a rectangles using the
    current pen and brush.

    \sa drawRect()
*/
void QPainter::drawRects(const QRectF *rects, int rectCount)
{
#ifdef QT_DEBUG_DRAW
    printf("QPainter::drawRects(), count=%d\n", rectCount);
#endif
    Q_D(QPainter);

    if (Q_UNLIKELY(!d->engine)) {
        qWarning("QPainter::drawRects: Painter not active");
        return;
    }

    if (rectCount <= 0)
        return;

    if (d->extended) {
        d->extended->drawRects(rects, rectCount);
        return;
    }

    d->updateState(d->state);

    d->engine->drawRects(rects, rectCount);
}

/*!
    \fn void QPainter::drawRects(const QRect *rectangles, int rectCount)
    \overload

    Draws the first \a rectCount of the given \a rectangles using the
    current pen and brush.
*/
void QPainter::drawRects(const QRect *rects, int rectCount)
{
#ifdef QT_DEBUG_DRAW
    printf("QPainter::drawRects(), count=%d\n", rectCount);
#endif
    Q_D(QPainter);

    if (Q_UNLIKELY(!d->engine)) {
        qWarning("QPainter::drawRects: Painter not active");
        return;
    }

    if (rectCount <= 0)
        return;

    if (d->extended) {
        d->extended->drawRects(rects, rectCount);
        return;
    }

    d->updateState(d->state);

    d->engine->drawRects(rects, rectCount);
}

/*!
    \fn void QPainter::drawRects(const QVector<QRectF> &rectangles)
    \overload

    Draws the given \a rectangles using the current pen and brush.
*/

/*!
    \fn void QPainter::drawRects(const QVector<QRect> &rectangles)

    \overload

    Draws the given \a rectangles using the current pen and brush.
*/

/*!
  \fn void QPainter::drawPoint(const QPointF &position)

    Draws a single point at the given \a position using the current
    pen's color.

    \sa {Coordinate System}
*/

/*!
    \fn void QPainter::drawPoint(const QPoint &position)
    \overload

    Draws a single point at the given \a position using the current
    pen's color.
*/

/*! \fn void QPainter::drawPoint(int x, int y)

    \overload

    Draws a single point at position (\a x, \a y).
*/

/*!
    Draws the first \a pointCount points in the array \a points using
    the current pen's color.

    \sa {Coordinate System}
*/
void QPainter::drawPoints(const QPointF *points, int pointCount)
{
#ifdef QT_DEBUG_DRAW
    printf("QPainter::drawPoints(), count=%d\n", pointCount);
#endif
    Q_D(QPainter);

    if (Q_UNLIKELY(!d->engine)) {
        qWarning("QPainter::drawPoints: Painter not active");
        return;
    }

    if (pointCount <= 0)
        return;

    if (d->extended) {
        d->extended->drawPoints(points, pointCount);
        return;
    }

    d->updateState(d->state);

    d->engine->drawPoints(points, pointCount);
}

/*!
    \overload

    Draws the first \a pointCount points in the array \a points using
    the current pen's color.
*/

void QPainter::drawPoints(const QPoint *points, int pointCount)
{
#ifdef QT_DEBUG_DRAW
    printf("QPainter::drawPoints(), count=%d\n", pointCount);
#endif
    Q_D(QPainter);

    if (Q_UNLIKELY(!d->engine)) {
        qWarning("QPainter::drawPoints: Painter not active");
        return;
    }

    if (pointCount <= 0)
        return;

    if (d->extended) {
        d->extended->drawPoints(points, pointCount);
        return;
    }

    d->updateState(d->state);

    d->engine->drawPoints(points, pointCount);
}

/*!
    \fn void QPainter::drawPoints(const QPolygonF &points)

    \overload

    Draws the points in the vector  \a points.
*/

/*!
    \fn void QPainter::drawPoints(const QPolygon &points)

    \overload

    Draws the points in the vector  \a points.
*/

/*!
    \fn void QPainter::drawPoints(const QPolygon &polygon, int index,
    int count)

    \overload
    \compat

    Draws \a count points in the vector \a polygon starting on \a index
    using the current pen.

    Use drawPoints() combined with QPolygon::constData() instead.

    \oldcode
        QPainter painter(this);
        painter.drawPoints(polygon, index, count);
    \newcode
        int pointCount = (count == -1) ?  polygon.size() - index : count;

        QPainter painter(this);
        painter.drawPoints(polygon.constData() + index, pointCount);
    \endcode
*/

/*!
    Sets the background mode of the painter to the given \a mode

    Qt::TransparentMode (the default) draws stippled lines and text
    without setting the background pixels.  Qt::OpaqueMode fills these
    space with the current background color.

    Note that in order to draw a bitmap or pixmap transparently, you
    must use QPixmap::setMask().

    \sa backgroundMode(), setBackground(),
    {QPainter#Settings}{Settings}
*/

void QPainter::setBackgroundMode(Qt::BGMode mode)
{
#ifdef QT_DEBUG_DRAW
    printf("QPainter::setBackgroundMode(), mode=%d\n", mode);
#endif

    Q_D(QPainter);
    if (Q_UNLIKELY(!d->engine)) {
        qWarning("QPainter::setBackgroundMode: Painter not active");
        return;
    }
    if (d->state->bgMode == mode)
        return;

    d->state->bgMode = mode;
    if (!d->extended) {
        d->state->dirtyFlags |= QPaintEngine::DirtyBackgroundMode;
    }
}

/*!
    Returns the current background mode.

    \sa setBackgroundMode(), {QPainter#Settings}{Settings}
*/
Qt::BGMode QPainter::backgroundMode() const
{
    Q_D(const QPainter);
    if (Q_UNLIKELY(!d->engine)) {
        qWarning("QPainter::backgroundMode: Painter not active");
        return Qt::TransparentMode;
    }
    return d->state->bgMode;
}


/*!
    \overload

    Sets the painter's pen to have style Qt::SolidLine, width 0 and the
    specified \a color.
*/

void QPainter::setPen(const QColor &color)
{
#ifdef QT_DEBUG_DRAW
    printf("QPainter::setPen(), color=%04x\n", color.rgb());
#endif
    Q_D(QPainter);
    if (Q_UNLIKELY(!d->engine)) {
        qWarning("QPainter::setPen: Painter not active");
        return;
    }

    if (d->state->pen.style() == Qt::SolidLine
        && d->state->pen.widthF() == 0
        && d->state->pen.isSolid()
        && d->state->pen.color() == color)
        return;

    QPen pen(color.isValid() ? color : QColor(Qt::black), 0, Qt::SolidLine);

    d->state->pen = pen;
    if (d->extended)
        d->extended->penChanged();
    else
        d->state->dirtyFlags |= QPaintEngine::DirtyPen;
}

/*!
    Sets the painter's pen to be the given \a pen.

    The \a pen defines how to draw lines and outlines, and it also
    defines the text color.

    \sa pen(), {QPainter#Settings}{Settings}
*/

void QPainter::setPen(const QPen &pen)
{

#ifdef QT_DEBUG_DRAW
    printf("QPainter::setPen(), color=%04x, (brushStyle=%d) style=%d, cap=%d, join=%d\n",
           pen.color().rgb(), pen.brush().style(), pen.style(), pen.capStyle(), pen.joinStyle());
#endif
    Q_D(QPainter);
    if (Q_UNLIKELY(!d->engine)) {
        qWarning("QPainter::setPen: Painter not active");
        return;
    }

    if (d->state->pen == pen)
        return;

    d->state->pen = pen;

    if (d->extended) {
        d->extended->penChanged();
        return;
    }

    d->state->dirtyFlags |= QPaintEngine::DirtyPen;
}

/*!
    \overload

    Sets the painter's pen to have the given \a style, width 0 and
    black color.
*/

void QPainter::setPen(Qt::PenStyle style)
{
    Q_D(QPainter);
    if (Q_UNLIKELY(!d->engine)) {
        qWarning("QPainter::setPen: Painter not active");
        return;
    }

    if (d->state->pen.style() == style
        && (style == Qt::NoPen || (d->state->pen.widthF() == 0
                                   && d->state->pen.isSolid()
                                   && d->state->pen.color() == QColor(Qt::black))))
        return;

    // QPen(Qt::NoPen) is to avoid creating QPenPrivate, including its brush (from the color)
    // Note that this works well as long as QPen(Qt::NoPen) returns a black, zero-width pen
    d->state->pen = (style == Qt::NoPen) ? QPen(Qt::NoPen) : QPen(Qt::black, 0, style);

    if (d->extended)
        d->extended->penChanged();
    else
        d->state->dirtyFlags |= QPaintEngine::DirtyPen;

}

/*!
    Returns the painter's current pen.

    \sa setPen(), {QPainter#Settings}{Settings}
*/

const QPen &QPainter::pen() const
{
    Q_D(const QPainter);
    if (Q_UNLIKELY(!d->engine)) {
        qWarning("QPainter::pen: Painter not active");
        return d->fakeState()->pen;
    }
    return d->state->pen;
}


/*!
    Sets the painter's brush to the given \a brush.

    The painter's brush defines how shapes are filled.

    \sa brush(), {QPainter#Settings}{Settings}
*/

void QPainter::setBrush(const QBrush &brush)
{
#ifdef QT_DEBUG_DRAW
    printf("QPainter::setBrush(), color=%04x, style=%d\n", brush.color().rgb(), brush.style());
#endif
    Q_D(QPainter);
    if (Q_UNLIKELY(!d->engine)) {
        qWarning("QPainter::setBrush: Painter not active");
        return;
    }

    if (d->state->brush == brush)
        return;

    if (d->extended) {
        d->state->brush = brush;
        d->extended->brushChanged();
        return;
    }

    d->state->brush = brush;
    d->state->dirtyFlags |= QPaintEngine::DirtyBrush;
}


/*!
    \overload

    Sets the painter's brush to black color and the specified \a
    style.
*/

void QPainter::setBrush(Qt::BrushStyle style)
{
    Q_D(QPainter);
    if (Q_UNLIKELY(!d->engine)) {
        qWarning("QPainter::setBrush: Painter not active");
        return;
    }
    if (d->state->brush.style() == style &&
        (style == Qt::NoBrush
         || (style == Qt::SolidPattern && d->state->brush.color() == QColor(0, 0, 0))))
        return;
    d->state->brush = QBrush(Qt::black, style);
    if (d->extended)
        d->extended->brushChanged();
    else
        d->state->dirtyFlags |= QPaintEngine::DirtyBrush;
}

/*!
    Returns the painter's current brush.

    \sa QPainter::setBrush(), {QPainter#Settings}{Settings}
*/

const QBrush &QPainter::brush() const
{
    Q_D(const QPainter);
    if (Q_UNLIKELY(!d->engine)) {
        qWarning("QPainter::brush: Painter not active");
        return d->fakeState()->brush;
    }
    return d->state->brush;
}

/*!
    \fn void QPainter::setBackground(const QBrush &brush)

    Sets the background brush of the painter to the given \a brush.

    The background brush is the brush that is filled in when drawing
    opaque text, stippled lines and bitmaps. The background brush has
    no effect in transparent background mode (which is the default).

    \sa background(), setBackgroundMode(),
    {QPainter#Settings}{Settings}
*/

void QPainter::setBackground(const QBrush &bg)
{
#ifdef QT_DEBUG_DRAW
    printf("QPainter::setBackground(), color=%04x, style=%d\n", bg.color().rgb(), bg.style());
#endif

    Q_D(QPainter);
    if (Q_UNLIKELY(!d->engine)) {
        qWarning("QPainter::setBackground: Painter not active");
        return;
    }
    d->state->bgBrush = bg;
    if (!d->extended)
        d->state->dirtyFlags |= QPaintEngine::DirtyBackground;
}

/*!
    Sets the painter's font to the given \a font.

    This font is used by subsequent drawText() functions. The text
    color is the same as the pen color.

    If you set a font that isn't available, Qt finds a close match.
    font() will return what you set using setFont() and fontInfo() returns the
    font actually being used (which may be the same).

    \sa font(), drawText(), {QPainter#Settings}{Settings}
*/

void QPainter::setFont(const QFont &font)
{
    Q_D(QPainter);

#ifdef QT_DEBUG_DRAW
    printf("QPainter::setFont(), family=%s, pointSize=%d\n", font.family().toLatin1().constData(), font.pointSize());
#endif

    if (Q_UNLIKELY(!d->engine)) {
        qWarning("QPainter::setFont: Painter not active");
        return;
    }

    d->state->font = QFont(font.resolve(d->state->deviceFont), device());
    if (!d->extended)
        d->state->dirtyFlags |= QPaintEngine::DirtyFont;
}

/*!
    Returns the currently set font used for drawing text.

    \sa setFont(), drawText(), {QPainter#Settings}{Settings}
*/
const QFont &QPainter::font() const
{
    Q_D(const QPainter);
    if (Q_UNLIKELY(!d->engine)) {
        qWarning("QPainter::font: Painter not active");
        return d->fakeState()->font;
    }
    return d->state->font;
}

/*!
    \since 4.4

    Draws the given rectangle \a rect with rounded corners.

    The \a xRadius and \a yRadius arguments specify the radii
    of the ellipses defining the corners of the rounded rectangle.
    When \a mode is Qt::RelativeSize, \a xRadius and
    \a yRadius are specified in percentage of half the rectangle's
    width and height respectively, and should be in the range
    0.0 to 100.0.

    A filled rectangle has a size of rect.size(). A stroked rectangle
    has a size of rect.size() plus the pen width.

    \table 100%
    \row
    \o \inlineimage qpainter-roundrect.png
    \o
    \snippet doc/src/snippets/code/src_gui_painting_qpainter.cpp 8
    \endtable

    \sa drawRect(), QPen
*/
void QPainter::drawRoundedRect(const QRectF &rect, qreal xRadius, qreal yRadius, Qt::SizeMode mode)
{
#ifdef QT_DEBUG_DRAW
    printf("QPainter::drawRoundedRect(), [%.2f,%.2f,%.2f,%.2f]\n", rect.x(), rect.y(), rect.width(), rect.height());
#endif
    Q_D(QPainter);

    if (!d->engine)
        return;

    if (xRadius <= 0 || yRadius <= 0) {             // draw normal rectangle
        drawRect(rect);
        return;
    }

    if (d->extended) {
        d->extended->drawRoundedRect(rect, xRadius, yRadius, mode);
        return;
    }

    QPainterPath path;
    path.addRoundedRect(rect, xRadius, yRadius, mode);
    drawPath(path);
}

/*!
    \fn void QPainter::drawRoundedRect(const QRect &rect, qreal xRadius, qreal yRadius,
                                       Qt::SizeMode mode = Qt::AbsoluteSize);
    \since 4.4
    \overload

    Draws the given rectangle \a rect with rounded corners.
*/

/*!
    \fn void QPainter::drawRoundedRect(int x, int y, int w, int h, qreal xRadius, qreal yRadius,
                                       Qt::SizeMode mode = Qt::AbsoluteSize);
    \since 4.4
    \overload

    Draws the given rectangle \a x, \a y, \a w, \a h with rounded corners.
*/

/*!
    \obsolete

    Draws a rectangle \a r with rounded corners.

    The \a xRnd and \a yRnd arguments specify how rounded the corners
    should be. 0 is angled corners, 99 is maximum roundedness.

    A filled rectangle has a size of r.size(). A stroked rectangle
    has a size of r.size() plus the pen width.

    \sa drawRoundedRect()
*/
void QPainter::drawRoundRect(const QRectF &r, int xRnd, int yRnd)
{
    drawRoundedRect(r, xRnd, yRnd, Qt::RelativeSize);
}


/*!
    \fn void QPainter::drawRoundRect(const QRect &r, int xRnd = 25, int yRnd = 25)

    \overload
    \obsolete

    Draws the rectangle \a r with rounded corners.
*/

/*!
    \obsolete

    \fn QPainter::drawRoundRect(int x, int y, int w, int h, int xRnd, int yRnd)

    \overload

    Draws the rectangle \a x, \a y, \a w, \a h with rounded corners.
*/

/*!
    \fn void QPainter::drawEllipse(const QRectF &rectangle)

    Draws the ellipse defined by the given \a rectangle.

    A filled ellipse has a size of \a{rectangle}.\l
    {QRect::size()}{size()}. A stroked ellipse has a size of
    \a{rectangle}.\l {QRect::size()}{size()} plus the pen width.

    \table 100%
    \row
    \o \inlineimage qpainter-ellipse.png
    \o
    \snippet doc/src/snippets/code/src_gui_painting_qpainter.cpp 9
    \endtable

    \sa drawPie(), {Coordinate System}
*/
void QPainter::drawEllipse(const QRectF &r)
{
#ifdef QT_DEBUG_DRAW
    printf("QPainter::drawEllipse(), [%.2f,%.2f,%.2f,%.2f]\n", r.x(), r.y(), r.width(), r.height());
#endif
    Q_D(QPainter);

    if (!d->engine)
        return;

    QRectF rect(r.normalized());

    if (d->extended) {
        d->extended->drawEllipse(rect);
        return;
    }

    d->updateState(d->state);

    d->engine->drawEllipse(rect);
}

/*!
    \fn QPainter::drawEllipse(const QRect &rectangle)

    \overload

    Draws the ellipse defined by the given \a rectangle.
*/
void QPainter::drawEllipse(const QRect &r)
{
#ifdef QT_DEBUG_DRAW
    printf("QPainter::drawEllipse(), [%d,%d,%d,%d]\n", r.x(), r.y(), r.width(), r.height());
#endif
    Q_D(QPainter);

    if (!d->engine)
        return;

    QRect rect(r.normalized());

    if (d->extended) {
        d->extended->drawEllipse(rect);
        return;
    }

    d->updateState(d->state);

    d->engine->drawEllipse(rect);
}

/*!
    \fn QPainter::drawEllipse(int x, int y, int width, int height)

    \overload

    Draws the ellipse defined by the rectangle beginning at (\a{x},
    \a{y}) with the given \a width and \a height.
*/

/*!
    \since 4.4

    \fn QPainter::drawEllipse(const QPointF &center, qreal rx, qreal ry)

    \overload

    Draws the ellipse positioned at \a{center} with radii \a{rx} and \a{ry}.
*/

/*!
    \since 4.4

    \fn QPainter::drawEllipse(const QPoint &center, int rx, int ry)

    \overload

    Draws the ellipse positioned at \a{center} with radii \a{rx} and \a{ry}.
*/

/*!
    \fn void QPainter::drawArc(const QRectF &rectangle, int startAngle, int spanAngle)

    Draws the arc defined by the given \a rectangle, \a startAngle and
    \a spanAngle.

    The \a startAngle and \a spanAngle must be specified in 1/16th of
    a degree, i.e. a full circle equals 5760 (16 * 360). Positive
    values for the angles mean counter-clockwise while negative values
    mean the clockwise direction. Zero degrees is at the 3 o'clock
    position.

    \table 100%
    \row
    \o \inlineimage qpainter-arc.png
    \o
    \snippet doc/src/snippets/code/src_gui_painting_qpainter.cpp 10
    \endtable

    \sa drawPie(), drawChord(), {Coordinate System}
*/

void QPainter::drawArc(const QRectF &r, int a, int alen)
{
#ifdef QT_DEBUG_DRAW
    printf("QPainter::drawArc(), [%.2f,%.2f,%.2f,%.2f], angle=%d, sweep=%d\n",
           r.x(), r.y(), r.width(), r.height(), a/16, alen/16);
#endif
    Q_D(QPainter);

    if (!d->engine)
        return;

    QRectF rect = r.normalized();

    QPainterPath path;
    path.arcMoveTo(rect, a/qreal(16.0));
    path.arcTo(rect, a/qreal(16.0), alen/qreal(16.0));
    strokePath(path, d->state->pen);
}

/*! \fn void QPainter::drawArc(const QRect &rectangle, int startAngle,
                               int spanAngle)

    \overload

    Draws the arc defined by the given \a rectangle, \a startAngle and
    \a spanAngle.
*/

/*!
    \fn void QPainter::drawArc(int x, int y, int width, int height,
                               int startAngle, int spanAngle)

    \overload

    Draws the arc defined by the rectangle beginning at (\a x, \a y)
    with the specified \a width and \a height, and the given \a
    startAngle and \a spanAngle.
*/

/*!
    \fn void QPainter::drawPie(const QRectF &rectangle, int startAngle, int spanAngle)

    Draws a pie defined by the given \a rectangle, \a startAngle and
    and \a spanAngle.

    The pie is filled with the current brush().

    The startAngle and spanAngle must be specified in 1/16th of a
    degree, i.e. a full circle equals 5760 (16 * 360). Positive values
    for the angles mean counter-clockwise while negative values mean
    the clockwise direction. Zero degrees is at the 3 o'clock
    position.

    \table 100%
    \row
    \o \inlineimage qpainter-pie.png
    \o
    \snippet doc/src/snippets/code/src_gui_painting_qpainter.cpp 11
    \endtable

    \sa drawEllipse(), drawChord(), {Coordinate System}
*/
void QPainter::drawPie(const QRectF &r, int a, int alen)
{
#ifdef QT_DEBUG_DRAW
    printf("QPainter::drawPie(), [%.2f,%.2f,%.2f,%.2f], angle=%d, sweep=%d\n",
           r.x(), r.y(), r.width(), r.height(), a/16, alen/16);
#endif
    Q_D(QPainter);

    if (!d->engine)
        return;

    if (a > (360*16)) {
        a = a % (360*16);
    } else if (a < 0) {
        a = a % (360*16);
        if (a < 0) a += (360*16);
    }

    QRectF rect = r.normalized();

    QPainterPath path;
    path.moveTo(rect.center());
    path.arcTo(rect.x(), rect.y(), rect.width(), rect.height(), a/qreal(16.0), alen/qreal(16.0));
    path.closeSubpath();
    drawPath(path);

}

/*!
    \fn void QPainter::drawPie(const QRect &rectangle, int startAngle, int spanAngle)
    \overload

    Draws a pie defined by the given \a rectangle, \a startAngle and
    and \a spanAngle.
*/

/*!
    \fn void QPainter::drawPie(int x, int y, int width, int height, int
    startAngle, int spanAngle)

    \overload

    Draws the pie defined by the rectangle beginning at (\a x, \a y) with
    the specified \a width and \a height, and the given \a startAngle and
    \a spanAngle.
*/

/*!
    \fn void QPainter::drawChord(const QRectF &rectangle, int startAngle, int spanAngle)

    Draws the chord defined by the given \a rectangle, \a startAngle and
    \a spanAngle.  The chord is filled with the current brush().

    The startAngle and spanAngle must be specified in 1/16th of a
    degree, i.e. a full circle equals 5760 (16 * 360). Positive values
    for the angles mean counter-clockwise while negative values mean
    the clockwise direction. Zero degrees is at the 3 o'clock
    position.

    \table 100%
    \row
    \o \inlineimage qpainter-chord.png
    \o
    \snippet doc/src/snippets/code/src_gui_painting_qpainter.cpp 12
    \endtable

    \sa drawArc(), drawPie(), {Coordinate System}
*/
void QPainter::drawChord(const QRectF &r, int a, int alen)
{
#ifdef QT_DEBUG_DRAW
    printf("QPainter::drawChord(), [%.2f,%.2f,%.2f,%.2f], angle=%d, sweep=%d\n",
           r.x(), r.y(), r.width(), r.height(), a/16, alen/16);
#endif
    Q_D(QPainter);

    if (!d->engine)
        return;

    QRectF rect = r.normalized();

    QPainterPath path;
    path.arcMoveTo(rect, a/qreal(16.0));
    path.arcTo(rect, a/qreal(16.0), alen/qreal(16.0));
    path.closeSubpath();
    drawPath(path);
}
/*!
    \fn void QPainter::drawChord(const QRect &rectangle, int startAngle, int spanAngle)

    \overload

    Draws the chord defined by the given \a rectangle, \a startAngle and
    \a spanAngle.
*/

/*!
    \fn void QPainter::drawChord(int x, int y, int width, int height, int
    startAngle, int spanAngle)

    \overload

   Draws the chord defined by the rectangle beginning at (\a x, \a y)
   with the specified \a width and \a height, and the given \a
   startAngle and \a spanAngle.
*/


/*!
    Draws the first \a lineCount lines in the array \a lines
    using the current pen.

    \sa drawLine(), drawPolyline()
*/
void QPainter::drawLines(const QLineF *lines, int lineCount)
{
#ifdef QT_DEBUG_DRAW
    printf("QPainter::drawLines(), line count=%d\n", lineCount);
#endif

    Q_D(QPainter);

    if (!d->engine || lineCount < 1)
        return;

    if (d->extended) {
        d->extended->drawLines(lines, lineCount);
        return;
    }

    d->updateState(d->state);

    d->engine->drawLines(lines, lineCount);
}

/*!
    \fn void QPainter::drawLines(const QLine *lines, int lineCount)
    \overload

    Draws the first \a lineCount lines in the array \a lines
    using the current pen.
*/
void QPainter::drawLines(const QLine *lines, int lineCount)
{
#ifdef QT_DEBUG_DRAW
    printf("QPainter::drawLine(), line count=%d\n", lineCount);
#endif

    Q_D(QPainter);

    if (!d->engine || lineCount < 1)
        return;

    if (d->extended) {
        d->extended->drawLines(lines, lineCount);
        return;
    }

    d->updateState(d->state);

    d->engine->drawLines(lines, lineCount);
}

/*!
    \overload

    Draws the first \a lineCount lines in the array \a pointPairs
    using the current pen.  The lines are specified as pairs of points
    so the number of entries in \a pointPairs must be at least \a
    lineCount * 2.
*/
void QPainter::drawLines(const QPointF *pointPairs, int lineCount)
{
    Q_ASSERT(sizeof(QLineF) == 2*sizeof(QPointF));

    drawLines((QLineF*)pointPairs, lineCount);
}

/*!
    \overload

    Draws the first \a lineCount lines in the array \a pointPairs
    using the current pen.
*/
void QPainter::drawLines(const QPoint *pointPairs, int lineCount)
{
    Q_ASSERT(sizeof(QLine) == 2*sizeof(QPoint));

    drawLines((QLine*)pointPairs, lineCount);
}


/*!
    \fn void QPainter::drawLines(const QVector<QPointF> &pointPairs)
    \overload

    Draws a line for each pair of points in the vector \a pointPairs
    using the current pen. If there is an odd number of points in the
    array, the last point will be ignored.
*/

/*!
    \fn void QPainter::drawLines(const QVector<QPoint> &pointPairs)
    \overload

    Draws a line for each pair of points in the vector \a pointPairs
    using the current pen.
*/

/*!
    \fn void QPainter::drawLines(const QVector<QLineF> &lines)
    \overload

    Draws the set of lines defined by the list \a lines using the
    current pen and brush.
*/

/*!
    \fn void QPainter::drawLines(const QVector<QLine> &lines)
    \overload

    Draws the set of lines defined by the list \a lines using the
    current pen and brush.
*/

/*!
    Draws the polyline defined by the first \a pointCount points in \a
    points using the current pen.

    Note that unlike the drawPolygon() function the last point is \e
    not connected to the first, neither is the polyline filled.

    \table 100%
    \row
    \o
    \snippet doc/src/snippets/code/src_gui_painting_qpainter.cpp 13
    \endtable

    \sa drawLines(), drawPolygon(), {Coordinate System}
*/
void QPainter::drawPolyline(const QPointF *points, int pointCount)
{
#ifdef QT_DEBUG_DRAW
    printf("QPainter::drawPolyline(), count=%d\n", pointCount);
#endif
    Q_D(QPainter);

    if (!d->engine || pointCount < 2)
        return;

    if (d->extended) {
        d->extended->drawPolygon(points, pointCount, QPaintEngine::PolylineMode);
        return;
    }

    d->updateState(d->state);

    d->engine->drawPolygon(points, pointCount, QPaintEngine::PolylineMode);
}

/*!
    \overload

    Draws the polyline defined by the first \a pointCount points in \a
    points using the current pen.
 */
void QPainter::drawPolyline(const QPoint *points, int pointCount)
{
#ifdef QT_DEBUG_DRAW
    printf("QPainter::drawPolyline(), count=%d\n", pointCount);
#endif
    Q_D(QPainter);

    if (!d->engine || pointCount < 2)
        return;

    if (d->extended) {
        d->extended->drawPolygon(points, pointCount, QPaintEngine::PolylineMode);
        return;
    }

    d->updateState(d->state);

    d->engine->drawPolygon(points, pointCount, QPaintEngine::PolylineMode);
}

/*!
    \fn void QPainter::drawPolyline(const QPolygon &polygon, int index, int
    count)

    \overload
    \compat

    Draws the polyline defined by the \a count lines of the given \a
    polygon starting at \a index (\a index defaults to 0).

    Use drawPolyline() combined with QPolygon::constData() instead.

    \oldcode
        QPainter painter(this);
        painter.drawPolyline(polygon, index, count);
    \newcode
        int pointCount = (count == -1) ?  polygon.size() - index : count;

        QPainter painter(this);
        painter.drawPolyline(polygon.constData() + index, pointCount);
    \endcode
*/

/*!
    \fn void QPainter::drawPolyline(const QPolygonF &points)

    \overload

    Draws the polyline defined by the given \a points using the
    current pen.
*/

/*!
    \fn void QPainter::drawPolyline(const QPolygon &points)

    \overload

    Draws the polyline defined by the given \a points using the
    current pen.
*/

/*!
    Draws the polygon defined by the first \a pointCount points in the
    array \a points using the current pen and brush.

    \table 100%
    \row
    \o \inlineimage qpainter-polygon.png
    \o
    \snippet doc/src/snippets/code/src_gui_painting_qpainter.cpp 14
    \endtable

    The first point is implicitly connected to the last point, and the
    polygon is filled with the current brush().

    If \a fillRule is Qt::WindingFill, the polygon is filled using the
    winding fill algorithm.  If \a fillRule is Qt::OddEvenFill, the
    polygon is filled using the odd-even fill algorithm. See
    \l{Qt::FillRule} for a more detailed description of these fill
    rules.

    \sa  drawConvexPolygon(), drawPolyline(), {Coordinate System}
*/
void QPainter::drawPolygon(const QPointF *points, int pointCount, Qt::FillRule fillRule)
{
#ifdef QT_DEBUG_DRAW
    printf("QPainter::drawPolygon(), count=%d\n", pointCount);
#endif

    Q_D(QPainter);

    if (!d->engine || pointCount < 2)
        return;

    if (d->extended) {
        d->extended->drawPolygon(points, pointCount, QPaintEngine::PolygonDrawMode(fillRule));
        return;
    }

    d->updateState(d->state);

    d->engine->drawPolygon(points, pointCount, QPaintEngine::PolygonDrawMode(fillRule));
}

/*! \overload

    Draws the polygon defined by the first \a pointCount points in the
    array \a points.
*/
void QPainter::drawPolygon(const QPoint *points, int pointCount, Qt::FillRule fillRule)
{
#ifdef QT_DEBUG_DRAW
    printf("QPainter::drawPolygon(), count=%d\n", pointCount);
#endif

    Q_D(QPainter);

    if (!d->engine || pointCount < 2)
        return;

    if (d->extended) {
        d->extended->drawPolygon(points, pointCount, QPaintEngine::PolygonDrawMode(fillRule));
        return;
    }

    d->updateState(d->state);

    d->engine->drawPolygon(points, pointCount, QPaintEngine::PolygonDrawMode(fillRule));
}

/*! \fn void QPainter::drawPolygon(const QPolygonF &polygon, bool winding, int index = 0,
                                   int count = -1)
    \compat
    \overload

    Use drawPolygon() combined with QPolygonF::constData() instead.

    \oldcode
        QPainter painter(this);
        painter.drawPolygon(polygon, winding, index, count);
    \newcode
        int pointCount = (count == -1) ?  polygon.size() - index : count;
        int fillRule = winding ? Qt::WindingFill : Qt::OddEvenFill;

        QPainter painter(this);
        painter.drawPolygon( polygon.constData() + index, pointCount, fillRule);
    \endcode
*/

/*! \fn void QPainter::drawPolygon(const QPolygon &polygon, bool winding,
                                   int index = 0, int count = -1)

    \compat
    \overload

    Use drawPolygon() combined with QPolygon::constData() instead.

    \oldcode
        QPainter painter(this);
        painter.drawPolygon(polygon, winding, index, count);
    \newcode
        int pointCount = (count == -1) ?  polygon.size() - index : count;
        int fillRule = winding ? Qt::WindingFill : Qt::OddEvenFill;

        QPainter painter(this);
        painter.drawPolygon( polygon.constData() + index, pointCount, fillRule);
    \endcode
*/

/*! \fn void QPainter::drawPolygon(const QPolygonF &points, Qt::FillRule fillRule)

    \overload

    Draws the polygon defined by the given \a points using the fill
    rule \a fillRule.
*/

/*! \fn void QPainter::drawPolygon(const QPolygon &points, Qt::FillRule fillRule)

    \overload

    Draws the polygon defined by the given \a points using the fill
    rule \a fillRule.
*/

/*!
    \fn void QPainter::drawConvexPolygon(const QPointF *points, int pointCount)

    Draws the convex polygon defined by the first \a pointCount points
    in the array \a points using the current pen.

    \table 100%
    \row
    \o \inlineimage qpainter-polygon.png
    \o
    \snippet doc/src/snippets/code/src_gui_painting_qpainter.cpp 15
    \endtable

    The first point is implicitly connected to the last point, and the
    polygon is filled with the current brush().  If the supplied
    polygon is not convex, i.e. it contains at least one angle larger
    than 180 degrees, the results are undefined.

    On some platforms (e.g. X11), the drawConvexPolygon() function can
    be faster than the drawPolygon() function.

    \sa drawPolygon(), drawPolyline(), {Coordinate System}
*/

/*!
    \fn void QPainter::drawConvexPolygon(const QPoint *points, int pointCount)
    \overload

    Draws the convex polygon defined by the first \a pointCount points
    in the array \a points using the current pen.
*/

/*!
    \fn void QPainter::drawConvexPolygon(const QPolygonF &polygon)

    \overload

    Draws the convex polygon defined by \a polygon using the current
    pen and brush.
*/

/*!
    \fn void QPainter::drawConvexPolygon(const QPolygon &polygon)
    \overload

    Draws the convex polygon defined by \a polygon using the current
    pen and brush.
*/

/*!
    \fn void QPainter::drawConvexPolygon(const QPolygonF &polygon, int
    index, int count)

    \compat
    \overload

    Use drawConvexPolygon() combined with QPolygonF::constData()
    instead.

    \oldcode
        QPainter painter(this);
        painter.drawConvexPolygon(polygon, index, count);
    \newcode
        int pointCount = (count == -1) ?  polygon.size() - index : count;

        QPainter painter(this);
        painter.drawConvexPolygon(polygon.constData() + index, pointCount);
    \endcode
*/

/*!
    \fn void QPainter::drawConvexPolygon(const QPolygon &polygon, int
    index, int count)

    \compat
    \overload

    Use drawConvexPolygon() combined with QPolygon::constData()
    instead.

    \oldcode
        QPainter painter(this);
        painter.drawConvexPolygon(polygon, index, count);
    \newcode
        int pointCount = (count == -1) ?  polygon.size() - index : count;

        QPainter painter(this);
        painter.drawConvexPolygon(polygon.constData() + index, pointCount);
    \endcode
*/

void QPainter::drawConvexPolygon(const QPoint *points, int pointCount)
{
#ifdef QT_DEBUG_DRAW
    printf("QPainter::drawConvexPolygon(), count=%d\n", pointCount);
#endif

    Q_D(QPainter);

    if (!d->engine || pointCount < 2)
        return;

    if (d->extended) {
        d->extended->drawPolygon(points, pointCount, QPaintEngine::ConvexMode);
        return;
    }

    d->updateState(d->state);

    d->engine->drawPolygon(points, pointCount, QPaintEngine::ConvexMode);
}

void QPainter::drawConvexPolygon(const QPointF *points, int pointCount)
{
#ifdef QT_DEBUG_DRAW
    printf("QPainter::drawConvexPolygon(), count=%d\n", pointCount);
#endif

    Q_D(QPainter);

    if (!d->engine || pointCount < 2)
        return;

    if (d->extended) {
        d->extended->drawPolygon(points, pointCount, QPaintEngine::ConvexMode);
        return;
    }

    d->updateState(d->state);

    d->engine->drawPolygon(points, pointCount, QPaintEngine::ConvexMode);
}

static inline QPointF roundInDeviceCoordinates(const QPointF &p, const QTransform &m)
{
    return m.inverted().map(QPointF(m.map(p).toPoint()));
}

/*!
    \fn void QPainter::drawPixmap(const QRectF &target, const QPixmap &pixmap, const QRectF &source)

    Draws the rectangular portion \a source of the given \a pixmap
    into the given \a target in the paint device.

    \note The pixmap is scaled to fit the rectangle, if both the pixmap and rectangle size disagree.

    \table 100%
    \row
    \o
    \snippet doc/src/snippets/code/src_gui_painting_qpainter.cpp 16
    \endtable

    If \a pixmap is a QBitmap it is drawn with the bits that are "set"
    using the pens color. If backgroundMode is Qt::OpaqueMode, the
    "unset" bits are drawn using the color of the background brush; if
    backgroundMode is Qt::TransparentMode, the "unset" bits are
    transparent. Drawing bitmaps with gradient or texture colors is
    not supported.

    \sa drawImage()
*/
void QPainter::drawPixmap(const QPointF &p, const QPixmap &pm)
{
#if defined QT_DEBUG_DRAW
    printf("QPainter::drawPixmap(), p=[%.2f,%.2f], pix=[%d,%d]\n",
           p.x(), p.y(), pm.width(), pm.height());
#endif

    const int w = pm.width();
    const int h = pm.height();

    drawPixmap(QRectF(p.x(), p.y(), w, h), pm, QRectF(0, 0, w, h));
}

void QPainter::drawPixmap(const QRectF &r, const QPixmap &pm, const QRectF &sr)
{
#if defined QT_DEBUG_DRAW
    printf("QPainter::drawPixmap(), target=[%.2f,%.2f,%.2f,%.2f], pix=[%d,%d], source=[%.2f,%.2f,%.2f,%.2f]\n",
           r.x(), r.y(), r.width(), r.height(),
           pm.width(), pm.height(),
           sr.x(), sr.y(), sr.width(), sr.height());
#endif

    Q_D(QPainter);
    if (!d->engine || pm.isNull())
        return;
#ifndef QT_NO_DEBUG
    qt_painter_thread_test(d->device->devType(), "drawPixmap()", true);
#endif

    qreal x = r.x();
    qreal y = r.y();
    qreal w = r.width();
    qreal h = r.height();
    qreal sx = sr.x();
    qreal sy = sr.y();
    qreal sw = sr.width();
    qreal sh = sr.height();

    // Sanity-check clipping
    if (sw <= 0)
        sw = pm.width() - sx;

    if (sh <= 0)
        sh = pm.height() - sy;

    if (w < 0)
        w = sw;
    if (h < 0)
        h = sh;

    if (sx < 0) {
        qreal w_ratio = sx * w/sw;
        x -= w_ratio;
        w += w_ratio;
        sw += sx;
        sx = 0;
    }

    if (sy < 0) {
        qreal h_ratio = sy * h/sh;
        y -= h_ratio;
        h += h_ratio;
        sh += sy;
        sy = 0;
    }

    if (sw + sx > pm.width()) {
        qreal delta = sw - (pm.width() - sx);
        qreal w_ratio = delta * w/sw;
        sw -= delta;
        w -= w_ratio;
    }

    if (sh + sy > pm.height()) {
        qreal delta = sh - (pm.height() - sy);
        qreal h_ratio = delta * h/sh;
        sh -= delta;
        h -= h_ratio;
    }

    if (w == 0 || h == 0 || sw <= 0 || sh <= 0)
        return;

    if (d->extended) {
        d->extended->drawPixmap(QRectF(x, y, w, h), pm, QRectF(sx, sy, sw, sh));
        return;
    }

    // Emulate opaque background for bitmaps
    if (d->state->bgMode == Qt::OpaqueMode && pm.isQBitmap())
        fillRect(QRectF(x, y, w, h), d->state->bgBrush.color());

    d->updateState(d->state);

    if ((d->state->matrix.type() > QTransform::TxTranslate
         && !d->engine->hasFeature(QPaintEngine::PixmapTransform))
        || (!d->state->matrix.isAffine() && !d->engine->hasFeature(QPaintEngine::PerspectiveTransform))
        || (d->state->opacity != 1.0 && !d->engine->hasFeature(QPaintEngine::ConstantOpacity))
        || ((sw != w || sh != h) && !d->engine->hasFeature(QPaintEngine::PixmapTransform)))
    {
        save();
        // If there is no rotation involved we have to make sure we use the
        // antialiased and not the aliased coordinate system by rounding the coordinates.
        if (d->state->matrix.type() <= QTransform::TxScale) {
            const QPointF p = roundInDeviceCoordinates(QPointF(x, y), d->state->matrix);
            x = p.x();
            y = p.y();
        }

        if (d->state->matrix.type() <= QTransform::TxTranslate && sw == w && sh == h) {
            sx = qRound(sx);
            sy = qRound(sy);
            sw = qRound(sw);
            sh = qRound(sh);
        }

        translate(x, y);
        scale(w / sw, h / sh);
        setBackgroundMode(Qt::TransparentMode);
        QBrush brush;

        if (sw == pm.width() && sh == pm.height())
            brush = QBrush(d->state->pen.color(), pm);
        else
            brush = QBrush(d->state->pen.color(), pm.copy(sx, sy, sw, sh));

        setBrush(brush);
        setPen(Qt::NoPen);

        drawRect(QRectF(0, 0, sw, sh));
        restore();
    } else {
        if (!d->engine->hasFeature(QPaintEngine::PixmapTransform)) {
            x += d->state->matrix.dx();
            y += d->state->matrix.dy();
        }
        d->engine->drawPixmap(QRectF(x, y, w, h), pm, QRectF(sx, sy, sw, sh));
    }
}


/*!
    \fn void QPainter::drawPixmap(const QRect &target, const QPixmap &pixmap,
                                  const QRect &source)
    \overload

    Draws the rectangular portion \a source of the given \a pixmap
    into the given \a target in the paint device.

    \note The pixmap is scaled to fit the rectangle, if both the pixmap and rectangle size disagree.
*/

/*!
    \fn void QPainter::drawPixmap(const QPointF &point, const QPixmap &pixmap,
                                  const QRectF &source)
    \overload

    Draws the rectangular portion \a source of the given \a pixmap
    with its origin at the given \a point.
*/

/*!
    \fn void QPainter::drawPixmap(const QPoint &point, const QPixmap &pixmap,
                                  const QRect &source)

    \overload

    Draws the rectangular portion \a source of the given \a pixmap
    with its origin at the given \a point.
*/

/*!
    \fn void QPainter::drawPixmap(const QPointF &point, const QPixmap &pixmap)
    \overload

    Draws the given \a pixmap with its origin at the given \a point.
*/

/*!
    \fn void QPainter::drawPixmap(const QPoint &point, const QPixmap &pixmap)
    \overload

    Draws the given \a pixmap with its origin at the given \a point.
*/

/*!
    \fn void QPainter::drawPixmap(int x, int y, const QPixmap &pixmap)

    \overload

    Draws the given \a pixmap at position (\a{x}, \a{y}).
*/

/*!
    \fn void QPainter::drawPixmap(const QRect &rectangle, const QPixmap &pixmap)
    \overload

    Draws the given \a  pixmap into the given \a rectangle.

    \note The pixmap is scaled to fit the rectangle, if both the pixmap and rectangle size disagree.
*/

/*!
    \fn void QPainter::drawPixmap(int x, int y, int width, int height,
    const QPixmap &pixmap)

    \overload

    Draws the \a pixmap into the rectangle at position (\a{x}, \a{y})
    with  the given \a width and \a height.
*/

/*!
    \fn void QPainter::drawPixmap(int x, int y, int w, int h, const QPixmap &pixmap,
                                  int sx, int sy, int sw, int sh)

    \overload

    Draws the rectangular portion with the origin (\a{sx}, \a{sy}),
    width \a sw and height \a sh, of the given \a pixmap , at the
    point (\a{x}, \a{y}), with a width of \a w and a height of \a h.
    If sw or sh are equal to zero the width/height of the pixmap
    is used and adjusted by the offset sx/sy;
*/

/*!
    \fn void QPainter::drawPixmap(int x, int y, const QPixmap &pixmap,
                                  int sx, int sy, int sw, int sh)

    \overload

    Draws a pixmap at (\a{x}, \a{y}) by copying a part of the given \a
    pixmap into the paint device.

    (\a{x}, \a{y}) specifies the top-left point in the paint device that is
    to be drawn onto. (\a{sx}, \a{sy}) specifies the top-left point in \a
    pixmap that is to be drawn. The default is (0, 0).

    (\a{sw}, \a{sh}) specifies the size of the pixmap that is to be drawn.
    The default, (0, 0) (and negative) means all the way to the
    bottom-right of the pixmap.
*/

void QPainter::drawImage(const QPointF &p, const QImage &image)
{
#if defined QT_DEBUG_DRAW
    printf("QPainter::drawImage(), p=[%.2f,%.2f], img=[%d,%d]\n",
           p.x(), p.y(), image.width(), image.height());
#endif

    const int w = image.width();
    const int h = image.height();

    drawImage(QRectF(p.x(), p.y(), w, h), image, QRectF(0, 0, w, h), Qt::AutoColor);
}

void QPainter::drawImage(const QRectF &targetRect, const QImage &image, const QRectF &sourceRect,
                         Qt::ImageConversionFlags flags)
{
    Q_D(QPainter);

    if (!d->engine || image.isNull())
        return;

    qreal x = targetRect.x();
    qreal y = targetRect.y();
    qreal w = targetRect.width();
    qreal h = targetRect.height();
    qreal sx = sourceRect.x();
    qreal sy = sourceRect.y();
    qreal sw = sourceRect.width();
    qreal sh = sourceRect.height();

    // Sanity-check clipping
    if (sw <= 0)
        sw = image.width() - sx;

    if (sh <= 0)
        sh = image.height() - sy;

    if (w < 0)
        w = sw;
    if (h < 0)
        h = sh;

    if (sx < 0) {
        qreal w_ratio = sx * w/sw;
        x -= w_ratio;
        w += w_ratio;
        sw += sx;
        sx = 0;
    }

    if (sy < 0) {
        qreal h_ratio = sy * h/sh;
        y -= h_ratio;
        h += h_ratio;
        sh += sy;
        sy = 0;
    }

    if (sw + sx > image.width()) {
        qreal delta = sw - (image.width() - sx);
        qreal w_ratio = delta * w/sw;
        sw -= delta;
        w -= w_ratio;
    }

    if (sh + sy > image.height()) {
        qreal delta = sh - (image.height() - sy);
        qreal h_ratio = delta * h/sh;
        sh -= delta;
        h -= h_ratio;
    }

    if (w == 0 || h == 0 || sw <= 0 || sh <= 0)
        return;

    if (d->extended) {
        d->extended->drawImage(QRectF(x, y, w, h), image, QRectF(sx, sy, sw, sh), flags);
        return;
    }

    d->updateState(d->state);

    if (((d->state->matrix.type() > QTransform::TxTranslate || (sw != w || sh != h))
         && !d->engine->hasFeature(QPaintEngine::PixmapTransform))
        || (!d->state->matrix.isAffine() && !d->engine->hasFeature(QPaintEngine::PerspectiveTransform))
        || (d->state->opacity != 1.0 && !d->engine->hasFeature(QPaintEngine::ConstantOpacity)))
    {
        save();
        // If there is no rotation involved we have to make sure we use the
        // antialiased and not the aliased coordinate system by rounding the coordinates.
        if (d->state->matrix.type() <= QTransform::TxScale) {
            const QPointF p = roundInDeviceCoordinates(QPointF(x, y), d->state->matrix);
            x = p.x();
            y = p.y();
        }

        if (d->state->matrix.type() <= QTransform::TxTranslate && sw == w && sh == h) {
            sx = qRound(sx);
            sy = qRound(sy);
            sw = qRound(sw);
            sh = qRound(sh);
        }
        translate(x, y);
        scale(w / sw, h / sh);
        setBackgroundMode(Qt::TransparentMode);
        QBrush brush(image);
        setBrush(brush);
        setPen(Qt::NoPen);
        setBrushOrigin(QPointF(-sx, -sy));

        drawRect(QRectF(0, 0, sw, sh));
        restore();
        return;
    }

    if (d->state->matrix.type() == QTransform::TxTranslate
        && !d->engine->hasFeature(QPaintEngine::PixmapTransform)) {
        x += d->state->matrix.dx();
        y += d->state->matrix.dy();
    }

    d->engine->drawImage(QRectF(x, y, w, h), image, QRectF(sx, sy, sw, sh), flags);
}

/*!
    \fn void QPainter::drawText(const QPointF &position, const QString &text)

    Draws the given \a text with the currently defined text direction,
    beginning at the given \a position.

    This function does not handle the newline character (\n), as it cannot
    break text into multiple lines, and it cannot display the newline character.
    Use the QPainter::drawText() overload that takes a rectangle instead
    if you want to draw multiple lines of text with the newline character, or
    if you want the text to be wrapped.

    By default, QPainter draws text anti-aliased.

    \note The y-position is used as the baseline of the font.
*/

void QPainter::drawText(const QPointF &p, const QString &str)
{
#ifdef QT_DEBUG_DRAW
    printf("QPainter::drawText(), pos=[%.2f,%.2f], str='%s'\n", p.x(), p.y(), str.toLatin1().constData());
#endif

    Q_D(QPainter);

    if (!d->engine || str.isEmpty() || pen().style() == Qt::NoPen)
        return;

    bool toggleantialiasing = (!(renderHints() & QPainter::Antialiasing));
    if (toggleantialiasing) {
        setRenderHint(QPainter::Antialiasing, true);
    }

    QPainterPath textpath;
    textpath.setFillRule(Qt::WindingFill);
    textpath.addText(p, d->state->font, str);
    fillPath(textpath, d->state->pen.brush());

    if (toggleantialiasing) {
        setRenderHint(QPainter::Antialiasing, false);
    }
}

void QPainter::drawText(const QRect &r, int flags, const QString &str, QRect *br)
{
#ifdef QT_DEBUG_DRAW
    printf("QPainter::drawText(), r=[%d,%d,%d,%d], flags=%d, str='%s'\n",
           r.x(), r.y(), r.width(), r.height(), flags, str.toLatin1().constData());
#endif

    Q_D(QPainter);

    if (!d->engine || str.length() == 0 || pen().style() == Qt::NoPen)
        return;

    if (!d->extended)
        d->updateState(d->state);

    QRectF bounds;
    qt_format_text(d->state->font, r, flags, 0, str, br ? &bounds : nullptr, this);
    if (br)
        *br = bounds.toAlignedRect();
}

/*!
    \fn void QPainter::drawText(const QPoint &position, const QString &text)

    \overload

    Draws the given \a text with the currently defined text direction,
    beginning at the given \a position.

    By default, QPainter draws text anti-aliased.

    \note The y-position is used as the baseline of the font.

*/

/*!
    \fn void QPainter::drawText(const QRectF &rectangle, int flags, const QString &text, QRectF *boundingRect)
    \overload

    Draws the given \a text within the provided \a rectangle.

    \table 100%
    \row
    \o \inlineimage qpainter-text.png
    \o
    \snippet doc/src/snippets/code/src_gui_painting_qpainter.cpp 17
    \endtable

    The \a boundingRect (if not null) is set to the what the bounding rectangle
    should be in order to enclose the whole text. The \a flags argument is a bitwise
    OR of the following flags:

    \list
    \o Qt::AlignLeft
    \o Qt::AlignRight
    \o Qt::AlignHCenter
    \o Qt::AlignJustify
    \o Qt::AlignTop
    \o Qt::AlignBottom
    \o Qt::AlignVCenter
    \o Qt::AlignCenter
    \o Qt::TextDontClip
    \o Qt::TextSingleLine
    \o Qt::TextShowMnemonic
    \o Qt::TextWordWrap
    \o Qt::TextIncludeTrailingSpaces
    \endlist

    \sa Qt::AlignmentFlag, Qt::TextFlag, boundingRect(), layoutDirection()

    By default, QPainter draws text anti-aliased.

    \note The y-coordinate of \a rectangle is used as the top of the font.
*/
void QPainter::drawText(const QRectF &r, int flags, const QString &str, QRectF *br)
{
#ifdef QT_DEBUG_DRAW
    printf("QPainter::drawText(), r=[%.2f,%.2f,%.2f,%.2f], flags=%d, str='%s'\n",
           r.x(), r.y(), r.width(), r.height(), flags, str.toLatin1().constData());
#endif

    Q_D(QPainter);

    if (!d->engine || str.length() == 0 || pen().style() == Qt::NoPen)
        return;

    if (!d->extended)
        d->updateState(d->state);

    qt_format_text(d->state->font, r, flags, 0, str, br, this);
}

/*!
    \fn void QPainter::drawText(const QRect &rectangle, int flags, const QString &text, QRect *boundingRect)
    \overload

    Draws the given \a text within the provided \a rectangle according
    to the specified \a flags. The \a boundingRect (if not null) is set to
    the what the bounding rectangle should be in order to enclose the whole text.

    By default, QPainter draws text anti-aliased.

    \note The y-coordinate of \a rectangle is used as the top of the font.
*/

/*!
    \fn void QPainter::drawText(int x, int y, const QString &text)

    \overload

    Draws the given \a text at position (\a{x}, \a{y}), using the painter's
    currently defined text direction.

    By default, QPainter draws text anti-aliased.

    \note The y-position is used as the baseline of the font.

*/

/*!
    \fn void QPainter::drawText(int x, int y, int width, int height, int flags,
                                const QString &text, QRect *boundingRect)

    \overload

    Draws the given \a text within the rectangle with origin (\a{x},
    \a{y}), \a width and \a height.

    The \a boundingRect (if not null) is set to the actual bounding
    rectangle of the output.  The \a flags argument is a bitwise OR of
    the following flags:

    \list
    \o Qt::AlignLeft
    \o Qt::AlignRight
    \o Qt::AlignHCenter
    \o Qt::AlignJustify
    \o Qt::AlignTop
    \o Qt::AlignBottom
    \o Qt::AlignVCenter
    \o Qt::AlignCenter
    \o Qt::TextSingleLine
    \o Qt::TextShowMnemonic
    \o Qt::TextWordWrap
    \endlist

    By default, QPainter draws text anti-aliased.

    \note The y-position is used as the top of the font.

    \sa Qt::AlignmentFlag, Qt::TextFlag
*/

/*!
    \fn void QPainter::drawText(const QRectF &rectangle, const QString &text,
        const QTextOption &option)
    \overload

    Draws the given \a text in the \a rectangle specified using the \a option
    to control its positioning and orientation.

    By default, QPainter draws text anti-aliased.

    \note The y-coordinate of \a rectangle is used as the top of the font.
*/
void QPainter::drawText(const QRectF &r, const QString &text, const QTextOption &o)
{
#ifdef QT_DEBUG_DRAW
    printf("QPainter::drawText(), r=[%.2f,%.2f,%.2f,%.2f], str='%s'\n",
           r.x(), r.y(), r.width(), r.height(), text.toLatin1().constData());
#endif

    Q_D(QPainter);

    if (!d->engine || text.length() == 0 || pen().style() == Qt::NoPen)
        return;

    if (!d->extended)
        d->updateState(d->state);

    qt_format_text(d->state->font, r, 0, &o, text, nullptr, this);
}

/*!
    \fn void QPainter::drawTextItem(int x, int y, const QTextItem &ti)

    \internal
    \overload
*/

/*!
    \fn void QPainter::drawTextItem(const QPoint &p, const QTextItem &ti)

    \internal
    \overload

    Draws the text item \a ti at position \a p.
*/

/*!
    \fn void QPainter::drawTextItem(const QPointF &p, const QTextItem &ti)

    \internal
    \since 4.1

    Draws the text item \a ti at position \a p.

    This method ignores the painters background mode and
    color. drawText and qt_format_text have to do it themselves, as
    only they know the extents of the complete string.

    It ignores the font set on the painter as the text item has one of its own.

    The underline and strikeout parameters of the text items font are
    ignored aswell. You'll need to pass in the correct flags to get
    underlining and strikeout.
*/

static QPixmap generateWavyPixmap(qreal maxRadius, const QPen &pen)
{
    const qreal radiusBase = qMax(qreal(1), maxRadius);

    const QByteArray key = qHexString(
        "qt_waveunderline_%f_",
        static_cast<float>(radiusBase)
    ) + pen.color().name().toLatin1();

    QPixmap pixmap;
    if (QPixmapCache::find(key, pixmap))
        return pixmap;

    const qreal halfPeriod = qMax(qreal(2), qreal(radiusBase * 1.61803399)); // the golden ratio
    const int width = qCeil(100 / (2 * halfPeriod)) * (2 * halfPeriod);
    const int radius = qFloor(radiusBase);

    QPainterPath path;

    qreal xs = 0;
    qreal ys = radius;

    while (xs < width) {
        xs += halfPeriod;
        ys = -ys;
        path.quadTo(xs - halfPeriod / 2, ys, xs, 0);
    }

    pixmap = QPixmap(width, radius * 2);
    pixmap.fill(Qt::transparent);
    {
        QPen wavePen = pen;
        wavePen.setCapStyle(Qt::SquareCap);

        // This is to protect against making the line too fat, as happens on Mac OS X
        // due to it having a rather thick width for the regular underline.
        const qreal maxPenWidth = .8 * radius;
        if (wavePen.widthF() > maxPenWidth)
            wavePen.setWidth(maxPenWidth);

        QPainter imgPainter(&pixmap);
        imgPainter.setPen(wavePen);
        imgPainter.setRenderHint(QPainter::Antialiasing);
        imgPainter.translate(0, radius);
        imgPainter.drawPath(path);
    }

    QPixmapCache::insert(key, pixmap);

    return pixmap;
}

static void drawTextItemDecoration(QPainter *painter, const QPointF &pos, const QFontEngine *fe,
                                   QTextCharFormat::UnderlineStyle underlineStyle,
                                   QTextItem::RenderFlags flags, qreal width,
                                   const QTextCharFormat &charFormat)
{
    if (underlineStyle == QTextCharFormat::NoUnderline)
        return;

    const QPen oldPen = painter->pen();
    const QBrush oldBrush = painter->brush();
    painter->setBrush(Qt::NoBrush);
    QPen pen = oldPen;
    pen.setStyle(Qt::SolidLine);
    pen.setWidthF(fe->lineThickness().toReal());
    pen.setCapStyle(Qt::FlatCap);

    const qreal underlineOffset = fe->underlinePosition().toReal();
    // deliberately ceil the offset to avoid the underline coming too close to
    // the text above it.
    const qreal underlinePos = pos.y() + qCeil(underlineOffset);

    if (underlineStyle == QTextCharFormat::SpellCheckUnderline) {
        underlineStyle = QTextCharFormat::UnderlineStyle(QApplication::style()->styleHint(QStyle::SH_SpellCheckUnderlineStyle));
    }

    if (underlineStyle == QTextCharFormat::WaveUnderline) {
        painter->save();
        painter->translate(0, pos.y() + 1);

        QColor uc = charFormat.underlineColor();
        if (uc.isValid())
            pen.setColor(uc);

        // Adapt wave to underlineOffset or pen width, whatever is larger, to make it work on all platforms
        const QPixmap wave = generateWavyPixmap(qMax(underlineOffset, pen.widthF()), pen);
        const int descent = (int) fe->descent().toReal();

        painter->setBrushOrigin(painter->brushOrigin().x(), 0);
        painter->fillRect(pos.x(), 0, qCeil(width), qMin(wave.height(), descent), wave);
        painter->restore();
    } else if (underlineStyle != QTextCharFormat::NoUnderline) {
        QLineF underLine(pos.x(), underlinePos, pos.x() + qFloor(width), underlinePos);

        QColor uc = charFormat.underlineColor();
        if (uc.isValid())
            pen.setColor(uc);

        pen.setStyle((Qt::PenStyle)(underlineStyle));
        painter->setPen(pen);
        painter->drawLine(underLine);
    }
}

void QPainter::drawTextItem(const QPointF &p, const QTextItem &_ti)
{
#ifdef QT_DEBUG_DRAW
    printf("QPainter::drawTextItem(), pos=[%.f,%.f], str='%s'\n",
           p.x(), p.y(), qPrintable(_ti.text()));
#endif

    Q_D(QPainter);

    if (!d->engine)
        return;

#ifndef QT_NO_DEBUG
    qt_painter_thread_test(d->device->devType(),
                           "text and fonts",
                           QFontDatabase::supportsThreadedFontRendering());
#endif

    const QTextItemInt &ti = static_cast<const QTextItemInt &>(_ti);

    if (!d->extended && d->state->bgMode == Qt::OpaqueMode) {
        QRectF rect(p.x(), p.y() - ti.ascent.toReal(), ti.width.toReal(), (ti.ascent + ti.descent + 1).toReal());
        fillRect(rect, d->state->bgBrush);
    }

    if (pen().style() == Qt::NoPen)
        return;

    const QPainter::RenderHints oldRenderHints = d->state->renderHints;
    if (!(d->state->renderHints & QPainter::Antialiasing) && d->state->matrix.type() >= QTransform::TxScale) {
        // draw antialias decoration (underline/overline/strikeout) with
        // transformed text

        bool aa = true;
        const QTransform &m = d->state->matrix;
        if (d->state->matrix.type() < QTransform::TxShear) {
            bool isPlain90DegreeRotation =
                (qFuzzyIsNull(m.m11())
                 && qFuzzyIsNull(m.m12() - qreal(1))
                 && qFuzzyIsNull(m.m21() + qreal(1))
                 && qFuzzyIsNull(m.m22())
                    )
                ||
                (qFuzzyIsNull(m.m11() + qreal(1))
                 && qFuzzyIsNull(m.m12())
                 && qFuzzyIsNull(m.m21())
                 && qFuzzyIsNull(m.m22() + qreal(1))
                    )
                ||
                (qFuzzyIsNull(m.m11())
                 && qFuzzyIsNull(m.m12() + qreal(1))
                 && qFuzzyIsNull(m.m21() - qreal(1))
                 && qFuzzyIsNull(m.m22())
                    )
                ;
            aa = !isPlain90DegreeRotation;
        }
        if (aa)
            setRenderHint(QPainter::Antialiasing, true);
    }

    if (!d->extended)
        d->updateState(d->state);

    if (!ti.glyphs.numGlyphs) {
        // nothing to do
    } else {
        if (d->extended)
            d->extended->drawTextItem(p, ti);
        else
            d->engine->drawTextItem(p, ti);
    }
    drawTextItemDecoration(this, p, ti.fontEngine, ti.underlineStyle, ti.flags, ti.width.toReal(),
                           ti.charFormat);

    if (d->state->renderHints != oldRenderHints) {
        d->state->renderHints = oldRenderHints;
        if (d->extended)
            d->extended->renderHintsChanged();
        else
            d->state->dirtyFlags |= QPaintEngine::DirtyHints;
    }
}

/*!
    \fn QRectF QPainter::boundingRect(const QRectF &rectangle, int flags, const QString &text)

    Returns the bounding rectangle of the \a text as it will appear
    when drawn inside the given \a rectangle with the specified \a
    flags using the currently set font(); i.e the function tells you
    where the drawText() function will draw when given the same
    arguments.

    If the \a text does not fit within the given \a rectangle using
    the specified \a flags, the function returns the required
    rectangle.

    The \a flags argument is a bitwise OR of the following flags:
    \list
         \o Qt::AlignLeft
         \o Qt::AlignRight
         \o Qt::AlignHCenter
         \o Qt::AlignTop
         \o Qt::AlignBottom
         \o Qt::AlignVCenter
         \o Qt::AlignCenter
         \o Qt::TextSingleLine
         \o Qt::TextShowMnemonic
         \o Qt::TextWordWrap
         \o Qt::TextIncludeTrailingSpaces
    \endlist
    If several of the horizontal or several of the vertical alignment
    flags are set, the resulting alignment is undefined.

    \sa drawText(), Qt::Alignment, Qt::TextFlag
*/

/*!
    \fn QRect QPainter::boundingRect(const QRect &rectangle, int flags,
                                     const QString &text)

    \overload

    Returns the bounding rectangle of the \a text as it will appear
    when drawn inside the given \a rectangle with the specified \a
    flags using the currently set font().
*/

/*!
    \fn QRect QPainter::boundingRect(int x, int y, int w, int h, int flags,
                                     const QString &text);

    \overload

    Returns the bounding rectangle of the given \a text as it will
    appear when drawn inside the rectangle beginning at the point
    (\a{x}, \a{y}) with width \a w and height \a h.
*/
QRect QPainter::boundingRect(const QRect &rect, int flags, const QString &str)
{
    if (str.isEmpty())
        return QRect(rect.x(),rect.y(), 0,0);
    QRect brect;
    drawText(rect, flags | Qt::TextDontPrint, str, &brect);
    return brect;
}



QRectF QPainter::boundingRect(const QRectF &rect, int flags, const QString &str)
{
    if (str.isEmpty())
        return QRectF(rect.x(),rect.y(), 0,0);
    QRectF brect;
    drawText(rect, flags | Qt::TextDontPrint, str, &brect);
    return brect;
}

/*!
    \fn QRectF QPainter::boundingRect(const QRectF &rectangle,
        const QString &text, const QTextOption &option)

    \overload

    Instead of specifying flags as a bitwise OR of the
    Qt::AlignmentFlag and Qt::TextFlag, this overloaded function takes
    an \a option argument. The QTextOption class provides a
    description of general rich text properties.

    \sa QTextOption
*/
QRectF QPainter::boundingRect(const QRectF &r, const QString &text, const QTextOption &o)
{
    Q_D(QPainter);

    if (!d->engine || text.length() == 0)
        return QRectF(r.x(),r.y(), 0,0);

    QRectF br;
    qt_format_text(d->state->font, r, Qt::TextDontPrint, &o, text, &br, this);
    return br;
}

/*!
    \fn void QPainter::drawTiledPixmap(const QRectF &rectangle, const QPixmap &pixmap, const QPointF &position)

    Draws a tiled \a pixmap, inside the given \a rectangle with its
    origin at the given \a position.

    Calling drawTiledPixmap() is similar to calling drawPixmap()
    several times to fill (tile) an area with a pixmap, but is
    potentially much more efficient depending on the underlying window
    system.

    \sa drawPixmap()
*/
void QPainter::drawTiledPixmap(const QRectF &r, const QPixmap &pixmap, const QPointF &sp)
{
#ifdef QT_DEBUG_DRAW
    printf("QPainter::drawTiledPixmap(), target=[%.2f,%.2f,%.2f,%.2f], pix=[%d,%d], offset=[%.2f,%.2f]\n",
           r.x(), r.y(), r.width(), r.height(),
           pixmap.width(), pixmap.height(),
           sp.x(), sp.y());
#endif

    Q_D(QPainter);
    if (!d->engine || pixmap.isNull() || r.isEmpty())
        return;

#ifndef QT_NO_DEBUG
    qt_painter_thread_test(d->device->devType(), "drawTiledPixmap()", true);
#endif

    qreal sw = pixmap.width();
    qreal sh = pixmap.height();
    qreal sx = sp.x();
    qreal sy = sp.y();
    if (sx < 0)
        sx = qRound(sw) - qRound(-sx) % qRound(sw);
    else
        sx = qRound(sx) % qRound(sw);
    if (sy < 0)
        sy = qRound(sh) - -qRound(sy) % qRound(sh);
    else
        sy = qRound(sy) % qRound(sh);


    if (d->extended) {
        d->extended->drawTiledPixmap(r, pixmap, QPointF(sx, sy));
        return;
    }

    if (d->state->bgMode == Qt::OpaqueMode && pixmap.isQBitmap())
        fillRect(r, d->state->bgBrush);

    d->updateState(d->state);
    if ((d->state->matrix.type() > QTransform::TxTranslate
        && !d->engine->hasFeature(QPaintEngine::PixmapTransform))
        || (d->state->opacity != 1.0 && !d->engine->hasFeature(QPaintEngine::ConstantOpacity)))
    {
        save();
        setBackgroundMode(Qt::TransparentMode);
        setBrush(QBrush(d->state->pen.color(), pixmap));
        setPen(Qt::NoPen);

        // If there is no rotation involved we have to make sure we use the
        // antialiased and not the aliased coordinate system by rounding the coordinates.
        if (d->state->matrix.type() <= QTransform::TxScale) {
            const QPointF p = roundInDeviceCoordinates(r.topLeft(), d->state->matrix);

            if (d->state->matrix.type() <= QTransform::TxTranslate) {
                sx = qRound(sx);
                sy = qRound(sy);
            }

            setBrushOrigin(QPointF(r.x()-sx, r.y()-sy));
            drawRect(QRectF(p, r.size()));
        } else {
            setBrushOrigin(QPointF(r.x()-sx, r.y()-sy));
            drawRect(r);
        }
        restore();
        return;
    }

    qreal x = r.x();
    qreal y = r.y();
    if (d->state->matrix.type() == QTransform::TxTranslate
        && !d->engine->hasFeature(QPaintEngine::PixmapTransform)) {
        x += d->state->matrix.dx();
        y += d->state->matrix.dy();
    }

    d->engine->drawTiledPixmap(QRectF(x, y, r.width(), r.height()), pixmap, QPointF(sx, sy));
}

/*!
    \fn QPainter::drawTiledPixmap(const QRect &rectangle, const QPixmap &pixmap,
                                  const QPoint &position = QPoint())
    \overload

    Draws a tiled \a pixmap, inside the given \a rectangle with its
    origin at the given \a position.
*/

/*!
    \fn void QPainter::drawTiledPixmap(int x, int y, int width, int height, const
         QPixmap &pixmap, int sx, int sy);
    \overload

    Draws a tiled \a pixmap in the specified rectangle.

    (\a{x}, \a{y}) specifies the top-left point in the paint device
    that is to be drawn onto; with the given \a width and \a
    height. (\a{sx}, \a{sy}) specifies the top-left point in the \a
    pixmap that is to be drawn; this defaults to (0, 0).
*/

/*!
    \fn void QPainter::eraseRect(const QRectF &rectangle)

    Erases the area inside the given \a rectangle. Equivalent to
    calling
    \snippet doc/src/snippets/code/src_gui_painting_qpainter.cpp 19

    \sa fillRect()
*/
void QPainter::eraseRect(const QRectF &r)
{
    Q_D(QPainter);

    fillRect(r, d->state->bgBrush);
}

static inline bool needsResolving(const QBrush &brush)
{
    Qt::BrushStyle s = brush.style();
    return ((s == Qt::LinearGradientPattern || s == Qt::RadialGradientPattern) &&
            brush.gradient()->coordinateMode() == QGradient::ObjectBoundingMode);
}

/*!
    \fn void QPainter::eraseRect(const QRect &rectangle)
    \overload

    Erases the area inside the given  \a rectangle.
*/

/*!
    \fn void QPainter::eraseRect(int x, int y, int width, int height)
    \overload

    Erases the area inside the rectangle beginning at (\a x, \a y)
    with the given \a width and \a height.
*/


/*!
    \fn void QPainter::fillRect(int x, int y, int width, int height, Qt::BrushStyle style)
    \overload

    Fills the rectangle beginning at (\a{x}, \a{y}) with the given \a
    width and \a height, using the brush \a style specified.

    \since 4.5
*/

/*!
    \fn void QPainter::fillRect(const QRect &rectangle, Qt::BrushStyle style)
    \overload

    Fills the given \a rectangle  with the brush \a style specified.

    \since 4.5
*/

/*!
    \fn void QPainter::fillRect(const QRectF &rectangle, Qt::BrushStyle style)
    \overload

    Fills the given \a rectangle  with the brush \a style specified.

    \since 4.5
*/

/*!
    \fn void QPainter::fillRect(const QRectF &rectangle, const QBrush &brush)

    Fills the given \a rectangle  with the \a brush specified.

    Alternatively, you can specify a QColor instead of a QBrush; the
    QBrush constructor (taking a QColor argument) will automatically
    create a solid pattern brush.

    \sa drawRect()
*/
void QPainter::fillRect(const QRectF &r, const QBrush &brush)
{
    Q_D(QPainter);

    if (!d->engine)
        return;

    if (d->extended) {
        const QGradient *g = brush.gradient();
        if (!g || g->coordinateMode() == QGradient::LogicalMode) {
            d->extended->fillRect(r, brush);
            return;
        }
    }

    QPen oldPen = pen();
    QBrush oldBrush = this->brush();
    setPen(Qt::NoPen);
    setBrush(brush);
    drawRect(r);
    setBrush(oldBrush);
    setPen(oldPen);
}

/*!
    \fn void QPainter::fillRect(const QRect &rectangle, const QBrush &brush)
    \overload

    Fills the given \a rectangle with the specified \a brush.
*/

void QPainter::fillRect(const QRect &r, const QBrush &brush)
{
    Q_D(QPainter);

    if (!d->engine)
        return;

    if (d->extended) {
        const QGradient *g = brush.gradient();
        if (!g || g->coordinateMode() == QGradient::LogicalMode) {
            d->extended->fillRect(r, brush);
            return;
        }
    }

    QPen oldPen = pen();
    QBrush oldBrush = this->brush();
    setPen(Qt::NoPen);
    setBrush(brush);
    drawRect(r);
    setBrush(oldBrush);
    setPen(oldPen);
}

/*!
    \fn void QPainter::fillRect(const QRect &rectangle, const QColor &color)
    \overload

    Fills the given \a rectangle with the \a color specified.

    \since 4.5
*/
void QPainter::fillRect(const QRect &r, const QColor &color)
{
    Q_D(QPainter);

    if (!d->engine)
        return;

    if (d->extended) {
        d->extended->fillRect(r, color);
        return;
    }

    fillRect(r, QBrush(color));
}


/*!
    \fn void QPainter::fillRect(const QRectF &rectangle, const QColor &color)
    \overload

    Fills the given \a rectangle with the \a color specified.

    \since 4.5
*/
void QPainter::fillRect(const QRectF &r, const QColor &color)
{
    Q_D(QPainter);

    if (!d->engine)
        return;

    if (d->extended) {
        d->extended->fillRect(r, color);
        return;
    }

    fillRect(r, QBrush(color));
}

/*!
    \fn void QPainter::fillRect(int x, int y, int width, int height, const QBrush &brush)

    \overload

    Fills the rectangle beginning at (\a{x}, \a{y}) with the given \a
    width and \a height, using the given \a brush.
*/

/*!
    \fn void QPainter::fillRect(int x, int y, int width, int height, const QColor &color)

    \overload

    Fills the rectangle beginning at (\a{x}, \a{y}) with the given \a
    width and \a height, using the given \a color.

    \since 4.5
*/

/*!
    \fn void QPainter::fillRect(int x, int y, int width, int height, Qt::GlobalColor color)

    \overload

    Fills the rectangle beginning at (\a{x}, \a{y}) with the given \a
    width and \a height, using the given \a color.

    \since 4.5
*/

/*!
    \fn void QPainter::fillRect(const QRect &rectangle, Qt::GlobalColor color);

    \overload

    Fills the given \a rectangle with the specified \a color.

    \since 4.5
*/

/*!
    \fn void QPainter::fillRect(const QRectF &rectangle, Qt::GlobalColor color);

    \overload

    Fills the given \a rectangle with the specified \a color.

    \since 4.5
*/

/*!
    Sets the given render \a hint on the painter if \a on is true;
    otherwise clears the render hint.

    \sa setRenderHints(), renderHints(), {QPainter#Rendering
    Quality}{Rendering Quality}
*/
void QPainter::setRenderHint(RenderHint hint, bool on)
{
    // ### possibly inline
    setRenderHints(hint, on);
}

/*!
    \since 4.2

    Sets the given render \a hints on the painter if \a on is true;
    otherwise clears the render hints.

    \sa setRenderHint(), renderHints(), {QPainter#Rendering
    Quality}{Rendering Quality}
*/

void QPainter::setRenderHints(RenderHints hints, bool on)
{
    Q_D(QPainter);

    if (Q_UNLIKELY(!d->engine)) {
        qWarning("QPainter::setRenderHint: Painter must be active to set rendering hints");
        return;
    }

    if (on)
        d->state->renderHints |= hints;
    else
        d->state->renderHints &= ~hints;

    if (d->extended)
        d->extended->renderHintsChanged();
    else
        d->state->dirtyFlags |= QPaintEngine::DirtyHints;
}

/*!
    Returns a flag that specifies the rendering hints that are set for
    this painter.

    \sa testRenderHint(), {QPainter#Rendering Quality}{Rendering Quality}
*/
QPainter::RenderHints QPainter::renderHints() const
{
    Q_D(const QPainter);

    if (!d->engine)
        return 0;

    return d->state->renderHints;
}

/*!
    \fn bool QPainter::testRenderHint(RenderHint hint) const
    \since 4.3

    Returns true if \a hint is set; otherwise returns false.

    \sa renderHints(), setRenderHint()
*/

/*!
    Returns true if view transformation is enabled; otherwise returns
    false.

    \sa setViewTransformEnabled(), worldTransform()
*/

bool QPainter::viewTransformEnabled() const
{
    Q_D(const QPainter);
    if (Q_UNLIKELY(!d->engine)) {
        qWarning("QPainter::viewTransformEnabled: Painter not active");
        return false;
    }
    return d->state->VxF;
}

/*!
    \fn void QPainter::setWindow(const QRect &rectangle)

    Sets the painter's window to the given \a rectangle, and enables
    view transformations.

    The window rectangle is part of the view transformation. The
    window specifies the logical coordinate system. Its sister, the
    viewport(), specifies the device coordinate system.

    The default window rectangle is the same as the device's
    rectangle.

    \sa window(), viewTransformEnabled(), {Coordinate
    System#Window-Viewport Conversion}{Window-Viewport Conversion}
*/

/*!
    \fn void QPainter::setWindow(int x, int y, int width, int height)
    \overload

    Sets the painter's window to the rectangle beginning at (\a x, \a
    y) and the given \a width and \a height.
*/

void QPainter::setWindow(const QRect &r)
{
#ifdef QT_DEBUG_DRAW
    printf("QPainter::setWindow(), [%d,%d,%d,%d]\n", r.x(), r.y(), r.width(), r.height());
#endif

    Q_D(QPainter);

    if (Q_UNLIKELY(!d->engine)) {
        qWarning("QPainter::setWindow: Painter not active");
        return;
    }

    d->state->wx = r.x();
    d->state->wy = r.y();
    d->state->ww = r.width();
    d->state->wh = r.height();

    d->state->VxF = true;
    d->updateMatrix();
}

/*!
    Returns the window rectangle.

    \sa setWindow(), setViewTransformEnabled()
*/

QRect QPainter::window() const
{
    Q_D(const QPainter);
    if (Q_UNLIKELY(!d->engine)) {
        qWarning("QPainter::window: Painter not active");
        return QRect();
    }
    return QRect(d->state->wx, d->state->wy, d->state->ww, d->state->wh);
}

/*!
    \fn void QPainter::setViewport(const QRect &rectangle)

    Sets the painter's viewport rectangle to the given \a rectangle,
    and enables view transformations.

    The viewport rectangle is part of the view transformation. The
    viewport specifies the device coordinate system. Its sister, the
    window(), specifies the logical coordinate system.

    The default viewport rectangle is the same as the device's
    rectangle.

    \sa viewport(), viewTransformEnabled() {Coordinate
    System#Window-Viewport Conversion}{Window-Viewport Conversion}
*/

/*!
    \fn void QPainter::setViewport(int x, int y, int width, int height)
    \overload

    Sets the painter's viewport rectangle to be the rectangle
    beginning at (\a x, \a y) with the given \a width and \a height.
*/

void QPainter::setViewport(const QRect &r)
{
#ifdef QT_DEBUG_DRAW
    printf("QPainter::setViewport(), [%d,%d,%d,%d]\n", r.x(), r.y(), r.width(), r.height());
#endif

    Q_D(QPainter);

    if (Q_UNLIKELY(!d->engine)) {
        qWarning("QPainter::setViewport: Painter not active");
        return;
    }

    d->state->vx = r.x();
    d->state->vy = r.y();
    d->state->vw = r.width();
    d->state->vh = r.height();

    d->state->VxF = true;
    d->updateMatrix();
}

/*!
    Returns the viewport rectangle.

    \sa setViewport(), setViewTransformEnabled()
*/

QRect QPainter::viewport() const
{
    Q_D(const QPainter);
    if (Q_UNLIKELY(!d->engine)) {
        qWarning("QPainter::viewport: Painter not active");
        return QRect();
    }
    return QRect(d->state->vx, d->state->vy, d->state->vw, d->state->vh);
}

/*! \fn bool QPainter::hasViewXForm() const
    \compat

    Use viewTransformEnabled() instead.
*/

/*! \fn bool QPainter::hasWorldXForm() const
    \compat

    Use worldMatrixEnabled() instead.
*/

/*! \fn void QPainter::resetXForm()
    \compat

    Use resetTransform() instead.
*/

/*! \fn void QPainter::setViewXForm(bool enabled)
    \compat

    Use setViewTransformEnabled() instead.
*/

/*! \fn void QPainter::setWorldXForm(bool enabled)
    \compat

    Use setWorldMatrixEnabled() instead.
*/
/*!
    Enables view transformations if \a enable is true, or disables
    view transformations if \a enable is false.

    \sa viewTransformEnabled(), {Coordinate System#Window-Viewport
    Conversion}{Window-Viewport Conversion}
*/

void QPainter::setViewTransformEnabled(bool enable)
{
#ifdef QT_DEBUG_DRAW
    printf("QPainter::setViewTransformEnabled(), enable=%d\n", enable);
#endif

    Q_D(QPainter);

    if (Q_UNLIKELY(!d->engine)) {
        qWarning("QPainter::setViewTransformEnabled: Painter not active");
        return;
    }

    if (enable == d->state->VxF)
        return;

    d->state->VxF = enable;
    d->updateMatrix();
}

void qt_format_text(const QFont &fnt, const QRectF &_r,
                    int tf, const QString& str, QRectF *brect)
{
    qt_format_text(fnt, _r, tf, nullptr, str, brect, nullptr);
}
void qt_format_text(const QFont &fnt, const QRectF &_r,
                    int tf, const QTextOption *option, const QString& str, QRectF *brect,
                    QPainter *painter)
{
    Q_ASSERT(!((tf & ~Qt::TextDontPrint) != 0 && option != 0) ); // we either have an option or flags

    if (option) {
        tf |= option->alignment();
        if (option->wrapMode() != QTextOption::NoWrap)
            tf |= Qt::TextWordWrap;

        if (option->flags() & QTextOption::IncludeTrailingSpaces)
            tf |= Qt::TextIncludeTrailingSpaces;
    }

    // we need to copy r here to protect against the case (&r == brect).
    QRectF r(_r);

    bool dontclip  = (tf & Qt::TextDontClip);
    bool wordwrap  = (tf & Qt::TextWordWrap) || (tf & Qt::TextWrapAnywhere);
    bool singleline = (tf & Qt::TextSingleLine);
    bool showmnemonic = (tf & Qt::TextShowMnemonic);
    bool hidemnmemonic = (tf & Qt::TextHideMnemonic);

    Qt::LayoutDirection layout_direction;
    if (tf & Qt::TextForceLeftToRight)
        layout_direction = Qt::LeftToRight;
    else if (tf & Qt::TextForceRightToLeft)
        layout_direction = Qt::RightToLeft;
    else if (option)
        layout_direction = option->textDirection();
    else if (painter)
        layout_direction = painter->layoutDirection();
    else
        layout_direction = Qt::LeftToRight;

    tf = QStyle::visualAlignment(layout_direction, QFlag(tf));

    if (!painter)
        tf |= Qt::TextDontPrint;

    uint maxUnderlines = 0;
    int numUnderlines = 0;
    QStdVector<int> underlinePositions(1);

    QFontMetricsF fm(fnt);
    QString text = str;
    int offset = 0;
start_lengthVariant:
    bool hasMoreLengthVariants = false;
    int old_offset = offset;
    for (; offset < text.length(); offset++) {
        QChar chr = text.at(offset);
        // replace tabs with space for compatibility
        if (chr == QLatin1Char('\t')) {
            text[offset] = QLatin1Char(' ');
        } else if (chr == QLatin1Char('\r') || (singleline && chr == QLatin1Char('\n'))) {
            text[offset] = QLatin1Char(' ');
        } else if (chr == QLatin1Char('\n')) {
            text[offset] = QChar::LineSeparator;
        } else if (chr == QLatin1Char('&')) {
            ++maxUnderlines;
        } else if (chr == QChar(ushort(0x9c))) {
            // string with multiple length variants
            hasMoreLengthVariants = true;
            break;
        }
    }

    int length = offset - old_offset;
    if ((hidemnmemonic || showmnemonic) && maxUnderlines > 0) {
        underlinePositions.resize(maxUnderlines + 1);

        QChar *cout = text.data() + old_offset;
        QChar *cin = cout;
        int l = length;
        while (l) {
            if (*cin == QLatin1Char('&')) {
                ++cin;
                --length;
                --l;
                if (!l)
                    break;
                if (*cin != QLatin1Char('&') && !hidemnmemonic)
                    underlinePositions[numUnderlines++] = cout - text.data() - old_offset;
            }
            *cout = *cin;
            ++cout;
            ++cin;
            --l;
        }
    }

    // no need to do extra work for underlines if we don't paint
    if (tf & Qt::TextDontPrint)
        numUnderlines = 0;

    underlinePositions[numUnderlines] = -1;
    qreal height = 0;
    qreal width = 0;

    QString finalText = text.mid(old_offset, length);

    QTextOption textoption;
    if (option) {
        textoption = *option;
    }

    textoption.setTextDirection(layout_direction);
    if (tf & Qt::AlignJustify)
        textoption.setAlignment(Qt::AlignJustify);
    else
        textoption.setAlignment(Qt::AlignLeft); // do not do alignment twice

    if (!option && (tf & Qt::TextWrapAnywhere))
        textoption.setWrapMode(QTextOption::WrapAnywhere);

    QList<QTextLayout::FormatRange> formatoverrides;
    for (int i = 0; i < underlinePositions.size(); i++) {
        QTextLayout::FormatRange formatoverride;
        formatoverride.start = underlinePositions[i];
        formatoverride.length = 1;
        formatoverride.format.setFontUnderline(true);
        formatoverrides.append(formatoverride);
    }

    QTextLayout textLayout(finalText, fnt);
    textLayout.setCacheEnabled(true);
    textLayout.setTextOption(textoption);
    textLayout.setAdditionalFormats(formatoverrides);
    textLayout.setFlags(tf);

    if (finalText.isEmpty()) {
        height = fm.height();
        width = 0;
        tf |= Qt::TextDontPrint;
    } else {
        qreal lineWidth = 0x01000000;
        if (wordwrap)
            lineWidth = qMax<qreal>(0, r.width());
        if(!wordwrap)
            tf |= Qt::TextIncludeTrailingSpaces;
        textLayout.beginLayout();

        qreal leading = fm.leading();
        height = -leading;

        while (1) {
            QTextLine l = textLayout.createLine();
            if (!l.isValid())
                break;

            l.setLineWidth(lineWidth);
            height += leading;
            l.setPosition(QPointF(0., height));
            height += l.height();
            width = qMax(width, l.naturalTextWidth());
            if (!dontclip && !brect && height >= r.height())
                break;
        }
        textLayout.endLayout();
    }

    qreal yoff = 0;
    qreal xoff = 0;
    if (tf & Qt::AlignBottom) {
        yoff = r.height() - height;
    } else if (tf & Qt::AlignVCenter) {
        yoff = (r.height() - height)/2;
        if (painter) {
            QTransform::TransformationType type = painter->transform().type();
            if (type <= QTransform::TxScale) {
                // do the rounding manually to work around inconsistencies
                // in the paint engines when drawing on floating point offsets
                const qreal scale = painter->transform().m22();
                if (scale != 0)
                    yoff = -qRound(-yoff * scale) / scale;
            }
        }
    }
    if (tf & Qt::AlignRight) {
        xoff = r.width() - width;
    } else if (tf & Qt::AlignHCenter) {
        xoff = (r.width() - width)/2;
        if (painter) {
            QTransform::TransformationType type = painter->transform().type();
            if (type <= QTransform::TxScale) {
                // do the rounding manually to work around inconsistencies
                // in the paint engines when drawing on floating point offsets
                const qreal scale = painter->transform().m11();
                if (scale != 0)
                    xoff = qRound(xoff * scale) / scale;
            }
        }
    }
    QRectF bounds = QRectF(r.x() + xoff, r.y() + yoff, width, height);

    if (hasMoreLengthVariants && !(tf & Qt::TextLongestVariant) && !r.contains(bounds)) {
        offset++;
        goto start_lengthVariant;
    }
    if (brect)
        *brect = bounds;

    if (!(tf & Qt::TextDontPrint)) {
        bool restore = false;
        if (!dontclip && !r.contains(bounds)) {
            restore = true;
            painter->save();
            painter->setClipRect(r, Qt::IntersectClip);
        }

        for (int i = 0; i < textLayout.lineCount(); i++) {
            QTextLine line = textLayout.lineAt(i);

            qreal advance = line.horizontalAdvance();
            xoff = 0;
            if (tf & Qt::AlignRight) {
                xoff = r.width() - advance;
            }
            else if (tf & Qt::AlignHCenter)
                xoff = (r.width() - advance) / 2;

            line.draw(painter, QPointF(r.x() + xoff, r.y() + yoff));
        }

        if (restore) {
            painter->restore();
        }
    }
}

/*!
    Sets the layout direction used by the painter when drawing text,
    to the specified \a direction.

    The default is Qt::LayoutDirectionAuto, which will implicitly determine the
    direction from the text drawn.

    \sa QTextOption::setTextDirection(), layoutDirection(), drawText(), {QPainter#Settings}{Settings}
*/
void QPainter::setLayoutDirection(Qt::LayoutDirection direction)
{
    Q_D(QPainter);
    if (d->state)
        d->state->layoutDirection = direction;
}

/*!
    Returns the layout direction used by the painter when drawing text.

    \sa QTextOption::textDirection(), setLayoutDirection(), drawText(), {QPainter#Settings}{Settings}
*/
Qt::LayoutDirection QPainter::layoutDirection() const
{
    Q_D(const QPainter);
    return d->state ? d->state->layoutDirection : Qt::LayoutDirectionAuto;
}

QPainterState::QPainterState(const QPainterState *s)
    : brushOrigin(s->brushOrigin), font(s->font), deviceFont(s->deviceFont),
      pen(s->pen), brush(s->brush), bgBrush(s->bgBrush),
      clipRegion(s->clipRegion), clipPath(s->clipPath),
      clipOperation(s->clipOperation),
      renderHints(s->renderHints), clipInfo(s->clipInfo),
      worldMatrix(s->worldMatrix), matrix(s->matrix), redirectionMatrix(s->redirectionMatrix),
      wx(s->wx), wy(s->wy), ww(s->ww), wh(s->wh),
      vx(s->vx), vy(s->vy), vw(s->vw), vh(s->vh),
      opacity(s->opacity), WxF(s->WxF), VxF(s->VxF),
      clipEnabled(s->clipEnabled),
      changeFlags(0),
      bgMode(s->bgMode), painter(s->painter),
      layoutDirection(s->layoutDirection),
      composition_mode(s->composition_mode)
{
    dirtyFlags = s->dirtyFlags;
}

QPainterState::QPainterState()
    : brushOrigin(0, 0), bgBrush(Qt::white), clipOperation(Qt::NoClip),
      renderHints(0),
      wx(0), wy(0), ww(0), wh(0), vx(0), vy(0), vw(0), vh(0),
      opacity(1), WxF(false), VxF(false), clipEnabled(true),
      changeFlags(0),
      bgMode(Qt::TransparentMode), painter(0),
      layoutDirection(QApplication::layoutDirection()),
      composition_mode(QPainter::CompositionMode_SourceOver)
{
    dirtyFlags = 0;
}

QPainterState::~QPainterState()
{
}

/*!
    \fn void QPainter::setBackgroundColor(const QColor &color)

    Use setBackground() instead.
*/

/*!
    \fn const QColor &QPainter::backgroundColor() const

    Use background() and QBrush::color() instead.

    \oldcode
        QColor myColor = backgroundColor();
    \newcode
        QColor myColor = background().color();
    \endcode

    Note that the background can be a complex brush such as a texture
    or a gradient.
*/

/*!
    \fn void QPainter::drawText(int x, int y, const QString &text, int pos, int length)
    \compat

    Use drawText() combined with QString::mid() instead.

    \oldcode
        QPainter painter(this);
        painter.drawText(x, y, text, pos, length);
    \newcode
        QPainter painter(this);
        painter.drawText(x, y, text.mid(pos, length));
    \endcode
*/

/*!
    \fn void QPainter::drawText(const QPoint &point, const QString &text, int pos, int length)
    \compat

    Use drawText() combined with QString::mid() instead.

    \oldcode
        QPainter painter(this);
        painter.drawText(point, text, pos, length);
    \newcode
        QPainter painter(this);
        painter.drawText(point, text.mid(pos, length));
    \endcode
*/

/*!
    \fn void QPainter::drawText(int x, int y, const QString &text, int length)
    \compat

    Use drawText() combined with QString::left() instead.

    \oldcode
        QPainter painter(this);
        painter.drawText(x, y, text, length);
    \newcode
        QPainter painter(this);
        painter.drawText(x, y, text.left(length));
    \endcode
*/

/*!
    \fn void QPainter::drawText(const QPoint &point, const QString &text, int length)
    \compat

    Use drawText() combined with QString::left() instead.

    \oldcode
        QPainter painter(this);
        painter.drawText(point, text, length);
    \newcode
        QPainter painter(this);
        painter.drawText(point, text.left(length));
    \endcode
*/

/*!
    \fn bool QPainter::begin(QPaintDevice *device, const QWidget *init)
    \compat

    Use begin() instead.

    If the paint \a device is a QWidget, QPainter is initialized after
    the widget's settings automatically. Otherwise, you must call the
    initFrom() function to initialize the painters pen, background and
    font to the same as any given widget.

    \oldcode
        QPainter painter(this);
        painter.begin(device, init);
    \newcode
        QPainter painter(this);
        painter.begin(device);
        painter.initFrom(init);
    \endcode
*/

/*!
    \fn void QPainter::drawImage(const QRectF &target, const QImage &image, const QRectF &source,
                         Qt::ImageConversionFlags flags)

    Draws the rectangular portion \a source of the given \a image
    into the \a target rectangle in the paint device.

    \note The image is scaled to fit the rectangle, if both the image and rectangle size disagree.

    If the image needs to be modified to fit in a lower-resolution
    result (e.g. converting from 32-bit to 8-bit), use the \a flags to
    specify how you would prefer this to happen.

    \table 100%
    \row
    \o
    \snippet doc/src/snippets/code/src_gui_painting_qpainter.cpp 20
    \endtable

    \sa drawPixmap()
*/

/*!
    \fn void QPainter::drawImage(const QRect &target, const QImage &image, const QRect &source,
                                 Qt::ImageConversionFlags flags)
    \overload

    Draws the rectangular portion \a source of the given \a image
    into the \a target rectangle in the paint device.

    \note The image is scaled to fit the rectangle, if both the image and rectangle size disagree.
*/

/*!
    \fn void QPainter::drawImage(const QPointF &point, const QImage &image)

    \overload

    Draws the given \a image at the given \a point.
*/

/*!
    \fn void QPainter::drawImage(const QPoint &point, const QImage &image)

    \overload

    Draws the given \a image at the given \a point.
*/

/*!
    \fn void QPainter::drawImage(const QPointF &point, const QImage &image, const QRectF &source,
                                 Qt::ImageConversionFlags flags = 0)

    \overload

    Draws the rectangular portion \a source of the given \a image with
    its origin at the given \a point.
*/

/*!
    \fn void QPainter::drawImage(const QPoint &point, const QImage &image, const QRect &source,
                                 Qt::ImageConversionFlags flags = 0)
    \overload

    Draws the rectangular portion \a source of the given \a image with
    its origin at the given \a point.
*/

/*!
    \fn void QPainter::drawImage(const QRectF &rectangle, const QImage &image)

    \overload

    Draws the given \a image into the given \a rectangle.

    \note The image is scaled to fit the rectangle, if both the image and rectangle size disagree.
*/

/*!
    \fn void QPainter::drawImage(const QRect &rectangle, const QImage &image)

    \overload

    Draws the given \a image into the given \a rectangle.

   \note The image is scaled to fit the rectangle, if both the image and rectangle size disagree.
*/

/*!
    \fn void QPainter::drawImage(int x, int y, const QImage &image,
                                 int sx, int sy, int sw, int sh,
                                 Qt::ImageConversionFlags flags)
    \overload

    Draws an image at (\a{x}, \a{y}) by copying a part of \a image into
    the paint device.

    (\a{x}, \a{y}) specifies the top-left point in the paint device that is
    to be drawn onto. (\a{sx}, \a{sy}) specifies the top-left point in \a
    image that is to be drawn. The default is (0, 0).

    (\a{sw}, \a{sh}) specifies the size of the image that is to be drawn.
    The default, (0, 0) (and negative) means all the way to the
    bottom-right of the image.
*/

/*!
    \fn void QPainter::redirect(QPaintDevice *pdev, QPaintDevice *replacement)

    Use setRedirected() instead.
*/

/*!
    \fn QPaintDevice *QPainter::redirect(QPaintDevice *pdev)

    Use redirected() instead.
*/

/*!
    \fn QRect QPainter::boundingRect(const QRect &rectangle, int flags,
                                     const QString &text, int length)
    \compat

    Returns the bounding rectangle for the given \a length of the \a
    text constrained by the provided \a rectangle.

    Use boundingRect() combined with QString::left() instead.

    \oldcode
        QRect rectangle = boundingRect(rect, flags, text, length);
    \newcode
        QRect rectangle = boundingRect(rect, flags, text.left(length));
    \endcode
*/

/*!
    \fn void QPainter::drawText(const QRect &rectangle, int flags, const QString &text,
                                int length, QRect *br)
    \compat

    Use drawText() combined with QString::left() instead.

    \oldcode
        QPainter painter(this);
        painter.drawText(rectangle, flags, text, length, br );
    \newcode
        QPainter painter(this);
        painter.drawText(rectangle, flags, text.left(length), br );
    \endcode
*/

/*!
    \fn QRect QPainter::boundingRect(int x, int y, int width, int height, int flags,
                                     const QString &text, int length);

    \compat

    Returns the bounding rectangle for the given \a length of the \a
    text constrained by the rectangle that begins at point (\a{x},
    \a{y}) with the given \a width and \a height.

    Use boundingRect() combined with QString::left() instead.

    \oldcode
        QRect rectangle = boundingRect(x, y, width, height, flags, text, length);
    \newcode
        QRect rectangle = boundingRect(x, y, width, height, flags, text.left(length));
    \endcode
*/

/*!
    \fn void QPainter::drawText(int x, int y, int width, int height, int flags,
                                const QString &text, int length, QRect *br)

    \compat

    Use drawText() combined with QString::left() instead.

    \oldcode
        QPainter painter(this);
        painter.drawText(x, y, width, height, flags, text, length, br );
    \newcode
        QPainter painter(this);
        painter.drawText(x, y, width, height, flags, text.left(length), br );
    \endcode
*/


/*!
    \class QPaintEngineState
    \since 4.1

    \brief The QPaintEngineState class provides information about the
    active paint engine's current state.
    \reentrant

    QPaintEngineState records which properties that have changed since
    the last time the paint engine was updated, as well as their
    current value.

    Which properties that have changed can at any time be retrieved
    using the state() function. This function returns an instance of
    the QPaintEngine::DirtyFlags type which stores an OR combination
    of QPaintEngine::DirtyFlag values. The QPaintEngine::DirtyFlag
    enum defines whether a property has changed since the last update
    or not.

    If a property is marked with a dirty flag, its current value can
    be retrieved using the corresponding get function:

    \target GetFunction

    \table
    \header \o Property Flag \o Current Property Value
    \row \o QPaintEngine::DirtyBackground \o backgroundBrush()
    \row \o QPaintEngine::DirtyBackgroundMode \o backgroundMode()
    \row \o QPaintEngine::DirtyBrush \o brush()
    \row \o QPaintEngine::DirtyBrushOrigin \o brushOrigin()
    \row \o QPaintEngine::DirtyClipRegion \e or QPaintEngine::DirtyClipPath
         \o clipOperation()
    \row \o QPaintEngine::DirtyClipPath \o clipPath()
    \row \o QPaintEngine::DirtyClipRegion \o clipRegion()
    \row \o QPaintEngine::DirtyCompositionMode \o compositionMode()
    \row \o QPaintEngine::DirtyFont \o font()
    \row \o QPaintEngine::DirtyTransform \o transform()
    \row \o QPaintEngine::DirtyClipEnabled \o isClipEnabled()
    \row \o QPaintEngine::DirtyPen \o pen()
    \row \o QPaintEngine::DirtyHints \o renderHints()
    \endtable

    The QPaintEngineState class also provide the painter() function
    which returns a pointer to the painter that is currently updating
    the paint engine.

    An instance of this class, representing the current state of the
    active paint engine, is passed as argument to the
    QPaintEngine::updateState() function. The only situation in which
    you will have to use this class directly is when implementing your
    own paint engine.

    \sa QPaintEngine
*/


/*!
    \fn QPaintEngine::DirtyFlags QPaintEngineState::state() const

    Returns a combination of flags identifying the set of properties
    that need to be updated when updating the paint engine's state
    (i.e. during a call to the QPaintEngine::updateState() function).

    \sa QPaintEngine::updateState()
*/


/*!
    Returns the pen in the current paint engine state.

    This variable should only be used when the state() returns a
    combination which includes the QPaintEngine::DirtyPen flag.

    \sa state(), QPaintEngine::updateState()
*/

QPen QPaintEngineState::pen() const
{
    return static_cast<const QPainterState *>(this)->pen;
}

/*!
    Returns the brush in the current paint engine state.

    This variable should only be used when the state() returns a
    combination which includes the QPaintEngine::DirtyBrush flag.

    \sa state(), QPaintEngine::updateState()
*/

QBrush QPaintEngineState::brush() const
{
    return static_cast<const QPainterState *>(this)->brush;
}

/*!
    Returns the brush origin in the current paint engine state.

    This variable should only be used when the state() returns a
    combination which includes the QPaintEngine::DirtyBrushOrigin flag.

    \sa state(), QPaintEngine::updateState()
*/

QPointF QPaintEngineState::brushOrigin() const
{
    return static_cast<const QPainterState *>(this)->brushOrigin;
}

/*!
    Returns the background brush in the current paint engine state.

    This variable should only be used when the state() returns a
    combination which includes the QPaintEngine::DirtyBackground flag.

    \sa state(), QPaintEngine::updateState()
*/

QBrush QPaintEngineState::backgroundBrush() const
{
    return static_cast<const QPainterState *>(this)->bgBrush;
}

/*!
    Returns the background mode in the current paint engine
    state.

    This variable should only be used when the state() returns a
    combination which includes the QPaintEngine::DirtyBackgroundMode flag.

    \sa state(), QPaintEngine::updateState()
*/

Qt::BGMode QPaintEngineState::backgroundMode() const
{
    return static_cast<const QPainterState *>(this)->bgMode;
}

/*!
    Returns the font in the current paint engine
    state.

    This variable should only be used when the state() returns a
    combination which includes the QPaintEngine::DirtyFont flag.

    \sa state(), QPaintEngine::updateState()
*/

QFont QPaintEngineState::font() const
{
    return static_cast<const QPainterState *>(this)->font;
}

/*!
    \since 4.2
    \obsolete

    Returns the matrix in the current paint engine
    state.

    \note It is advisable to use transform() instead of this function to
    preserve the properties of perspective transformations.

    This variable should only be used when the state() returns a
    combination which includes the QPaintEngine::DirtyTransform flag.

    \sa state(), QPaintEngine::updateState()
*/

QMatrix QPaintEngineState::matrix() const
{
    const QPainterState *st = static_cast<const QPainterState *>(this);

    return st->matrix.toAffine();
}

/*!
    \since 4.3

    Returns the matrix in the current paint engine state.

    This variable should only be used when the state() returns a
    combination which includes the QPaintEngine::DirtyTransform flag.

    \sa state(), QPaintEngine::updateState()
*/


QTransform QPaintEngineState::transform() const
{
    const QPainterState *st = static_cast<const QPainterState *>(this);

    return st->matrix;
}


/*!
    Returns the clip operation in the current paint engine
    state.

    This variable should only be used when the state() returns a
    combination which includes either the QPaintEngine::DirtyClipPath
    or the QPaintEngine::DirtyClipRegion flag.

    \sa state(), QPaintEngine::updateState()
*/

Qt::ClipOperation QPaintEngineState::clipOperation() const
{
    return static_cast<const QPainterState *>(this)->clipOperation;
}

/*!
    \since 4.3

    Returns whether the coordinate of the fill have been specified
    as bounded by the current rendering operation and have to be
    resolved (about the currently rendered primitive).
*/
bool QPaintEngineState::brushNeedsResolving() const
{
    const QBrush &brush = static_cast<const QPainterState *>(this)->brush;
    return needsResolving(brush);
}


/*!
    \since 4.3

    Returns whether the coordinate of the stroke have been specified
    as bounded by the current rendering operation and have to be
    resolved (about the currently rendered primitive).
*/
bool QPaintEngineState::penNeedsResolving() const
{
    const QPen &pen = static_cast<const QPainterState *>(this)->pen;
    return needsResolving(pen.brush());
}

/*!
    Returns the clip region in the current paint engine state.

    This variable should only be used when the state() returns a
    combination which includes the QPaintEngine::DirtyClipRegion flag.

    \sa state(), QPaintEngine::updateState()
*/

QRegion QPaintEngineState::clipRegion() const
{
    return static_cast<const QPainterState *>(this)->clipRegion;
}

/*!
    Returns the clip path in the current paint engine state.

    This variable should only be used when the state() returns a
    combination which includes the QPaintEngine::DirtyClipPath flag.

    \sa state(), QPaintEngine::updateState()
*/

QPainterPath QPaintEngineState::clipPath() const
{
    return static_cast<const QPainterState *>(this)->clipPath;
}

/*!
    Returns whether clipping is enabled or not in the current paint
    engine state.

    This variable should only be used when the state() returns a
    combination which includes the QPaintEngine::DirtyClipEnabled
    flag.

    \sa state(), QPaintEngine::updateState()
*/

bool QPaintEngineState::isClipEnabled() const
{
    return static_cast<const QPainterState *>(this)->clipEnabled;
}

/*!
    Returns the render hints in the current paint engine state.

    This variable should only be used when the state() returns a
    combination which includes the QPaintEngine::DirtyHints
    flag.

    \sa state(), QPaintEngine::updateState()
*/

QPainter::RenderHints QPaintEngineState::renderHints() const
{
    return static_cast<const QPainterState *>(this)->renderHints;
}

/*!
    Returns the composition mode in the current paint engine state.

    This variable should only be used when the state() returns a
    combination which includes the QPaintEngine::DirtyCompositionMode
    flag.

    \sa state(), QPaintEngine::updateState()
*/

QPainter::CompositionMode QPaintEngineState::compositionMode() const
{
    return static_cast<const QPainterState *>(this)->composition_mode;
}


/*!
    Returns a pointer to the painter currently updating the paint
    engine.
*/

QPainter *QPaintEngineState::painter() const
{
    return static_cast<const QPainterState *>(this)->painter;
}


/*!
    \since 4.2

    Returns the opacity in the current paint engine state.
*/

qreal QPaintEngineState::opacity() const
{
    return static_cast<const QPainterState *>(this)->opacity;
}

/*!
    \since 4.3

    Sets the world transformation matrix.
    If \a combine is true, the specified \a transform is combined with
    the current matrix; otherwise it replaces the current matrix.

    \sa transform() setWorldTransform()
*/

void QPainter::setTransform(const QTransform &transform, bool combine )
{
    setWorldTransform(transform, combine);
}

/*!
    Returns the world transformation matrix.

    \sa worldTransform()
*/

const QTransform & QPainter::transform() const
{
    return worldTransform();
}


/*!
    Returns the matrix that transforms from logical coordinates to
    device coordinates of the platform dependent paint device.

    This function is \e only needed when using platform painting
    commands on the platform dependent handle (Qt::HANDLE), and the
    platform does not do transformations nativly.

    The QPaintEngine::PaintEngineFeature enum can be queried to
    determine whether the platform performs the transformations or
    not.

    \sa worldTransform(), QPaintEngine::hasFeature(),
*/

const QTransform & QPainter::deviceTransform() const
{
    Q_D(const QPainter);
    if (Q_UNLIKELY(!d->engine)) {
        qWarning("QPainter::deviceTransform: Painter not active");
        return d->fakeState()->transform;
    }
    return d->state->matrix;
}


/*!
    Resets any transformations that were made using translate(),
    scale(), shear(), rotate(), setWorldTransform(), setViewport()
    and setWindow().

    \sa {Coordinate Transformations}
*/

void QPainter::resetTransform()
{
     Q_D(QPainter);
#ifdef QT_DEBUG_DRAW
    printf("QPainter::resetMatrix()\n");
#endif
    if (Q_UNLIKELY(!d->engine)) {
        qWarning("QPainter::resetMatrix: Painter not active");
        return;
    }

    d->state->wx = d->state->wy = d->state->vx = d->state->vy = 0;                        // default view origins
    d->state->ww = d->state->vw = d->device->metric(QPaintDevice::PdmWidth);
    d->state->wh = d->state->vh = d->device->metric(QPaintDevice::PdmHeight);
    d->state->worldMatrix = QTransform();
    setMatrixEnabled(false);
    setViewTransformEnabled(false);
    if (d->extended)
        d->extended->transformChanged();
    else
        d->state->dirtyFlags |= QPaintEngine::DirtyTransform;
}

/*!
    Sets the world transformation matrix.
    If \a combine is true, the specified \a matrix is combined with the current matrix;
    otherwise it replaces the current matrix.

    \sa transform(), setTransform()
*/

void QPainter::setWorldTransform(const QTransform &matrix, bool combine )
{
    Q_D(QPainter);

    if (Q_UNLIKELY(!d->engine)) {
        qWarning("QPainter::setWorldTransform: Painter not active");
        return;
    }

    if (combine)
        d->state->worldMatrix = matrix * d->state->worldMatrix;                        // combines
    else
        d->state->worldMatrix = matrix;                                // set new matrix

    d->state->WxF = true;
    d->updateMatrix();
}

/*!
    Returns the world transformation matrix.
*/

const QTransform & QPainter::worldTransform() const
{
    Q_D(const QPainter);
    if (Q_UNLIKELY(!d->engine)) {
        qWarning("QPainter::worldTransform: Painter not active");
        return d->fakeState()->transform;
    }
    return d->state->worldMatrix;
}

/*!
    Returns the transformation matrix combining the current
    window/viewport and world transformation.

    \sa setWorldTransform(), setWindow(), setViewport()
*/

QTransform QPainter::combinedTransform() const
{
    Q_D(const QPainter);
    if (Q_UNLIKELY(!d->engine)) {
        qWarning("QPainter::combinedTransform: Painter not active");
        return QTransform();
    }
    return d->state->worldMatrix * d->viewTransform();
}

/*! \fn Display *QPaintDevice::x11Display() const
    Use QX11Info::display() instead.

    \oldcode
        Display *display = widget->x11Display();
    \newcode
        Display *display = QX11Info::display();
    \endcode

    \sa QWidget::x11Info(), QX11Info::display()
*/

/*! \fn int QPaintDevice::x11Screen() const
    Use QX11Info::screen() instead.

    \oldcode
        int screen = widget->x11Screen();
    \newcode
        int screen = widget->x11Info().screen();
    \endcode

    \sa QWidget::x11Info(), QPixmap::x11Info()
*/

/*! \fn void *QPaintDevice::x11Visual() const
    Use QX11Info::visual() instead.

    \oldcode
        void *visual = widget->x11Visual();
    \newcode
        void *visual = widget->x11Info().visual();
    \endcode

    \sa QWidget::x11Info(), QPixmap::x11Info()
*/

/*! \fn int QPaintDevice::x11Depth() const
    Use QX11Info::depth() instead.

    \oldcode
        int depth = widget->x11Depth();
    \newcode
        int depth = widget->x11Info().depth();
    \endcode

    \sa QWidget::x11Info(), QPixmap::x11Info()
*/

/*! \fn int QPaintDevice::x11Cells() const
    Use QX11Info::cells() instead.

    \oldcode
        int cells = widget->x11Cells();
    \newcode
        int cells = widget->x11Info().cells();
    \endcode

    \sa QWidget::x11Info(), QPixmap::x11Info()
*/

/*! \fn Qt::HANDLE QPaintDevice::x11Colormap() const
    Use QX11Info::colormap() instead.

    \oldcode
        unsigned long screen = widget->x11Colormap();
    \newcode
        unsigned long screen = widget->x11Info().colormap();
    \endcode

    \sa QWidget::x11Info(), QPixmap::x11Info()
*/

/*! \fn bool QPaintDevice::x11DefaultColormap() const
    Use QX11Info::defaultColormap() instead.

    \oldcode
        bool isDefault = widget->x11DefaultColormap();
    \newcode
        bool isDefault = widget->x11Info().defaultColormap();
    \endcode

    \sa QWidget::x11Info(), QPixmap::x11Info()
*/

/*! \fn bool QPaintDevice::x11DefaultVisual() const
    Use QX11Info::defaultVisual() instead.

    \oldcode
        bool isDefault = widget->x11DefaultVisual();
    \newcode
        bool isDefault = widget->x11Info().defaultVisual();
    \endcode

    \sa QWidget::x11Info(), QPixmap::x11Info()
*/

/*! \fn void *QPaintDevice::x11AppVisual(int screen)
    Use QX11Info::visual() instead.

    \oldcode
        void *visual = QPaintDevice::x11AppVisual(screen);
    \newcode
        void *visual = widget->x11Info().appVisual(screen);
    \endcode

    \sa QWidget::x11Info(), QPixmap::x11Info()
*/

/*! \fn Qt::HANDLE QPaintDevice::x11AppColormap(int screen)
    Use QX11Info::colormap() instead.

    \oldcode
        unsigned long colormap = QPaintDevice::x11AppColormap(screen);
    \newcode
        unsigned long colormap = widget->x11Info().appColormap(screen);
    \endcode

    \sa QWidget::x11Info(), QPixmap::x11Info()
*/

/*! \fn Display *QPaintDevice::x11AppDisplay()
    Use QX11Info::display() instead.

    \oldcode
        Display *display = QPaintDevice::x11AppDisplay();
    \newcode
        Display *display = widget->x11Info().display();
    \endcode

    \sa QWidget::x11Info(), QPixmap::x11Info()
*/

/*! \fn int QPaintDevice::x11AppScreen()
    Use QX11Info::screen() instead.

    \oldcode
        int screen = QPaintDevice::x11AppScreen();
    \newcode
        int screen = widget->x11Info().appScreen();
    \endcode

    \sa QWidget::x11Info(), QPixmap::x11Info()
*/

/*! \fn int QPaintDevice::x11AppDepth(int screen)
    Use QX11Info::depth() instead.

    \oldcode
        int depth = QPaintDevice::x11AppDepth(screen);
    \newcode
        int depth = widget->x11Info().appDepth(screen);
    \endcode

    \sa QWidget::x11Info(), QPixmap::x11Info()
*/

/*! \fn int QPaintDevice::x11AppCells(int screen)
    Use QX11Info::cells() instead.

    \oldcode
        int cells = QPaintDevice::x11AppCells(screen);
    \newcode
        int cells = widget->x11Info().appCells(screen);
    \endcode

    \sa QWidget::x11Info(), QPixmap::x11Info()
*/

/*! \fn Qt::HANDLE QPaintDevice::x11AppRootWindow(int screen)
    Use QX11Info::appRootWindow() instead.

    \oldcode
        unsigned long window = QPaintDevice::x11AppRootWindow(screen);
    \newcode
        unsigned long window = widget->x11Info().appRootWindow(screen);
    \endcode

    \sa QWidget::x11Info(), QPixmap::x11Info()
*/

/*! \fn bool QPaintDevice::x11AppDefaultColormap(int screen)
    Use QX11Info::defaultColormap() instead.

    \oldcode
        bool isDefault = QPaintDevice::x11AppDefaultColormap(screen);
    \newcode
        bool isDefault = widget->x11Info().appDefaultColormap(screen);
    \endcode

    \sa QWidget::x11Info(), QPixmap::x11Info()
*/

/*! \fn bool QPaintDevice::x11AppDefaultVisual(int screen)
    Use QX11Info::defaultVisual() instead.

    \oldcode
        bool isDefault = QPaintDevice::x11AppDefaultVisual(screen);
    \newcode
        bool isDefault = widget->x11Info().appDefaultVisual(screen);
    \endcode

    \sa QWidget::x11Info(), QPixmap::x11Info()
*/

/*! \fn void QPaintDevice::x11SetAppDpiX(int dpi, int screen)
    Use QX11Info::setAppDpiX() instead.
*/

/*! \fn void QPaintDevice::x11SetAppDpiY(int dpi, int screen)
    Use QX11Info::setAppDpiY() instead.
*/

/*! \fn int QPaintDevice::x11AppDpiX(int screen)
    Use QX11Info::appDpiX() instead.

    \oldcode
        bool isDefault = QPaintDevice::x11AppDpiX(screen);
    \newcode
        bool isDefault = widget->x11Info().appDpiX(screen);
    \endcode

    \sa QWidget::x11Info(), QPixmap::x11Info()
*/

/*! \fn int QPaintDevice::x11AppDpiY(int screen)
    Use QX11Info::appDpiY() instead.

    \oldcode
        bool isDefault = QPaintDevice::x11AppDpiY(screen);
    \newcode
        bool isDefault = widget->x11Info().appDpiY(screen);
    \endcode

    \sa QWidget::x11Info(), QPixmap::x11Info()
*/

/*! \fn HDC QPaintDevice::getDC() const
  \internal
*/

/*! \fn void QPaintDevice::releaseDC(HDC) const
  \internal
*/

/*! \fn QWSDisplay *QPaintDevice::qwsDisplay()
    \internal
*/

QT_END_NAMESPACE

#include "moc_qpainter.h"

