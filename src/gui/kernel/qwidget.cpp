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

#include "qapplication.h"
#include "qapplication_p.h"
#include "qbrush.h"
#include "qcursor.h"
#include "qdesktopwidget.h"
#include "qevent.h"
#include "qhash.h"
#include "qlayout.h"
#include "qmenu.h"
#include "qmetaobject.h"
#include "qpixmap.h"
#include "qpointer.h"
#include "qstack.h"
#include "qstyle.h"
#include "qstylefactory.h"
#include "qvariant.h"
#include "qwidget.h"
#include "qstyleoption.h"
#include "qpainter.h"
#include "qtooltip.h"
#include "qwhatsthis.h"
#include "qdebug.h"
#include "qstyle_p.h"
#include "qwindowsurface_p.h"
#include "qbackingstore_p.h"
#include "qpaintengine_raster_p.h"
#include "qwidget_p.h"
#include "qaction_p.h"
#include "qlayout_p.h"
#include "qgraphicsproxywidget.h"
#include "qgraphicsscene.h"
#include "qgraphicsproxywidget_p.h"
#include "qabstractscrollarea.h"
#include "qabstractscrollarea_p.h"
#include "qevent_p.h"

#if defined(Q_WS_X11)
# include "qpaintengine_x11_p.h"
# include "qx11info_x11.h"
#endif


// widget/widget data creation count
//#define QWIDGET_EXTRA_DEBUG
//#define ALIEN_DEBUG

QT_BEGIN_NAMESPACE

static inline bool qRectIntersects(const QRect &r1, const QRect &r2)
{
    return (qMax(r1.left(), r2.left()) <= qMin(r1.right(), r2.right()) &&
            qMax(r1.top(), r2.top()) <= qMin(r1.bottom(), r2.bottom()));
}

extern QDesktopWidget *qt_desktopWidget; // qapplication.cpp

/*!
    \internal
    \class QWidgetBackingStoreTracker
    \brief Class which allows tracking of which widgets are using a given backing store

    QWidgetBackingStoreTracker is a thin wrapper around a QWidgetBackingStore pointer,
    which maintains a list of the QWidgets which are currently using the backing
    store.  This list is modified via the registerWidget and unregisterWidget functions.
 */

QWidgetBackingStoreTracker::QWidgetBackingStoreTracker()
    :   m_ptr(0)
{

}

QWidgetBackingStoreTracker::~QWidgetBackingStoreTracker()
{
    delete m_ptr;
}

/*!
    \internal
    Destroy the contained QWidgetBackingStore, if not null, and clear the list of
    widgets using the backing store, then create a new QWidgetBackingStore, providing
    the QWidget.
 */
void QWidgetBackingStoreTracker::create(QWidget *widget)
{
    destroy();
    m_ptr = new QWidgetBackingStore(widget);
}

/*!
    \internal
    Destroy the contained QWidgetBackingStore, if not null, and clear the list of
    widgets using the backing store.
 */
void QWidgetBackingStoreTracker::destroy()
{
    delete m_ptr;
    m_ptr = 0;
    m_widgets.clear();
}

/*!
    \internal
    Add the widget to the list of widgets currently using the backing store.
    If the widget was already in the list, this function is a no-op.
 */
void QWidgetBackingStoreTracker::registerWidget(QWidget *w)
{
    Q_ASSERT(m_ptr);
    Q_ASSERT(w->internalWinId());
    Q_ASSERT(qt_widget_private(w)->maybeBackingStore() == m_ptr);
    m_widgets.insert(w);
}

/*!
    \internal
    Remove the widget from the list of widgets currently using the backing store.
    If the widget was in the list, and removing it causes the list to be empty,
    the backing store is deleted.
    If the widget was not in the list, this function is a no-op.
 */
void QWidgetBackingStoreTracker::unregisterWidget(QWidget *w)
{
    if (m_widgets.remove(w) && m_widgets.isEmpty()) {
        delete m_ptr;
        m_ptr = 0;
    }
}

/*!
    \internal
    Recursively remove widget and all of its descendents.
 */
void QWidgetBackingStoreTracker::unregisterWidgetSubtree(QWidget *widget)
{
    unregisterWidget(widget);
    foreach (QObject *child, widget->children())
        if (QWidget *childWidget = qobject_cast<QWidget *>(child))
            unregisterWidgetSubtree(childWidget);
}

QWidgetPrivate::QWidgetPrivate()
    : QObjectPrivate()
      , extra(0)
      , focus_next(0)
      , focus_prev(0)
      , focus_child(0)
      , layout(0)
      , needsFlush(0)
      , redirectDev(0)
      , extraPaintEngine(0)
      , polished(0)
      , inheritedFontResolveMask(0)
      , inheritedPaletteResolveMask(0)
      , leftmargin(0)
      , topmargin(0)
      , rightmargin(0)
      , bottommargin(0)
      , leftLayoutItemMargin(0)
      , topLayoutItemMargin(0)
      , rightLayoutItemMargin(0)
      , bottomLayoutItemMargin(0)
      , hd(0)
      , size_policy(QSizePolicy::Preferred, QSizePolicy::Preferred)
      , fg_role(QPalette::NoRole)
      , bg_role(QPalette::NoRole)
      , dirtyOpaqueChildren(true)
      , isOpaque(false)
      , inDirtyList(false)
      , isScrolled(false)
      , isMoved(false)
      , inSetParent(false)
#if defined(Q_WS_X11)
      , picture(0)
#endif
{
    if (Q_UNLIKELY(!qApp)) {
        qFatal("QWidget: Must construct a QApplication before a QPaintDevice");
        return;
    }

    isWidget = true;
    widget_attributes = 0;
#ifdef QWIDGET_EXTRA_DEBUG
    static int count = 0;
    qDebug() << "widgets" << ++count;
#endif
}


QWidgetPrivate::~QWidgetPrivate()
{

    if (extra)
        deleteExtra();
}

QWindowSurface *QWidgetPrivate::createDefaultWindowSurface()
{
    Q_Q(QWidget);
    return new QWindowSurface(q);
}

/*!
    \internal
*/
void QWidgetPrivate::scrollChildren(int dx, int dy)
{
    Q_Q(QWidget);
    if (q->children().size() > 0) {        // scroll children
        QPoint pd(dx, dy);
        QObjectList childObjects = q->children();
        for (int i = 0; i < childObjects.size(); ++i) { // move all children
            QWidget *w = qobject_cast<QWidget*>(childObjects.at(i));
            if (w && !w->isWindow()) {
                QPoint oldp = w->pos();
                QRect  r(w->pos() + pd, w->size());
                w->data->crect = r;
                if (w->testAttribute(Qt::WA_WState_Created))
                    w->d_func()->setWSGeometry();
                w->d_func()->setDirtyOpaqueRegion();
                QMoveEvent e(r.topLeft(), oldp);
                QApplication::sendEvent(w, &e);
            }
        }
    }
}


/*!
    \property QWidget::autoFillBackground
    \brief whether the widget background is filled automatically
    \since 4.1

    If enabled, this property will cause Qt to fill the background of the
    widget before invoking the paint event. The color used is defined by the
    QPalette::Window color role from the widget's \l{QPalette}{palette}.

    In addition, Windows are always filled with QPalette::Window, unless the
    WA_OpaquePaintEvent or WA_NoSystemBackground attributes are set.

    This property cannot be turned off (i.e., set to false) if a widget's
    parent has a static gradient for its background.

    By default, this property is false.

    \sa Qt::WA_OpaquePaintEvent, Qt::WA_NoSystemBackground,
    {QWidget#Transparency and Double Buffering}{Transparency and Double Buffering}
*/
bool QWidget::autoFillBackground() const
{
    Q_D(const QWidget);
    return d->extra && d->extra->autoFillBackground;
}

void QWidget::setAutoFillBackground(bool enabled)
{
    Q_D(QWidget);
    if (!d->extra)
        d->createExtra();
    if (d->extra->autoFillBackground == enabled)
        return;

    d->extra->autoFillBackground = enabled;
    d->updateIsOpaque();
    update();
    d->updateIsOpaque();
}

/*!
    \class QWidget
    \brief The QWidget class is the base class of all user interface objects.

    \ingroup basicwidgets

    The widget is the atom of the user interface: it receives mouse, keyboard
    and other events from the window system, and paints a representation of
    itself on the screen. Every widget is rectangular, and they are sorted in a
    Z-order. A widget is clipped by its parent and by the widgets in front of
    it.

    A widget that is not embedded in a parent widget is called a window.
    Usually, windows have a frame and a title bar, although it is also possible
    to create windows without such decoration using suitable
    \l{Qt::WindowFlags}{window flags}). In Qt, QMainWindow and the various
    subclasses of QDialog are the most common window types.

    Every widget's constructor accepts one or two standard arguments:

    \list 1
        \i  \c{QWidget *parent = nullptr} is the parent of the new widget. If it is 0
            (the default), the new widget will be a window. If not, it will be
            a child of \e parent, and be constrained by \e parent's geometry
            (unless you specify Qt::Window as window flag).
        \i  \c{Qt::WindowFlags f = 0} (where available) sets the window flags;
            the default is suitable for almost all widgets, but to get, for
            example, a window without a window system frame, you must use
            special flags.
    \endlist

    QWidget has many member functions, but some of them have little direct
    functionality; for example, QWidget has a font property, but never uses
    this itself. There are many subclasses which provide real functionality,
    such as QLabel, QPushButton, QListWidget, and QTabWidget.


    \section1 Top-Level and Child Widgets

    A widget without a parent widget is always an independent window (top-level
    widget). For these widgets, setWindowTitle() and setWindowIcon() set the
    title bar and icon respectively.

    Non-window widgets are child widgets, displayed within their parent
    widgets. Most widgets in Qt are mainly useful as child widgets. For
    example, it is possible to display a button as a top-level window, but most
    people prefer to put their buttons inside other widgets, such as QDialog.

    \image parent-child-widgets.png A parent widget containing various child widgets.

    The diagram above shows a QGroupBox widget being used to hold various child
    widgets in a layout provided by QGridLayout. The QLabel child widgets have
    been outlined to indicate their full sizes.

    If you want to use a QWidget to hold child widgets you will usually want to
    add a layout to the parent QWidget. See \l{Layout Management} for more
    information.


    \section1 Composite Widgets

    When a widget is used as a container to group a number of child widgets, it
    is known as a composite widget. These can be created by constructing a
    widget with the required visual properties - a QFrame, for example - and
    adding child widgets to it, usually managed by a layout. The above diagram
    shows such a composite widget that was created using \l{Katie Designer}.

    Composite widgets can also be created by subclassing a standard widget,
    such as QWidget or QFrame, and adding the necessary layout and child
    widgets in the constructor of the subclass. Many of the \l{Qt Examples}
    {examples provided with Qt} use this approach, and it is also covered in
    the Qt \l{Tutorials}.


    \section1 Custom Widgets and Painting

    Since QWidget is a subclass of QPaintDevice, subclasses can be used to
    display custom content that is composed using a series of painting
    operations with an instance of the QPainter class. This approach contrasts
    with the canvas-style approach used by the \l{Graphics View}
    {Graphics View Framework} where items are added to a scene by the
    application and are rendered by the framework itself.

    Each widget performs all painting operations from within its paintEvent()
    function. This is called whenever the widget needs to be redrawn, either
    as a result of some external change or when requested by the application.

    The \l{widgets/analogclock}{Analog Clock example} shows how a simple widget
    can handle paint events.


    \section1 Size Hints and Size Policies

    When implementing a new widget, it is almost always useful to reimplement
    sizeHint() to provide a reasonable default size for the widget and to set
    the correct size policy with setSizePolicy().

    By default, composite widgets which do not provide a size hint will be
    sized according to the space requirements of their child widgets.

    The size policy lets you supply good default behavior for the layout
    management system, so that other widgets can contain and manage yours
    easily. The default size policy indicates that the size hint represents
    the preferred size of the widget, and this is often good enough for many
    widgets.

    \note The size of top-level widgets are constrained to 2/3 of the desktop's
    height and width. You can resize() the widget manually if these bounds are
    inadequate.


    \section1 Events

    Widgets respond to events that are typically caused by user actions. Qt
    delivers events to widgets by calling specific event handler functions with
    instances of QEvent subclasses containing information about each event.

    If your widget only contains child widgets, you probably do not need to
    implement any event handlers. If you want to detect a mouse click in a
    child widget call the child's underMouse() function inside the widget's
    mousePressEvent().

    The \l{widgets/scribble}{Scribble example} implements a wider set of
    events to handle mouse movement, button presses, and window resizing.

    You will need to supply the behavior and content for your own widgets, but
    here is a brief overview of the events that are relevant to QWidget,
    starting with the most common ones:

    \list
        \i  paintEvent() is called whenever the widget needs to be repainted.
            Every widget displaying custom content must implement it. Painting
            using a QPainter can only take place in a paintEvent() or a
            function called by a paintEvent().
        \i  resizeEvent() is called when the widget has been resized.
        \i  mousePressEvent() is called when a mouse button is pressed while
            the mouse cursor is inside the widget, or when the widget has
            grabbed the mouse using grabMouse(). Pressing the mouse without
            releasing it is effectively the same as calling grabMouse().
        \i  mouseReleaseEvent() is called when a mouse button is released. A
            widget receives mouse release events when it has received the
            corresponding mouse press event. This means that if the user
            presses the mouse inside \e your widget, then drags the mouse
            somewhere else before releasing the mouse button, \e your widget
            receives the release event. There is one exception: if a popup menu
            appears while the mouse button is held down, this popup immediately
            steals the mouse events.
        \i  mouseDoubleClickEvent() is called when the user double-clicks in
            the widget. If the user double-clicks, the widget receives a mouse
            press event, a mouse release event and finally this event instead
            of a second mouse press event. (Some mouse move events may also be
            received if the mouse is not held steady during this operation.) It
            is \e{not possible} to distinguish a click from a double-click
            until the second click arrives. (This is one reason why most GUI
            books recommend that double-clicks be an extension of
            single-clicks, rather than trigger a different action.)
    \endlist

    Widgets that accept keyboard input need to reimplement a few more event
    handlers:

    \list
        \i  keyPressEvent() is called whenever a key is pressed, and again when
            a key has been held down long enough for it to auto-repeat. The
            \key Tab and \key Shift+Tab keys are only passed to the widget if
            they are not used by the focus-change mechanisms. To force those
            keys to be processed by your widget, you must reimplement
            QWidget::event().
        \i  focusInEvent() is called when the widget gains keyboard focus
            (assuming you have called setFocusPolicy()). Well-behaved widgets
            indicate that they own the keyboard focus in a clear but discreet
            way.
        \i  focusOutEvent() is called when the widget loses keyboard focus.
    \endlist

    You may be required to also reimplement some of the less common event
    handlers:

    \list
        \i  mouseMoveEvent() is called whenever the mouse moves while a mouse
            button is held down. This can be useful during drag and drop
            operations. If you call \l{setMouseTracking()}{setMouseTracking}(true),
            you get mouse move events even when no buttons are held down.
            (See also the \l{Drag and Drop} guide.)
        \i  keyReleaseEvent() is called whenever a key is released and while it
            is held down (if the key is auto-repeating). In that case, the
            widget will receive a pair of key release and key press event for
            every repeat. The \key Tab and \key Shift+Tab keys are only passed
            to the widget if they are not used by the focus-change mechanisms.
            To force those keys to be processed by your widget, you must
            reimplement QWidget::event().
        \i  wheelEvent() is called whenever the user turns the mouse wheel
            while the widget has the focus.
        \i  enterEvent() is called when the mouse enters the widget's screen
            space. (This excludes screen space owned by any of the widget's
            children.)
        \i  leaveEvent() is called when the mouse leaves the widget's screen
            space. If the mouse enters a child widget it will not cause a
            leaveEvent().
        \i  moveEvent() is called when the widget has been moved relative to
            its parent.
        \i  closeEvent() is called when the user closes the widget (or when
            close() is called).
    \endlist

    There are also some rather obscure events described in the documentation
    for QEvent::Type. To handle these events, you need to reimplement event()
    directly.

    The default implementation of event() handles \key Tab and \key Shift+Tab
    (to move the keyboard focus), and passes on most of the other events to
    one of the more specialized handlers above.

    Events and the mechanism used to deliver them are covered in 
    \l{The Event System}.

    \section1 Groups of Functions and Properties

    \table
    \header \i Context \i Functions and Properties

    \row \i Window functions \i
        show(),
        hide(),
        raise(),
        lower(),
        close().

    \row \i Top-level windows \i
        \l windowModified, \l windowTitle, \l windowIcon, \l windowIconText,
        \l isActiveWindow, activateWindow(), \l minimized, showMinimized(),
        \l maximized, showMaximized(), \l fullScreen, showFullScreen(),
        showNormal().

    \row \i Window contents \i
        update(),
        repaint(),
        scroll().

    \row \i Geometry \i
        \l pos, x(), y(), \l rect, \l size, width(), height(), move(), resize(),
        \l sizePolicy, sizeHint(), minimumSizeHint(),
        updateGeometry(), layout(),
        \l frameGeometry, \l geometry, \l childrenRect, \l childrenRegion,
        adjustSize(),
        mapFromGlobal(), mapToGlobal(),
        mapFromParent(), mapToParent(),
        \l maximumSize, \l minimumSize, \l sizeIncrement,
        \l baseSize, setFixedSize()

    \row \i Mode \i
        \l visible, isVisibleTo(),
        \l enabled, isEnabledTo(),
        \l modal,
        isWindow(),
        \l mouseTracking,
        \l updatesEnabled,
        visibleRegion().

    \row \i Look and feel \i
        style(),
        setStyle(),
        \l cursor,
        \l font,
        \l palette,
        backgroundRole(), setBackgroundRole(),
        fontInfo(), fontMetrics().

    \row \i Keyboard focus functions \i
        \l focus, \l focusPolicy,
        setFocus(), clearFocus(), setTabOrder(), setFocusProxy(),
        focusNextChild(), focusPreviousChild().

    \row \i Mouse and keyboard grabbing \i
        grabMouse(), releaseMouse(),
        grabKeyboard(), releaseKeyboard(),
        mouseGrabber(), keyboardGrabber().

    \row \i Event handlers \i
        event(),
        mousePressEvent(),
        mouseReleaseEvent(),
        mouseDoubleClickEvent(),
        mouseMoveEvent(),
        keyPressEvent(),
        keyReleaseEvent(),
        focusInEvent(),
        focusOutEvent(),
        wheelEvent(),
        enterEvent(),
        leaveEvent(),
        paintEvent(),
        moveEvent(),
        resizeEvent(),
        closeEvent(),
        dragEnterEvent(),
        dragMoveEvent(),
        dragLeaveEvent(),
        dropEvent(),
        childEvent(),
        showEvent(),
        hideEvent(),
        customEvent().
        changeEvent(),

    \row \i System functions \i
        parentWidget(), window(), setParent(), winId(),
        find(), metric().

    \row \i Interactive help \i
        setToolTip(), setWhatsThis()

    \endtable


    \section1 Transparency and Double Buffering

    Since Qt 4.0, QWidget automatically double-buffers its painting, so there
    is no need to write double-buffering code in paintEvent() to avoid
    flicker.

    Since Qt 4.1, the contents of parent widgets are propagated by default
    to each of their children as long as Qt::WA_PaintOnScreen is not set.
    Custom widgets can be written to take advantage of this feature by
    updating irregular regions (to create non-rectangular child widgets), or
    painting with colors that have less than full alpha component. The
    following diagram shows how attributes and properties of a custom widget
    can be fine-tuned to achieve different effects.

    \image propagation-custom.png

    In the above diagram, a semi-transparent rectangular child widget with an
    area removed is constructed and added to a parent widget (a QLabel showing
    a pixmap). Then, different properties and widget attributes are set to
    achieve different effects:

    \list
        \i  The left widget has no additional properties or widget attributes
            set. This default state suits most custom widgets using
            transparency, are irregularly-shaped, or do not paint over their
            entire area with an opaque brush.
        \i  The center widget has the \l autoFillBackground property set. This
            property is used with custom widgets that rely on the widget to
            supply a default background, and do not paint over their entire
            area with an opaque brush.
        \i  The right widget has the Qt::WA_OpaquePaintEvent widget attribute
            set. This indicates that the widget will paint over its entire area
            with opaque colors. The widget's area will initially be
            \e{uninitialized}, represented in the diagram with a red diagonal
            grid pattern that shines through the overpainted area. The
            Qt::WA_OpaquePaintArea attribute is useful for widgets that need to
            paint their own specialized contents quickly and do not need a
            default filled background.
    \endlist

    To rapidly update custom widgets with simple background colors, such as
    real-time plotting or graphing widgets, it is better to define a suitable
    background color (using setBackgroundRole() with the
    QPalette::Window role), set the \l autoFillBackground property, and only
    implement the necessary drawing functionality in the widget's paintEvent().

    To rapidly update custom widgets that constantly paint over their entire
    areas with opaque content, e.g., video streaming widgets, it is better to
    set the widget's Qt::WA_OpaquePaintEvent, avoiding any unnecessary overhead
    associated with repainting the widget's background.

    If a widget has both the Qt::WA_OpaquePaintEvent widget attribute \e{and}
    the \l autoFillBackground property set, the Qt::WA_OpaquePaintEvent
    attribute takes precedence. Depending on your requirements, you should
    choose either one of them.

    Since Qt 4.1, the contents of parent widgets are also propagated to
    standard Qt widgets. This can lead to some unexpected results if the
    parent widget is decorated in a non-standard way, as shown in the diagram
    below.

    \image propagation-standard.png

    The scope for customizing the painting behavior of standard Qt widgets,
    without resorting to subclassing, is slightly less than that possible for
    custom widgets. Usually, the desired appearance of a standard widget can be
    achieved by setting its \l autoFillBackground property.


    \section1 Creating Translucent Windows

    Since Qt 4.5, it has been possible to create windows with translucent regions
    on window systems that support compositing.

    To enable this feature in a top-level widget, set its Qt::WA_TranslucentBackground
    attribute with setAttribute() and ensure that its background is painted with
    non-opaque colors in the regions you want to be partially transparent.

    Platform notes:

    \list
    \o X11: This feature relies on the use of an X server that supports ARGB visuals
    and a compositing window manager.
    \o Windows: The widget needs to have the Qt::FramelessWindowHint window flag set
    for the translucency to work.
    \endlist


    \section1 Native Widgets vs Alien Widgets

    Introduced in Qt 4.4, alien widgets are widgets unknown to the windowing
    system. They do not have a native window handle associated with them. This
    feature significantly speeds up widget painting, resizing, and removes flicker.

    Should you require the old behavior with native windows, you can choose
    one of the following options:

    \list 1
        \i  Use the \c{QT_USE_NATIVE_WINDOWS=1} in your environment.
        \i  Set the Qt::AA_NativeWindows attribute on your application. All
            widgets will be native widgets.
        \i  Set the Qt::WA_NativeWindow attribute on widgets: The widget itself
            and all of its ancestors will become native (unless
            Qt::WA_DontCreateNativeAncestors is set).
        \i  Call QWidget::winId to enforce a native window (this implies 3).
        \i  Set the Qt::WA_PaintOnScreen attribute to enforce a native window
            (this implies 3).
    \endlist

    \sa QEvent, QPainter, QGridLayout, QBoxLayout, addAction(), QAction,
    QMenuBar

*/

QWidgetMapper *QWidgetPrivate::mapper = 0;          // widget with wid
QWidgetSet *QWidgetPrivate::allWidgets = 0;         // widgets with no wid

/*****************************************************************************
  QWidget member functions
 *****************************************************************************/

/*!
    Constructs a widget which is a child of \a parent, with  widget
    flags set to \a f.

    If \a parent is 0, the new widget becomes a window. If
    \a parent is another widget, this widget becomes a child window
    inside \a parent. The new widget is deleted when its \a parent is
    deleted.

    The widget flags argument, \a f, is normally 0, but it can be set
    to customize the frame of a window (i.e. \a
    parent must be 0). To customize the frame, use a value composed
    from the bitwise OR of any of the \l{Qt::WindowFlags}{window flags}.

    If you add a child widget to an already visible widget you must
    explicitly show the child to make it visible.

    Note that the X11 version of Qt may not be able to deliver all
    combinations of style flags on all systems. This is because on
    X11, Qt can only ask the window manager, and the window manager
    can override the application's settings. On Windows, Qt can set
    whatever flags you want.

    \sa windowFlags
*/
QWidget::QWidget(QWidget *parent, Qt::WindowFlags f)
    : QObject(*new QWidgetPrivate, 0), QPaintDevice()
{
    d_func()->init(parent, f);
}


/*! \internal
*/
QWidget::QWidget(QWidgetPrivate &dd, QWidget* parent, Qt::WindowFlags f)
    : QObject(dd, 0), QPaintDevice()
{
    Q_D(QWidget);
    d->init(parent, f);
}

/*!
    \internal
*/
int QWidget::devType() const
{
    return QInternal::Widget;
}


//### w is a "this" ptr
void QWidgetPrivate::adjustFlags(Qt::WindowFlags &flags, QWidget *w)
{
    bool customize =  (flags & (Qt::CustomizeWindowHint
            | Qt::FramelessWindowHint
            | Qt::WindowTitleHint
            | Qt::WindowSystemMenuHint
            | Qt::WindowMinimizeButtonHint
            | Qt::WindowMaximizeButtonHint
            | Qt::WindowCloseButtonHint
            | Qt::WindowContextHelpButtonHint));

    uint type = (flags & Qt::WindowType_Mask);

    if ((type == Qt::Widget || type == Qt::SubWindow) && w && !w->parent()) {
        type = Qt::Window;
        flags |= Qt::Window;
    }

    if (flags & Qt::CustomizeWindowHint) {
        // modify window flags to make them consistent.
        // Since the old way of doing this would interpret WindowSystemMenuHint as a
        // close button and we can't change that behavior we can't just add this in.
        if (flags & (Qt::WindowMinMaxButtonsHint | Qt::WindowCloseButtonHint | Qt::WindowContextHelpButtonHint)) {
            flags |= Qt::WindowSystemMenuHint;
            flags |= Qt::WindowTitleHint;
            flags &= ~Qt::FramelessWindowHint;
        }
    } else if (customize && !(flags & Qt::FramelessWindowHint)) {
        // if any of the window hints that affect the titlebar are set
        // and the window is supposed to have frame, we add a titlebar
        // and system menu by default.
        flags |= Qt::WindowSystemMenuHint;
        flags |= Qt::WindowTitleHint;
    }
    if (customize)
        ; // don't modify window flags if the user explicitly set them.
    else if (type == Qt::Dialog)
        flags |= Qt::WindowTitleHint | Qt::WindowSystemMenuHint | Qt::WindowContextHelpButtonHint | Qt::WindowCloseButtonHint;
    else if (type == Qt::Tool)
        flags |= Qt::WindowTitleHint | Qt::WindowSystemMenuHint | Qt::WindowCloseButtonHint;
    else
        flags |= Qt::WindowTitleHint | Qt::WindowSystemMenuHint | Qt::WindowMinimizeButtonHint | Qt::WindowMaximizeButtonHint | Qt::WindowCloseButtonHint;


}

void QWidgetPrivate::init(QWidget *parentWidget, Qt::WindowFlags f)
{
    Q_Q(QWidget);
    if (Q_UNLIKELY(QApplication::type() == QApplication::Tty)) {
        qFatal("QWidget: Cannot create a QWidget when no GUI is being used");
    }

    Q_ASSERT(allWidgets);
    if (allWidgets)
        allWidgets->insert(q);

    if (parentWidget && parentWidget->windowType() == Qt::Desktop) {
#if defined(Q_WS_X11)
        // make sure the widget is created on the same screen as the
        // programmer specified desktop widget
        xinfo = parentWidget->d_func()->xinfo;
#endif
    }

    q->data = &data;

    if (!parent) {
        Q_ASSERT_X(q->thread() == qApp->thread(), "QWidget",
                   "Widgets must be created in the GUI thread.");
    }

    data.fstrut_dirty = true;

    data.winid = 0;
    data.window_flags = f;
    data.window_state = 0;
    data.focus_policy = Qt::NoFocus;
    data.context_menu_policy = Qt::DefaultContextMenu;
    data.window_modality = Qt::NonModal;

    data.is_closing = false;
    data.in_show = false;
    data.in_destructor = false;

    q->setAttribute(Qt::WA_QuitOnClose); // might be cleared in adjustQuitOnCloseAttribute()
    adjustQuitOnCloseAttribute();

    q->setAttribute(Qt::WA_WState_Hidden);

    //give potential windows a bigger "pre-initial" size; create_sys() will give them a new size later
    data.crect = parentWidget ? QRect(0,0,100,30) : QRect(0,0,640,480);

    focus_next = focus_prev = q;

    if ((f & Qt::WindowType_Mask) == Qt::Desktop)
        q->create();
    else if (parentWidget)
        q->setParent(parentWidget, data.window_flags);
    else {
        adjustFlags(data.window_flags, q);
        resolveLayoutDirection();
        // opaque system background?
        const QBrush &background = q->palette().brush(QPalette::Window);
        setOpaque(q->isWindow() && background.style() != Qt::NoBrush && background.isOpaque());
    }
    data.fnt = QFont(data.fnt, q);
#if defined(Q_WS_X11)
    data.fnt.x11SetScreen(xinfo.screen());
#endif // Q_WS_X11

    q->setAttribute(Qt::WA_PendingMoveEvent);
    q->setAttribute(Qt::WA_PendingResizeEvent);

    if (QApplication::testAttribute(Qt::AA_ImmediateWidgetCreation))
        q->create();


    QEvent e(QEvent::Create);
    QApplication::sendEvent(q, &e);
    QApplication::postEvent(q, new QEvent(QEvent::PolishRequest));

    extraPaintEngine = 0;
}



void QWidgetPrivate::createRecursively()
{
    Q_Q(QWidget);
    q->create(0, true, true);
    for (int i = 0; i < children.size(); ++i) {
        QWidget *child = qobject_cast<QWidget *>(children.at(i));
        if (child && !child->isHidden() && !child->isWindow() && !child->testAttribute(Qt::WA_WState_Created))
            child->d_func()->createRecursively();
    }
}




/*!
    Creates a new widget window if \a window is 0, otherwise sets the
    widget's window to \a window.

    Initializes the window (sets the geometry etc.) if \a
    initializeWindow is true. If \a initializeWindow is false, no
    initialization is performed. This parameter only makes sense if \a
    window is a valid window.

    Destroys the old window if \a destroyOldWindow is true. If \a
    destroyOldWindow is false, you are responsible for destroying the
    window yourself (using platform native code).

    The QWidget constructor calls create(0,true,true) to create a
    window for this widget.
*/

void QWidget::create(WId window, bool initializeWindow, bool destroyOldWindow)
{
    Q_D(QWidget);
    if (testAttribute(Qt::WA_WState_Created) && window == 0 && internalWinId())
        return;

    if (d->data.in_destructor)
        return;

    Qt::WindowType type = windowType();
    Qt::WindowFlags &flags = data->window_flags;

    if ((type == Qt::Widget || type == Qt::SubWindow) && !parentWidget()) {
        type = Qt::Window;
        flags |= Qt::Window;
    }

    if (QWidget *parent = parentWidget()) {
        if (type & Qt::Window) {
            if (!parent->testAttribute(Qt::WA_WState_Created))
                parent->createWinId();
        } else if (testAttribute(Qt::WA_NativeWindow) && !parent->internalWinId()
                   && !testAttribute(Qt::WA_DontCreateNativeAncestors)) {
            // We're about to create a native child widget that doesn't have a native parent;
            // enforce a native handle for the parent unless the Qt::WA_DontCreateNativeAncestors
            // attribute is set.
            d->createWinId(window);
            // Nothing more to do.
            Q_ASSERT(testAttribute(Qt::WA_WState_Created));
            Q_ASSERT(internalWinId());
            return;
        }
    }

    static const int paintOnScreenEnv = qgetenv("QT_ONSCREEN_PAINT").toInt();
    if (paintOnScreenEnv)
        setAttribute(Qt::WA_PaintOnScreen);

    if (QApplication::testAttribute(Qt::AA_NativeWindows))
        setAttribute(Qt::WA_NativeWindow);

#ifdef ALIEN_DEBUG
    qDebug() << "QWidget::create:" << this << "parent:" << parentWidget()
             << "Alien?" << !testAttribute(Qt::WA_NativeWindow);
#endif

    d->updateIsOpaque();

    setAttribute(Qt::WA_WState_Created);                        // set created flag
    d->create_sys(window, initializeWindow, destroyOldWindow);

    // a real toplevel window needs a backing store
    if (isWindow() && windowType() != Qt::Desktop) {
        d->topData()->backingStore.destroy();
        d->topData()->backingStore.create(this);
    }

    if (!isWindow() && parentWidget() && parentWidget()->testAttribute(Qt::WA_DropSiteRegistered))
        setAttribute(Qt::WA_DropSiteRegistered, true);


    // need to force the resting of the icon after changing parents
    if (testAttribute(Qt::WA_SetWindowIcon))
        d->setWindowIcon_sys(true);
    if (isWindow() && !d->topData()->iconText.isEmpty())
        d->setWindowIconText_helper(d->topData()->iconText);
    if (isWindow() && !d->topData()->caption.isEmpty())
        d->setWindowTitle_helper(d->topData()->caption);
    if (windowType() != Qt::Desktop) {
        d->updateSystemBackground();

        if (isWindow() && !testAttribute(Qt::WA_SetWindowIcon))
            d->setWindowIcon_sys();
    }
}

/*!
    Destroys the widget.

    All this widget's children are deleted first. The application
    exits if this widget is the main widget.
*/

QWidget::~QWidget()
{
    Q_D(QWidget);
    d->data.in_destructor = true;

    if (Q_UNLIKELY(paintingActive()))
        qWarning("QWidget: %s (%s) deleted while being painted", objectName().toLocal8Bit().data(), metaObject()->className());

#ifndef QT_NO_ACTION
    // remove all actions from this widget
    for (int i = 0; i < d->actions.size(); ++i) {
        QActionPrivate *apriv = d->actions.at(i)->d_func();
        apriv->widgets.removeAll(this);
    }
    d->actions.clear();
#endif

#ifndef QT_NO_SHORTCUT
    // Remove all shortcuts grabbed by this
    // widget, unless application is closing
    if (!QApplicationPrivate::is_app_closing && testAttribute(Qt::WA_GrabbedShortcut))
        qApp->d_func()->shortcutMap.removeShortcut(0, this);
#endif

    // delete layout while we still are a valid widget
    delete d->layout;
    d->layout = 0;
    // Remove myself from focus list

    Q_ASSERT(d->focus_next->d_func()->focus_prev == this);
    Q_ASSERT(d->focus_prev->d_func()->focus_next == this);

    if (d->focus_next != this) {
        d->focus_next->d_func()->focus_prev = d->focus_prev;
        d->focus_prev->d_func()->focus_next = d->focus_next;
        d->focus_next = d->focus_prev = 0;
    }

    clearFocus();

    d->setDirtyOpaqueRegion();

    if (isWindow() && isVisible() && internalWinId()) {
        d->close_helper(QWidgetPrivate::CloseNoEvent);
    }

#if defined(Q_WS_X11)
    else if (!internalWinId() && isVisible()) {
        qApp->d_func()->sendSyntheticEnterLeave(this);
    }
#endif

    if (QWidgetBackingStore *bs = d->maybeBackingStore()) {
        bs->removeDirtyWidget(this);
    }

    delete d->needsFlush;
    d->needsFlush = 0;

    // The next few lines are duplicated from QObject, but required here
    // since QWidget deletes is children itself
    bool blocked = d->blockSig;
    d->blockSig = false; // unblock signals so we always emit destroyed()

    if (d->isSignalConnected(0)) {
        emit destroyed(this);
    }

    d->blockSig = blocked;

    if (!d->children.isEmpty())
        d->deleteChildren();

    QApplication::removePostedEvents(this);

    destroy();                                        // platform-dependent cleanup

    if (QWidgetPrivate::allWidgets) // might have been deleted by ~QApplication
        QWidgetPrivate::allWidgets->remove(this);

    QEvent e(QEvent::Destroy);
    QCoreApplication::sendEvent(this, &e);
}

void QWidgetPrivate::setWinId(WId id)                // set widget identifier
{
    Q_Q(QWidget);
    // the user might create a widget with Qt::Desktop window
    // attribute (or create another QDesktopWidget instance), which
    // will have the same windowid (the root window id) as the
    // qt_desktopWidget. We should not add the second desktop widget
    // to the mapper.
    bool userDesktopWidget = (qt_desktopWidget && qt_desktopWidget != q && q->windowType() == Qt::Desktop);
    if (mapper && data.winid && !userDesktopWidget) {
        mapper->remove(data.winid);
    }

    const WId oldWinId = data.winid;

    data.winid = id;
#if defined(Q_WS_X11)
    hd = id; // X11: hd == ident
#endif
    if (mapper && id && !userDesktopWidget) {
        mapper->insert(data.winid, q);
    }

    if(oldWinId != id) {
        QEvent e(QEvent::WinIdChange);
        QCoreApplication::sendEvent(q, &e);
    }
}

void QWidgetPrivate::createTLExtra()
{
    if (!extra)
        createExtra();
    if (!extra->topextra) {
        QTLWExtra* x = extra->topextra = new QTLWExtra;
        x->icon = 0;
        x->iconPixmap = 0;
        x->windowSurface = 0;
        x->sharedPainter = 0;
        x->incw = x->inch = 0;
        x->basew = x->baseh = 0;
        x->frameStrut.setCoords(0, 0, 0, 0);
        x->normalGeometry = QRect(0,0,-1,-1);
        x->savedFlags = 0;
        x->opacity = 255;
        x->posFromMove = false;
        x->sizeAdjusted = false;
        x->inTopLevelResize = false;
        x->inRepaint = false;

        createTLSysExtra();
#ifdef QWIDGET_EXTRA_DEBUG
        static int count = 0;
        qDebug() << "tlextra" << ++count;
#endif
    }
}

/*!
  \internal
  Creates the widget extra data.
*/

void QWidgetPrivate::createExtra()
{
    if (!extra) {                                // if not exists
        extra = new QWExtra;
        extra->topextra = 0;
#ifndef QT_NO_GRAPHICSVIEW
        extra->proxyWidget = 0;
#endif
#ifndef QT_NO_CURSOR
        extra->curs = 0;
#endif
        extra->minw = 0;
        extra->minh = 0;
        extra->maxw = QWIDGETSIZE_MAX;
        extra->maxh = QWIDGETSIZE_MAX;
        extra->explicitMinSize = 0;
        extra->explicitMaxSize = 0;
        extra->autoFillBackground = false;
        extra->nativeChildrenForced = false;
        extra->inRenderWithPainter = false;
        extra->hasMask = false;
        extra->compress_events = true;
        createSysExtra();
#ifdef QWIDGET_EXTRA_DEBUG
        static int count = 0;
        qDebug() << "extra" << ++count;
#endif
    }
}


/*!
  \internal
  Deletes the widget extra data.
*/

void QWidgetPrivate::deleteExtra()
{
    if (extra) {                                // if exists
#ifndef QT_NO_CURSOR
        delete extra->curs;
#endif
        if (extra->topextra) {
            deleteTLSysExtra();
            extra->topextra->backingStore.destroy();
            delete extra->topextra->icon;
            if (extra->topextra->iconPixmap) {
                XFreePixmap(qt_x11Data->display, extra->topextra->iconPixmap);
            }
            delete extra->topextra->windowSurface;
            delete extra->topextra;
        }
        delete extra;
        extra = nullptr;
    }
}

/*
  Returns true if there are widgets above this which overlap with
  \a rect, which is in parent's coordinate system (same as crect).
*/

bool QWidgetPrivate::isOverlapped(const QRect &rect) const
{
    Q_Q(const QWidget);

    const QWidget *w = q;
    QRect r = rect;
    while (w) {
        if (w->isWindow())
            return false;
        QWidgetPrivate *pd = w->parentWidget()->d_func();
        bool above = false;
        for (int i = 0; i < pd->children.size(); ++i) {
            QWidget *sibling = qobject_cast<QWidget *>(pd->children.at(i));
            if (!sibling || !sibling->isVisible() || sibling->isWindow())
                continue;
            if (!above) {
                above = (sibling == w);
                continue;
            }

            if (qRectIntersects(sibling->data->crect, r)) {
                const QWExtra *siblingExtra = sibling->d_func()->extra;
                if (siblingExtra && siblingExtra->hasMask
                    && !siblingExtra->mask.translated(sibling->data->crect.topLeft()).intersects(r)) {
                    continue;
                }
                return true;
            }
        }
        w = w->parentWidget();
        r.translate(pd->data.crect.topLeft());
    }
    return false;
}

void QWidgetPrivate::syncBackingStore()
{
    if (paintOnScreen()) {
        repaint_sys(dirty);
        dirty = QRegion();
    } else if (QWidgetBackingStore *bs = maybeBackingStore()) {
        bs->sync();
    }
}

void QWidgetPrivate::syncBackingStore(const QRegion &region)
{
    if (paintOnScreen())
        repaint_sys(region);
    else if (QWidgetBackingStore *bs = maybeBackingStore()) {
        bs->sync(q_func(), region);
    }
}

void QWidgetPrivate::setUpdatesEnabled_helper(bool enable)
{
    Q_Q(QWidget);

    if (enable && !q->isWindow() && q->parentWidget() && !q->parentWidget()->updatesEnabled())
        return; // nothing we can do

    if (enable != q->testAttribute(Qt::WA_UpdatesDisabled))
        return; // nothing to do

    q->setAttribute(Qt::WA_UpdatesDisabled, !enable);
    if (enable)
        q->update();

    Qt::WidgetAttribute attribute = enable ? Qt::WA_ForceUpdatesDisabled : Qt::WA_UpdatesDisabled;
    for (int i = 0; i < children.size(); ++i) {
        QWidget *w = qobject_cast<QWidget *>(children.at(i));
        if (w && !w->isWindow() && !w->testAttribute(attribute))
            w->d_func()->setUpdatesEnabled_helper(enable);
    }
}

/*!
    \internal

    Propagate this widget's palette to all children, except style sheet
    widgets, and windows that don't enable window propagation (palettes don't
    normally propagate to windows).
*/
void QWidgetPrivate::propagatePaletteChange()
{
    Q_Q(QWidget);
    // Propagate a new inherited mask to all children.
#ifndef QT_NO_GRAPHICSVIEW
    if (!q->parentWidget() && extra && extra->proxyWidget) {
        QGraphicsProxyWidget *p = extra->proxyWidget;
        inheritedPaletteResolveMask = p->d_func()->inheritedPaletteResolveMask | p->palette().resolve();
    } else
#endif //QT_NO_GRAPHICSVIEW
        if (q->isWindow() && !q->testAttribute(Qt::WA_WindowPropagation)) {
        inheritedPaletteResolveMask = 0;
    }
    int mask = data.pal.resolve() | inheritedPaletteResolveMask;

    QEvent pc(QEvent::PaletteChange);
    QApplication::sendEvent(q, &pc);
    for (int i = 0; i < children.size(); ++i) {
        QWidget *w = qobject_cast<QWidget*>(children.at(i));
        if (w && (!w->isWindow() || w->testAttribute(Qt::WA_WindowPropagation))) {
            QWidgetPrivate *wd = w->d_func();
            wd->inheritedPaletteResolveMask = mask;
            wd->resolvePalette();
        }
    }
}

/*
  Returns the widget's clipping rectangle.
*/
QRect QWidgetPrivate::clipRect() const
{
    Q_Q(const QWidget);
    const QWidget * w = q;
    if (!w->isVisible())
        return QRect();
    QRect r = q->rect();
    int ox = 0;
    int oy = 0;
    while (w
            && w->isVisible()
            && !w->isWindow()
            && w->parentWidget()) {
        ox -= w->x();
        oy -= w->y();
        w = w->parentWidget();
        r &= QRect(ox, oy, w->width(), w->height());
    }
    return r;
}

/*
  Returns the widget's clipping region (without siblings).
*/
QRegion QWidgetPrivate::clipRegion() const
{
    Q_Q(const QWidget);
    if (!q->isVisible())
        return QRegion();
    QRegion r(q->rect());
    const QWidget * w = q;
    const QWidget *ignoreUpTo;
    int ox = 0;
    int oy = 0;
    while (w
           && w->isVisible()
           && !w->isWindow()
           && w->parentWidget()) {
        ox -= w->x();
        oy -= w->y();
        ignoreUpTo = w;
        w = w->parentWidget();
        r &= QRegion(ox, oy, w->width(), w->height());

        int i = 0;
        while(w->d_func()->children.at(i++) != static_cast<const QObject *>(ignoreUpTo))
            ;
        for ( ; i < w->d_func()->children.size(); ++i) {
            if(QWidget *sibling = qobject_cast<QWidget *>(w->d_func()->children.at(i))) {
                if(sibling->isVisible() && !sibling->isWindow()) {
                    QRect siblingRect(ox+sibling->x(), oy+sibling->y(),
                                      sibling->width(), sibling->height());
                    if (qRectIntersects(siblingRect, q->rect()))
                        r -= QRegion(siblingRect);
                }
            }
        }
    }
    return r;
}

void QWidgetPrivate::setDirtyOpaqueRegion()
{
    Q_Q(QWidget);

    dirtyOpaqueChildren = true;

    if (q->isWindow())
        return;

    QWidget *parent = q->parentWidget();
    if (!parent)
        return;

    // TODO: instead of setting dirtyflag, manipulate the dirtyregion directly?
    QWidgetPrivate *pd = parent->d_func();
    if (!pd->dirtyOpaqueChildren)
        pd->setDirtyOpaqueRegion();
}

const QRegion &QWidgetPrivate::getOpaqueChildren() const
{
    if (!dirtyOpaqueChildren)
        return opaqueChildren;

    QWidgetPrivate *that = const_cast<QWidgetPrivate*>(this);
    that->opaqueChildren = QRegion();

    for (int i = 0; i < children.size(); ++i) {
        QWidget *child = qobject_cast<QWidget *>(children.at(i));
        if (!child || !child->isVisible() || child->isWindow())
            continue;

        const QPoint offset = child->geometry().topLeft();
        QWidgetPrivate *childd = child->d_func();
        QRegion r = childd->isOpaque ? child->rect() : childd->getOpaqueChildren();
        if (childd->extra && childd->extra->hasMask)
            r &= childd->extra->mask;
        if (r.isEmpty())
            continue;
        r.translate(offset);
        that->opaqueChildren += r;
    }

    that->opaqueChildren &= q_func()->rect();
    that->dirtyOpaqueChildren = false;

    return that->opaqueChildren;
}

void QWidgetPrivate::subtractOpaqueChildren(QRegion &source, const QRect &clipRect) const
{
    if (children.isEmpty() || clipRect.isEmpty())
        return;

    const QRegion &r = getOpaqueChildren();
    if (!r.isEmpty())
        source -= (r & clipRect);
}

//subtract any relatives that are higher up than me --- this is too expensive !!!
void QWidgetPrivate::subtractOpaqueSiblings(QRegion &sourceRegion, bool *hasDirtySiblingsAbove,
                                            bool alsoNonOpaque) const
{
    Q_Q(const QWidget);
    static int disableSubtractOpaqueSiblings = qgetenv("QT_NO_SUBTRACTOPAQUESIBLINGS").toInt();
    if (disableSubtractOpaqueSiblings || q->isWindow())
        return;

    QRect clipBoundingRect;
    bool dirtyClipBoundingRect = true;

    QRegion parentClip;
    bool dirtyParentClip = true;

    QPoint parentOffset = data.crect.topLeft();

    const QWidget *w = q;

    while (w) {
        if (w->isWindow())
            break;
        QWidgetPrivate *pd = w->parentWidget()->d_func();
        const int myIndex = pd->children.indexOf(const_cast<QWidget *>(w));
        const QRect widgetGeometry = w->data->crect;
        for (int i = myIndex + 1; i < pd->children.size(); ++i) {
            QWidget *sibling = qobject_cast<QWidget *>(pd->children.at(i));
            if (!sibling || !sibling->isVisible() || sibling->isWindow())
                continue;

            const QRect siblingGeometry = sibling->data->crect;
            if (!qRectIntersects(siblingGeometry, widgetGeometry))
                continue;

            if (dirtyClipBoundingRect) {
                clipBoundingRect = sourceRegion.boundingRect();
                dirtyClipBoundingRect = false;
            }

            if (!qRectIntersects(siblingGeometry, clipBoundingRect.translated(parentOffset)))
                continue;

            if (dirtyParentClip) {
                parentClip = sourceRegion.translated(parentOffset);
                dirtyParentClip = false;
            }

            const QPoint siblingPos(sibling->data->crect.topLeft());
            const QRect siblingClipRect(sibling->d_func()->clipRect());
            QRegion siblingDirty(parentClip);
            siblingDirty &= (siblingClipRect.translated(siblingPos));
            const bool hasMask = sibling->d_func()->extra && sibling->d_func()->extra->hasMask;
            if (hasMask)
                siblingDirty &= sibling->d_func()->extra->mask.translated(siblingPos);
            if (siblingDirty.isEmpty())
                continue;

            if (sibling->d_func()->isOpaque || alsoNonOpaque) {
                if (hasMask) {
                    siblingDirty.translate(-parentOffset);
                    sourceRegion -= siblingDirty;
                } else {
                    sourceRegion -= siblingGeometry.translated(-parentOffset);
                }
            } else {
                if (hasDirtySiblingsAbove)
                    *hasDirtySiblingsAbove = true;
                if (sibling->d_func()->children.isEmpty())
                    continue;
                QRegion opaqueSiblingChildren(sibling->d_func()->getOpaqueChildren());
                opaqueSiblingChildren.translate(-parentOffset + siblingPos);
                sourceRegion -= opaqueSiblingChildren;
            }
            if (sourceRegion.isEmpty())
                return;

            dirtyClipBoundingRect = true;
            dirtyParentClip = true;
        }

        w = w->parentWidget();
        parentOffset += pd->data.crect.topLeft();
        dirtyParentClip = true;
    }
}

void QWidgetPrivate::clipToEffectiveMask(QRegion &region) const
{
    Q_Q(const QWidget);

    const QWidget *w = q;
    QPoint offset;

    while (w) {
        const QWidgetPrivate *wd = w->d_func();
        if (wd->extra && wd->extra->hasMask)
            region &= (w != q) ? wd->extra->mask.translated(offset) : wd->extra->mask;
        if (w->isWindow())
            return;
        offset -= wd->data.crect.topLeft();
        w = w->parentWidget();
    }
}

bool QWidgetPrivate::paintOnScreen() const
{
#if defined(QT_NO_BACKINGSTORE)
    return true;
#else
    Q_Q(const QWidget);
    if (q->testAttribute(Qt::WA_PaintOnScreen)
            || (!q->isWindow() && q->window()->testAttribute(Qt::WA_PaintOnScreen))) {
        return true;
    }

    return false;
#endif
}

void QWidgetPrivate::updateIsOpaque()
{
    // hw: todo: only needed if opacity actually changed
    setDirtyOpaqueRegion();

    Q_Q(QWidget);

    if (q->testAttribute(Qt::WA_OpaquePaintEvent) || q->testAttribute(Qt::WA_PaintOnScreen)) {
        setOpaque(true);
        return;
    }

    const QPalette &pal = q->palette();

    if (q->autoFillBackground()) {
        const QBrush &autoFillBrush = pal.brush(q->backgroundRole());
        if (autoFillBrush.style() != Qt::NoBrush && autoFillBrush.isOpaque()) {
            setOpaque(true);
            return;
        }
    }

    if (q->isWindow() && !q->testAttribute(Qt::WA_NoSystemBackground)) {
        const QBrush &windowBrush = q->palette().brush(QPalette::Window);
        if (windowBrush.style() != Qt::NoBrush && windowBrush.isOpaque()) {
            setOpaque(true);
            return;
        }
    }
    setOpaque(false);
}

void QWidgetPrivate::setOpaque(bool opaque)
{
    if (isOpaque == opaque)
        return;
    isOpaque = opaque;
#ifdef Q_WS_X11
    x11UpdateIsOpaque();
#endif
}

void QWidgetPrivate::updateIsTranslucent()
{
#ifdef Q_WS_X11
    x11UpdateIsOpaque();
#endif
}

/*!
    \fn void QPixmap::fill(const QWidget *widget, const QPoint &offset)

    Fills the pixmap with the \a widget's background color or pixmap
    according to the given offset.

    The QPoint \a offset defines a point in widget coordinates to
    which the pixmap's top-left pixel will be mapped to. This is only
    significant if the widget has a background pixmap; otherwise the
    pixmap will simply be filled with the background color of the
    widget.
*/

void QPixmap::fill( const QWidget *widget, const QPoint &off )
{
    QPainter p(this);
    p.translate(-off);
    widget->d_func()->paintBackground(&p, QRect(off, size()));
}

static inline void fillRegion(QPainter *painter, const QRegion &rgn, const QBrush &brush)
{
    Q_ASSERT(painter);

    if (brush.style() == Qt::TexturePattern) {
        const QRect rect(rgn.boundingRect());
        painter->setClipRegion(rgn);
        painter->drawTiledPixmap(rect, brush.texture(), rect.topLeft());

    } else if (brush.gradient()
               && brush.gradient()->coordinateMode() == QGradient::ObjectBoundingMode) {
        painter->save();
        painter->setClipRegion(rgn);
        painter->fillRect(0, 0, painter->device()->width(), painter->device()->height(), brush);
        painter->restore();
    } else {
        QPen oldPen = painter->pen();
        QBrush oldBrush = painter->brush();
        painter->setPen(Qt::NoPen);
        painter->setBrush(brush);
        painter->drawRects(rgn.rects());
        painter->setBrush(oldBrush);
        painter->setPen(oldPen);
    }
}

void QWidgetPrivate::paintBackground(QPainter *painter, const QRegion &rgn, int flags) const
{
    Q_Q(const QWidget);

#ifndef QT_NO_SCROLLAREA
    bool resetBrushOrigin = false;
    QPointF oldBrushOrigin;
    //If we are painting the viewport of a scrollarea, we must apply an offset to the brush in case we are drawing a texture
    QAbstractScrollArea *scrollArea = qobject_cast<QAbstractScrollArea *>(parent);
    if (scrollArea && scrollArea->viewport() == q) {
        QObjectData *scrollPrivate = static_cast<QWidget *>(scrollArea)->d_ptr;
        QAbstractScrollAreaPrivate *priv = static_cast<QAbstractScrollAreaPrivate *>(scrollPrivate);
        oldBrushOrigin = painter->brushOrigin();
        resetBrushOrigin = true;
        painter->setBrushOrigin(-priv->contentsOffset());

    }
#endif // QT_NO_SCROLLAREA

    const QBrush autoFillBrush = q->palette().brush(q->backgroundRole());

    if ((flags & DrawAsRoot) && !(q->autoFillBackground() && autoFillBrush.isOpaque())) {
        fillRegion(painter, rgn, q->palette().brush(QPalette::Window));
    }

    if (q->autoFillBackground())
        fillRegion(painter, rgn, autoFillBrush);

    if (q->testAttribute(Qt::WA_StyledBackground)) {
        painter->setClipRegion(rgn);
        QStyleOption opt;
        opt.initFrom(q);
        q->style()->drawPrimitive(QStyle::PE_Widget, &opt, painter, q);
    }

#ifndef QT_NO_SCROLLAREA
    if (resetBrushOrigin)
        painter->setBrushOrigin(oldBrushOrigin);
#endif // QT_NO_SCROLLAREA
}

/*
  \internal
  This function is called when a widget is hidden or destroyed.
  It resets some application global pointers that should only refer active,
  visible widgets.
*/

extern QWidget *qt_button_down;

void QWidgetPrivate::deactivateWidgetCleanup()
{
    Q_Q(QWidget);
    // If this was the active application window, reset it
    if (QApplication::activeWindow() == q)
        QApplication::setActiveWindow(0);
    // If the is the active mouse press widget, reset it
    if (q == qt_button_down)
        qt_button_down = 0;
}


/*!
    Returns a pointer to the widget with window identifer/handle \a
    id.

    The window identifier type depends on the underlying window
    system, see \c qwindowdefs.h for the actual definition. If there
    is no widget with this identifier, 0 is returned.
*/

QWidget *QWidget::find(WId id)
{
    return QWidgetPrivate::mapper ? QWidgetPrivate::mapper->value(id, 0) : 0;
}



/*!
    \fn WId QWidget::internalWinId() const
    \internal
    Returns the window system identifier of the widget, or 0 if the widget is not created yet.

*/

/*!
    \fn WId QWidget::winId() const

    Returns the window system identifier of the widget.

    Portable in principle, but if you use it you are probably about to
    do something non-portable. Be careful.

    If a widget is non-native (alien) and winId() is invoked on it, that widget
    will be provided a native handle.

    This value may change at run-time. An event with type QEvent::WinIdChange
    will be sent to the widget following a change in window system identifier.

    \sa find()
*/
WId QWidget::winId() const
{
    if (!testAttribute(Qt::WA_WState_Created) || !internalWinId()) {
#ifdef ALIEN_DEBUG
        qDebug() << "QWidget::winId: creating native window for" << this;
#endif
        QWidget *that = const_cast<QWidget*>(this);
        that->setAttribute(Qt::WA_NativeWindow);
        that->d_func()->createWinId();
        return that->data->winid;
    }
    return data->winid;
}


void QWidgetPrivate::createWinId(WId winid)
{
    Q_Q(QWidget);

#ifdef ALIEN_DEBUG
    qDebug() << "QWidgetPrivate::createWinId for" << q << winid;
#endif
    const bool forceNativeWindow = q->testAttribute(Qt::WA_NativeWindow);
    if (!q->testAttribute(Qt::WA_WState_Created) || (forceNativeWindow && !q->internalWinId())) {
        if (!q->isWindow()) {
            QWidget *parent = q->parentWidget();
            QWidgetPrivate *pd = parent->d_func();
            if (forceNativeWindow && !q->testAttribute(Qt::WA_DontCreateNativeAncestors))
                parent->setAttribute(Qt::WA_NativeWindow);
            if (!parent->internalWinId()) {
                pd->createWinId();
            }

            for (int i = 0; i < pd->children.size(); ++i) {
                QWidget *w = qobject_cast<QWidget *>(pd->children.at(i));
                if (w && !w->isWindow() && (!w->testAttribute(Qt::WA_WState_Created)
                                            || (!w->internalWinId() && w->testAttribute(Qt::WA_NativeWindow)))) {
                    if (w!=q) {
                        w->create();
                    } else {
                        w->create(winid);
                        // if the window has already been created, we
                        // need to raise it to its proper stacking position
                        if (winid)
                            w->raise();
                    }
                }
            }
        } else {
            q->create();
        }
    }
}


/*!
\internal
Ensures that the widget has a window system identifier, i.e. that it is known to the windowing system.

*/

void QWidget::createWinId()
{
    Q_D(QWidget);
#ifdef ALIEN_DEBUG
    qDebug()  << "QWidget::createWinId" << this;
#endif
//    qWarning("QWidget::createWinId is obsolete, please fix your code.");
    d->createWinId();
}

/*!
    \since 4.4

    Returns the effective window system identifier of the widget, i.e. the
    native parent's window system identifier.

    If the widget is native, this function returns the native widget ID.
    Otherwise, the window ID of the first native parent widget, i.e., the
    top-level widget that contains this widget, is returned.

    \note We recommend that you do not store this value as it is likely to
    change at run-time.

    \sa nativeParentWidget()
*/
WId QWidget::effectiveWinId() const
{
    WId id = internalWinId();
    if (id || !testAttribute(Qt::WA_WState_Created))
        return id;
    QWidget *realParent = nativeParentWidget();
    if (!realParent && d_func()->inSetParent) {
        // In transitional state. This is really just a workaround. The real problem
        // is that QWidgetPrivate::setParent_sys (platform specific code) first sets
        // the window id to 0 (setWinId(0)) before it sets the Qt::WA_WState_Created
        // attribute to false. The correct way is to do it the other way around, and
        // in that case the Qt::WA_WState_Created logic above will kick in and
        // return 0 whenever the widget is in a transitional state. However, changing
        // the original logic for all platforms is far more intrusive and might
        // break existing applications.
        // Note: The widget can only be in a transitional state when changing its
        // parent -- everything else is an internal error -- hence explicitly checking
        // against 'inSetParent' rather than doing an unconditional return whenever
        // 'realParent' is 0 (which may cause strange artifacts and headache later).
        return 0;
    }
    // This widget *must* have a native parent widget.
    Q_ASSERT(realParent);
    Q_ASSERT(realParent->internalWinId());
    return realParent->internalWinId();
}

/*!
    \sa QWidget::setStyle(), QApplication::setStyle(), QApplication::style()
*/

QStyle *QWidget::style() const
{
    Q_D(const QWidget);

    if (d->extra && d->extra->style)
        return d->extra->style;
    return QApplication::style();
}

/*!
    Sets the widget's GUI style to \a style. The ownership of the style
    object is not transferred.

    If no style is set, the widget uses the application's style,
    QApplication::style() instead.

    Setting a widget's style has no effect on existing or future child
    widgets.

    \warning This function is particularly useful for demonstration
    purposes, where you want to show Qt's styling capabilities. Real
    applications should avoid it and use one consistent GUI style
    instead.

    \sa style(), QStyle, QApplication::style(), QApplication::setStyle()
*/

void QWidget::setStyle(QStyle *style)
{
    Q_D(QWidget);
    setAttribute(Qt::WA_SetStyle, style != 0);
    d->createExtra();

    QStyle *oldStyle = this->style();

    d->extra->style = style;

    // repolish
    if (windowType() != Qt::Desktop) {
        if (d->polished) {
            oldStyle->unpolish(this);
            this->style()->polish(this);
        }
    }

    QEvent e(QEvent::StyleChange);
    QApplication::sendEvent(this, &e);
}

/*!
    \fn bool QWidget::isWindow() const

    Returns true if the widget is an independent window, otherwise
    returns false.

    A window is a widget that isn't visually the child of any other
    widget and that usually has a frame and a
    \l{QWidget::setWindowTitle()}{window title}.

    A window can have a \l{QWidget::parentWidget()}{parent widget}.
    It will then be grouped with its parent and deleted when the
    parent is deleted, minimized when the parent is minimized etc. If
    supported by the window manager, it will also have a common
    taskbar entry with its parent.

    QDialog and QMainWindow widgets are by default windows, even if a
    parent widget is specified in the constructor. This behavior is
    specified by the Qt::Window flag.

    \sa window(), isModal(), parentWidget()
*/

/*!
    \property QWidget::modal
    \brief whether the widget is a modal widget

    This property only makes sense for windows. A modal widget
    prevents widgets in all other windows from getting any input.

    By default, this property is false.

    \sa isWindow(), windowModality, QDialog
*/

/*!
    \property QWidget::windowModality
    \brief which windows are blocked by the modal widget
    \since 4.1

    This property only makes sense for windows. A modal widget
    prevents widgets in other windows from getting input. The value of
    this property controls which windows are blocked when the widget
    is visible. Changing this property while the window is visible has
    no effect; you must hide() the widget first, then show() it again.

    By default, this property is Qt::NonModal.

    \sa isWindow(), QWidget::modal, QDialog
*/

Qt::WindowModality QWidget::windowModality() const
{
    return data->window_modality;
}

void QWidget::setWindowModality(Qt::WindowModality windowModality)
{
    data->window_modality = windowModality;
    setAttribute(Qt::WA_ShowModal, (data->window_modality != Qt::NonModal));
    setAttribute(Qt::WA_SetWindowModality, true);
}

/*!
    \fn bool QWidget::underMouse() const

    Returns true if the widget is under the mouse cursor; otherwise
    returns false.

    This value is not updated properly during drag and drop
    operations.

    \sa enterEvent(), leaveEvent()
*/

/*!
    \property QWidget::minimized
    \brief whether this widget is minimized (iconified)

    This property is only relevant for windows.

    By default, this property is false.

    \sa showMinimized(), visible, show(), hide(), showNormal(), maximized
*/
bool QWidget::isMinimized() const
{ return data->window_state & Qt::WindowMinimized; }

/*!
    Shows the widget minimized, as an icon.

    Calling this function only affects \l{isWindow()}{windows}.

    \sa showNormal(), showMaximized(), show(), hide(), isVisible(),
        isMinimized()
*/
void QWidget::showMinimized()
{
    bool isMin = isMinimized();
    if (isMin && isVisible())
        return;

    ensurePolished();

    if (!isMin)
        setWindowState((windowState() & ~Qt::WindowActive) | Qt::WindowMinimized);
    show();
}

/*!
    \property QWidget::maximized
    \brief whether this widget is maximized

    This property is only relevant for windows.

    \note Due to limitations on some window systems, this does not always
    report the expected results (e.g., if the user on X11 maximizes the
    window via the window manager, Qt has no way of distinguishing this
    from any other resize). This is expected to improve as window manager
    protocols evolve.

    By default, this property is false.

    \sa windowState(), showMaximized(), visible, show(), hide(), showNormal(), minimized
*/
bool QWidget::isMaximized() const
{ return data->window_state & Qt::WindowMaximized; }



/*!
    Returns the current window state. The window state is a OR'ed
    combination of Qt::WindowState: Qt::WindowMinimized,
    Qt::WindowMaximized, Qt::WindowFullScreen, and Qt::WindowActive.

  \sa Qt::WindowState setWindowState()
 */
Qt::WindowStates QWidget::windowState() const
{
    return data->window_state;
}

/*!\internal

   The function sets the window state on child widgets similar to
   setWindowState(). The difference is that the window state changed
   event has the isOverride() flag set. It exists mainly to keep
   QWorkspace working.
 */
void QWidget::overrideWindowState(Qt::WindowStates newstate)
{
    QWindowStateChangeEvent e(data->window_state, true);
    data->window_state  = newstate;
    QApplication::sendEvent(this, &e);
}

/*!
    \fn void QWidget::setWindowState(Qt::WindowStates windowState)

    Sets the window state to \a windowState. The window state is a OR'ed
    combination of Qt::WindowState: Qt::WindowMinimized,
    Qt::WindowMaximized, Qt::WindowFullScreen, and Qt::WindowActive.

    If the window is not visible (i.e. isVisible() returns false), the
    window state will take effect when show() is called. For visible
    windows, the change is immediate. For example, to toggle between
    full-screen and normal mode, use the following code:

    \snippet doc/src/snippets/code/src_gui_kernel_qwidget.cpp 0

    In order to restore and activate a minimized window (while
    preserving its maximized and/or full-screen state), use the following:

    \snippet doc/src/snippets/code/src_gui_kernel_qwidget.cpp 1

    Calling this function will hide the widget. You must call show() to make
    the widget visible again.

    \note On some window systems Qt::WindowActive is not immediate, and may be
    ignored in certain cases.

    When the window state changes, the widget receives a changeEvent()
    of type QEvent::WindowStateChange.

    \sa Qt::WindowState windowState()
*/

/*!
    \property QWidget::fullScreen
    \brief whether the widget is shown in full screen mode

    A widget in full screen mode occupies the whole screen area and does not
    display window decorations, such as a title bar.

    By default, this property is false.

    \sa windowState(), minimized, maximized
*/
bool QWidget::isFullScreen() const
{ return data->window_state & Qt::WindowFullScreen; }

/*!
    Shows the widget in full-screen mode.

    Calling this function only affects \l{isWindow()}{windows}.

    To return from full-screen mode, call showNormal().

    Full-screen mode works fine under Windows, but has certain
    problems under X. These problems are due to limitations of the
    ICCCM protocol that specifies the communication between X11
    clients and the window manager. ICCCM simply does not understand
    the concept of non-decorated full-screen windows. Therefore, the
    best we can do is to request a borderless window and place and
    resize it to fill the entire screen. Depending on the window
    manager, this may or may not work. The borderless window is
    requested using MOTIF hints, which are at least partially
    supported by virtually all modern window managers.

    An alternative would be to bypass the window manager entirely and
    create a window with the Qt::X11BypassWindowManagerHint flag. This
    has other severe problems though, like totally broken keyboard focus
    and very strange effects on desktop changes or when the user raises
    other windows.

    X11 window managers that follow modern post-ICCCM specifications
    support full-screen mode properly.

    \sa showNormal(), showMaximized(), show(), hide(), isVisible()
*/
void QWidget::showFullScreen()
{
    ensurePolished();

    setWindowState((windowState() & ~(Qt::WindowMinimized | Qt::WindowMaximized))
                   | Qt::WindowFullScreen);
    show();
    activateWindow();
}

/*!
    Shows the widget maximized.

    Calling this function only affects \l{isWindow()}{windows}.

    On X11, this function may not work properly with certain window
    managers. See the \l{Window Geometry} documentation for an explanation.

    \sa setWindowState(), showNormal(), showMinimized(), show(), hide(), isVisible()
*/
void QWidget::showMaximized()
{
    ensurePolished();

    setWindowState((windowState() & ~(Qt::WindowMinimized | Qt::WindowFullScreen))
                   | Qt::WindowMaximized);
    show();
}

/*!
    Restores the widget after it has been maximized or minimized.

    Calling this function only affects \l{isWindow()}{windows}.

    \sa setWindowState(), showMinimized(), showMaximized(), show(), hide(), isVisible()
*/
void QWidget::showNormal()
{
    ensurePolished();

    setWindowState(windowState() & ~(Qt::WindowMinimized
                                     | Qt::WindowMaximized
                                     | Qt::WindowFullScreen));
    show();
}

/*!
    Returns true if this widget would become enabled if \a ancestor is
    enabled; otherwise returns false.



    This is the case if neither the widget itself nor every parent up
    to but excluding \a ancestor has been explicitly disabled.

    isEnabledTo(0) is equivalent to isEnabled().

    \sa setEnabled() enabled
*/

bool QWidget::isEnabledTo(QWidget* ancestor) const
{
    const QWidget * w = this;
    while (!w->testAttribute(Qt::WA_ForceDisabled)
            && !w->isWindow()
            && w->parentWidget()
            && w->parentWidget() != ancestor)
        w = w->parentWidget();
    return !w->testAttribute(Qt::WA_ForceDisabled);
}

#ifndef QT_NO_ACTION
/*!
    Appends the action \a action to this widget's list of actions.

    All QWidgets have a list of \l{QAction}s, however they can be
    represented graphically in many different ways. The default use of
    the QAction list (as returned by actions()) is to create a context
    QMenu.

    A QWidget should only have one of each action and adding an action
    it already has will not cause the same action to be in the widget twice.

    The ownership of \a action is not transferred to this QWidget.

    \sa removeAction(), insertAction(), actions(), QMenu
*/
void QWidget::addAction(QAction *action)
{
    insertAction(0, action);
}

/*!
    Appends the actions \a actions to this widget's list of actions.

    \sa removeAction(), QMenu, addAction()
*/
void QWidget::addActions(const QList<QAction*> &actions)
{
    for(int i = 0; i < actions.count(); i++)
        insertAction(0, actions.at(i));
}

/*!
    Inserts the action \a action to this widget's list of actions,
    before the action \a before. It appends the action if \a before is 0 or
    \a before is not a valid action for this widget.

    A QWidget should only have one of each action.

    \sa removeAction(), addAction(), QMenu, contextMenuPolicy, actions()
*/
void QWidget::insertAction(QAction *before, QAction *action)
{
    if(Q_UNLIKELY(!action)) {
        qWarning("QWidget::insertAction: Attempt to insert null action");
        return;
    }

    Q_D(QWidget);
    if(d->actions.contains(action))
        removeAction(action);

    int pos = d->actions.indexOf(before);
    if (pos < 0) {
        before = 0;
        pos = d->actions.size();
    }
    d->actions.insert(pos, action);

    QActionPrivate *apriv = action->d_func();
    apriv->widgets.append(this);

    QActionEvent e(QEvent::ActionAdded, action, before);
    QApplication::sendEvent(this, &e);
}

/*!
    Inserts the actions \a actions to this widget's list of actions,
    before the action \a before. It appends the action if \a before is 0 or
    \a before is not a valid action for this widget.

    A QWidget can have at most one of each action.

    \sa removeAction(), QMenu, insertAction(), contextMenuPolicy
*/
void QWidget::insertActions(QAction *before, const QList<QAction*> &actions)
{
    for(int i = 0; i < actions.count(); ++i)
        insertAction(before, actions.at(i));
}

/*!
    Removes the action \a action from this widget's list of actions.
    \sa insertAction(), actions(), insertAction()
*/
void QWidget::removeAction(QAction *action)
{
    if (!action)
        return;

    Q_D(QWidget);

    QActionPrivate *apriv = action->d_func();
    apriv->widgets.removeAll(this);

    if (d->actions.removeAll(action)) {
        QActionEvent e(QEvent::ActionRemoved, action);
        QApplication::sendEvent(this, &e);
    }
}

/*!
    Returns the (possibly empty) list of this widget's actions.

    \sa contextMenuPolicy, insertAction(), removeAction()
*/
QList<QAction*> QWidget::actions() const
{
    Q_D(const QWidget);
    return d->actions;
}
#endif // QT_NO_ACTION

/*!
    \property QWidget::enabled
    \brief whether the widget is enabled

    In general an enabled widget handles keyboard and mouse events; a disabled
    widget does not. An exception is made with \l{QAbstractButton}.

    Some widgets display themselves differently when they are
    disabled. For example a button might draw its label grayed out. If
    your widget needs to know when it becomes enabled or disabled, you
    can use the changeEvent() with type QEvent::EnabledChange.

    Disabling a widget implicitly disables all its children. Enabling
    respectively enables all child widgets unless they have been
    explicitly disabled.

    By default, this property is true.

    \sa isEnabledTo(), QKeyEvent, QMouseEvent, changeEvent()
*/
void QWidget::setEnabled(bool enable)
{
    Q_D(QWidget);
    setAttribute(Qt::WA_ForceDisabled, !enable);
    d->setEnabled_helper(enable);
}

void QWidgetPrivate::setEnabled_helper(bool enable)
{
    Q_Q(QWidget);

    if (enable && !q->isWindow() && q->parentWidget() && !q->parentWidget()->isEnabled())
        return; // nothing we can do

    if (enable != q->testAttribute(Qt::WA_Disabled))
        return; // nothing to do

    q->setAttribute(Qt::WA_Disabled, !enable);
    updateSystemBackground();

    if (!enable && q->window()->focusWidget() == q) {
        bool parentIsEnabled = (!q->parentWidget() || q->parentWidget()->isEnabled());
        if (!parentIsEnabled || !q->focusNextChild())
            q->clearFocus();
    }

    Qt::WidgetAttribute attribute = enable ? Qt::WA_ForceDisabled : Qt::WA_Disabled;
    for (int i = 0; i < children.size(); ++i) {
        QWidget *w = qobject_cast<QWidget *>(children.at(i));
        if (w && !w->testAttribute(attribute))
            w->d_func()->setEnabled_helper(enable);
    }
#if defined(Q_WS_X11)
    if (q->testAttribute(Qt::WA_SetCursor) || q->isWindow()) {
        // enforce the windows behavior of clearing the cursor on
        // disabled widgets
        qt_x11_enforce_cursor(q, false);
    }
#endif
    QEvent e(QEvent::EnabledChange);
    QApplication::sendEvent(q, &e);
}

/*!
    \property QWidget::acceptDrops
    \brief whether drop events are enabled for this widget

    Setting this property to true announces to the system that this
    widget \e may be able to accept drop events.

    If the widget is the desktop (windowType() == Qt::Desktop), this may
    fail if another application is using the desktop; you can call
    acceptDrops() to test if this occurs.

    \warning Do not modify this property in a drag and drop event handler.

    By default, this property is false.

    \sa {Drag and Drop}
*/
bool QWidget::acceptDrops() const
{
    return testAttribute(Qt::WA_AcceptDrops);
}

void QWidget::setAcceptDrops(bool on)
{
    setAttribute(Qt::WA_AcceptDrops, on);

}

/*!
    \fn void QWidget::fontChange(const QFont &)

    \internal
    \obsolete
*/

/*!
    \fn void QWidget::styleChange(QStyle& style)

    \internal
    \obsolete
*/

/*!
    Disables widget input events if \a disable is true; otherwise
    enables input events.

    See the \l enabled documentation for more information.

    \sa isEnabledTo(), QKeyEvent, QMouseEvent, changeEvent()
*/
void QWidget::setDisabled(bool disable)
{
    setEnabled(!disable);
}

/*!
    \property QWidget::frameGeometry
    \brief geometry of the widget relative to its parent including any
    window frame

    See the \l{Window Geometry} documentation for an overview of geometry
    issues with windows.

    By default, this property contains a value that depends on the user's
    platform and screen geometry.

    \sa geometry() x() y() pos()
*/
QRect QWidget::frameGeometry() const
{
    Q_D(const QWidget);
    if (isWindow() && ! (windowType() == Qt::Popup)) {
        QRect fs = d->frameStrut();
        return QRect(data->crect.x() - fs.left(),
                     data->crect.y() - fs.top(),
                     data->crect.width() + fs.left() + fs.right(),
                     data->crect.height() + fs.top() + fs.bottom());
    }
    return data->crect;
}

/*!
    \property QWidget::x

    \brief the x coordinate of the widget relative to its parent including
    any window frame

    See the \l{Window Geometry} documentation for an overview of geometry
    issues with windows.

    By default, this property has a value of 0.

    \sa frameGeometry, y, pos
*/
int QWidget::x() const
{
    Q_D(const QWidget);
    if (isWindow() && ! (windowType() == Qt::Popup))
        return data->crect.x() - d->frameStrut().left();
    return data->crect.x();
}

/*!
    \property QWidget::y
    \brief the y coordinate of the widget relative to its parent and
    including any window frame

    See the \l{Window Geometry} documentation for an overview of geometry
    issues with windows.

    By default, this property has a value of 0.

    \sa frameGeometry, x, pos
*/
int QWidget::y() const
{
    Q_D(const QWidget);
    if (isWindow() && ! (windowType() == Qt::Popup))
        return data->crect.y() - d->frameStrut().top();
    return data->crect.y();
}

/*!
    \property QWidget::pos
    \brief the position of the widget within its parent widget

    If the widget is a window, the position is that of the widget on
    the desktop, including its frame.

    When changing the position, the widget, if visible, receives a
    move event (moveEvent()) immediately. If the widget is not
    currently visible, it is guaranteed to receive an event before it
    is shown.

    By default, this property contains a position that refers to the
    origin.

    \warning Calling move() or setGeometry() inside moveEvent() can
    lead to infinite recursion.

    See the \l{Window Geometry} documentation for an overview of geometry
    issues with windows.

    \sa frameGeometry, size x(), y()
*/
QPoint QWidget::pos() const
{
    Q_D(const QWidget);
    if (isWindow() && ! (windowType() == Qt::Popup)) {
        QRect fs = d->frameStrut();
        return QPoint(data->crect.x() - fs.left(), data->crect.y() - fs.top());
    }
    return data->crect.topLeft();
}

/*!
    \property QWidget::geometry
    \brief the geometry of the widget relative to its parent and
    excluding the window frame

    When changing the geometry, the widget, if visible, receives a
    move event (moveEvent()) and/or a resize event (resizeEvent())
    immediately. If the widget is not currently visible, it is
    guaranteed to receive appropriate events before it is shown.

    The size component is adjusted if it lies outside the range
    defined by minimumSize() and maximumSize().

    \warning Calling setGeometry() inside resizeEvent() or moveEvent()
    can lead to infinite recursion.

    See the \l{Window Geometry} documentation for an overview of geometry
    issues with windows.

    By default, this property contains a value that depends on the user's
    platform and screen geometry.

    \sa frameGeometry(), rect(), move(), resize(), moveEvent(),
        resizeEvent(), minimumSize(), maximumSize()
*/

/*!
    \property QWidget::normalGeometry

    \brief the geometry of the widget as it will appear when shown as
    a normal (not maximized or full screen) top-level widget

    For child widgets this property always holds an empty rectangle.

    By default, this property contains an empty rectangle.

    \sa QWidget::windowState(), QWidget::geometry
*/

/*!
    \property QWidget::size
    \brief the size of the widget excluding any window frame

    If the widget is visible when it is being resized, it receives a resize event
    (resizeEvent()) immediately. If the widget is not currently
    visible, it is guaranteed to receive an event before it is shown.

    The size is adjusted if it lies outside the range defined by
    minimumSize() and maximumSize().

    By default, this property contains a value that depends on the user's
    platform and screen geometry.

    \warning Calling resize() or setGeometry() inside resizeEvent() can
    lead to infinite recursion.

    \note Setting the size to \c{QSize(0, 0)} will cause the widget to not
    appear on screen. This also applies to windows.

    \sa pos, geometry, minimumSize, maximumSize, resizeEvent(), adjustSize()
*/

/*!
    \property QWidget::width
    \brief the width of the widget excluding any window frame

    See the \l{Window Geometry} documentation for an overview of geometry
    issues with windows.

    \note Do not use this function to find the width of a screen on
    a \l{QDesktopWidget}{multiple screen desktop}. Read
    \l{QDesktopWidget#Screen Geometry}{this note} for details.

    By default, this property contains a value that depends on the user's
    platform and screen geometry.

    \sa geometry, height, size
*/

/*!
    \property QWidget::height
    \brief the height of the widget excluding any window frame

    See the \l{Window Geometry} documentation for an overview of geometry
    issues with windows.

    \note Do not use this function to find the height of a screen
    on a \l{QDesktopWidget}{multiple screen desktop}. Read
    \l{QDesktopWidget#Screen Geometry}{this note} for details.

    By default, this property contains a value that depends on the user's
    platform and screen geometry.

    \sa geometry, width, size
*/

/*!
    \property QWidget::rect
    \brief the internal geometry of the widget excluding any window
    frame

    The rect property equals QRect(0, 0, width(), height()).

    See the \l{Window Geometry} documentation for an overview of geometry
    issues with windows.

    By default, this property contains a value that depends on the user's
    platform and screen geometry.

    \sa size
*/


QRect QWidget::normalGeometry() const
{
    Q_D(const QWidget);
    if (!d->extra || !d->extra->topextra)
        return QRect();

    if (!isMaximized() && !isFullScreen())
        return geometry();

    return d->topData()->normalGeometry;
}


/*!
    \property QWidget::childrenRect
    \brief the bounding rectangle of the widget's children

    Hidden children are excluded.

    By default, for a widget with no children, this property contains a
    rectangle with zero width and height located at the origin.

    \sa childrenRegion() geometry()
*/

QRect QWidget::childrenRect() const
{
    Q_D(const QWidget);
    QRect r(0, 0, 0, 0);
    for (int i = 0; i < d->children.size(); ++i) {
        QWidget *w = qobject_cast<QWidget *>(d->children.at(i));
        if (w && !w->isWindow() && !w->isHidden())
            r |= w->geometry();
    }
    return r;
}

/*!
    \property QWidget::childrenRegion
    \brief the combined region occupied by the widget's children

    Hidden children are excluded.

    By default, for a widget with no children, this property contains an
    empty region.

    \sa childrenRect() geometry() mask()
*/

QRegion QWidget::childrenRegion() const
{
    Q_D(const QWidget);
    QRegion r;
    for (int i = 0; i < d->children.size(); ++i) {
        QWidget *w = qobject_cast<QWidget *>(d->children.at(i));
        if (w && !w->isWindow() && !w->isHidden()) {
            QRegion mask = w->mask();
            if (mask.isEmpty())
                r |= w->geometry();
            else
                r |= mask.translated(w->pos());
        }
    }
    return r;
}


/*!
    \property QWidget::minimumSize
    \brief the widget's minimum size

    The widget cannot be resized to a smaller size than the minimum
    widget size. The widget's size is forced to the minimum size if
    the current size is smaller.

    The minimum size set by this function will override the minimum size
    defined by QLayout. In order to unset the minimum size, use a
    value of \c{QSize(0, 0)}.

    By default, this property contains a size with zero width and height.

    \sa minimumWidth, minimumHeight, maximumSize, sizeIncrement
*/

QSize QWidget::minimumSize() const
{
    Q_D(const QWidget);
    return d->extra ? QSize(d->extra->minw, d->extra->minh) : QSize(0, 0);
}

/*!
    \property QWidget::maximumSize
    \brief the widget's maximum size in pixels

    The widget cannot be resized to a larger size than the maximum
    widget size.

    By default, this property contains a size in which both width and height
    have values of 16777215.

    \note The definition of the \c QWIDGETSIZE_MAX macro limits the maximum size
    of widgets.

    \sa maximumWidth, maximumHeight, minimumSize, sizeIncrement
*/

QSize QWidget::maximumSize() const
{
    Q_D(const QWidget);
    return d->extra ? QSize(d->extra->maxw, d->extra->maxh)
                 : QSize(QWIDGETSIZE_MAX, QWIDGETSIZE_MAX);
}


/*!
    \property QWidget::minimumWidth
    \brief the widget's minimum width in pixels

    This property corresponds to the width held by the \l minimumSize property.

    By default, this property has a value of 0.

    \sa minimumSize, minimumHeight
*/

/*!
    \property QWidget::minimumHeight
    \brief the widget's minimum height in pixels

    This property corresponds to the height held by the \l minimumSize property.

    By default, this property has a value of 0.

    \sa minimumSize, minimumWidth
*/

/*!
    \property QWidget::maximumWidth
    \brief the widget's maximum width in pixels

    This property corresponds to the width held by the \l maximumSize property.

    By default, this property contains a value of 16777215.

    \note The definition of the \c QWIDGETSIZE_MAX macro limits the maximum size
    of widgets.

    \sa maximumSize, maximumHeight
*/

/*!
    \property QWidget::maximumHeight
    \brief the widget's maximum height in pixels

    This property corresponds to the height held by the \l maximumSize property.

    By default, this property contains a value of 16777215.

    \note The definition of the \c QWIDGETSIZE_MAX macro limits the maximum size
    of widgets.

    \sa maximumSize, maximumWidth
*/

/*!
    \property QWidget::sizeIncrement
    \brief the size increment of the widget

    When the user resizes the window, the size will move in steps of
    sizeIncrement().width() pixels horizontally and
    sizeIncrement.height() pixels vertically, with baseSize() as the
    basis. Preferred widget sizes are for non-negative integers \e i
    and \e j:
    \snippet doc/src/snippets/code/src_gui_kernel_qwidget.cpp 2

    Note that while you can set the size increment for all widgets, it
    only affects windows.

    By default, this property contains a size with zero width and height.

    \warning The size increment has no effect under Windows, and may
    be disregarded by the window manager on X11.

    \sa size, minimumSize, maximumSize
*/
QSize QWidget::sizeIncrement() const
{
    Q_D(const QWidget);
    return (d->extra && d->extra->topextra)
        ? QSize(d->extra->topextra->incw, d->extra->topextra->inch)
        : QSize(0, 0);
}

/*!
    \property QWidget::baseSize
    \brief the base size of the widget

    The base size is used to calculate a proper widget size if the
    widget defines sizeIncrement().

    By default, for a newly-created widget, this property contains a size with
    zero width and height.

    \sa setSizeIncrement()
*/

QSize QWidget::baseSize() const
{
    Q_D(const QWidget);
    return (d->extra != 0 && d->extra->topextra != 0)
        ? QSize(d->extra->topextra->basew, d->extra->topextra->baseh)
        : QSize(0, 0);
}

bool QWidgetPrivate::setMinimumSize_helper(int &minw, int &minh)
{
    Q_Q(QWidget);

    int mw = minw, mh = minh;
    if (mw == QWIDGETSIZE_MAX)
        mw = 0;
    if (mh == QWIDGETSIZE_MAX)
        mh = 0;
    if (Q_UNLIKELY(minw > QWIDGETSIZE_MAX || minh > QWIDGETSIZE_MAX)) {
        qWarning("QWidget::setMinimumSize: (%s/%s) "
                "The largest allowed size is (%d,%d)",
                 q->objectName().toLocal8Bit().data(), q->metaObject()->className(), QWIDGETSIZE_MAX,
                QWIDGETSIZE_MAX);
        minw = mw = qMin<int>(minw, QWIDGETSIZE_MAX);
        minh = mh = qMin<int>(minh, QWIDGETSIZE_MAX);
    }
    if (Q_UNLIKELY(minw < 0 || minh < 0)) {
        qWarning("QWidget::setMinimumSize: (%s/%s) Negative sizes (%d,%d) "
                "are not possible",
                q->objectName().toLocal8Bit().data(), q->metaObject()->className(), minw, minh);
        minw = mw = qMax(minw, 0);
        minh = mh = qMax(minh, 0);
    }
    createExtra();
    if (extra->minw == mw && extra->minh == mh)
        return false;
    extra->minw = mw;
    extra->minh = mh;
    extra->explicitMinSize = 0;
    if (mw)
        extra->explicitMinSize |= Qt::Horizontal;
    if (mh)
        extra->explicitMinSize |= Qt::Vertical;
    return true;
}

/*!
    \overload

    This function corresponds to setMinimumSize(QSize(minw, minh)).
    Sets the minimum width to \a minw and the minimum height to \a
    minh.
*/

void QWidget::setMinimumSize(int minw, int minh)
{
    Q_D(QWidget);
    if (!d->setMinimumSize_helper(minw, minh))
        return;

    if (isWindow())
        d->setConstraints_sys();
    if (minw > width() || minh > height()) {
        bool resized = testAttribute(Qt::WA_Resized);
        bool maximized = isMaximized();
        resize(qMax(minw,width()), qMax(minh,height()));
        setAttribute(Qt::WA_Resized, resized); //not a user resize
        if (maximized)
            data->window_state |= Qt::WindowMaximized;
    }
#ifndef QT_NO_GRAPHICSVIEW
    if (d->extra) {
        if (d->extra->proxyWidget)
            d->extra->proxyWidget->setMinimumSize(minw, minh);
    }
#endif
    d->updateGeometry_helper(d->extra->minw == d->extra->maxw && d->extra->minh == d->extra->maxh);
}

bool QWidgetPrivate::setMaximumSize_helper(int &maxw, int &maxh)
{
    Q_Q(QWidget);
    if (Q_UNLIKELY(maxw > QWIDGETSIZE_MAX || maxh > QWIDGETSIZE_MAX)) {
        qWarning("QWidget::setMaximumSize: (%s/%s) "
                "The largest allowed size is (%d,%d)",
                 q->objectName().toLocal8Bit().data(), q->metaObject()->className(), QWIDGETSIZE_MAX,
                QWIDGETSIZE_MAX);
        maxw = qMin<int>(maxw, QWIDGETSIZE_MAX);
        maxh = qMin<int>(maxh, QWIDGETSIZE_MAX);
    }
    if (Q_UNLIKELY(maxw < 0 || maxh < 0)) {
        qWarning("QWidget::setMaximumSize: (%s/%s) Negative sizes (%d,%d) "
                "are not possible",
                q->objectName().toLocal8Bit().data(), q->metaObject()->className(), maxw, maxh);
        maxw = qMax(maxw, 0);
        maxh = qMax(maxh, 0);
    }
    createExtra();
    if (extra->maxw == maxw && extra->maxh == maxh)
        return false;
    extra->maxw = maxw;
    extra->maxh = maxh;
    extra->explicitMaxSize = 0;
    if (maxw != QWIDGETSIZE_MAX)
        extra->explicitMaxSize |= Qt::Horizontal;
    if (maxh != QWIDGETSIZE_MAX)
        extra->explicitMaxSize |= Qt::Vertical;
    return true;
}

/*!
    \overload

    This function corresponds to setMaximumSize(QSize(\a maxw, \a
    maxh)). Sets the maximum width to \a maxw and the maximum height
    to \a maxh.
*/
void QWidget::setMaximumSize(int maxw, int maxh)
{
    Q_D(QWidget);
    if (!d->setMaximumSize_helper(maxw, maxh))
        return;

    if (isWindow())
        d->setConstraints_sys();
    if (maxw < width() || maxh < height()) {
        bool resized = testAttribute(Qt::WA_Resized);
        resize(qMin(maxw,width()), qMin(maxh,height()));
        setAttribute(Qt::WA_Resized, resized); //not a user resize
    }

#ifndef QT_NO_GRAPHICSVIEW
    if (d->extra) {
        if (d->extra->proxyWidget)
            d->extra->proxyWidget->setMaximumSize(maxw, maxh);
    }
#endif

    d->updateGeometry_helper(d->extra->minw == d->extra->maxw && d->extra->minh == d->extra->maxh);
}

/*!
    \overload

    Sets the x (width) size increment to \a w and the y (height) size
    increment to \a h.
*/
void QWidget::setSizeIncrement(int w, int h)
{
    Q_D(QWidget);
    d->createTLExtra();
    QTLWExtra* x = d->topData();
    if (x->incw == w && x->inch == h)
        return;
    x->incw = w;
    x->inch = h;
    if (isWindow())
        d->setConstraints_sys();
}

/*!
    \overload

    This corresponds to setBaseSize(QSize(\a basew, \a baseh)). Sets
    the widgets base size to width \a basew and height \a baseh.
*/
void QWidget::setBaseSize(int basew, int baseh)
{
    Q_D(QWidget);
    d->createTLExtra();
    QTLWExtra* x = d->topData();
    if (x->basew == basew && x->baseh == baseh)
        return;
    x->basew = basew;
    x->baseh = baseh;
    if (isWindow())
        d->setConstraints_sys();
}

/*!
    Sets both the minimum and maximum sizes of the widget to \a s,
    thereby preventing it from ever growing or shrinking.

    This will override the default size constraints set by QLayout.

    To remove constraints, set the size to QWIDGETSIZE_MAX.

    Alternatively, if you want the widget to have a
    fixed size based on its contents, you can call
    QLayout::setSizeConstraint(QLayout::SetFixedSize);

    \sa maximumSize, minimumSize
*/

void QWidget::setFixedSize(const QSize & s)
{
    setFixedSize(s.width(), s.height());
}


/*!
    \fn void QWidget::setFixedSize(int w, int h)
    \overload

    Sets the width of the widget to \a w and the height to \a h.
*/

void QWidget::setFixedSize(int w, int h)
{
    Q_D(QWidget);
    bool minSizeSet = d->setMinimumSize_helper(w, h);
    bool maxSizeSet = d->setMaximumSize_helper(w, h);
    if (!minSizeSet && !maxSizeSet)
        return;

    if (isWindow())
        d->setConstraints_sys();
    else
        d->updateGeometry_helper(true);

    if (w != QWIDGETSIZE_MAX || h != QWIDGETSIZE_MAX)
        resize(w, h);
}

void QWidget::setMinimumWidth(int w)
{
    Q_D(QWidget);
    d->createExtra();
    Qt::Orientations expl = d->extra->explicitMinSize;
    if (w)
        expl |= Qt::Horizontal;
    setMinimumSize(w, minimumSize().height());
    d->extra->explicitMinSize = expl;
}

void QWidget::setMinimumHeight(int h)
{
    Q_D(QWidget);
    d->createExtra();
    Qt::Orientations expl = d->extra->explicitMinSize;
    if (h)
        expl |= Qt::Vertical;
    setMinimumSize(minimumSize().width(), h);
    d->extra->explicitMinSize = expl;
}

void QWidget::setMaximumWidth(int w)
{
    Q_D(QWidget);
    d->createExtra();
    Qt::Orientations expl = d->extra->explicitMaxSize;
    if (w != QWIDGETSIZE_MAX)
        expl |= Qt::Horizontal;
    setMaximumSize(w, maximumSize().height());
    d->extra->explicitMaxSize = expl;
}

void QWidget::setMaximumHeight(int h)
{
    Q_D(QWidget);
    d->createExtra();
    Qt::Orientations expl = d->extra->explicitMaxSize;
    if (h != QWIDGETSIZE_MAX)
        expl |= Qt::Vertical;
    setMaximumSize(maximumSize().width(), h);
    d->extra->explicitMaxSize = expl;
}

/*!
    Sets both the minimum and maximum width of the widget to \a w
    without changing the heights. Provided for convenience.

    \sa sizeHint() minimumSize() maximumSize() setFixedSize()
*/

void QWidget::setFixedWidth(int w)
{
    Q_D(QWidget);
    d->createExtra();
    Qt::Orientations explMin = d->extra->explicitMinSize | Qt::Horizontal;
    Qt::Orientations explMax = d->extra->explicitMaxSize | Qt::Horizontal;
    setMinimumSize(w, minimumSize().height());
    setMaximumSize(w, maximumSize().height());
    d->extra->explicitMinSize = explMin;
    d->extra->explicitMaxSize = explMax;
}


/*!
    Sets both the minimum and maximum heights of the widget to \a h
    without changing the widths. Provided for convenience.

    \sa sizeHint() minimumSize() maximumSize() setFixedSize()
*/

void QWidget::setFixedHeight(int h)
{
    Q_D(QWidget);
    d->createExtra();
    Qt::Orientations explMin = d->extra->explicitMinSize | Qt::Vertical;
    Qt::Orientations explMax = d->extra->explicitMaxSize | Qt::Vertical;
    setMinimumSize(minimumSize().width(), h);
    setMaximumSize(maximumSize().width(), h);
    d->extra->explicitMinSize = explMin;
    d->extra->explicitMaxSize = explMax;
}


/*!
    Translates the widget coordinate \a pos to the coordinate system
    of \a parent. The \a parent must not be 0 and must be a parent
    of the calling widget.

    \sa mapFrom() mapToParent() mapToGlobal() underMouse()
*/

QPoint QWidget::mapTo(QWidget * parent, const QPoint & pos) const
{
    QPoint p = pos;
    if (parent) {
        const QWidget * w = this;
        while (w != parent) {
            Q_ASSERT_X(w, "QWidget::mapTo(QWidget *parent, const QPoint &pos)",
                       "parent must be in parent hierarchy");
            p = w->mapToParent(p);
            w = w->parentWidget();
        }
    }
    return p;
}


/*!
    Translates the widget coordinate \a pos from the coordinate system
    of \a parent to this widget's coordinate system. The \a parent
    must not be 0 and must be a parent of the calling widget.

    \sa mapTo() mapFromParent() mapFromGlobal() underMouse()
*/

QPoint QWidget::mapFrom(QWidget * parent, const QPoint & pos) const
{
    QPoint p(pos);
    if (parent) {
        const QWidget * w = this;
        while (w != parent) {
            Q_ASSERT_X(w, "QWidget::mapFrom(QWidget *parent, const QPoint &pos)",
                       "parent must be in parent hierarchy");

            p = w->mapFromParent(p);
            w = w->parentWidget();
        }
    }
    return p;
}


/*!
    Translates the widget coordinate \a pos to a coordinate in the
    parent widget.

    Same as mapToGlobal() if the widget has no parent.

    \sa mapFromParent() mapTo() mapToGlobal() underMouse()
*/

QPoint QWidget::mapToParent(const QPoint &pos) const
{
    return pos + data->crect.topLeft();
}

/*!
    Translates the parent widget coordinate \a pos to widget
    coordinates.

    Same as mapFromGlobal() if the widget has no parent.

    \sa mapToParent() mapFrom() mapFromGlobal() underMouse()
*/

QPoint QWidget::mapFromParent(const QPoint &pos) const
{
    return pos - data->crect.topLeft();
}


/*!
    Returns the window for this widget, i.e. the next ancestor widget
    that has (or could have) a window-system frame.

    If the widget is a window, the widget itself is returned.

    Typical usage is changing the window title:

    \snippet doc/src/snippets/code/src_gui_kernel_qwidget.cpp 3

    \sa isWindow()
*/

QWidget *QWidget::window() const
{
    QWidget *w = (QWidget *)this;
    QWidget *p = w->parentWidget();
    while (!w->isWindow() && p) {
        w = p;
        p = p->parentWidget();
    }
    return w;
}

/*!
    \since 4.4

    Returns the native parent for this widget, i.e. the next ancestor widget
    that has a system identifier, or 0 if it does not have any native parent.

    \sa effectiveWinId()
*/
QWidget *QWidget::nativeParentWidget() const
{
    QWidget *parent = parentWidget();
    while (parent && !parent->internalWinId())
        parent = parent->parentWidget();
    return parent;
}

/*!
  Returns the background role of the widget.

  The background role defines the brush from the widget's \l palette that
  is used to render the background.

  If no explicit background role is set, the widget inherts its parent
  widget's background role.

  \sa setBackgroundRole(), foregroundRole()
 */
QPalette::ColorRole QWidget::backgroundRole() const
{

    const QWidget *w = this;
    do {
        QPalette::ColorRole role = w->d_func()->bg_role;
        if (role != QPalette::NoRole)
            return role;
        if (w->isWindow() || w->windowType() == Qt::SubWindow)
            break;
        w = w->parentWidget();
    } while (w);
    return QPalette::Window;
}

/*!
  Sets the background role of the widget to \a role.

  The background role defines the brush from the widget's \l palette that
  is used to render the background.

  If \a role is QPalette::NoRole, then the widget inherits its
  parent's background role.

  Note that styles are free to choose any color from the palette.
  You can modify the palette or set a style sheet if you don't
  achieve the result you want with setBackgroundRole().

  \sa backgroundRole(), foregroundRole()
 */

void QWidget::setBackgroundRole(QPalette::ColorRole role)
{
    Q_D(QWidget);
    d->bg_role = role;
    d->updateSystemBackground();
    d->propagatePaletteChange();
    d->updateIsOpaque();
}

/*!
  Returns the foreground role.

  The foreground role defines the color from the widget's \l palette that
  is used to draw the foreground.

  If no explicit foreground role is set, the function returns a role
  that contrasts with the background role.

  \sa setForegroundRole(), backgroundRole()
 */
QPalette::ColorRole QWidget::foregroundRole() const
{
    Q_D(const QWidget);
    QPalette::ColorRole rl = QPalette::ColorRole(d->fg_role);
    if (rl != QPalette::NoRole)
        return rl;
    QPalette::ColorRole role = QPalette::WindowText;
    switch (backgroundRole()) {
    case QPalette::Button:
        role = QPalette::ButtonText;
        break;
    case QPalette::Base:
        role = QPalette::Text;
        break;
    case QPalette::Dark:
    case QPalette::Shadow:
        role = QPalette::Light;
        break;
    case QPalette::Highlight:
        role = QPalette::HighlightedText;
        break;
    case QPalette::ToolTipBase:
        role = QPalette::ToolTipText;
        break;
    default:
        ;
    }
    return role;
}

/*!
  Sets the foreground role of the widget to \a role.

  The foreground role defines the color from the widget's \l palette that
  is used to draw the foreground.

  If \a role is QPalette::NoRole, the widget uses a foreground role
  that contrasts with the background role.

  Note that styles are free to choose any color from the palette.
  You can modify the palette or set a style sheet if you don't
  achieve the result you want with setForegroundRole().

  \sa foregroundRole(), backgroundRole()
 */
void QWidget::setForegroundRole(QPalette::ColorRole role)
{
    Q_D(QWidget);
    d->fg_role = role;
    d->updateSystemBackground();
    d->propagatePaletteChange();
}

/*!
    \property QWidget::palette
    \brief the widget's palette

    This property describes the widget's palette. The palette is used by the
    widget's style when rendering standard components, and is available as a
    means to ensure that custom widgets can maintain consistency with the
    native platform's look and feel. It's common that different platforms, or
    different styles, have different palettes.

    When you assign a new palette to a widget, the color roles from this
    palette are combined with the widget's default palette to form the
    widget's final palette. The palette entry for the widget's background role
    is used to fill the widget's background (see QWidget::autoFillBackground),
    and the foreground role initializes QPainter's pen.

    The default depends on the system environment. QApplication maintains a
    system/theme palette which serves as a default for all widgets.  There may
    also be special palette defaults for certain types of widgets (e.g., on
    Windows XP and Vista, all classes that derive from QMenuBar have a special
    default palette). You can also define default palettes for widgets
    yourself by passing a custom palette and the name of a widget to
    QApplication::setPalette(). Finally, the style always has the option of
    polishing the palette as it's assigned (see QStyle::polish()).

    QWidget propagates explicit palette roles from parent to child. If you
    assign a brush or color to a specific role on a palette and assign that
    palette to a widget, that role will propagate to all the widget's
    children, overriding any system defaults for that role. Note that palettes
    by default don't propagate to windows (see isWindow()) unless the
    Qt::WA_WindowPropagation attribute is enabled.

    QWidget's palette propagation is similar to its font propagation.

    The current style, which is used to render the content of all standard Qt
    widgets, is free to choose colors and brushes from the widget palette, or
    in some cases, to ignore the palette (partially, or completely). In
    particular, certain styles like GTK style, Mac style, Windows XP, and
    Vista style, depend on third party APIs to render the content of widgets,
    and these styles typically do not follow the palette. Because of this,
    assigning roles to a widget's palette is not guaranteed to change the
    appearance of the widget.

    \sa QApplication::palette(), QWidget::font()
*/
const QPalette &QWidget::palette() const
{
    if (!isEnabled()) {
        data->pal.setCurrentColorGroup(QPalette::Disabled);
    } else if (!isVisible() || isActiveWindow()) {
        data->pal.setCurrentColorGroup(QPalette::Active);
    } else {
        data->pal.setCurrentColorGroup(QPalette::Inactive);
    }
    return data->pal;
}

void QWidget::setPalette(const QPalette &palette)
{
    Q_D(QWidget);
    setAttribute(Qt::WA_SetPalette, palette.resolve() != 0);

    // Determine which palette is inherited from this widget's ancestors and
    // QApplication::palette, resolve this against \a palette (attributes from
    // the inherited palette are copied over this widget's palette). Then
    // propagate this palette to this widget's children.
    QPalette naturalPalette = d->naturalWidgetPalette(d->inheritedPaletteResolveMask);
    QPalette resolvedPalette = palette.resolve(naturalPalette);
    d->setPalette_helper(resolvedPalette);
}

/*!
    \internal

    Returns the palette that the widget \a w inherits from its ancestors and
    QApplication::palette. \a inheritedMask is the combination of the widget's
    ancestors palette request masks (i.e., which attributes from the parent
    widget's palette are implicitly imposed on this widget by the user). Note
    that this font does not take into account the palette set on \a w itself.
*/
QPalette QWidgetPrivate::naturalWidgetPalette(uint inheritedMask) const
{
    Q_Q(const QWidget);
    QPalette naturalPalette = QApplication::palette(q);
    if ((!q->isWindow() || q->testAttribute(Qt::WA_WindowPropagation)
#ifndef QT_NO_GRAPHICSVIEW
            || (extra && extra->proxyWidget)
#endif //QT_NO_GRAPHICSVIEW
            )) {
        if (QWidget *p = q->parentWidget()) {
            if (!naturalPalette.isCopyOf(QApplication::palette())) {
                QPalette inheritedPalette = p->palette();
                inheritedPalette.resolve(inheritedMask);
                naturalPalette = inheritedPalette.resolve(naturalPalette);
            } else {
                naturalPalette = p->palette();
            }
        }
#ifndef QT_NO_GRAPHICSVIEW
        else if (extra && extra->proxyWidget) {
            QPalette inheritedPalette = extra->proxyWidget->palette();
            inheritedPalette.resolve(inheritedMask);
            naturalPalette = inheritedPalette.resolve(naturalPalette);
        }
#endif //QT_NO_GRAPHICSVIEW
    }
    naturalPalette.resolve(0);
    return naturalPalette;
}
/*!
    \internal

    Determine which palette is inherited from this widget's ancestors and
    QApplication::palette, resolve this against this widget's palette
    (attributes from the inherited palette are copied over this widget's
    palette). Then propagate this palette to this widget's children.
*/
void QWidgetPrivate::resolvePalette()
{
    QPalette naturalPalette = naturalWidgetPalette(inheritedPaletteResolveMask);
    QPalette resolvedPalette = data.pal.resolve(naturalPalette);
    setPalette_helper(resolvedPalette);
}

void QWidgetPrivate::setPalette_helper(const QPalette &palette)
{
    Q_Q(QWidget);
    if (data.pal == palette && data.pal.resolve() == palette.resolve())
        return;
    data.pal = palette;
    updateSystemBackground();
    propagatePaletteChange();
    updateIsOpaque();
    q->update();
    updateIsOpaque();
}

/*!
    \property QWidget::font
    \brief the font currently set for the widget

    This property describes the widget's requested font. The font is used by
    the widget's style when rendering standard components, and is available as
    a means to ensure that custom widgets can maintain consistency with the
    native platform's look and feel. It's common that different platforms, or
    different styles, define different fonts for an application.

    When you assign a new font to a widget, the properties from this font are
    combined with the widget's default font to form the widget's final
    font. You can call fontInfo() to get a copy of the widget's final
    font. The final font is also used to initialize QPainter's font.

    The default depends on the system environment. QApplication maintains a
    system/theme font which serves as a default for all widgets.  There may
    also be special font defaults for certain types of widgets. You can also
    define default fonts for widgets yourself by passing a custom font and the
    name of a widget to QApplication::setFont(). Finally, the font is matched
    against Qt's font database to find the best match.

    QWidget propagates explicit font properties from parent to child. If you
    change a specific property on a font and assign that font to a widget,
    that property will propagate to all the widget's children, overriding any
    system defaults for that property. Note that fonts by default don't
    propagate to windows (see isWindow()) unless the Qt::WA_WindowPropagation
    attribute is enabled.

    QWidget's font propagation is similar to its palette propagation.

    The current style, which is used to render the content of all standard Qt
    widgets, is free to choose to use the widget font, or in some cases, to
    ignore it (partially, or completely). In particular, certain styles like
    GTK style, Mac style, Windows XP, and Vista style, apply special
    modifications to the widget font to match the platform's native look and
    feel. Because of this, assigning properties to a widget's font is not
    guaranteed to change the appearance of the widget. Instead, you may choose
    to apply a \l styleSheet.

    \note If \l{Qt Style Sheets} are used on the same widget as setFont(),
    style sheets will take precedence if the settings conflict.

    \sa fontInfo(), fontMetrics()
*/

void QWidget::setFont(const QFont &font)
{
    Q_D(QWidget);

    setAttribute(Qt::WA_SetFont, font.resolve() != 0);

    // Determine which font is inherited from this widget's ancestors and
    // QApplication::font, resolve this against \a font (attributes from the
    // inherited font are copied over). Then propagate this font to this
    // widget's children.
    QFont naturalFont = d->naturalWidgetFont(d->inheritedFontResolveMask);
    QFont resolvedFont = font.resolve(naturalFont);
    d->setFont_helper(resolvedFont);
}

/*
    \internal

    Returns the font that the widget \a w inherits from its ancestors and
    QApplication::font. \a inheritedMask is the combination of the widget's
    ancestors font request masks (i.e., which attributes from the parent
    widget's font are implicitly imposed on this widget by the user). Note
    that this font does not take into account the font set on \a w itself.
*/
QFont QWidgetPrivate::naturalWidgetFont(uint inheritedMask) const
{
    Q_Q(const QWidget);
    QFont naturalFont = QApplication::font(q);
    if ((!q->isWindow() || q->testAttribute(Qt::WA_WindowPropagation)
#ifndef QT_NO_GRAPHICSVIEW
            || (extra && extra->proxyWidget)
#endif //QT_NO_GRAPHICSVIEW
            )) {
        if (QWidget *p = q->parentWidget()) {
            if (!naturalFont.isCopyOf(QApplication::font())) {
                QFont inheritedFont = p->font();
                inheritedFont.resolve(inheritedMask);
                naturalFont = inheritedFont.resolve(naturalFont);
            } else {
                naturalFont = p->font();
            }
        }
#ifndef QT_NO_GRAPHICSVIEW
        else if (extra && extra->proxyWidget) {
            QFont inheritedFont = extra->proxyWidget->font();
            inheritedFont.resolve(inheritedMask);
            naturalFont = inheritedFont.resolve(naturalFont);
        }
#endif //QT_NO_GRAPHICSVIEW
    }
    naturalFont.resolve(0);
    return naturalFont;
}

/*!
    \internal

    Determine which font is implicitly imposed on this widget by its ancestors
    and QApplication::font, resolve this against its own font (attributes from
    the implicit font are copied over). Then propagate this font to this
    widget's children.
*/
void QWidgetPrivate::resolveFont()
{
    QFont naturalFont = naturalWidgetFont(inheritedFontResolveMask);
    QFont resolvedFont = data.fnt.resolve(naturalFont);
    setFont_helper(resolvedFont);
}

/*!
    \internal

    Assign \a font to this widget, and propagate it to all children, except
    style sheet widgets (handled differently) and windows that don't enable
    window propagation.  \a implicitMask is the union of all ancestor widgets'
    font request masks, and determines which attributes from this widget's
    font should propagate.
*/
void QWidgetPrivate::updateFont(const QFont &font)
{
    Q_Q(QWidget);

    QFont old = data.fnt;
    data.fnt = QFont(font, q);
#if defined(Q_WS_X11)
    // make sure the font set on this widget is associated with the correct screen
    data.fnt.x11SetScreen(xinfo.screen());
#endif
    // Combine new mask with natural mask and propagate to children.
#ifndef QT_NO_GRAPHICSVIEW
    if (!q->parentWidget() && extra && extra->proxyWidget) {
        QGraphicsProxyWidget *p = extra->proxyWidget;
        inheritedFontResolveMask = p->d_func()->inheritedFontResolveMask | p->font().resolve();
    } else
#endif //QT_NO_GRAPHICSVIEW
    if (q->isWindow() && !q->testAttribute(Qt::WA_WindowPropagation)) {
        inheritedFontResolveMask = 0;
    }
    uint newMask = data.fnt.resolve() | inheritedFontResolveMask;

    for (int i = 0; i < children.size(); ++i) {
        QWidget *w = qobject_cast<QWidget*>(children.at(i));
        if (w && (!w->isWindow() || w->testAttribute(Qt::WA_WindowPropagation))) {
            // Propagate font changes.
            QWidgetPrivate *wd = w->d_func();
            wd->inheritedFontResolveMask = newMask;
            wd->resolveFont();
        }
    }

    QEvent e(QEvent::FontChange);
    QApplication::sendEvent(q, &e);
    q->fontChange(old);
}

void QWidgetPrivate::setLayoutDirection_helper(Qt::LayoutDirection direction)
{
    Q_Q(QWidget);

    if ( (direction == Qt::RightToLeft) == q->testAttribute(Qt::WA_RightToLeft))
        return;
    q->setAttribute(Qt::WA_RightToLeft, (direction == Qt::RightToLeft));
    if (!children.isEmpty()) {
        for (int i = 0; i < children.size(); ++i) {
            QWidget *w = qobject_cast<QWidget*>(children.at(i));
            if (w && !w->isWindow() && !w->testAttribute(Qt::WA_SetLayoutDirection))
                w->d_func()->setLayoutDirection_helper(direction);
        }
    }
    QEvent e(QEvent::LayoutDirectionChange);
    QApplication::sendEvent(q, &e);
}

void QWidgetPrivate::resolveLayoutDirection()
{
    Q_Q(const QWidget);
    if (!q->testAttribute(Qt::WA_SetLayoutDirection))
        setLayoutDirection_helper(q->isWindow() ? QApplication::layoutDirection() : q->parentWidget()->layoutDirection());
}

/*!
    \property QWidget::layoutDirection

    \brief the layout direction for this widget

    By default, this property is set to Qt::LeftToRight.

    When the layout direction is set on a widget, it will propagate to
    the widget's children, but not to a child that is a window and not
    to a child for which setLayoutDirection() has been explicitly
    called. Also, child widgets added \e after setLayoutDirection()
    has been called for the parent do not inherit the parent's layout
    direction.

    This method no longer affects text layout direction since Qt 4.7.

    \sa QApplication::layoutDirection
*/
void QWidget::setLayoutDirection(Qt::LayoutDirection direction)
{
    Q_D(QWidget);

    if (direction == Qt::LayoutDirectionAuto) {
        unsetLayoutDirection();
        return;
    }

    setAttribute(Qt::WA_SetLayoutDirection);
    d->setLayoutDirection_helper(direction);
}

Qt::LayoutDirection QWidget::layoutDirection() const
{
    return testAttribute(Qt::WA_RightToLeft) ? Qt::RightToLeft : Qt::LeftToRight;
}

void QWidget::unsetLayoutDirection()
{
    Q_D(QWidget);
    setAttribute(Qt::WA_SetLayoutDirection, false);
    d->resolveLayoutDirection();
}

/*!
    \fn QFontMetrics QWidget::fontMetrics() const

    Returns the font metrics for the widget's current font.
    Equivalent to QFontMetrics(widget->font()).

    \sa font(), fontInfo(), setFont()
*/

/*!
    \property QWidget::cursor
    \brief the cursor shape for this widget

    The mouse cursor will assume this shape when it's over this
    widget. See the \link Qt::CursorShape list of predefined cursor
    objects\endlink for a range of useful shapes.

    An editor widget might use an I-beam cursor:
    \snippet doc/src/snippets/code/src_gui_kernel_qwidget.cpp 6

    If no cursor has been set, or after a call to unsetCursor(), the
    parent's cursor is used.

    By default, this property contains a cursor with the Qt::ArrowCursor
    shape.

    Some underlying window implementations will reset the cursor if it
    leaves a widget even if the mouse is grabbed. If you want to have
    a cursor set for all widgets, even when outside the window, consider
    QApplication::setOverrideCursor().

    \sa QApplication::setOverrideCursor()
*/

#ifndef QT_NO_CURSOR
QCursor QWidget::cursor() const
{
    Q_D(const QWidget);
    if (testAttribute(Qt::WA_SetCursor))
        return (d->extra && d->extra->curs)
            ? *d->extra->curs
            : QCursor(Qt::ArrowCursor);
    if (isWindow() || !parentWidget())
        return QCursor(Qt::ArrowCursor);
    return parentWidget()->cursor();
}

void QWidget::setCursor(const QCursor &cursor)
{
    Q_D(QWidget);
    if (cursor.shape() != Qt::ArrowCursor || (d->extra && d->extra->curs)) {
        d->createExtra();
        QCursor *newCursor = new QCursor(cursor);
        delete d->extra->curs;
        d->extra->curs = newCursor;
    }
    setAttribute(Qt::WA_SetCursor);
    d->setCursor_sys(cursor);

    QEvent event(QEvent::CursorChange);
    QApplication::sendEvent(this, &event);
}

void QWidget::unsetCursor()
{
    Q_D(QWidget);
    if (d->extra) {
        delete d->extra->curs;
        d->extra->curs = 0;
    }
    if (!isWindow())
        setAttribute(Qt::WA_SetCursor, false);
    d->unsetCursor_sys();

    QEvent event(QEvent::CursorChange);
    QApplication::sendEvent(this, &event);
}

#endif

/*!
    \enum QWidget::RenderFlag

    This enum describes how to render the widget when calling QWidget::render().

    \value DrawWindowBackground If you enable this option, the widget's background
    is rendered into the target even if autoFillBackground is not set. By default,
    this option is enabled.

    \value DrawChildren If you enable this option, the widget's children
    are rendered recursively into the target. By default, this option is enabled.

    \value IgnoreMask If you enable this option, the widget's QWidget::mask()
    is ignored when rendering into the target. By default, this option is disabled.

    \since 4.3
*/

/*!
    \since 4.3

    Renders the \a sourceRegion of this widget into the \a target
    using \a renderFlags to determine how to render. Rendering
    starts at \a targetOffset in the \a target. For example:

    \snippet doc/src/snippets/code/src_gui_kernel_qwidget.cpp 7

    If \a sourceRegion is a null region, this function will use QWidget::rect() as
    the region, i.e. the entire widget.

    Ensure that you call QPainter::end() for the \a target device's
    active painter (if any) before rendering. For example:

    \snippet doc/src/snippets/code/src_gui_kernel_qwidget.cpp 8
*/
void QWidget::render(QPaintDevice *target, const QPoint &targetOffset,
                     const QRegion &sourceRegion, RenderFlags renderFlags)
{
    d_func()->render(target, targetOffset, sourceRegion, renderFlags, false);
}

/*!
    \overload

    Renders the widget into the \a painter's QPainter::device().

    Transformations and settings applied to the \a painter will be used
    when rendering.

    \note The \a painter must be active. On Mac OS X the widget will be
    rendered into a QPixmap and then drawn by the \a painter.

    \sa QPainter::device()
*/
void QWidget::render(QPainter *painter, const QPoint &targetOffset,
                     const QRegion &sourceRegion, RenderFlags renderFlags)
{
    if (Q_UNLIKELY(!painter)) {
        qWarning("QWidget::render: Null pointer to painter");
        return;
    } else if (Q_UNLIKELY(!painter->isActive())) {
        qWarning("QWidget::render: Cannot render with an inactive painter");
        return;
    }

    const qreal opacity = painter->opacity();
    if (qFuzzyIsNull(opacity))
        return; // Fully transparent.

    Q_D(QWidget);
    const bool inRenderWithPainter = d->extra && d->extra->inRenderWithPainter;
    const QRegion toBePainted = !inRenderWithPainter ? d->prepareToRender(sourceRegion, renderFlags)
                                                     : sourceRegion;
    if (toBePainted.isEmpty())
        return;

    if (!d->extra)
        d->createExtra();
    d->extra->inRenderWithPainter = true;

    QPaintEngine *engine = painter->paintEngine();
    Q_ASSERT(engine);
    QPaintEnginePrivate *enginePriv = engine->d_func();
    Q_ASSERT(enginePriv);
    QPaintDevice *target = engine->paintDevice();
    Q_ASSERT(target);

    // Render via a pixmap when dealing with non-opaque painters or printers.
    if (!inRenderWithPainter && (opacity < 1.0 || (target->devType() == QInternal::Printer))) {
        d->render_helper(painter, targetOffset, toBePainted, renderFlags);
        d->extra->inRenderWithPainter = false;
        return;
    }

    // Set new shared painter.
    QPainter *oldPainter = d->sharedPainter();
    d->setSharedPainter(painter);

    // Save current system clip, viewport and transform,
    const QTransform oldTransform = enginePriv->systemTransform;
    const QRegion oldSystemClip = enginePriv->systemClip;
    const QRegion oldSystemViewport = enginePriv->systemViewport;

    // This ensures that all painting triggered by render() is clipped to the current engine clip.
    if (painter->hasClipping()) {
        const QRegion painterClip = painter->deviceTransform().map(painter->clipRegion());
        enginePriv->setSystemViewport(oldSystemClip.isEmpty() ? painterClip : oldSystemClip & painterClip);
    } else {
        enginePriv->setSystemViewport(oldSystemClip);
    }

    render(target, targetOffset, toBePainted, renderFlags);

    // Restore system clip, viewport and transform.
    enginePriv->systemClip = oldSystemClip;
    enginePriv->setSystemViewport(oldSystemViewport);
    enginePriv->setSystemTransform(oldTransform);

    // Restore shared painter.
    d->setSharedPainter(oldPainter);

    d->extra->inRenderWithPainter = false;
}

bool QWidgetPrivate::isAboutToShow() const
{
    if (data.in_show)
        return true;

    Q_Q(const QWidget);
    if (q->isHidden())
        return false;

    // The widget will be shown if any of its ancestors are about to show.
    QWidget *parent = q->parentWidget();
    return parent ? parent->d_func()->isAboutToShow() : false;
}

QRegion QWidgetPrivate::prepareToRender(const QRegion &region, QWidget::RenderFlags renderFlags)
{
    Q_Q(QWidget);
    const bool isVisible = q->isVisible();

    // Make sure the widget is laid out correctly.
    if (!isVisible && !isAboutToShow()) {
        QWidget *topLevel = q->window();
        (void)topLevel->d_func()->topData(); // Make sure we at least have top-data.
        topLevel->ensurePolished();

        // Invalidate the layout of hidden ancestors (incl. myself) and pretend
        // they're not explicitly hidden.
        QWidget *widget = q;
        QWidgetList hiddenWidgets;
        while (widget) {
            if (widget->isHidden()) {
                widget->setAttribute(Qt::WA_WState_Hidden, false);
                hiddenWidgets.append(widget);
                if (!widget->isWindow() && widget->parentWidget()->d_func()->layout)
                    widget->d_func()->updateGeometry_helper(true);
            }
            widget = widget->parentWidget();
        }

        // Activate top-level layout.
        if (topLevel->d_func()->layout)
            topLevel->d_func()->layout->activate();

        // Adjust size if necessary.
        QTLWExtra *topLevelExtra = topLevel->d_func()->maybeTopData();
        if (topLevelExtra && !topLevelExtra->sizeAdjusted
            && !topLevel->testAttribute(Qt::WA_Resized)) {
            topLevel->adjustSize();
            topLevel->setAttribute(Qt::WA_Resized, false);
        }

        // Activate child layouts.
        topLevel->d_func()->activateChildLayoutsRecursively();

        // We're not cheating with WA_WState_Hidden anymore.
        for (int i = 0; i < hiddenWidgets.size(); ++i) {
            QWidget *widget = hiddenWidgets.at(i);
            widget->setAttribute(Qt::WA_WState_Hidden);
            if (!widget->isWindow() && widget->parentWidget()->d_func()->layout)
                widget->parentWidget()->d_func()->layout->invalidate();
        }
    } else if (isVisible) {
        q->window()->d_func()->sendPendingMoveAndResizeEvents(true, true);
    }

    // Calculate the region to be painted.
    QRegion toBePainted = !region.isEmpty() ? region : QRegion(q->rect());
    if (!(renderFlags & QWidget::IgnoreMask) && extra && extra->hasMask)
        toBePainted &= extra->mask;
    return toBePainted;
}

void QWidgetPrivate::render_helper(QPainter *painter, const QPoint &targetOffset, const QRegion &toBePainted,
                                   QWidget::RenderFlags renderFlags)
{
    Q_ASSERT(painter);
    Q_ASSERT(!toBePainted.isEmpty());

    Q_Q(QWidget);
    const QTransform originalTransform = painter->worldTransform();
    if (!originalTransform.isScaling()) {
        // Render via a image.
        const QRect rect = toBePainted.boundingRect();
        const QSize size = rect.size();
        if (size.isNull())
            return;

        QImage image(size, QImage::Format_ARGB32_Premultiplied);
        if (!(renderFlags & QWidget::DrawWindowBackground) || !isOpaque)
            image.fill(Qt::transparent);
        q->render(&image, QPoint(), toBePainted, renderFlags);

        painter->drawImage(targetOffset, image);

    } else {
        // Render via a image in device coordinates (to avoid image scaling).
        QTransform transform = originalTransform;
        transform.translate(targetOffset.x(), targetOffset.y());

        QPaintDevice *device = painter->device();
        Q_ASSERT(device);

        // Calculate device rect.
        const QRectF rect(toBePainted.boundingRect());
        QRect deviceRect = transform.mapRect(QRectF(0, 0, rect.width(), rect.height())).toAlignedRect();
        deviceRect &= QRect(0, 0, device->width(), device->height());

        QImage image(deviceRect.size(), QImage::Format_ARGB32_Premultiplied);
        image.fill(Qt::transparent);

        // Create a image device coordinate painter.
        QPainter imagePainter(&image);
        imagePainter.setRenderHints(painter->renderHints());
        transform *= QTransform::fromTranslate(-deviceRect.x(), -deviceRect.y());
        imagePainter.setTransform(transform);

        q->render(&imagePainter, QPoint(), toBePainted, renderFlags);
        imagePainter.end();

        // And then draw the image.
        painter->setTransform(QTransform());
        painter->drawImage(deviceRect.topLeft(), image);
        painter->setTransform(originalTransform);
    }
}

void QWidgetPrivate::drawWidget(QPaintDevice *pdev, const QRegion &rgn, const QPoint &offset, int flags,
                                QPainter *sharedPainter, QWidgetBackingStore *backingStore)
{
    if (rgn.isEmpty())
        return;

    Q_Q(QWidget);

    const bool asRoot = flags & DrawAsRoot;
    const bool alsoOnScreen = flags & DrawPaintOnScreen;
    const bool recursive = flags & DrawRecursive;
    const bool alsoInvisible = flags & DrawInvisible;

    Q_ASSERT(sharedPainter ? sharedPainter->isActive() : true);

    QRegion toBePainted(rgn);
    if (asRoot && !alsoInvisible)
        toBePainted &= clipRect(); //(rgn & visibleRegion());
    if (!(flags & DontSubtractOpaqueChildren))
        subtractOpaqueChildren(toBePainted, q->rect());

    if (!toBePainted.isEmpty()) {
        bool onScreen = paintOnScreen();
        if (!onScreen || alsoOnScreen) {
            //update the "in paint event" flag
            if (Q_UNLIKELY(q->testAttribute(Qt::WA_WState_InPaintEvent)))
                qWarning("QWidget::repaint: Recursive repaint detected");
            q->setAttribute(Qt::WA_WState_InPaintEvent);

            //clip away the new area
#ifndef QT_NO_DEBUG
            bool flushed = QWidgetBackingStore::flushPaint(q, toBePainted);
#endif
            QPaintEngine *paintEngine = pdev->paintEngine();
            if (paintEngine) {
                setRedirected(pdev, -offset);

                if (sharedPainter)
                    paintEngine->d_func()->systemClip = toBePainted;
                else
                    paintEngine->d_func()->systemRect = q->data->crect;

                //paint the background
                if ((asRoot || q->autoFillBackground() || onScreen || q->testAttribute(Qt::WA_StyledBackground))
                    && !q->testAttribute(Qt::WA_OpaquePaintEvent) && !q->testAttribute(Qt::WA_NoSystemBackground)) {
                    QPainter p(q);
                    paintBackground(&p, toBePainted, (asRoot || onScreen) ? flags | DrawAsRoot : 0);
                }

                if (!sharedPainter)
                    paintEngine->d_func()->systemClip = toBePainted.translated(offset);
            }

#if 0
            qDebug() << "painting" << q << "opaque ==" << isOpaque();
            qDebug() << "clipping to" << toBePainted << "location == " << offset
                     << "geometry ==" << QRect(q->mapTo(q->window(), QPoint(0, 0)), q->size());
#endif

            //actually send the paint event
            QPaintEvent e(toBePainted);
            QCoreApplication::sendSpontaneousEvent(q, &e);
            if (backingStore && !onScreen && !asRoot && (q->internalWinId() || !q->nativeParentWidget()->isWindow()))
                backingStore->markDirtyOnScreen(toBePainted, q, offset);

            //restore
            if (paintEngine) {
                restoreRedirected();
                if (!sharedPainter)
                    paintEngine->d_func()->systemRect = QRect();
                else
                    paintEngine->d_func()->currentClipWidget = 0;
                paintEngine->d_func()->systemClip = QRegion();
            }
            q->setAttribute(Qt::WA_WState_InPaintEvent, false);
            if (Q_UNLIKELY(q->paintingActive()))
                qWarning("QWidget::repaint: It is dangerous to leave painters active on a widget outside of the PaintEvent");

#ifndef QT_NO_DEBUG
            if (flushed)
                QWidgetBackingStore::unflushPaint(q, toBePainted);
#endif
        } else if (q->isWindow()) {
            QPaintEngine *engine = pdev->paintEngine();
            if (engine) {
                QPainter p(pdev);
                p.setClipRegion(toBePainted);
                const QBrush bg = q->palette().brush(QPalette::Window);
                if (bg.style() == Qt::TexturePattern)
                    p.drawTiledPixmap(q->rect(), bg.texture());
                else
                    p.fillRect(q->rect(), bg);
            }
        }
    }

    if (recursive && !children.isEmpty()) {
        paintSiblingsRecursive(pdev, children, children.size() - 1, rgn, offset, flags & ~DrawAsRoot,
                               sharedPainter, backingStore);
    }
}

void QWidgetPrivate::render(QPaintDevice *target, const QPoint &targetOffset,
                            const QRegion &sourceRegion, QWidget::RenderFlags renderFlags,
                            bool readyToRender)
{
    if (Q_UNLIKELY(!target)) {
        qWarning("QWidget::render: null pointer to paint device");
        return;
    }

    const bool inRenderWithPainter = extra && extra->inRenderWithPainter;
    QRegion paintRegion = !inRenderWithPainter && !readyToRender
                          ? prepareToRender(sourceRegion, renderFlags)
                          : sourceRegion;
    if (paintRegion.isEmpty())
        return;

    QPainter *oldSharedPainter = inRenderWithPainter ? sharedPainter() : 0;

    // Use the target's shared painter if set (typically set when doing
    // "other->render(widget);" in the widget's paintEvent.
    if (target->devType() == QInternal::Widget) {
        QWidgetPrivate *targetPrivate = static_cast<QWidget *>(target)->d_func();
        if (targetPrivate->extra && targetPrivate->extra->inRenderWithPainter) {
            QPainter *targetPainter = targetPrivate->sharedPainter();
            if (targetPainter && targetPainter->isActive())
                setSharedPainter(targetPainter);
        }
    }

    // Use the target's redirected device if set and adjust offset and paint
    // region accordingly. This is typically the case when people call render
    // from the paintEvent.
    QPoint offset = targetOffset;
    offset -= paintRegion.boundingRect().topLeft();
    QPoint redirectionOffset;
    QPaintDevice *redirected = 0;

    if (target->devType() == QInternal::Widget)
        redirected = static_cast<QWidget *>(target)->d_func()->redirected(&redirectionOffset);

    if (redirected) {
        target = redirected;
        offset -= redirectionOffset;
    }

    if (!inRenderWithPainter) { // Clip handled by shared painter (in qpainter.cpp).
        if (QPaintEngine *targetEngine = target->paintEngine()) {
            const QRegion targetSystemClip = targetEngine->systemClip();
            if (!targetSystemClip.isEmpty())
                paintRegion &= targetSystemClip.translated(-offset);
        }
    }

    // Set backingstore flags.
    int flags = DrawPaintOnScreen | DrawInvisible;
    if (renderFlags & QWidget::DrawWindowBackground)
        flags |= DrawAsRoot;

    if (renderFlags & QWidget::DrawChildren)
        flags |= DrawRecursive;
    else
        flags |= DontSubtractOpaqueChildren;

    if (target->devType() == QInternal::Printer) {
        QPainter p(target);
        render_helper(&p, targetOffset, paintRegion, renderFlags);
        return;
    }

    // Render via backingstore.
    drawWidget(target, paintRegion, offset, flags, sharedPainter());

    // Restore shared painter.
    if (oldSharedPainter)
        setSharedPainter(oldSharedPainter);
}

void QWidgetPrivate::paintSiblingsRecursive(QPaintDevice *pdev, const QObjectList& siblings, int index, const QRegion &rgn,
                                            const QPoint &offset, int flags,
                                            QPainter *sharedPainter, QWidgetBackingStore *backingStore)
{
    QWidget *w = 0;
    QRect boundingRect;
    bool dirtyBoundingRect = true;
    const bool exludeOpaqueChildren = (flags & DontDrawOpaqueChildren);

    do {
        QWidget *x =  qobject_cast<QWidget*>(siblings.at(index));
        if (x && !(exludeOpaqueChildren && x->d_func()->isOpaque) && !x->isHidden() && !x->isWindow()) {
            if (dirtyBoundingRect) {
                boundingRect = rgn.boundingRect();
                dirtyBoundingRect = false;
            }

            if (qRectIntersects(boundingRect, x->data->crect)) {
                w = x;
                break;
            }
        }
        --index;
    } while (index >= 0);

    if (!w)
        return;

    QWidgetPrivate *wd = w->d_func();
    const QPoint widgetPos(w->data->crect.topLeft());
    const bool hasMask = wd->extra && wd->extra->hasMask;
    if (index > 0) {
        QRegion wr(rgn);
        if (wd->isOpaque)
            wr -= hasMask ? wd->extra->mask.translated(widgetPos) : w->data->crect;
        paintSiblingsRecursive(pdev, siblings, --index, wr, offset, flags,
                               sharedPainter, backingStore);
    }

    if (w->updatesEnabled()
#ifndef QT_NO_GRAPHICSVIEW
            && (!w->d_func()->extra || !w->d_func()->extra->proxyWidget)
#endif //QT_NO_GRAPHICSVIEW
       ) {
        QRegion wRegion(rgn);
        wRegion &= w->data->crect;
        wRegion.translate(-widgetPos);
        if (hasMask)
            wRegion &= wd->extra->mask;
        wd->drawWidget(pdev, wRegion, offset + widgetPos, flags, sharedPainter, backingStore);
    }
}

#ifndef QT_NO_GRAPHICSVIEW
/*!
    \internal

    Finds the nearest widget embedded in a graphics proxy widget along the chain formed by this
    widget and its ancestors. The search starts at \a origin (inclusive).
    If successful, the function returns the proxy that embeds the widget, or 0 if no embedded
    widget was found.
*/
QGraphicsProxyWidget * QWidgetPrivate::nearestGraphicsProxyWidget(const QWidget *origin)
{
    if (origin) {
        QWExtra *extra = origin->d_func()->extra;
        if (extra && extra->proxyWidget)
            return extra->proxyWidget;
        return nearestGraphicsProxyWidget(origin->parentWidget());
    }
    return 0;
}
#endif

/*!
    \property QWidget::locale
    \brief the widget's locale
    \since 4.3

    As long as no special locale has been set, this is either
    the parent's locale or (if this widget is a top level widget),
    the system locale.

    If the widget displays dates or numbers, these should be formatted
    using the widget's locale.

    \sa QLocale QLocale::system()
*/

void QWidgetPrivate::setLocale_helper(const QLocale &loc, bool forceUpdate)
{
    Q_Q(QWidget);
    if (locale == loc && !forceUpdate)
        return;

    locale = loc;

    if (!children.isEmpty()) {
        for (int i = 0; i < children.size(); ++i) {
            QWidget *w = qobject_cast<QWidget*>(children.at(i));
            if (!w)
                continue;
            if (w->testAttribute(Qt::WA_SetLocale))
                continue;
            if (w->isWindow() && !w->testAttribute(Qt::WA_WindowPropagation))
                continue;
            w->d_func()->setLocale_helper(loc, forceUpdate);
        }
    }
    QEvent e(QEvent::LocaleChange);
    QApplication::sendEvent(q, &e);
}

void QWidget::setLocale(const QLocale &locale)
{
    Q_D(QWidget);

    setAttribute(Qt::WA_SetLocale);
    d->setLocale_helper(locale);
}

QLocale QWidget::locale() const
{
    Q_D(const QWidget);

    return d->locale;
}

void QWidgetPrivate::resolveLocale()
{
    Q_Q(const QWidget);

    if (!q->testAttribute(Qt::WA_SetLocale)) {
        setLocale_helper(q->isWindow()
                            ? QLocale()
                            : q->parentWidget()->locale());
    }
}

void QWidget::unsetLocale()
{
    Q_D(QWidget);
    setAttribute(Qt::WA_SetLocale, false);
    d->resolveLocale();
}

/*!
    \property QWidget::windowTitle
    \brief the window title (caption)

    This property only makes sense for top-level widgets, such as
    windows and dialogs. If no caption has been set, the title is an
    empty string.

    If you use the \l windowModified mechanism, the window title must
    contain a "[*]" placeholder, which indicates where the '*' should
    appear. Normally, it should appear right after the file name
    (e.g., "document1.txt[*] - Text Editor"). If the \l
    windowModified property is false (the default), the placeholder
    is simply removed.

    \sa windowIcon, windowIconText, windowModified
*/
QString QWidget::windowTitle() const
{
    Q_D(const QWidget);
    if (d->extra && d->extra->topextra) {
        if (!d->extra->topextra->caption.isEmpty())
            return d->extra->topextra->caption;
    }
    return QString();
}

/*!
    Returns a modified window title with the [*] place holder
    replaced according to the rules described in QWidget::setWindowTitle

    This function assumes that "[*]" can be quoted by another
    "[*]", so it will replace two place holders by one and
    a single last one by either "*" or nothing depending on
    the modified flag.

    \internal
*/
QString qt_setWindowTitle_helperHelper(const QString &title, const QWidget *widget)
{
    Q_ASSERT(widget);

    QString cap = title;

    if (cap.isEmpty())
        return cap;

    QLatin1String placeHolder("[*]");
    int placeHolderLength = 3; // QLatin1String doesn't have length()

    int index = cap.indexOf(placeHolder);

    // here the magic begins
    while (index != -1) {
        index += placeHolderLength;
        int count = 1;
        while (cap.indexOf(placeHolder, index) == index) {
            ++count;
            index += placeHolderLength;
        }

        if (count%2) { // odd number of [*] -> replace last one
            int lastIndex = cap.lastIndexOf(placeHolder, index - 1);
            if (widget->isWindowModified()
             && widget->style()->styleHint(QStyle::SH_TitleBar_ModifyNotification, 0, widget))
                cap.replace(lastIndex, 3, QWidget::tr("*"));
            else
                cap.remove(lastIndex, 3);
        }

        index = cap.indexOf(placeHolder, index);
    }

    cap.replace(QLatin1String("[*][*]"), placeHolder);

    return cap;
}

void QWidgetPrivate::setWindowTitle_helper(const QString &title)
{
    Q_Q(QWidget);
    if (q->testAttribute(Qt::WA_WState_Created))
        setWindowTitle_sys(qt_setWindowTitle_helperHelper(title, q));
}

void QWidgetPrivate::setWindowIconText_helper(const QString &title)
{
    Q_Q(QWidget);
    if (q->testAttribute(Qt::WA_WState_Created))
        setWindowIconText_sys(qt_setWindowTitle_helperHelper(title, q));
}

void QWidget::setWindowIconText(const QString &iconText)
{
    if (QWidget::windowIconText() == iconText)
        return;

    Q_D(QWidget);
    d->topData()->iconText = iconText;
    d->setWindowIconText_helper(iconText);

    QEvent e(QEvent::IconTextChange);
    QApplication::sendEvent(this, &e);
}

void QWidget::setWindowTitle(const QString &title)
{
    if (QWidget::windowTitle() == title && !title.isEmpty() && !title.isNull())
        return;

    Q_D(QWidget);
    d->topData()->caption = title;
    d->setWindowTitle_helper(title);

    QEvent e(QEvent::WindowTitleChange);
    QApplication::sendEvent(this, &e);
}


/*!
    \property QWidget::windowIcon
    \brief the widget's icon

    This property only makes sense for windows. If no icon
    has been set, windowIcon() returns the application icon
    (QApplication::windowIcon()).

    \sa windowIconText, windowTitle
*/
QIcon QWidget::windowIcon() const
{
    const QWidget *w = this;
    while (w) {
        const QWidgetPrivate *d = w->d_func();
        if (d->extra && d->extra->topextra && d->extra->topextra->icon)
            return *d->extra->topextra->icon;
        w = w->parentWidget();
    }
    return QApplication::windowIcon();
}

void QWidgetPrivate::setWindowIcon_helper()
{
    QEvent e(QEvent::WindowIconChange);
    QApplication::sendEvent(q_func(), &e);
    for (int i = 0; i < children.size(); ++i) {
        QWidget *w = qobject_cast<QWidget *>(children.at(i));
        if (w && !w->isWindow())
            QApplication::sendEvent(w, &e);
    }
}

void QWidget::setWindowIcon(const QIcon &icon)
{
    Q_D(QWidget);

    setAttribute(Qt::WA_SetWindowIcon, !icon.isNull());
    d->createTLExtra();

    if (!d->extra->topextra->icon)
        d->extra->topextra->icon = new QIcon();
    *d->extra->topextra->icon = icon;

    if (d->extra->topextra->iconPixmap) {
        XFreePixmap(qt_x11Data->display, d->extra->topextra->iconPixmap);
    }
    d->extra->topextra->iconPixmap = 0;

    d->setWindowIcon_sys();
    d->setWindowIcon_helper();
}


/*!
    \property QWidget::windowIconText
    \brief the widget's icon text

    This property only makes sense for windows. If no icon
    text has been set, this functions returns an empty string.

    \sa windowIcon, windowTitle
*/

QString QWidget::windowIconText() const
{
    Q_D(const QWidget);
    return (d->extra && d->extra->topextra) ? d->extra->topextra->iconText : QString();
}

/*!
    Returns the window's role, or an empty string.

    \sa windowIcon, windowTitle
*/

QString QWidget::windowRole() const
{
    Q_D(const QWidget);
    return (d->extra && d->extra->topextra) ? d->extra->topextra->role : QString();
}

/*!
    Sets the window's role to \a role. This only makes sense for
    windows on X11.
*/
void QWidget::setWindowRole(const QString &role)
{
#if defined(Q_WS_X11)
    Q_D(QWidget);
    d->topData()->role = role;
    d->setWindowRole();
#else
    Q_UNUSED(role)
#endif
}

/*!
    \property QWidget::mouseTracking
    \brief whether mouse tracking is enabled for the widget

    If mouse tracking is disabled (the default), the widget only
    receives mouse move events when at least one mouse button is
    pressed while the mouse is being moved.

    If mouse tracking is enabled, the widget receives mouse move
    events even if no buttons are pressed.

    \sa mouseMoveEvent()
*/


/*!
    Sets the widget's focus proxy to widget \a w. If \a w is 0, the
    function resets this widget to have no focus proxy.

    Some widgets can "have focus", but create a child widget, such as
    QLineEdit, to actually handle the focus. In this case, the widget
    can set the line edit to be its focus proxy.

    setFocusProxy() sets the widget which will actually get focus when
    "this widget" gets it. If there is a focus proxy, setFocus() and
    hasFocus() operate on the focus proxy.

    \sa focusProxy()
*/

void QWidget::setFocusProxy(QWidget * w)
{
    Q_D(QWidget);
    if (!w && !d->extra)
        return;

    for (QWidget* fp  = w; fp; fp = fp->focusProxy()) {
        if (Q_UNLIKELY(fp == this)) {
            qWarning("QWidget: %s (%s) already in focus proxy chain", metaObject()->className(), objectName().toLocal8Bit().constData());
            return;
        }
    }

    d->createExtra();
    d->extra->focus_proxy = w;
}


/*!
    Returns the focus proxy, or 0 if there is no focus proxy.

    \sa setFocusProxy()
*/

QWidget * QWidget::focusProxy() const
{
    Q_D(const QWidget);
    return d->extra ? (QWidget *)d->extra->focus_proxy : 0;
}


/*!
    \property QWidget::focus
    \brief whether this widget (or its focus proxy) has the keyboard
    input focus

    By default, this property is false.

    \note Obtaining the value of this property for a widget is effectively equivalent
    to checking whether QApplication::focusWidget() refers to the widget.

    \sa setFocus(), clearFocus(), setFocusPolicy(), QApplication::focusWidget()
*/
bool QWidget::hasFocus() const
{
    const QWidget* w = this;
    while (w->d_func()->extra && w->d_func()->extra->focus_proxy)
        w = w->d_func()->extra->focus_proxy;
    if (QWidget *window = w->window()) {
#ifndef QT_NO_GRAPHICSVIEW
        QWExtra *e = window->d_func()->extra;
        if (e && e->proxyWidget && e->proxyWidget->hasFocus() && window->focusWidget() == w)
            return true;
#endif
    }
    return (QApplication::focusWidget() == w);
}

/*!
    Gives the keyboard input focus to this widget (or its focus
    proxy) if this widget or one of its parents is the \link
    isActiveWindow() active window\endlink. The \a reason argument will
    be passed into any focus event sent from this function, it is used
    to give an explanation of what caused the widget to get focus.
    If the window is not active, the widget will be given the focus when
    the window becomes active.

    First, a focus out event is sent to the focus widget (if any) to
    tell it that it is about to lose the focus. Then a focus in event
    is sent to this widget to tell it that it just received the focus.
    (Nothing happens if the focus in and focus out widgets are the
    same.)

    setFocus() gives focus to a widget regardless of its focus policy,
    but does not clear any keyboard grab (see grabKeyboard()).

    Be aware that if the widget is hidden, it will not accept focus
    until it is shown.

    \warning If you call setFocus() in a function which may itself be
    called from focusOutEvent() or focusInEvent(), you may get an
    infinite recursion.

    \sa hasFocus(), clearFocus(), focusInEvent(), focusOutEvent(),
    setFocusPolicy(), focusWidget(), QApplication::focusWidget(), grabKeyboard(),
    grabMouse(), {Keyboard Focus}, QEvent::RequestSoftwareInputPanel
*/

void QWidget::setFocus(Qt::FocusReason reason)
{
    if (!isEnabled())
        return;

    QWidget *f = this;
    while (f->d_func()->extra && f->d_func()->extra->focus_proxy)
        f = f->d_func()->extra->focus_proxy;

    if (QApplication::focusWidget() == f)
        return;

#ifndef QT_NO_GRAPHICSVIEW
    QWidget *previousProxyFocus = 0;
    if (QWExtra *topData = window()->d_func()->extra) {
        if (topData->proxyWidget && topData->proxyWidget->hasFocus()) {
            previousProxyFocus = topData->proxyWidget->widget()->focusWidget();
            if (previousProxyFocus && previousProxyFocus->focusProxy())
                previousProxyFocus = previousProxyFocus->focusProxy();
            if (previousProxyFocus == this && !topData->proxyWidget->d_func()->proxyIsGivingFocus)
                return;
        }
    }
#endif

    QWidget *w = f;
    if (isHidden()) {
        while (w && w->isHidden()) {
            w->d_func()->focus_child = f;
            w = w->isWindow() ? 0 : w->parentWidget();
        }
    } else {
        while (w) {
            w->d_func()->focus_child = f;
            w = w->isWindow() ? 0 : w->parentWidget();
        }
    }

#ifndef QT_NO_GRAPHICSVIEW
    // Update proxy state
    if (QWExtra *topData = window()->d_func()->extra) {
        if (topData->proxyWidget && !topData->proxyWidget->hasFocus()) {
            topData->proxyWidget->d_func()->focusFromWidgetToProxy = true;
            topData->proxyWidget->setFocus(reason);
            topData->proxyWidget->d_func()->focusFromWidgetToProxy = false;
        }
    }
#endif

    if (f->isActiveWindow()) {
        QApplicationPrivate::setFocusWidget(f, reason);
#ifndef QT_NO_GRAPHICSVIEW
        if (QWExtra *topData = window()->d_func()->extra) {
            if (topData->proxyWidget) {
                if (previousProxyFocus && previousProxyFocus != f) {
                    // Send event to self
                    QFocusEvent event(QEvent::FocusOut, reason);
                    QPointer<QWidget> that = previousProxyFocus;
                    QApplication::sendEvent(previousProxyFocus, &event);
                    if (that)
                        QApplication::sendEvent(that->style(), &event);
                }
                if (!isHidden()) {
                    // Send event to self
                    QFocusEvent event(QEvent::FocusIn, reason);
                    QPointer<QWidget> that = f;
                    QApplication::sendEvent(f, &event);
                    if (that)
                        QApplication::sendEvent(that->style(), &event);
                }
            }
        }
#endif
    }
}

/*!
    \fn void QWidget::setFocus()
    \overload

    Gives the keyboard input focus to this widget (or its focus
    proxy) if this widget or one of its parents is the
    \l{isActiveWindow()}{active window}.
*/

/*!
    Takes keyboard input focus from the widget.

    If the widget has active focus, a \link focusOutEvent() focus out
    event\endlink is sent to this widget to tell it that it is about
    to lose the focus.

    This widget must enable focus setting in order to get the keyboard
    input focus, i.e. it must call setFocusPolicy().

    \sa hasFocus(), setFocus(), focusInEvent(), focusOutEvent(),
    setFocusPolicy(), QApplication::focusWidget()
*/

void QWidget::clearFocus()
{
    QWidget *w = this;
    while (w) {
        if (w->d_func()->focus_child == this)
            w->d_func()->focus_child = 0;
        w = w->parentWidget();
    }
#ifndef QT_NO_GRAPHICSVIEW
    QWExtra *topData = d_func()->extra;
    if (topData && topData->proxyWidget)
        topData->proxyWidget->clearFocus();
#endif

    if (hasFocus()) {
        // Update proxy state
        QApplicationPrivate::setFocusWidget(0, Qt::OtherFocusReason);
    }
}


/*!
    \fn bool QWidget::focusNextChild()

    Finds a new widget to give the keyboard focus to, as appropriate
    for \key Tab, and returns true if it can find a new widget, or
    false if it can't.

    \sa focusPreviousChild()
*/

/*!
    \fn bool QWidget::focusPreviousChild()

    Finds a new widget to give the keyboard focus to, as appropriate
    for \key Shift+Tab, and returns true if it can find a new widget,
    or false if it can't.

    \sa focusNextChild()
*/

/*!
    Finds a new widget to give the keyboard focus to, as appropriate
    for Tab and Shift+Tab, and returns true if it can find a new
    widget, or false if it can't.

    If \a next is true, this function searches forward, if \a next
    is false, it searches backward.

    Sometimes, you will want to reimplement this function. For
    example, a web browser might reimplement it to move its "current
    active link" forward or backward, and call
    focusNextPrevChild() only when it reaches the last or
    first link on the "page".

    Child widgets call focusNextPrevChild() on their parent widgets,
    but only the window that contains the child widgets decides where
    to redirect focus. By reimplementing this function for an object,
    you thus gain control of focus traversal for all child widgets.

    \sa focusNextChild(), focusPreviousChild()
*/

bool QWidget::focusNextPrevChild(bool next)
{
    Q_D(QWidget);
    QWidget* p = parentWidget();
    bool isSubWindow = (windowType() == Qt::SubWindow);
    if (!isWindow() && !isSubWindow && p)
        return p->focusNextPrevChild(next);
#ifndef QT_NO_GRAPHICSVIEW
    if (d->extra && d->extra->proxyWidget)
        return d->extra->proxyWidget->focusNextPrevChild(next);
#endif
    QWidget *w = QApplicationPrivate::focusNextPrevChild_helper(this, next);
    if (!w) return false;

    w->setFocus(next ? Qt::TabFocusReason : Qt::BacktabFocusReason);
    return true;
}

/*!
    Returns the last child of this widget that setFocus had been
    called on.  For top level widgets this is the widget that will get
    focus in case this window gets activated

    This is not the same as QApplication::focusWidget(), which returns
    the focus widget in the currently active window.
*/

QWidget *QWidget::focusWidget() const
{
    return d_func()->focus_child;
}

/*!
    Returns the next widget in this widget's focus chain.

    \sa previousInFocusChain()
*/
QWidget *QWidget::nextInFocusChain() const
{
    return d_func()->focus_next;
}

/*!
    \brief The previousInFocusChain function returns the previous
    widget in this widget's focus chain.

    \sa nextInFocusChain()

    \since 4.6
*/
QWidget *QWidget::previousInFocusChain() const
{
    return d_func()->focus_prev;
}

/*!
    \property QWidget::isActiveWindow
    \brief whether this widget's window is the active window

    The active window is the window that contains the widget that has
    keyboard focus (The window may still have focus if it has no
    widgets or none of its widgets accepts keyboard focus).

    When popup windows are visible, this property is true for both the
    active window \e and for the popup.

    By default, this property is false.

    \sa activateWindow(), QApplication::activeWindow()
*/
bool QWidget::isActiveWindow() const
{
    QWidget *tlw = window();
    if(tlw == QApplication::activeWindow() || (isVisible() && (tlw->windowType() == Qt::Popup)))
        return true;

#ifndef QT_NO_GRAPHICSVIEW
    if (QWExtra *tlwExtra = tlw->d_func()->extra) {
        if (isVisible() && tlwExtra->proxyWidget)
            return tlwExtra->proxyWidget->isActiveWindow();
    }
#endif

    if(style()->styleHint(QStyle::SH_Widget_ShareActivation, 0, this)) {
        if(tlw->windowType() == Qt::Tool &&
           !tlw->isModal() &&
           (!tlw->parentWidget() || tlw->parentWidget()->isActiveWindow()))
           return true;
        QWidget *w = QApplication::activeWindow();
        while(w && tlw->windowType() == Qt::Tool &&
              !w->isModal() && w->parentWidget()) {
            w = w->parentWidget()->window();
            if(w == tlw)
                return true;
        }
    }
    return false;
}

/*!
    Puts the \a second widget after the \a first widget in the focus order.

    Note that since the tab order of the \a second widget is changed, you
    should order a chain like this:

    \snippet doc/src/snippets/code/src_gui_kernel_qwidget.cpp 9

    \e not like this:

    \snippet doc/src/snippets/code/src_gui_kernel_qwidget.cpp 10

    If \a first or \a second has a focus proxy, setTabOrder()
    correctly substitutes the proxy.

    \sa setFocusPolicy(), setFocusProxy(), {Keyboard Focus}
*/
void QWidget::setTabOrder(QWidget* first, QWidget *second)
{
    if (!first || !second || first->focusPolicy() == Qt::NoFocus || second->focusPolicy() == Qt::NoFocus)
        return;

    if (Q_UNLIKELY(first->window() != second->window())) {
        qWarning("QWidget::setTabOrder: 'first' and 'second' must be in the same window");
        return;
    }

    QWidget *fp = first->focusProxy();
    if (fp) {
        // If first is redirected, set first to the last child of first
        // that can take keyboard focus so that second is inserted after
        // that last child, and the focus order within first is (more
        // likely to be) preserved.
        QList<QWidget *> l = first->findChildren<QWidget *>();
        for (int i = l.size()-1; i >= 0; --i) {
            QWidget * next = l.at(i);
            if (next->window() == fp->window()) {
                fp = next;
                if (fp->focusPolicy() != Qt::NoFocus)
                    break;
            }
        }
        first = fp;
    }

    if (fp == second)
        return;

    if (QWidget *sp = second->focusProxy())
        second = sp;

//    QWidget *fp = first->d_func()->focus_prev;
    QWidget *fn = first->d_func()->focus_next;

    if (fn == second || first == second)
        return;

    QWidget *sp = second->d_func()->focus_prev;
    QWidget *sn = second->d_func()->focus_next;

    fn->d_func()->focus_prev = second;
    first->d_func()->focus_next = second;

    second->d_func()->focus_next = fn;
    second->d_func()->focus_prev = first;

    sp->d_func()->focus_next = sn;
    sn->d_func()->focus_prev = sp;


    Q_ASSERT(first->d_func()->focus_next->d_func()->focus_prev == first);
    Q_ASSERT(first->d_func()->focus_prev->d_func()->focus_next == first);

    Q_ASSERT(second->d_func()->focus_next->d_func()->focus_prev == second);
    Q_ASSERT(second->d_func()->focus_prev->d_func()->focus_next == second);
}

/*!\internal

  Moves the relevant subwidgets of this widget from the \a oldtlw's
  tab chain to that of the new parent, if there's anything to move and
  we're really moving

  This function is called from QWidget::reparent() *after* the widget
  has been reparented.

  \sa reparent()
*/

void QWidgetPrivate::reparentFocusWidgets(QWidget * oldtlw)
{
    Q_Q(QWidget);
    if (oldtlw == q->window())
        return; // nothing to do

    if(focus_child)
        focus_child->clearFocus();

    // separate the focus chain into new (children of myself) and old (the rest)
    QWidget *firstOld = 0;
    //QWidget *firstNew = q; //invariant
    QWidget *o = 0; // last in the old list
    QWidget *n = q; // last in the new list

    bool prevWasNew = true;
    QWidget *w = focus_next;

    //Note: for efficiency, we do not maintain the list invariant inside the loop
    //we append items to the relevant list, and we optimize by not changing pointers
    //when subsequent items are going into the same list.
    while (w  != q) {
        bool currentIsNew =  q->isAncestorOf(w);
        if (currentIsNew) {
            if (!prevWasNew) {
                //prev was old -- append to new list
                n->d_func()->focus_next = w;
                w->d_func()->focus_prev = n;
            }
            n = w;
        } else {
            if (prevWasNew) {
                //prev was new -- append to old list, if there is one
                if (o) {
                    o->d_func()->focus_next = w;
                    w->d_func()->focus_prev = o;
                } else {
                    // "create" the old list
                    firstOld = w;
                }
            }
            o = w;
        }
        w = w->d_func()->focus_next;
        prevWasNew = currentIsNew;
    }

    //repair the old list:
    if (firstOld) {
        o->d_func()->focus_next = firstOld;
        firstOld->d_func()->focus_prev = o;
    }

    if (!q->isWindow()) {
        QWidget *topLevel = q->window();
        //insert new chain into toplevel's chain

        QWidget *prev = topLevel->d_func()->focus_prev;

        topLevel->d_func()->focus_prev = n;
        prev->d_func()->focus_next = q;

        focus_prev = prev;
        n->d_func()->focus_next = topLevel;
    } else {
        //repair the new list
            n->d_func()->focus_next = q;
            focus_prev = n;
    }

}

/*!\internal

  Measures the shortest distance from a point to a rect.

  This function is called from QDesktopwidget::screen(QPoint) to find the
  closest screen for a point.
  In directional KeypadNavigation, it is called to find the closest
  widget to the current focus widget center.
*/
int QWidgetPrivate::pointToRect(const QPoint &p, const QRect &r)
{
    int dx = 0;
    int dy = 0;
    if (p.x() < r.left())
        dx = r.left() - p.x();
    else if (p.x() > r.right())
        dx = p.x() - r.right();
    if (p.y() < r.top())
        dy = r.top() - p.y();
    else if (p.y() > r.bottom())
        dy = p.y() - r.bottom();
    return dx + dy;
}

/*!
    \property QWidget::frameSize
    \brief the size of the widget including any window frame

    By default, this property contains a value that depends on the user's
    platform and screen geometry.
*/
QSize QWidget::frameSize() const
{
    Q_D(const QWidget);
    if (isWindow() && !(windowType() == Qt::Popup)) {
        QRect fs = d->frameStrut();
        return QSize(data->crect.width() + fs.left() + fs.right(),
                      data->crect.height() + fs.top() + fs.bottom());
    }
    return data->crect.size();
}

/*! \fn void QWidget::move(int x, int y)

    \overload

    This corresponds to move(QPoint(\a x, \a y)).
*/

void QWidget::move(const QPoint &p)
{
    Q_D(QWidget);
    setAttribute(Qt::WA_Moved);
    if (isWindow())
        d->topData()->posFromMove = true;
    if (testAttribute(Qt::WA_WState_Created)) {
        d->setGeometry_sys(p.x() + geometry().x() - QWidget::x(),
                       p.y() + geometry().y() - QWidget::y(),
                       width(), height(), true);
        d->setDirtyOpaqueRegion();
    } else {
        data->crect.moveTopLeft(p); // no frame yet
        setAttribute(Qt::WA_PendingMoveEvent);
    }
}

/*! \fn void QWidget::resize(int w, int h)
    \overload

    This corresponds to resize(QSize(\a w, \a h)).
*/

void QWidget::resize(const QSize &s)
{
    Q_D(QWidget);
    setAttribute(Qt::WA_Resized);
    if (testAttribute(Qt::WA_WState_Created)) {
        d->setGeometry_sys(geometry().x(), geometry().y(), s.width(), s.height(), false);
        d->setDirtyOpaqueRegion();
    } else {
        data->crect.setSize(s.boundedTo(maximumSize()).expandedTo(minimumSize()));
        setAttribute(Qt::WA_PendingResizeEvent);
    }
}

void QWidget::setGeometry(const QRect &r)
{
    Q_D(QWidget);
    setAttribute(Qt::WA_Resized);
    setAttribute(Qt::WA_Moved);
    if (isWindow())
        d->topData()->posFromMove = false;
    if (testAttribute(Qt::WA_WState_Created)) {
        d->setGeometry_sys(r.x(), r.y(), r.width(), r.height(), true);
        d->setDirtyOpaqueRegion();
    } else {
        data->crect.setTopLeft(r.topLeft());
        data->crect.setSize(r.size().boundedTo(maximumSize()).expandedTo(minimumSize()));
        setAttribute(Qt::WA_PendingMoveEvent);
        setAttribute(Qt::WA_PendingResizeEvent);
    }
}

/*!
    \since 4.2
    Saves the current geometry and state for top-level widgets.

    To save the geometry when the window closes, you can
    implement a close event like this:

    \snippet doc/src/snippets/code/src_gui_kernel_qwidget.cpp 11

    See the \l{Window Geometry} documentation for an overview of geometry
    issues with windows.

    Use QMainWindow::saveState() to save the geometry and the state of
    toolbars and dock widgets.

    \sa restoreGeometry(), QMainWindow::saveState(), QMainWindow::restoreState()
*/
QByteArray QWidget::saveGeometry() const
{
    QByteArray array;
    QDataStream stream(&array, QIODevice::WriteOnly);
    quint16 majorVersion = 2;
    stream << majorVersion
           << frameGeometry()
           << normalGeometry()
           << qint32(QApplication::desktop()->screenNumber(this))
           << quint8(windowState() & Qt::WindowMaximized)
           << quint8(windowState() & Qt::WindowFullScreen);
    return array;
}

/*!
    \since 4.2

    Restores the geometry and state top-level widgets stored in the
    byte array \a geometry. Returns true on success; otherwise
    returns false.

    If the restored geometry is off-screen, it will be modified to be
    inside the available screen geometry.

    To restore geometry saved using QSettings, you can use code like
    this:

    \snippet doc/src/snippets/code/src_gui_kernel_qwidget.cpp 12

    See the \l{Window Geometry} documentation for an overview of geometry
    issues with windows.

    Use QMainWindow::restoreState() to restore the geometry and the
    state of toolbars and dock widgets.

    \sa saveGeometry(), QSettings, QMainWindow::saveState(), QMainWindow::restoreState()
*/
bool QWidget::restoreGeometry(const QByteArray &geometry)
{
    if (geometry.size() < 4)
        return false;
    QDataStream stream(geometry);

    const quint16 currentMajorVersion = 2;
    quint16 majorVersion = 0;

    stream >> majorVersion;

    if (majorVersion != currentMajorVersion)
        return false;

    QRect restoredFrameGeometry;
    QRect restoredNormalGeometry;
    qint32 restoredScreenNumber;
    quint8 maximized;
    quint8 fullScreen;

    stream >> restoredFrameGeometry
           >> restoredNormalGeometry
           >> restoredScreenNumber
           >> maximized
           >> fullScreen;

    const int frameHeight = 20;
    if (!restoredFrameGeometry.isValid())
        restoredFrameGeometry = QRect(QPoint(0,0), sizeHint());

    if (!restoredNormalGeometry.isValid())
        restoredNormalGeometry = QRect(QPoint(0, frameHeight), sizeHint());
    if (!restoredNormalGeometry.isValid()) {
        // use the widget's adjustedSize if the sizeHint() doesn't help
        restoredNormalGeometry.setSize(restoredNormalGeometry
                                       .size()
                                       .expandedTo(d_func()->adjustedSize()));
    }

    const QDesktopWidget * const desktop = QApplication::desktop();
    if (restoredScreenNumber >= desktop->screenCount())
        restoredScreenNumber = desktop->primaryScreen();

    const QRect availableGeometry = desktop->availableGeometry(restoredScreenNumber);

    // Modify the restored geometry if we are about to restore to coordinates
    // that would make the window "lost". This happens if:
    // - The restored geometry is completely oustside the available geometry
    // - The title bar is outside the available geometry.
    if (!restoredFrameGeometry.intersects(availableGeometry)) {
        restoredFrameGeometry.moveBottom(qMin(restoredFrameGeometry.bottom(), availableGeometry.bottom()));
        restoredFrameGeometry.moveLeft(qMax(restoredFrameGeometry.left(), availableGeometry.left()));
        restoredFrameGeometry.moveRight(qMin(restoredFrameGeometry.right(), availableGeometry.right()));
    }
    restoredFrameGeometry.moveTop(qMax(restoredFrameGeometry.top(), availableGeometry.top()));

    if (!restoredNormalGeometry.intersects(availableGeometry)) {
        restoredNormalGeometry.moveBottom(qMin(restoredNormalGeometry.bottom(), availableGeometry.bottom()));
        restoredNormalGeometry.moveLeft(qMax(restoredNormalGeometry.left(), availableGeometry.left()));
        restoredNormalGeometry.moveRight(qMin(restoredNormalGeometry.right(), availableGeometry.right()));
    }
    restoredNormalGeometry.moveTop(qMax(restoredNormalGeometry.top(), availableGeometry.top() + frameHeight));

    if (maximized || fullScreen) {
        // set geomerty before setting the window state to make
        // sure the window is maximized to the right screen.
        Qt::WindowStates ws = windowState();
        setGeometry(restoredNormalGeometry);
        if (maximized)
            ws |= Qt::WindowMaximized;
        if (fullScreen)
            ws |= Qt::WindowFullScreen;
       setWindowState(ws);
       d_func()->topData()->normalGeometry = restoredNormalGeometry;
    } else {
        QPoint offset;
#ifdef Q_WS_X11
        if (isFullScreen())
            offset = d_func()->topData()->fullScreenOffset;
#endif
        setWindowState(windowState() & ~(Qt::WindowMaximized | Qt::WindowFullScreen));
        move(restoredFrameGeometry.topLeft() + offset);
        resize(restoredNormalGeometry.size());
    }
    return true;
}

/*!\fn void QWidget::setGeometry(int x, int y, int w, int h)
    \overload

    This corresponds to setGeometry(QRect(\a x, \a y, \a w, \a h)).
*/

/*!
  Sets the margins around the contents of the widget to have the sizes
  \a left, \a top, \a right, and \a bottom. The margins are used by
  the layout system, and may be used by subclasses to specify the area
  to draw in (e.g. excluding the frame).

  Changing the margins will trigger a resizeEvent().

  \sa contentsRect(), getContentsMargins()
*/
void QWidget::setContentsMargins(int left, int top, int right, int bottom)
{
    Q_D(QWidget);
    if (left == d->leftmargin && top == d->topmargin
         && right == d->rightmargin && bottom == d->bottommargin)
        return;
    d->leftmargin = left;
    d->topmargin = top;
    d->rightmargin = right;
    d->bottommargin = bottom;

    if (QLayout *l=d->layout)
        l->update(); //force activate; will do updateGeometry
    else
        updateGeometry();

    // ### Qt 5: compat, remove
    if (isVisible()) {
        update();
        QResizeEvent e(data->crect.size(), data->crect.size());
        QApplication::sendEvent(this, &e);
    } else {
        setAttribute(Qt::WA_PendingResizeEvent, true);
    }

    QEvent e(QEvent::ContentsRectChange);
    QApplication::sendEvent(this, &e);
}

/*!
  \overload
  \since 4.6

  \brief The setContentsMargins function sets the margins around the
  widget's contents.

  Sets the margins around the contents of the widget to have the
  sizes determined by \a margins. The margins are
  used by the layout system, and may be used by subclasses to
  specify the area to draw in (e.g. excluding the frame).

  Changing the margins will trigger a resizeEvent().

  \sa contentsRect(), getContentsMargins()
*/
void QWidget::setContentsMargins(const QMargins &margins)
{
    setContentsMargins(margins.left(), margins.top(),
                       margins.right(), margins.bottom());
}

/*!
  Returns the widget's contents margins for \a left, \a top, \a
  right, and \a bottom.

  \sa setContentsMargins(), contentsRect()
 */
void QWidget::getContentsMargins(int *left, int *top, int *right, int *bottom) const
{
    Q_D(const QWidget);
    if (left)
        *left = d->leftmargin;
    if (top)
        *top = d->topmargin;
    if (right)
        *right = d->rightmargin;
    if (bottom)
        *bottom = d->bottommargin;
}

/*!
  \since 4.6

  \brief The contentsMargins function returns the widget's contents margins.

  \sa getContentsMargins(), setContentsMargins(), contentsRect()
 */
QMargins QWidget::contentsMargins() const
{
    Q_D(const QWidget);
    return QMargins(d->leftmargin, d->topmargin, d->rightmargin, d->bottommargin);
}


/*!
    Returns the area inside the widget's margins.

    \sa setContentsMargins(), getContentsMargins()
*/
QRect QWidget::contentsRect() const
{
    Q_D(const QWidget);
    return QRect(QPoint(d->leftmargin, d->topmargin),
                 QPoint(data->crect.width() - 1 - d->rightmargin,
                        data->crect.height() - 1 - d->bottommargin));

}



/*!
  \fn void QWidget::customContextMenuRequested(const QPoint &pos)

  This signal is emitted when the widget's \l contextMenuPolicy is
  Qt::CustomContextMenu, and the user has requested a context menu on
  the widget. The position \a pos is the position of the context menu
  event that the widget receives. Normally this is in widget
  coordinates. The exception to this rule is QAbstractScrollArea and
  its subclasses that map the context menu event to coordinates of the
  \link QAbstractScrollArea::viewport() viewport() \endlink .


  \sa mapToGlobal() QMenu contextMenuPolicy
*/


/*!
    \property QWidget::contextMenuPolicy
    \brief how the widget shows a context menu

    The default value of this property is Qt::DefaultContextMenu,
    which means the contextMenuEvent() handler is called. Other values
    are Qt::NoContextMenu, Qt::PreventContextMenu,
    Qt::ActionsContextMenu, and Qt::CustomContextMenu. With
    Qt::CustomContextMenu, the signal customContextMenuRequested() is
    emitted.

    \sa contextMenuEvent(), customContextMenuRequested(), actions()
*/

Qt::ContextMenuPolicy QWidget::contextMenuPolicy() const
{
    return data->context_menu_policy;
}

void QWidget::setContextMenuPolicy(Qt::ContextMenuPolicy policy)
{
    data->context_menu_policy = policy;
}

/*!
    \property QWidget::focusPolicy
    \brief the way the widget accepts keyboard focus

    The policy is Qt::TabFocus if the widget accepts keyboard
    focus by tabbing, Qt::ClickFocus if the widget accepts
    focus by clicking, Qt::StrongFocus if it accepts both, and
    Qt::NoFocus (the default) if it does not accept focus at
    all.

    You must enable keyboard focus for a widget if it processes
    keyboard events. This is normally done from the widget's
    constructor. For instance, the QLineEdit constructor calls
    setFocusPolicy(Qt::StrongFocus).

    If the widget has a focus proxy, then the focus policy will
    be propagated to it.

    \sa focusInEvent(), focusOutEvent(), keyPressEvent(), keyReleaseEvent(), enabled
*/


Qt::FocusPolicy QWidget::focusPolicy() const
{
    return data->focus_policy;
}

void QWidget::setFocusPolicy(Qt::FocusPolicy policy)
{
    data->focus_policy = policy;
    Q_D(QWidget);
    if (d->extra && d->extra->focus_proxy)
        d->extra->focus_proxy->setFocusPolicy(policy);
}

/*!
    \property QWidget::updatesEnabled
    \brief whether updates are enabled

    An updates enabled widget receives paint events and has a system
    background; a disabled widget does not. This also implies that
    calling update() and repaint() has no effect if updates are
    disabled.

    By default, this property is true.

    setUpdatesEnabled() is normally used to disable updates for a
    short period of time, for instance to avoid screen flicker during
    large changes. In Qt, widgets normally do not generate screen
    flicker, but on X11 the server might erase regions on the screen
    when widgets get hidden before they can be replaced by other
    widgets. Disabling updates solves this.

    Example:
    \snippet doc/src/snippets/code/src_gui_kernel_qwidget.cpp 13

    Disabling a widget implicitly disables all its children. Enabling a widget
    enables all child widgets \e except top-level widgets or those that
    have been explicitly disabled. Re-enabling updates implicitly calls
    update() on the widget.

    \sa paintEvent()
*/
void QWidget::setUpdatesEnabled(bool enable)
{
    Q_D(QWidget);
    setAttribute(Qt::WA_ForceUpdatesDisabled, !enable);
    d->setUpdatesEnabled_helper(enable);
}

/*!  \fn void QWidget::show()

    Shows the widget and its child widgets. This function is
    equivalent to setVisible(true).

    \sa raise(), showEvent(), hide(), setVisible(), showMinimized(), showMaximized(),
    showNormal(), isVisible()
*/


/*! \internal

   Makes the widget visible in the isVisible() meaning of the word.
   It is only called for toplevels or widgets with visible parents.
 */
void QWidgetPrivate::show_recursive()
{
    Q_Q(QWidget);
    // polish if necessary

    if (!q->testAttribute(Qt::WA_WState_Created))
        createRecursively();
    q->ensurePolished();

    if (!q->isWindow() && q->parentWidget()->d_func()->layout && !q->parentWidget()->data->in_show)
        q->parentWidget()->d_func()->layout->activate();
    // activate our layout before we and our children become visible
    if (layout)
        layout->activate();

    show_helper();
}

void QWidgetPrivate::sendPendingMoveAndResizeEvents(bool recursive, bool disableUpdates)
{
    Q_Q(QWidget);

    disableUpdates = disableUpdates && q->updatesEnabled();
    if (disableUpdates)
        q->setAttribute(Qt::WA_UpdatesDisabled);

    if (q->testAttribute(Qt::WA_PendingMoveEvent)) {
        QMoveEvent e(data.crect.topLeft(), data.crect.topLeft());
        QApplication::sendEvent(q, &e);
        q->setAttribute(Qt::WA_PendingMoveEvent, false);
    }

    if (q->testAttribute(Qt::WA_PendingResizeEvent)) {
        QResizeEvent e(data.crect.size(), QSize());
        QApplication::sendEvent(q, &e);
        q->setAttribute(Qt::WA_PendingResizeEvent, false);
    }

    if (disableUpdates)
        q->setAttribute(Qt::WA_UpdatesDisabled, false);

    if (!recursive)
        return;

    for (int i = 0; i < children.size(); ++i) {
        if (QWidget *child = qobject_cast<QWidget *>(children.at(i)))
            child->d_func()->sendPendingMoveAndResizeEvents(recursive, disableUpdates);
    }
}

void QWidgetPrivate::activateChildLayoutsRecursively()
{
    sendPendingMoveAndResizeEvents(false, true);

    for (int i = 0; i < children.size(); ++i) {
        QWidget *child = qobject_cast<QWidget *>(children.at(i));
        if (!child || child->isHidden() || child->isWindow())
            continue;

        child->ensurePolished();

        // Activate child's layout
        QWidgetPrivate *childPrivate = child->d_func();
        if (childPrivate->layout)
            childPrivate->layout->activate();

        // Pretend we're visible.
        const bool wasVisible = child->isVisible();
        if (!wasVisible)
            child->setAttribute(Qt::WA_WState_Visible);

        // Do the same for all my children.
        childPrivate->activateChildLayoutsRecursively();

        // We're not cheating anymore.
        if (!wasVisible)
            child->setAttribute(Qt::WA_WState_Visible, false);
    }
}

void QWidgetPrivate::show_helper()
{
    Q_Q(QWidget);
    data.in_show = true; // qws optimization
    // make sure we receive pending move and resize events
    sendPendingMoveAndResizeEvents();

    // become visible before showing all children
    q->setAttribute(Qt::WA_WState_Visible);

    // finally show all children recursively
    showChildren(false);



    // popup handling: new popups and tools need to be raised, and
    // existing popups must be closed. Also propagate the current
    // windows's KeyboardFocusChange status.
    if (q->isWindow()) {
        if ((q->windowType() == Qt::Tool) || (q->windowType() == Qt::Popup) || q->windowType() == Qt::ToolTip) {
            q->raise();
            if (q->parentWidget() && q->parentWidget()->window()->testAttribute(Qt::WA_KeyboardFocusChange))
                q->setAttribute(Qt::WA_KeyboardFocusChange);
        } else {
            while (QApplication::activePopupWidget()) {
                if (!QApplication::activePopupWidget()->close())
                    break;
            }
        }
    }

    // Automatic embedding of child windows of widgets already embedded into
    // QGraphicsProxyWidget when they are shown the first time.
    bool isEmbedded = false;
#ifndef QT_NO_GRAPHICSVIEW
    if (q->isWindow()) {
        isEmbedded = (q->graphicsProxyWidget() != nullptr);
        if (!isEmbedded && !bypassGraphicsProxyWidget(q)) {
            QGraphicsProxyWidget *ancestorProxy = nearestGraphicsProxyWidget(q->parentWidget());
            if (ancestorProxy) {
                isEmbedded = true;
                ancestorProxy->d_func()->embedSubWindow(q);
            }
        }
    }
#else
    Q_UNUSED(isEmbedded);
#endif

    // send the show event before showing the window
    QShowEvent showEvent;
    QApplication::sendEvent(q, &showEvent);

    if (!isEmbedded && q->isModal() && q->isWindow())
        // QApplicationPrivate::enterModal *before* show, otherwise the initial
        // stacking might be wrong
        QApplicationPrivate::enterModal(q);


    show_sys();

    if (!isEmbedded && q->windowType() == Qt::Popup)
        qApp->d_func()->openPopup(q);

    if (QApplicationPrivate::hidden_focus_widget == q) {
        QApplicationPrivate::hidden_focus_widget = 0;
        q->setFocus(Qt::OtherFocusReason);
    }

    // Process events when showing a Qt::SplashScreen widget before the event loop
    // is spinnning; otherwise it might not show up on particular platforms.
    // This makes QSplashScreen behave the same on all platforms.
    if (!qApp->d_func()->in_exec && q->windowType() == Qt::SplashScreen)
        QApplication::processEvents();

    data.in_show = false;  // reset qws optimization
}

/*! \fn void QWidget::hide()

    Hides the widget. This function is equivalent to
    setVisible(false).


    \note If you are working with QDialog or its subclasses and you invoke
    the show() function after this function, the dialog will be displayed in
    its original position.

    \sa hideEvent(), isHidden(), show(), setVisible(), isVisible(), close()
*/

/*!\internal
 */
void QWidgetPrivate::hide_helper()
{
    Q_Q(QWidget);

    bool isEmbedded = false;
#if !defined QT_NO_GRAPHICSVIEW
    isEmbedded = q->isWindow() && !bypassGraphicsProxyWidget(q) && nearestGraphicsProxyWidget(q->parentWidget()) != 0;
#else
    Q_UNUSED(isEmbedded);
#endif

    if (!isEmbedded && (q->windowType() == Qt::Popup))
        qApp->d_func()->closePopup(q);

    // Move test modal here.  Otherwise, a modal dialog could get
    // destroyed and we lose all access to its parent because we haven't
    // left modality.  (Eg. modal Progress Dialog)
    if (!isEmbedded && q->isModal() && q->isWindow())
        QApplicationPrivate::leaveModal(q);

    q->setAttribute(Qt::WA_Mapped, false);
    hide_sys();

    bool wasVisible = q->testAttribute(Qt::WA_WState_Visible);

    if (wasVisible) {
        q->setAttribute(Qt::WA_WState_Visible, false);

    }

    QHideEvent hideEvent;
    QApplication::sendEvent(q, &hideEvent);
    hideChildren(false);

    // next bit tries to move the focus if the focus widget is now
    // hidden.
    if (wasVisible) {
#if defined(Q_WS_X11)
        qApp->d_func()->sendSyntheticEnterLeave(q);
#endif

        QWidget *fw = QApplication::focusWidget();
        while (fw &&  !fw->isWindow()) {
            if (fw == q) {
                q->focusNextPrevChild(true);
                break;
            }
            fw = fw->parentWidget();
        }
    }

    if (QWidgetBackingStore *bs = maybeBackingStore())
        bs->removeDirtyWidget(q);
}

/*!
    \fn bool QWidget::isHidden() const

    Returns true if the widget is hidden, otherwise returns false.

    A hidden widget will only become visible when show() is called on
    it. It will not be automatically shown when the parent is shown.

    To check visibility, use !isVisible() instead (notice the exclamation mark).

    isHidden() implies !isVisible(), but a widget can be not visible
    and not hidden at the same time. This is the case for widgets that are children of
    widgets that are not visible.


    Widgets are hidden if:
    \list
        \o they were created as independent windows,
        \o they were created as children of visible widgets,
        \o hide() or setVisible(false) was called.
    \endlist
*/


void QWidget::setVisible(bool visible)
{
    if (visible) { // show
        if (testAttribute(Qt::WA_WState_ExplicitShowHide) && !testAttribute(Qt::WA_WState_Hidden))
            return;

        Q_D(QWidget);

        // Designer uses a trick to make grabWidget work without showing
        if (!isWindow() && parentWidget() && parentWidget()->isVisible()
            && !parentWidget()->testAttribute(Qt::WA_WState_Created))
            parentWidget()->window()->d_func()->createRecursively();

        //we have to at least create toplevels before applyX11SpecificCommandLineArguments
        //but not children of non-visible parents
        QWidget *pw = parentWidget();
        if (!testAttribute(Qt::WA_WState_Created)
            && (isWindow() || pw->testAttribute(Qt::WA_WState_Created))) {
            create();
        }

#if defined(Q_WS_X11)
        if (windowType() == Qt::Window)
            QApplicationPrivate::applyX11SpecificCommandLineArguments(this);
#endif

        bool wasResized = testAttribute(Qt::WA_Resized);
        Qt::WindowStates initialWindowState = windowState();

        // polish if necessary
        ensurePolished();

        // remember that show was called explicitly
        setAttribute(Qt::WA_WState_ExplicitShowHide);
        // whether we need to inform the parent widget immediately
        bool needUpdateGeometry = !isWindow() && testAttribute(Qt::WA_WState_Hidden);
        // we are no longer hidden
        setAttribute(Qt::WA_WState_Hidden, false);

        if (needUpdateGeometry)
            d->updateGeometry_helper(true);

        // activate our layout before we and our children become visible
        if (d->layout)
            d->layout->activate();

        if (!isWindow()) {
            QWidget *parent = parentWidget();
            while (parent && parent->isVisible() && parent->d_func()->layout  && !parent->data->in_show) {
                parent->d_func()->layout->activate();
                if (parent->isWindow())
                    break;
                parent = parent->parentWidget();
            }
            if (parent)
                parent->d_func()->setDirtyOpaqueRegion();
        }

        // adjust size if necessary
        if (!wasResized
            && (isWindow() || !parentWidget()->d_func()->layout))  {
            if (isWindow()) {
                adjustSize();
                if (windowState() != initialWindowState)
                    setWindowState(initialWindowState);
            } else {
                adjustSize();
            }
            setAttribute(Qt::WA_Resized, false);
        }

        setAttribute(Qt::WA_KeyboardFocusChange, false);

        if (isWindow() || parentWidget()->isVisible()) {
            // remove posted quit events when showing a new window
            QCoreApplication::removePostedEvents(qApp, QEvent::Quit);

            d->show_helper();

#if defined(Q_WS_X11)
            qApp->d_func()->sendSyntheticEnterLeave(this);
#endif
        }

        QEvent showToParentEvent(QEvent::ShowToParent);
        QApplication::sendEvent(this, &showToParentEvent);
    } else { // hide
        if (testAttribute(Qt::WA_WState_ExplicitShowHide) && testAttribute(Qt::WA_WState_Hidden))
            return;
        if (QApplicationPrivate::hidden_focus_widget == this)
            QApplicationPrivate::hidden_focus_widget = 0;

        Q_D(QWidget);

        // hw: The test on getOpaqueRegion() needs to be more intelligent
        // currently it doesn't work if the widget is hidden (the region will
        // be clipped). The real check should be testing the cached region
        // (and dirty flag) directly.
        if (!isWindow() && parentWidget()) // && !d->getOpaqueRegion().isEmpty())
            parentWidget()->d_func()->setDirtyOpaqueRegion();

        setAttribute(Qt::WA_WState_Hidden);
        setAttribute(Qt::WA_WState_ExplicitShowHide);
        if (testAttribute(Qt::WA_WState_Created))
            d->hide_helper();

        // invalidate layout similar to updateGeometry()
        if (!isWindow() && parentWidget()) {
            if (parentWidget()->d_func()->layout)
                parentWidget()->d_func()->layout->invalidate();
            else if (parentWidget()->isVisible())
                QApplication::postEvent(parentWidget(), new QEvent(QEvent::LayoutRequest));
        }

        QEvent hideToParentEvent(QEvent::HideToParent);
        QApplication::sendEvent(this, &hideToParentEvent);
    }
}

/*!\fn void QWidget::setHidden(bool hidden)

    Convenience function, equivalent to setVisible(!\a hidden).
*/

void QWidgetPrivate::_q_showIfNotHidden()
{
    Q_Q(QWidget);
    if ( !(q->isHidden() && q->testAttribute(Qt::WA_WState_ExplicitShowHide)) )
        q->setVisible(true);
}

void QWidgetPrivate::showChildren(bool spontaneous)
{
    QList<QObject*> childList = children;
    for (int i = 0; i < childList.size(); ++i) {
        QWidget *widget = qobject_cast<QWidget*>(childList.at(i));
        if (!widget
            || widget->isWindow()
            || widget->testAttribute(Qt::WA_WState_Hidden))
            continue;
        if (spontaneous) {
            widget->setAttribute(Qt::WA_Mapped);
            widget->d_func()->showChildren(true);
            QShowEvent e;
            QApplication::sendSpontaneousEvent(widget, &e);
        } else {
            if (widget->testAttribute(Qt::WA_WState_ExplicitShowHide))
                widget->d_func()->show_recursive();
            else
                widget->show();
        }
    }
}

void QWidgetPrivate::hideChildren(bool spontaneous)
{
    QList<QObject*> childList = children;
    for (int i = 0; i < childList.size(); ++i) {
        QWidget *widget = qobject_cast<QWidget*>(childList.at(i));
        if (!widget || widget->isWindow() || widget->testAttribute(Qt::WA_WState_Hidden))
            continue;
        if (spontaneous)
            widget->setAttribute(Qt::WA_Mapped, false);
        else
            widget->setAttribute(Qt::WA_WState_Visible, false);
        widget->d_func()->hideChildren(spontaneous);
        QHideEvent e;
        if (spontaneous) {
            QApplication::sendSpontaneousEvent(widget, &e);
        } else {
            QApplication::sendEvent(widget, &e);
            if (widget->internalWinId()
                && widget->testAttribute(Qt::WA_DontCreateNativeAncestors)) {
                // hide_sys() on an ancestor won't have any affect on this
                // widget, so it needs an explicit hide_sys() of its own
                widget->d_func()->hide_sys();
            }
        }
#if defined(Q_WS_X11)
        qApp->d_func()->sendSyntheticEnterLeave(widget);
#endif
    }
}

bool QWidgetPrivate::close_helper(CloseMode mode)
{
    if (data.is_closing)
        return true;

    Q_Q(QWidget);
    data.is_closing = true;

    QPointer<QWidget> that = q;
    QPointer<QWidget> parentWidget = q->parentWidget();

    bool quitOnClose = q->testAttribute(Qt::WA_QuitOnClose);
    if (mode != CloseNoEvent) {
        QCloseEvent e;
        if (mode == CloseWithSpontaneousEvent)
            QApplication::sendSpontaneousEvent(q, &e);
        else
            QApplication::sendEvent(q, &e);
        if (!that.isNull() && !e.isAccepted()) {
            data.is_closing = false;
            return false;
        }
    }

    if (!that.isNull() && !q->isHidden())
        q->hide();

    // Attempt to close the application only if this has WA_QuitOnClose set and a non-visible parent
    quitOnClose = quitOnClose && (parentWidget.isNull() || !parentWidget->isVisible());

    if (quitOnClose) {
        /* if there is no non-withdrawn primary window left (except
           the ones without QuitOnClose), we emit the lastWindowClosed
           signal */
        QWidgetList list = QApplication::topLevelWidgets();
        bool lastWindowClosed = true;
        for (int i = 0; i < list.size(); ++i) {
            QWidget *w = list.at(i);
            if (!w->isVisible() || w->parentWidget() || !w->testAttribute(Qt::WA_QuitOnClose))
                continue;
            lastWindowClosed = false;
            break;
        }
        if (lastWindowClosed)
            QApplicationPrivate::emitLastWindowClosed();
    }

    if (!that.isNull()) {
        data.is_closing = false;
        if (q->testAttribute(Qt::WA_DeleteOnClose)) {
            q->setAttribute(Qt::WA_DeleteOnClose, false);
            q->deleteLater();
        }
    }
    return true;
}


/*!
    Closes this widget. Returns true if the widget was closed;
    otherwise returns false.

    First it sends the widget a QCloseEvent. The widget is \link
    hide() hidden\endlink if it \link QCloseEvent::accept()
    accepts\endlink the close event. If it \link QCloseEvent::ignore()
    ignores\endlink the event, nothing happens. The default
    implementation of QWidget::closeEvent() accepts the close event.

    If the widget has the Qt::WA_DeleteOnClose flag, the widget
    is also deleted. A close events is delivered to the widget no
    matter if the widget is visible or not.

    The \l QApplication::lastWindowClosed() signal is emitted when the
    last visible primary window (i.e. window with no parent) with the
    Qt::WA_QuitOnClose attribute set is closed. By default this
    attribute is set for all widgets except transient windows such as
    splash screens, tool windows, and popup menus.

*/

bool QWidget::close()
{
    return d_func()->close_helper(QWidgetPrivate::CloseWithEvent);
}

/*!
    \property QWidget::visible
    \brief whether the widget is visible

    Calling setVisible(true) or show() sets the widget to visible
    status if all its parent widgets up to the window are visible. If
    an ancestor is not visible, the widget won't become visible until
    all its ancestors are shown. If its size or position has changed,
    Qt guarantees that a widget gets move and resize events just
    before it is shown. If the widget has not been resized yet, Qt
    will adjust the widget's size to a useful default using
    adjustSize().

    Calling setVisible(false) or hide() hides a widget explicitly. An
    explicitly hidden widget will never become visible, even if all
    its ancestors become visible, unless you show it.

    A widget receives show and hide events when its visibility status
    changes. Between a hide and a show event, there is no need to
    waste CPU cycles preparing or displaying information to the user.
    A video application, for example, might simply stop generating new
    frames.

    A widget that happens to be obscured by other windows on the
    screen is considered to be visible. The same applies to iconified
    windows and windows that exist on another virtual
    desktop (on platforms that support this concept). A widget
    receives spontaneous show and hide events when its mapping status
    is changed by the window system, e.g. a spontaneous hide event
    when the user minimizes the window, and a spontaneous show event
    when the window is restored again.

    You almost never have to reimplement the setVisible() function. If
    you need to change some settings before a widget is shown, use
    showEvent() instead. If you need to do some delayed initialization
    use the Polish event delivered to the event() function.

    \sa show(), hide(), isHidden(), isVisibleTo(), isMinimized(),
    showEvent(), hideEvent()
*/


/*!
    Returns true if this widget would become visible if \a ancestor is
    shown; otherwise returns false.

    The true case occurs if neither the widget itself nor any parent
    up to but excluding \a ancestor has been explicitly hidden.

    This function will still return true if the widget is obscured by
    other windows on the screen, but could be physically visible if it
    or they were to be moved.

    isVisibleTo(0) is identical to isVisible().

    \sa show() hide() isVisible()
*/

bool QWidget::isVisibleTo(const QWidget* ancestor) const
{
    if (!ancestor)
        return isVisible();
    const QWidget * w = this;
    while (!w->isHidden()
            && !w->isWindow()
            && w->parentWidget()
            && w->parentWidget() != ancestor)
        w = w->parentWidget();
    return !w->isHidden();
}


/*!
    Returns the unobscured region where paint events can occur.

    For visible widgets, this is an approximation of the area not
    covered by other widgets; otherwise, this is an empty region.

    The repaint() function calls this function if necessary, so in
    general you do not need to call it.

*/
QRegion QWidget::visibleRegion() const
{
    Q_D(const QWidget);

    QRect clipRect = d->clipRect();
    if (clipRect.isEmpty())
        return QRegion();
    QRegion r(clipRect);
    d->subtractOpaqueChildren(r, clipRect);
    d->subtractOpaqueSiblings(r);
    return r;
}


QSize QWidgetPrivate::adjustedSize() const
{
    Q_Q(const QWidget);

    QSize s = q->sizeHint();

    if (q->isWindow()) {
        Qt::Orientations exp;
        if (layout) {
            if (layout->hasHeightForWidth())
                s.setHeight(layout->totalHeightForWidth(s.width()));
            exp = layout->expandingDirections();
        } else
        {
            if (q->sizePolicy().hasHeightForWidth())
                s.setHeight(q->heightForWidth(s.width()));
            exp = q->sizePolicy().expandingDirections();
        }
        if (exp & Qt::Horizontal)
            s.setWidth(qMax(s.width(), 200));
        if (exp & Qt::Vertical)
            s.setHeight(qMax(s.height(), 100));
#if defined(Q_WS_X11)
        const QRect screen = QApplication::desktop()->screenGeometry(q->x11Info().screen());
#else // all others
        const QRect screen = QApplication::desktop()->screenGeometry(q->pos());
#endif
        s.setWidth(qMin(s.width(), screen.width()*2/3));
        s.setHeight(qMin(s.height(), screen.height()*2/3));
        if (QTLWExtra *extra = maybeTopData())
            extra->sizeAdjusted = true;
    }

    if (!s.isValid()) {
        QRect r = q->childrenRect(); // get children rectangle
        if (r.isNull())
            return s;
        s = r.size() + QSize(2 * r.x(), 2 * r.y());
    }

    return s;
}

/*!
    Adjusts the size of the widget to fit its contents.

    This function uses sizeHint() if it is valid, i.e., the size hint's width
    and height are \>= 0. Otherwise, it sets the size to the children
    rectangle that covers all child widgets (the union of all child widget
    rectangles).

    For windows, the screen size is also taken into account. If the sizeHint()
    is less than (200, 100) and the size policy is \l{QSizePolicy::Expanding}
    {expanding}, the window will be at least (200, 100). The maximum size of
    a window is 2/3 of the screen's width and height.

    \sa sizeHint(), childrenRect()
*/

void QWidget::adjustSize()
{
    Q_D(QWidget);
    ensurePolished();
    const QSize s = d->adjustedSize();

    if (d->layout)
        d->layout->activate();

    if (s.isValid())
        resize(s);
}


/*!
    \property QWidget::sizeHint
    \brief the recommended size for the widget

    If the value of this property is an invalid size, no size is
    recommended.

    The default implementation of sizeHint() returns an invalid size
    if there is no layout for this widget, and returns the layout's
    preferred size otherwise.

    \sa QSize::isValid(), minimumSizeHint(), sizePolicy(),
    setMinimumSize(), updateGeometry()
*/

QSize QWidget::sizeHint() const
{
    Q_D(const QWidget);
    if (d->layout)
        return d->layout->totalSizeHint();
    return QSize(-1, -1);
}

/*!
    \property QWidget::minimumSizeHint
    \brief the recommended minimum size for the widget

    If the value of this property is an invalid size, no minimum size
    is recommended.

    The default implementation of minimumSizeHint() returns an invalid
    size if there is no layout for this widget, and returns the
    layout's minimum size otherwise. Most built-in widgets reimplement
    minimumSizeHint().

    \l QLayout will never resize a widget to a size smaller than the
    minimum size hint unless minimumSize() is set or the size policy is
    set to QSizePolicy::Ignore. If minimumSize() is set, the minimum
    size hint will be ignored.

    \sa QSize::isValid(), resize(), setMinimumSize(), sizePolicy()
*/
QSize QWidget::minimumSizeHint() const
{
    Q_D(const QWidget);
    if (d->layout)
        return d->layout->totalMinimumSize();
    return QSize(-1, -1);
}


/*!
    \fn QWidget *QWidget::parentWidget() const

    Returns the parent of this widget, or 0 if it does not have any
    parent widget.
*/


/*!
    Returns true if this widget is a parent, (or grandparent and so on
    to any level), of the given \a child, and both widgets are within
    the same window; otherwise returns false.
*/

bool QWidget::isAncestorOf(const QWidget *child) const
{
    while (child) {
        if (child == this)
            return true;
        if (child->isWindow())
            return false;
        child = child->parentWidget();
    }
    return false;
}

/*****************************************************************************
  QWidget event handling
 *****************************************************************************/

/*!
    This is the main event handler; it handles event \a event. You can
    reimplement this function in a subclass, but we recommend using
    one of the specialized event handlers instead.

    Key press and release events are treated differently from other
    events. event() checks for Tab and Shift+Tab and tries to move the
    focus appropriately. If there is no widget to move the focus to
    (or the key press is not Tab or Shift+Tab), event() calls
    keyPressEvent().

    Mouse and tablet event handling is also slightly special: only
    when the widget is \l enabled, event() will call the specialized
    handlers such as mousePressEvent(); otherwise it will discard the
    event.

    This function returns true if the event was recognized, otherwise
    it returns false.  If the recognized event was accepted (see \l
    QEvent::accepted), any further processing such as event
    propagation to the parent widget stops.

    \sa closeEvent(), focusInEvent(), focusOutEvent(), enterEvent(),
    keyPressEvent(), keyReleaseEvent(), leaveEvent(),
    mouseDoubleClickEvent(), mouseMoveEvent(), mousePressEvent(),
    mouseReleaseEvent(), moveEvent(), paintEvent(), resizeEvent(),
    QObject::event(), QObject::timerEvent()
*/

bool QWidget::event(QEvent *event)
{
    Q_D(QWidget);

    // ignore mouse events when disabled
    if (!isEnabled()) {
        switch(event->type()) {
        case QEvent::MouseButtonPress:
        case QEvent::MouseButtonRelease:
        case QEvent::MouseButtonDblClick:
        case QEvent::MouseMove:
        case QEvent::ContextMenu:
#ifndef QT_NO_WHEELEVENT
        case QEvent::Wheel:
#endif
            return false;
        default:
            break;
        }
    }
    switch (event->type()) {
    case QEvent::MouseMove:
        mouseMoveEvent((QMouseEvent*)event);
        break;

    case QEvent::MouseButtonPress:
        mousePressEvent((QMouseEvent*)event);
        break;

    case QEvent::MouseButtonRelease:
        mouseReleaseEvent((QMouseEvent*)event);
        break;

    case QEvent::MouseButtonDblClick:
        mouseDoubleClickEvent((QMouseEvent*)event);
        break;

#ifndef QT_NO_WHEELEVENT
    case QEvent::Wheel:
        wheelEvent((QWheelEvent*)event);
        break;
#endif
    case QEvent::KeyPress: {
        QKeyEvent *k = (QKeyEvent *)event;
        bool res = false;
        if (!(k->modifiers() & (Qt::ControlModifier | Qt::AltModifier))) {  //### Add MetaModifier?
            if (k->key() == Qt::Key_Backtab
                || (k->key() == Qt::Key_Tab && (k->modifiers() & Qt::ShiftModifier)))
                res = focusNextPrevChild(false);
            else if (k->key() == Qt::Key_Tab)
                res = focusNextPrevChild(true);
            if (res)
                break;
        }
        keyPressEvent(k);
#ifndef QT_NO_WHATSTHIS
        if (!k->isAccepted()
            && k->modifiers() & Qt::ShiftModifier && k->key() == Qt::Key_F1
            && d->whatsThis.size()) {
            QWhatsThis::showText(mapToGlobal(QCursor::pos()), d->whatsThis, this);
            k->accept();
        }
#endif
    }
        break;

    case QEvent::KeyRelease:
        keyReleaseEvent((QKeyEvent*)event);
        // fall through
    case QEvent::ShortcutOverride:
        break;

    case QEvent::PolishRequest:
        ensurePolished();
        break;

    case QEvent::Polish: {
        style()->polish(this);
        setAttribute(Qt::WA_WState_Polished);
        if (!QApplication::font(this).isCopyOf(QApplication::font()))
            d->resolveFont();
        if (!QApplication::palette(this).isCopyOf(QApplication::palette()))
            d->resolvePalette();
    }
        break;

    case QEvent::ApplicationWindowIconChange:
        if (isWindow() && !testAttribute(Qt::WA_SetWindowIcon)) {
            d->setWindowIcon_sys();
            d->setWindowIcon_helper();
        }
        break;
    case QEvent::FocusIn:
        focusInEvent((QFocusEvent*)event);
        break;

    case QEvent::FocusOut:
        focusOutEvent((QFocusEvent*)event);
        break;

    case QEvent::Enter:
#ifndef QT_NO_STATUSTIP
        if (d->statusTip.size()) {
            QStatusTipEvent tip(d->statusTip);
            QApplication::sendEvent(const_cast<QWidget *>(this), &tip);
        }
#endif
        enterEvent(event);
        break;

    case QEvent::Leave:
#ifndef QT_NO_STATUSTIP
        if (d->statusTip.size()) {
            QString empty;
            QStatusTipEvent tip(empty);
            QApplication::sendEvent(const_cast<QWidget *>(this), &tip);
        }
#endif
        leaveEvent(event);
        break;

    case QEvent::HoverEnter:
    case QEvent::HoverLeave:
        update();
        break;

    case QEvent::Paint:
        // At this point the event has to be delivered, regardless
        // whether the widget isVisible() or not because it
        // already went through the filters
        paintEvent((QPaintEvent*)event);
        break;

    case QEvent::Move:
        moveEvent((QMoveEvent*)event);
        break;

    case QEvent::Resize:
        resizeEvent((QResizeEvent*)event);
        break;

    case QEvent::Close:
        closeEvent((QCloseEvent *)event);
        break;

#ifndef QT_NO_CONTEXTMENU
    case QEvent::ContextMenu:
        switch (data->context_menu_policy) {
        case Qt::PreventContextMenu:
            break;
        case Qt::DefaultContextMenu:
            contextMenuEvent(static_cast<QContextMenuEvent *>(event));
            break;
        case Qt::CustomContextMenu:
            emit customContextMenuRequested(static_cast<QContextMenuEvent *>(event)->pos());
            break;
#ifndef QT_NO_MENU
        case Qt::ActionsContextMenu:
            if (d->actions.count()) {
                QMenu::exec(d->actions, static_cast<QContextMenuEvent *>(event)->globalPos(),
                            0, this);
                break;
            }
            // fall through
#endif
        default:
            event->ignore();
            break;
        }
        break;
#endif // QT_NO_CONTEXTMENU

#ifndef QT_NO_DRAGANDDROP
    case QEvent::Drop:
        dropEvent((QDropEvent*) event);
        break;

    case QEvent::DragEnter:
        dragEnterEvent((QDragEnterEvent*) event);
        break;

    case QEvent::DragMove:
        dragMoveEvent((QDragMoveEvent*) event);
        break;

    case QEvent::DragLeave:
        dragLeaveEvent((QDragLeaveEvent*) event);
        break;
#endif

    case QEvent::Show:
        showEvent((QShowEvent*) event);
        break;

    case QEvent::Hide:
        hideEvent((QHideEvent*) event);
        break;

    case QEvent::ShowWindowRequest:
        if (!isHidden())
            d->show_sys();
        break;

    case QEvent::ApplicationFontChange:
        d->resolveFont();
        break;
    case QEvent::ApplicationPaletteChange:
        if (!(windowType() == Qt::Desktop))
            d->resolvePalette();
        break;

    case QEvent::ActivationChange:
    case QEvent::EnabledChange:
    case QEvent::FontChange:
    case QEvent::StyleChange:
    case QEvent::PaletteChange:
    case QEvent::WindowTitleChange:
    case QEvent::IconTextChange:
    case QEvent::ModifiedChange:
    case QEvent::MouseTrackingChange:
    case QEvent::ParentChange:
    case QEvent::WindowStateChange:
    case QEvent::LocaleChange:
    case QEvent::ContentsRectChange:
        changeEvent(event);
        break;

    case QEvent::WindowActivate:
    case QEvent::WindowDeactivate: {
        if (isVisible() && !palette().isEqual(QPalette::Active, QPalette::Inactive))
            update();
        QList<QObject*> childList = d->children;
        for (int i = 0; i < childList.size(); ++i) {
            QWidget *w = qobject_cast<QWidget *>(childList.at(i));
            if (w && w->isVisible() && !w->isWindow())
                QApplication::sendEvent(w, event);
        }
        break; }

    case QEvent::LanguageChange: {
        changeEvent(event);
        QList<QObject*> childList = d->children;
        for (int i = 0; i < childList.size(); ++i) {
            QObject *o = childList.at(i);
            if (o)
                QApplication::sendEvent(o, event);
        }
        update();
        break;
    }

    case QEvent::ApplicationLayoutDirectionChange:
        d->resolveLayoutDirection();
        break;

    case QEvent::LayoutDirectionChange:
        if (d->layout)
            d->layout->invalidate();
        update();
        changeEvent(event);
        break;
    case QEvent::UpdateRequest:
        d->syncBackingStore();
        break;
    case QEvent::UpdateLater:
        update(static_cast<QUpdateLaterEvent*>(event)->region());
        break;

    case QEvent::WindowBlocked:
    case QEvent::WindowUnblocked: {
        QList<QObject*> childList = d->children;
        for (int i = 0; i < childList.size(); ++i) {
            QObject *o = childList.at(i);
            if (o && o != QApplication::activeModalWidget()) {
                if (qobject_cast<QWidget *>(o) && static_cast<QWidget *>(o)->isWindow()) {
                    // do not forward the event to child windows,
                    // QApplication does this for us
                    continue;
                }
                QApplication::sendEvent(o, event);
            }
        }
        break;
    }
#ifndef QT_NO_TOOLTIP
    case QEvent::ToolTip:
        if (!d->toolTip.isEmpty())
            QToolTip::showText(static_cast<QHelpEvent*>(event)->globalPos(), d->toolTip, this);
        else
            event->ignore();
        break;
#endif
#ifndef QT_NO_WHATSTHIS
    case QEvent::WhatsThis:
        if (d->whatsThis.size())
            QWhatsThis::showText(static_cast<QHelpEvent *>(event)->globalPos(), d->whatsThis, this);
        else
            event->ignore();
        break;
    case QEvent::QueryWhatsThis:
        if (d->whatsThis.isEmpty())
            event->ignore();
        break;
#endif
#ifndef QT_NO_ACTION
    case QEvent::ActionAdded:
    case QEvent::ActionRemoved:
    case QEvent::ActionChanged:
        actionEvent((QActionEvent*)event);
        break;
#endif

    case QEvent::KeyboardLayoutChange: {
        changeEvent(event);

        // inform children of the change
        QList<QObject*> childList = d->children;
        for (int i = 0; i < childList.size(); ++i) {
            QWidget *w = qobject_cast<QWidget *>(childList.at(i));
            if (w && w->isVisible() && !w->isWindow())
                QApplication::sendEvent(w, event);
        }
        break;
    }
    default:
        return QObject::event(event);
    }
    return true;
}

/*!
  This event handler can be reimplemented to handle state changes.

  The state being changed in this event can be retrieved through the \a event
  supplied.

  Change events include:
  QEvent::ActivationChange, QEvent::EnabledChange, QEvent::FontChange,
  QEvent::StyleChange, QEvent::PaletteChange,
  QEvent::WindowTitleChange, QEvent::IconTextChange,
  QEvent::ModifiedChange, QEvent::MouseTrackingChange,
  QEvent::ParentChange, QEvent::WindowStateChange,
  QEvent::LanguageChange, QEvent::LocaleChange,
  QEvent::LayoutDirectionChange.

*/
void QWidget::changeEvent(QEvent * event)
{
    switch(event->type()) {
    case QEvent::EnabledChange: {
        update();
        break;
    }

    case QEvent::FontChange:
    case QEvent::StyleChange: {
        Q_D(QWidget);
        update();
        updateGeometry();
        if (d->layout)
            d->layout->invalidate();
        break;
    }

    case QEvent::PaletteChange: {
        update();
        break;
    }

    default:
        break;
    }
}

/*!
    This event handler, for event \a event, can be reimplemented in a
    subclass to receive mouse move events for the widget.

    If mouse tracking is switched off, mouse move events only occur if
    a mouse button is pressed while the mouse is being moved. If mouse
    tracking is switched on, mouse move events occur even if no mouse
    button is pressed.

    QMouseEvent::pos() reports the position of the mouse cursor,
    relative to this widget. For press and release events, the
    position is usually the same as the position of the last mouse
    move event, but it might be different if the user's hand shakes.
    This is a feature of the underlying window system, not Qt.

    If you want to show a tooltip immediately, while the mouse is
    moving (e.g., to get the mouse coordinates with QMouseEvent::pos()
    and show them as a tooltip), you must first enable mouse tracking
    as described above. Then, to ensure that the tooltip is updated
    immediately, you must call QToolTip::showText() instead of
    setToolTip() in your implementation of mouseMoveEvent().

    \sa setMouseTracking(), mousePressEvent(), mouseReleaseEvent(),
    mouseDoubleClickEvent(), event(), QMouseEvent, {Scribble Example}
*/

void QWidget::mouseMoveEvent(QMouseEvent *event)
{
    event->ignore();
}

/*!
    This event handler, for event \a event, can be reimplemented in a
    subclass to receive mouse press events for the widget.

    If you create new widgets in the mousePressEvent() the
    mouseReleaseEvent() may not end up where you expect, depending on
    the underlying window system (or X11 window manager), the widgets'
    location and maybe more.

    The default implementation implements the closing of popup widgets
    when you click outside the window. For other widget types it does
    nothing.

    \sa mouseReleaseEvent(), mouseDoubleClickEvent(),
    mouseMoveEvent(), event(), QMouseEvent, {Scribble Example}
*/

void QWidget::mousePressEvent(QMouseEvent *event)
{
    event->ignore();
    if ((windowType() == Qt::Popup)) {
        event->accept();
        QWidget* w;
        while ((w = QApplication::activePopupWidget()) && w != this){
            w->close();
            if (QApplication::activePopupWidget() == w) // widget does not want to disappear
                w->hide(); // hide at least
        }
        if (!rect().contains(event->pos())){
            close();
        }
    }
}

/*!
    This event handler, for event \a event, can be reimplemented in a
    subclass to receive mouse release events for the widget.

    \sa mousePressEvent(), mouseDoubleClickEvent(),
    mouseMoveEvent(), event(), QMouseEvent, {Scribble Example}
*/

void QWidget::mouseReleaseEvent(QMouseEvent *event)
{
    event->ignore();
}

/*!
    This event handler, for event \a event, can be reimplemented in a
    subclass to receive mouse double click events for the widget.

    The default implementation generates a normal mouse press event.

    \note The widget will also receive mouse press and mouse release
    events in addition to the double click event. It is up to the
    developer to ensure that the application interprets these events
    correctly.

    \sa mousePressEvent(), mouseReleaseEvent() mouseMoveEvent(),
    event(), QMouseEvent
*/

void QWidget::mouseDoubleClickEvent(QMouseEvent *event)
{
    mousePressEvent(event);                        // try mouse press event
}

#ifndef QT_NO_WHEELEVENT
/*!
    This event handler, for event \a event, can be reimplemented in a
    subclass to receive wheel events for the widget.

    If you reimplement this handler, it is very important that you
    \link QWheelEvent ignore()\endlink the event if you do not handle
    it, so that the widget's parent can interpret it.

    The default implementation ignores the event.

    \sa QWheelEvent::ignore(), QWheelEvent::accept(), event(),
    QWheelEvent
*/

void QWidget::wheelEvent(QWheelEvent *event)
{
    event->ignore();
}
#endif // QT_NO_WHEELEVENT

/*!
    This event handler, for event \a event, can be reimplemented in a
    subclass to receive key press events for the widget.

    A widget must call setFocusPolicy() to accept focus initially and
    have focus in order to receive a key press event.

    If you reimplement this handler, it is very important that you
    call the base class implementation if you do not act upon the key.

    The default implementation closes popup widgets if the user
    presses Esc. Otherwise the event is ignored, so that the widget's
    parent can interpret it.

    Note that QKeyEvent starts with isAccepted() == true, so you do not
    need to call QKeyEvent::accept() - just do not call the base class
    implementation if you act upon the key.

    \sa keyReleaseEvent(), setFocusPolicy(),
    focusInEvent(), focusOutEvent(), event(), QKeyEvent, {Tetrix Example}
*/

void QWidget::keyPressEvent(QKeyEvent *event)
{
    if ((windowType() == Qt::Popup) && event->key() == Qt::Key_Escape) {
        event->accept();
        close();
    } else {
        event->ignore();
    }
}

/*!
    This event handler, for event \a event, can be reimplemented in a
    subclass to receive key release events for the widget.

    A widget must \link setFocusPolicy() accept focus\endlink
    initially and \link hasFocus() have focus\endlink in order to
    receive a key release event.

    If you reimplement this handler, it is very important that you
    call the base class implementation if you do not act upon the key.

    The default implementation ignores the event, so that the widget's
    parent can interpret it.

    Note that QKeyEvent starts with isAccepted() == true, so you do not
    need to call QKeyEvent::accept() - just do not call the base class
    implementation if you act upon the key.

    \sa keyPressEvent(), QKeyEvent::ignore(), setFocusPolicy(),
    focusInEvent(), focusOutEvent(), event(), QKeyEvent
*/

void QWidget::keyReleaseEvent(QKeyEvent *event)
{
    event->ignore();
}

/*!
    \fn void QWidget::focusInEvent(QFocusEvent *event)

    This event handler can be reimplemented in a subclass to receive
    keyboard focus events (focus received) for the widget. The event
    is passed in the \a event parameter

    A widget normally must setFocusPolicy() to something other than
    Qt::NoFocus in order to receive focus events. (Note that the
    application programmer can call setFocus() on any widget, even
    those that do not normally accept focus.)

    The default implementation updates the widget (except for windows
    that do not specify a focusPolicy()).

    \sa focusOutEvent(), setFocusPolicy(), keyPressEvent(),
    keyReleaseEvent(), event(), QFocusEvent
*/

void QWidget::focusInEvent(QFocusEvent *)
{
    if (focusPolicy() != Qt::NoFocus || !isWindow()) {
        update();
    }
}

/*!
    \fn void QWidget::focusOutEvent(QFocusEvent *event)

    This event handler can be reimplemented in a subclass to receive
    keyboard focus events (focus lost) for the widget. The events is
    passed in the \a event parameter.

    A widget normally must setFocusPolicy() to something other than
    Qt::NoFocus in order to receive focus events. (Note that the
    application programmer can call setFocus() on any widget, even
    those that do not normally accept focus.)

    The default implementation updates the widget (except for windows
    that do not specify a focusPolicy()).

    \sa focusInEvent(), setFocusPolicy(), keyPressEvent(),
    keyReleaseEvent(), event(), QFocusEvent
*/

void QWidget::focusOutEvent(QFocusEvent *)
{
    if (focusPolicy() != Qt::NoFocus || !isWindow())
        update();
}

/*!
    \fn void QWidget::enterEvent(QEvent *event)

    This event handler can be reimplemented in a subclass to receive
    widget enter events which are passed in the \a event parameter.

    An event is sent to the widget when the mouse cursor enters the
    widget.

    \sa leaveEvent(), mouseMoveEvent(), event()
*/

void QWidget::enterEvent(QEvent *)
{
}

/*!
    \fn void QWidget::leaveEvent(QEvent *event)

    This event handler can be reimplemented in a subclass to receive
    widget leave events which are passed in the \a event parameter.

    A leave event is sent to the widget when the mouse cursor leaves
    the widget.

    \sa enterEvent(), mouseMoveEvent(), event()
*/

void QWidget::leaveEvent(QEvent *)
{
}

/*!
    \fn void QWidget::paintEvent(QPaintEvent *event)

    This event handler can be reimplemented in a subclass to receive paint
    events passed in \a event.

    A paint event is a request to repaint all or part of a widget. It can
    happen for one of the following reasons:

    \list
        \o repaint() or update() was invoked,
        \o the widget was obscured and has now been uncovered, or
        \o many other reasons.
    \endlist

    Many widgets can simply repaint their entire surface when asked to, but
    some slow widgets need to optimize by painting only the requested region:
    QPaintEvent::region(). This speed optimization does not change the result,
    as painting is clipped to that region during event processing. QListView
    and QTableView do this, for example.

    Qt also tries to speed up painting by merging multiple paint events into
    one. When update() is called several times or the window system sends
    several paint events, Qt merges these events into one event with a larger
    region (see QRegion::united()). The repaint() function does not permit this
    optimization, so we suggest using update() whenever possible.

    When the paint event occurs, the update region has normally been erased, so
    you are painting on the widget's background.

    The background can be set using setBackgroundRole() and setPalette().

    Since Qt 4.0, QWidget automatically double-buffers its painting, so there
    is no need to write double-buffering code in paintEvent() to avoid flicker.

    \note Generally, you should refrain from calling update() or repaint()
    \bold{inside} a paintEvent(). For example, calling update() or repaint() on
    children inside a paintevent() results in undefined behavior; the child may
    or may not get a paint event.

    \warning If you are using a custom paint engine without Qt's backingstore,
    Qt::WA_PaintOnScreen must be set. Otherwise, QWidget::paintEngine() will
    never be called; the backingstore will be used instead.

    \sa event(), repaint(), update(), QPainter, QPixmap, QPaintEvent,
    {Analog Clock Example}
*/

void QWidget::paintEvent(QPaintEvent *)
{
}


/*!
    \fn void QWidget::moveEvent(QMoveEvent *event)

    This event handler can be reimplemented in a subclass to receive
    widget move events which are passed in the \a event parameter.
    When the widget receives this event, it is already at the new
    position.

    The old position is accessible through QMoveEvent::oldPos().

    \sa resizeEvent(), event(), move(), QMoveEvent
*/

void QWidget::moveEvent(QMoveEvent *)
{
}


/*!
    This event handler can be reimplemented in a subclass to receive
    widget resize events which are passed in the \a event parameter.
    When resizeEvent() is called, the widget already has its new
    geometry. The old size is accessible through
    QResizeEvent::oldSize().

    The widget will be erased and receive a paint event immediately
    after processing the resize event. No drawing need be (or should
    be) done inside this handler.


    \sa moveEvent(), event(), resize(), QResizeEvent, paintEvent(),
        {Scribble Example}
*/

void QWidget::resizeEvent(QResizeEvent * /* event */)
{
}

#ifndef QT_NO_ACTION
/*!
    \fn void QWidget::actionEvent(QActionEvent *event)

    This event handler is called with the given \a event whenever the
    widget's actions are changed.

    \sa addAction(), insertAction(), removeAction(), actions(), QActionEvent
*/
void QWidget::actionEvent(QActionEvent *)
{
}
#endif

/*!
    This event handler is called with the given \a event when Qt receives a window
    close request for a top-level widget from the window system.

    By default, the event is accepted and the widget is closed. You can reimplement
    this function to change the way the widget responds to window close requests.
    For example, you can prevent the window from closing by calling \l{QEvent::}{ignore()}
    on all events.

    Main window applications typically use reimplementations of this function to check
    whether the user's work has been saved and ask for permission before closing.
    For example, the \l{Application Example} uses a helper function to determine whether
    or not to close the window:

    \snippet mainwindows/application/mainwindow.cpp 3
    \snippet mainwindows/application/mainwindow.cpp 4

    \sa event(), hide(), close(), QCloseEvent, {Application Example}
*/

void QWidget::closeEvent(QCloseEvent *event)
{
    event->accept();
}

#ifndef QT_NO_CONTEXTMENU
/*!
    This event handler, for event \a event, can be reimplemented in a
    subclass to receive widget context menu events.

    The handler is called when the widget's \l contextMenuPolicy is
    Qt::DefaultContextMenu.

    The default implementation ignores the context event.
    See the \l QContextMenuEvent documentation for more details.

    \sa event(), QContextMenuEvent customContextMenuRequested()
*/

void QWidget::contextMenuEvent(QContextMenuEvent *event)
{
    event->ignore();
}
#endif // QT_NO_CONTEXTMENU

#ifndef QT_NO_DRAGANDDROP

/*!
    \fn void QWidget::dragEnterEvent(QDragEnterEvent *event)

    This event handler is called when a drag is in progress and the
    mouse enters this widget. The event is passed in the \a event parameter.

    If the event is ignored, the widget won't receive any \l{dragMoveEvent()}{drag
    move events}.

    See the \link dnd.html Drag-and-drop documentation\endlink for an
    overview of how to provide drag-and-drop in your application.

    \sa QDrag, QDragEnterEvent
*/
void QWidget::dragEnterEvent(QDragEnterEvent *)
{
}

/*!
    \fn void QWidget::dragMoveEvent(QDragMoveEvent *event)

    This event handler is called if a drag is in progress, and when
    any of the following conditions occur: the cursor enters this widget,
    the cursor moves within this widget, or a modifier key is pressed on
    the keyboard while this widget has the focus. The event is passed
    in the \a event parameter.

    See the \link dnd.html Drag-and-drop documentation\endlink for an
    overview of how to provide drag-and-drop in your application.

    \sa QDrag, QDragMoveEvent
*/
void QWidget::dragMoveEvent(QDragMoveEvent *)
{
}

/*!
    \fn void QWidget::dragLeaveEvent(QDragLeaveEvent *event)

    This event handler is called when a drag is in progress and the
    mouse leaves this widget. The event is passed in the \a event
    parameter.

    See the \link dnd.html Drag-and-drop documentation\endlink for an
    overview of how to provide drag-and-drop in your application.

    \sa QDrag, QDragLeaveEvent
*/
void QWidget::dragLeaveEvent(QDragLeaveEvent *)
{
}

/*!
    \fn void QWidget::dropEvent(QDropEvent *event)

    This event handler is called when the drag is dropped on this
    widget. The event is passed in the \a event parameter.

    See the \link dnd.html Drag-and-drop documentation\endlink for an
    overview of how to provide drag-and-drop in your application.

    \sa QDrag, QDropEvent
*/
void QWidget::dropEvent(QDropEvent *)
{
}

#endif // QT_NO_DRAGANDDROP

/*!
    \fn void QWidget::showEvent(QShowEvent *event)

    This event handler can be reimplemented in a subclass to receive
    widget show events which are passed in the \a event parameter.

    Non-spontaneous show events are sent to widgets immediately
    before they are shown. The spontaneous show events of windows are
    delivered afterwards.

    Note: A widget receives spontaneous show and hide events when its
    mapping status is changed by the window system, e.g. a spontaneous
    hide event when the user minimizes the window, and a spontaneous
    show event when the window is restored again. After receiving a
    spontaneous hide event, a widget is still considered visible in
    the sense of isVisible().

    \sa visible, event(), QShowEvent
*/
void QWidget::showEvent(QShowEvent *)
{
}

/*!
    \fn void QWidget::hideEvent(QHideEvent *event)

    This event handler can be reimplemented in a subclass to receive
    widget hide events. The event is passed in the \a event parameter.

    Hide events are sent to widgets immediately after they have been
    hidden.

    Note: A widget receives spontaneous show and hide events when its
    mapping status is changed by the window system, e.g. a spontaneous
    hide event when the user minimizes the window, and a spontaneous
    show event when the window is restored again. After receiving a
    spontaneous hide event, a widget is still considered visible in
    the sense of isVisible().

    \sa visible, event(), QHideEvent
*/
void QWidget::hideEvent(QHideEvent *)
{
}

/*
    \fn QWidget::x11Event(MSG *)

    This special event handler can be reimplemented in a subclass to receive
    native X11 events.

    In your reimplementation of this function, if you want to stop Qt from
    handling the event, return true. If you return false, this native event
    is passed back to Qt, which translates it into a Qt event and sends it to
    the widget.

    \note Events are only delivered to this event handler if the widget is
    native.

    \warning This function is not portable.

    \sa QApplication::x11EventFilter(), QWidget::winId()
*/

#if defined(Q_WS_X11)

/*!
    \fn bool QWidget::x11Event(XEvent *event)

    This special event handler can be reimplemented in a subclass to receive
    native X11 events passed in the \a event parameter.

    In your reimplementation of this function, if you want to stop Qt from
    handling the event, return true. If you return false, this native event
    is passed back to Qt, which translates it into a Qt event and sends it to
    the widget.

    \note Events are only delivered to this event handler if the widget is
    native.

    \warning This function is not portable.

    \sa QApplication::x11EventFilter(), QWidget::winId()
*/
bool QWidget::x11Event(XEvent *)
{
    return false;
}

#endif


/*!
    Ensures that the widget has been polished by QStyle (i.e., has a
    proper font and palette).

    QWidget calls this function after it has been fully constructed
    but before it is shown the very first time. You can call this
    function if you want to ensure that the widget is polished before
    doing an operation, e.g., the correct font size might be needed in
    the widget's sizeHint() reimplementation. Note that this function
    \e is called from the default implementation of sizeHint().

    Polishing is useful for final initialization that must happen after
    all constructors (from base classes as well as from subclasses)
    have been called.

    If you need to change some settings when a widget is polished,
    reimplement event() and handle the QEvent::Polish event type.

    \bold{Note:} The function is declared const so that it can be called from
    other const functions (e.g., sizeHint()).

    \sa event()
*/
void QWidget::ensurePolished() const
{
    Q_D(const QWidget);

    const QMetaObject *m = metaObject();
    if (m == d->polished)
        return;
    d->polished = m;

    QEvent e(QEvent::Polish);
    QCoreApplication::sendEvent(const_cast<QWidget *>(this), &e);

    // polish children after 'this'
    QList<QObject*> children = d->children;
    for (int i = 0; i < children.size(); ++i) {
        QObject *o = children.at(i);
        if(!o->isWidgetType())
            continue;
        if (QWidget *w = qobject_cast<QWidget *>(o))
            w->ensurePolished();
    }

    if (d->parent && d->sendChildEvents) {
        QChildEvent e(QEvent::ChildPolished, const_cast<QWidget *>(this));
        QCoreApplication::sendEvent(d->parent, &e);
    }
}

/*!
    Returns the mask currently set on a widget. If no mask is set the
    return value will be an empty region.

    \sa setMask(), clearMask(), QRegion::isEmpty(), {Shaped Clock Example}
*/
QRegion QWidget::mask() const
{
    Q_D(const QWidget);
    return d->extra ? d->extra->mask : QRegion();
}

/*!
    Returns the layout manager that is installed on this widget, or 0
    if no layout manager is installed.

    The layout manager sets the geometry of the widget's children
    that have been added to the layout.

    \sa setLayout(), sizePolicy(), {Layout Management}
*/
QLayout *QWidget::layout() const
{
    return d_func()->layout;
}


/*!
    \fn void QWidget::setLayout(QLayout *layout)

    Sets the layout manager for this widget to \a layout.

    If there already is a layout manager installed on this widget,
    QWidget won't let you install another. You must first delete the
    existing layout manager (returned by layout()) before you can
    call setLayout() with the new layout.

    If \a layout is the layout manger on a different widget, setLayout()
    will reparent the layout and make it the layout manager for this widget.

    Example:

    \snippet examples/uitools/textfinder/textfinder.cpp 3b

    An alternative to calling this function is to pass this widget to
    the layout's constructor.

    The QWidget will take ownership of \a layout.

    \sa layout(), {Layout Management}
*/

void QWidget::setLayout(QLayout *l)
{
    if (Q_UNLIKELY(!l)) {
        qWarning("QWidget::setLayout: Cannot set layout to 0");
        return;
    } else if (layout()) {
        if (Q_UNLIKELY(layout() != l)) {
            qWarning("QWidget::setLayout: Attempting to set QLayout \"%s\" on %s \"%s\", which already has a"
                     " layout", l->objectName().toLocal8Bit().data(), metaObject()->className(),
                     objectName().toLocal8Bit().data());
        }
        return;
    }

    QObject *oldParent = l->parent();
    if (oldParent && oldParent != this) {
        if (oldParent->isWidgetType()) {
            // Steal the layout off a widget parent. Takes effect when
            // morphing laid-out container widgets in Designer.
            QWidget *oldParentWidget = static_cast<QWidget *>(oldParent);
            oldParentWidget->takeLayout();
        } else {
            qWarning("QWidget::setLayout: Attempting to set QLayout \"%s\" on %s \"%s\", when the QLayout already has a parent",
                     l->objectName().toLocal8Bit().data(), metaObject()->className(),
                     objectName().toLocal8Bit().data());
            return;
        }
    }

    Q_D(QWidget);
    l->d_func()->topLevel = true;
    d->layout = l;
    if (oldParent != this) {
        l->setParent(this);
        l->d_func()->reparentChildWidgets(this);
        l->invalidate();
    }

    if (isWindow() && d->maybeTopData())
        d->topData()->sizeAdjusted = false;
}

/*!
    \fn QLayout *QWidget::takeLayout()

    Remove the layout from the widget.
    \since 4.5
*/

QLayout *QWidget::takeLayout()
{
    Q_D(QWidget);
    QLayout *l =  layout();
    if (!l)
        return 0;
    d->layout = 0;
    l->setParent(0);
    return l;
}

/*!
    \property QWidget::sizePolicy
    \brief the default layout behavior of the widget

    If there is a QLayout that manages this widget's children, the
    size policy specified by that layout is used. If there is no such
    QLayout, the result of this function is used.

    The default policy is Preferred/Preferred, which means that the
    widget can be freely resized, but prefers to be the size
    sizeHint() returns. Button-like widgets set the size policy to
    specify that they may stretch horizontally, but are fixed
    vertically. The same applies to lineedit controls (such as
    QLineEdit, QSpinBox or an editable QComboBox) and other
    horizontally orientated widgets (such as QProgressBar).
    QToolButton's are normally square, so they allow growth in both
    directions. Widgets that support different directions (such as
    QSlider, QScrollBar or QHeader) specify stretching in the
    respective direction only. Widgets that can provide scroll bars
    (usually subclasses of QScrollArea) tend to specify that they can
    use additional space, and that they can make do with less than
    sizeHint().

    \sa sizeHint() QLayout QSizePolicy updateGeometry()
*/
QSizePolicy QWidget::sizePolicy() const
{
    Q_D(const QWidget);
    return d->size_policy;
}

void QWidget::setSizePolicy(QSizePolicy policy)
{
    Q_D(QWidget);
    setAttribute(Qt::WA_WState_OwnSizePolicy);
    if (policy == d->size_policy)
        return;
    d->size_policy = policy;

#ifndef QT_NO_GRAPHICSVIEW
    if (QWExtra *extra = d->extra) {
        if (extra->proxyWidget)
            extra->proxyWidget->setSizePolicy(policy);
    }
#endif

    updateGeometry();

    if (isWindow() && d->maybeTopData())
        d->topData()->sizeAdjusted = false;
}

/*!
    \fn void QWidget::setSizePolicy(QSizePolicy::Policy horizontal, QSizePolicy::Policy vertical)
    \overload

    Sets the size policy of the widget to \a horizontal and \a
    vertical, with standard stretch and no height-for-width.

    \sa QSizePolicy::QSizePolicy()
*/

/*!
    Returns the preferred height for this widget, given the width \a w.

    If this widget has a layout, the default implementation returns
    the layout's preferred height.  if there is no layout, the default
    implementation returns -1 indicating that the preferred height
    does not depend on the width.
*/

int QWidget::heightForWidth(int w) const
{
    if (layout() && layout()->hasHeightForWidth())
        return layout()->totalHeightForWidth(w);
    return -1;
}


/*!
    \internal

    *virtual private*

    This is a bit hackish, but ideally we would have created a virtual function
    in the public API (however, too late...) so that subclasses could reimplement 
    their own function.
    Instead we add a virtual function to QWidgetPrivate.
    ### Qt5: move to public class and make virtual
*/ 
bool QWidgetPrivate::hasHeightForWidth() const
{
    return layout ? layout->hasHeightForWidth() : size_policy.hasHeightForWidth();
}

/*!
    \fn QWidget *QWidget::childAt(int x, int y) const

    Returns the visible child widget at the position (\a{x}, \a{y})
    in the widget's coordinate system. If there is no visible child
    widget at the specified position, the function returns 0.
*/

/*!
    \overload

    Returns the visible child widget at point \a p in the widget's own
    coordinate system.
*/

QWidget *QWidget::childAt(const QPoint &p) const
{
    return d_func()->childAt_helper(p, false);
}

QWidget *QWidgetPrivate::childAt_helper(const QPoint &p, bool ignoreChildrenInDestructor) const
{
    if (children.isEmpty())
        return 0;

    if (!pointInsideRectAndMask(p))
        return 0;
    return childAtRecursiveHelper(p, ignoreChildrenInDestructor);
}

QWidget *QWidgetPrivate::childAtRecursiveHelper(const QPoint &p, bool ignoreChildrenInDestructor) const
{
    for (int i = children.size() - 1; i >= 0; --i) {
        QWidget *child = qobject_cast<QWidget *>(children.at(i));
        if (!child || child->isWindow() || child->isHidden() || child->testAttribute(Qt::WA_TransparentForMouseEvents)
            || (ignoreChildrenInDestructor && child->data->in_destructor)) {
            continue;
        }

        // Map the point 'p' from parent coordinates to child coordinates.
        const QPoint childPoint = p - child->data->crect.topLeft();

        // Check if the point hits the child.
        if (!child->d_func()->pointInsideRectAndMask(childPoint))
            continue;

        // Do the same for the child's descendants.
        if (QWidget *w = child->d_func()->childAtRecursiveHelper(childPoint, ignoreChildrenInDestructor))
            return w;

        // We have found our target; namely the child at position 'p'.
        return child;
    }
    return 0;
}

void QWidgetPrivate::updateGeometry_helper(bool forceUpdate)
{
    Q_Q(QWidget);
    if (forceUpdate || !extra || extra->minw != extra->maxw || extra->minh != extra->maxh) {
        QWidget *parent;
        if (!q->isWindow() && !q->isHidden() && (parent = q->parentWidget())) {
            if (parent->d_func()->layout)
                parent->d_func()->layout->invalidate();
            else if (parent->isVisible())
                QApplication::postEvent(parent, new QEvent(QEvent::LayoutRequest));
        }
    }
}

/*!
    Notifies the layout system that this widget has changed and may
    need to change geometry.

    Call this function if the sizeHint() or sizePolicy() have changed.

    For explicitly hidden widgets, updateGeometry() is a no-op. The
    layout system will be notified as soon as the widget is shown.
*/

void QWidget::updateGeometry()
{
    Q_D(QWidget);
    d->updateGeometry_helper(false);
}

/*! \property QWidget::windowFlags

    Window flags are a combination of a type (e.g. Qt::Dialog) and
    zero or more hints to the window system (e.g.
    Qt::FramelessWindowHint).

    If the widget had type Qt::Widget or Qt::SubWindow and becomes a
    window (Qt::Window, Qt::Dialog, etc.), it is put at position (0,
    0) on the desktop. If the widget is a window and becomes a
    Qt::Widget or Qt::SubWindow, it is put at position (0, 0)
    relative to its parent widget.

    \note This function calls setParent() when changing the flags for
    a window, causing the widget to be hidden. You must call show() to make
    the widget visible again..

    \sa windowType(), {Window Flags Example}
*/
void QWidget::setWindowFlags(Qt::WindowFlags flags)
{
    if (data->window_flags == flags)
        return;

    Q_D(QWidget);

    if ((data->window_flags | flags) & Qt::Window) {
        // the old type was a window and/or the new type is a window
        QPoint oldPos = pos();
        bool visible = isVisible();
        setParent(parentWidget(), flags);

        // if both types are windows or neither of them are, we restore
        // the old position
        if (!((data->window_flags ^ flags) & Qt::Window)
            && (visible || testAttribute(Qt::WA_Moved))) {
            move(oldPos);
        }
        // for backward-compatibility we change Qt::WA_QuitOnClose attribute value only when the window was recreated.
        d->adjustQuitOnCloseAttribute();
    } else {
        data->window_flags = flags;
    }
}

/*!
    Sets the window flags for the widget to \a flags,
    \e without telling the window system.

    \warning Do not call this function unless you really know what
    you're doing.

    \sa setWindowFlags()
*/
void QWidget::overrideWindowFlags(Qt::WindowFlags flags)
{
    data->window_flags = flags;
}

/*!
    \fn Qt::WindowType QWidget::windowType() const

    Returns the window type of this widget. This is identical to
    windowFlags() & Qt::WindowType_Mask.

    \sa windowFlags
*/

/*!
    Sets the parent of the widget to \a parent, and resets the window
    flags. The widget is moved to position (0, 0) in its new parent.

    If the new parent widget is in a different window, the
    reparented widget and its children are appended to the end of the
    \l{setFocusPolicy()}{tab chain} of the new parent
    widget, in the same internal order as before. If one of the moved
    widgets had keyboard focus, setParent() calls clearFocus() for that
    widget.

    If the new parent widget is in the same window as the
    old parent, setting the parent doesn't change the tab order or
    keyboard focus.

    If the "new" parent widget is the old parent widget, this function
    does nothing.

    \note The widget becomes invisible as part of changing its parent,
    even if it was previously visible. You must call show() to make the
    widget visible again.

    \warning It is very unlikely that you will ever need this
    function. If you have a widget that changes its content
    dynamically, it is far easier to use \l QStackedWidget.

    \sa setWindowFlags()
*/
void QWidget::setParent(QWidget *parent)
{
    if (parent == parentWidget())
        return;
    setParent((QWidget*)parent, windowFlags() & ~Qt::WindowType_Mask);
}

/*!
    \overload

    This function also takes widget flags, \a f as an argument.
*/

void QWidget::setParent(QWidget *parent, Qt::WindowFlags f)
{
    Q_D(QWidget);
    d->inSetParent = true;
    bool resized = testAttribute(Qt::WA_Resized);
    bool wasCreated = testAttribute(Qt::WA_WState_Created);
    QWidget *oldtlw = window();

    QWidget *desktopWidget = 0;
    if (parent && parent->windowType() == Qt::Desktop)
        desktopWidget = parent;
    bool newParent = (parent != parentWidget()) || !wasCreated || desktopWidget;

#if defined(Q_WS_X11)
    if (newParent && parent && !desktopWidget) {
        if (testAttribute(Qt::WA_NativeWindow) && !qApp->testAttribute(Qt::AA_DontCreateNativeWidgetSiblings))
            parent->d_func()->enforceNativeChildren();
        else if (parent->d_func()->nativeChildrenForced() || parent->testAttribute(Qt::WA_PaintOnScreen))
            setAttribute(Qt::WA_NativeWindow);
    }
#endif

    if (wasCreated) {
        if (!testAttribute(Qt::WA_WState_Hidden)) {
            hide();
            setAttribute(Qt::WA_WState_ExplicitShowHide, false);
        }
        if (newParent) {
            QEvent e(QEvent::ParentAboutToChange);
            QApplication::sendEvent(this, &e);
        }
    }
    if (newParent && isAncestorOf(focusWidget()))
        focusWidget()->clearFocus();

    QTLWExtra *oldTopExtra = window()->d_func()->maybeTopData();
    QWidgetBackingStoreTracker *oldBsTracker = oldTopExtra ? &oldTopExtra->backingStore : 0;

    d->setParent_sys(parent, f);

    QTLWExtra *topExtra = window()->d_func()->maybeTopData();
    QWidgetBackingStoreTracker *bsTracker = topExtra ? &topExtra->backingStore : 0;
    if (oldBsTracker && oldBsTracker != bsTracker)
        oldBsTracker->unregisterWidgetSubtree(this);

    if (desktopWidget)
        parent = 0;

    if (QWidgetBackingStore *oldBs = oldtlw->d_func()->maybeBackingStore()) {
        if (newParent)
            oldBs->removeDirtyWidget(this);
    }

    if (QApplication::testAttribute(Qt::AA_ImmediateWidgetCreation)
        && !testAttribute(Qt::WA_WState_Created))
        create();

    d->reparentFocusWidgets(oldtlw);
    setAttribute(Qt::WA_Resized, resized);
    if (parent) {
        d->resolveFont();
        d->resolvePalette();
    }
    d->resolveLayoutDirection();
    d->resolveLocale();

    // Note: GL widgets under WGL or EGL will always need a ParentChange
    // event to handle recreation/rebinding of the GL context.
    if (newParent) {
        // propagate enabled updates enabled state to non-windows
        if (!isWindow()) {
            if (!testAttribute(Qt::WA_ForceDisabled))
                d->setEnabled_helper(parent ? parent->isEnabled() : true);
            if (!testAttribute(Qt::WA_ForceUpdatesDisabled))
                d->setUpdatesEnabled_helper(parent ? parent->updatesEnabled() : true);
        }

        // send and post remaining QObject events
        if (parent && d->sendChildEvents) {
            QChildEvent e(QEvent::ChildAdded, this);
            QApplication::sendEvent(parent, &e);
        }

        if (parent && d->sendChildEvents && d->polished) {
            QChildEvent e(QEvent::ChildPolished, this);
            QCoreApplication::sendEvent(parent, &e);
        }

        QEvent e(QEvent::ParentChange);
        QApplication::sendEvent(this, &e);
    }

    if (!wasCreated) {
        if (isWindow() || parentWidget()->isVisible())
            setAttribute(Qt::WA_WState_Hidden, true);
        else if (!testAttribute(Qt::WA_WState_ExplicitShowHide))
            setAttribute(Qt::WA_WState_Hidden, false);
    }

    d->updateIsOpaque();

#ifndef QT_NO_GRAPHICSVIEW
    // Embed the widget into a proxy if the parent is embedded.
    // ### Doesn't handle reparenting out of an embedded widget.
    if (oldtlw->graphicsProxyWidget()) {
        if (QGraphicsProxyWidget *ancestorProxy = d->nearestGraphicsProxyWidget(oldtlw))
            ancestorProxy->d_func()->unembedSubWindow(this);
    }
    if (isWindow() && parent && !graphicsProxyWidget() && !bypassGraphicsProxyWidget(this)) {
        if (QGraphicsProxyWidget *ancestorProxy = d->nearestGraphicsProxyWidget(parent))
            ancestorProxy->d_func()->embedSubWindow(this);
    }
#endif

    d->inSetParent = false;
}

/*!
    Scrolls the widget including its children \a dx pixels to the
    right and \a dy downward. Both \a dx and \a dy may be negative.

    After scrolling, the widgets will receive paint events for
    the areas that need to be repainted. For widgets that Qt knows to
    be opaque, this is only the newly exposed parts.
    For example, if an opaque widget is scrolled 8 pixels to the left,
    only an 8-pixel wide stripe at the right edge needs updating.

    Since widgets propagate the contents of their parents by default,
    you need to set the \l autoFillBackground property, or use
    setAttribute() to set the Qt::WA_OpaquePaintEvent attribute, to make
    a widget opaque.

    For widgets that use contents propagation, a scroll will cause an
    update of the entire scroll area.

    \sa {Transparency and Double Buffering}
*/

void QWidget::scroll(int dx, int dy)
{
    if ((!updatesEnabled() && children().size() == 0) || !isVisible())
        return;
    if (dx == 0 && dy == 0)
        return;
    Q_D(QWidget);
#ifndef QT_NO_GRAPHICSVIEW
    if (QGraphicsProxyWidget *proxy = QWidgetPrivate::nearestGraphicsProxyWidget(this)) {
        // Graphics View maintains its own dirty region as a list of rects;
        // until we can connect item updates directly to the view, we must
        // separately add a translated dirty region.
        if (!d->dirty.isEmpty()) {
            foreach (const QRect &rect, (d->dirty.translated(dx, dy)).rects())
                proxy->update(rect);
        }
        proxy->scroll(dx, dy, proxy->subWidgetRect(this));
        return;
    }
#endif
    d->setDirtyOpaqueRegion();
    d->scroll_sys(dx, dy);
}

/*!
    \overload

    This version only scrolls \a r and does not move the children of
    the widget.

    If \a r is empty or invalid, the result is undefined.

    \sa QScrollArea
*/
void QWidget::scroll(int dx, int dy, const QRect &r)
{

    if ((!updatesEnabled() && children().size() == 0) || !isVisible())
        return;
    if (dx == 0 && dy == 0)
        return;
    Q_D(QWidget);
#ifndef QT_NO_GRAPHICSVIEW
    if (QGraphicsProxyWidget *proxy = QWidgetPrivate::nearestGraphicsProxyWidget(this)) {
        // Graphics View maintains its own dirty region as a list of rects;
        // until we can connect item updates directly to the view, we must
        // separately add a translated dirty region.
        if (!d->dirty.isEmpty()) {
            foreach (const QRect &rect, (d->dirty.translated(dx, dy) & r).rects())
                proxy->update(rect);
        }
        proxy->scroll(dx, dy, r.translated(proxy->subWidgetRect(this).topLeft().toPoint()));
        return;
    }
#endif
    d->scroll_sys(dx, dy, r);
}

/*!
    Repaints the widget directly by calling paintEvent() immediately,
    unless updates are disabled or the widget is hidden.

    We suggest only using repaint() if you need an immediate repaint,
    for example during animation. In almost all circumstances update()
    is better, as it permits Qt to optimize for speed and minimize
    flicker.

    \warning If you call repaint() in a function which may itself be
    called from paintEvent(), you may get infinite recursion. The
    update() function never causes recursion.

    \sa update(), paintEvent(), setUpdatesEnabled()
*/

void QWidget::repaint()
{
    repaint(rect());
}

/*! \overload

    This version repaints a rectangle (\a x, \a y, \a w, \a h) inside
    the widget.

    If \a w is negative, it is replaced with \c{width() - x}, and if
    \a h is negative, it is replaced width \c{height() - y}.
*/
void QWidget::repaint(int x, int y, int w, int h)
{
    if (x > data->crect.width() || y > data->crect.height())
        return;

    if (w < 0)
        w = data->crect.width()  - x;
    if (h < 0)
        h = data->crect.height() - y;

    repaint(QRect(x, y, w, h));
}

/*! \overload

    This version repaints a rectangle \a rect inside the widget.
*/
void QWidget::repaint(const QRect &rect)
{
    if (testAttribute(Qt::WA_WState_ConfigPending)) {
        update(rect);
        return;
    }

    if (!isVisible() || !updatesEnabled() || rect.isEmpty())
        return;

    QTLWExtra *tlwExtra = window()->d_func()->maybeTopData();
    if (tlwExtra && !tlwExtra->inTopLevelResize && tlwExtra->backingStore) {
        tlwExtra->inRepaint = true;
        tlwExtra->backingStore->markDirty(rect, this, true);
        tlwExtra->inRepaint = false;
    }
}

/*!
    \overload

    This version repaints a region \a rgn inside the widget.
*/
void QWidget::repaint(const QRegion &rgn)
{
    if (testAttribute(Qt::WA_WState_ConfigPending)) {
        update(rgn);
        return;
    }

    if (!isVisible() || !updatesEnabled() || rgn.isEmpty())
        return;

    QTLWExtra *tlwExtra = window()->d_func()->maybeTopData();
    if (tlwExtra && !tlwExtra->inTopLevelResize && tlwExtra->backingStore) {
        tlwExtra->inRepaint = true;
        tlwExtra->backingStore->markDirty(rgn, this, true);
        tlwExtra->inRepaint = false;
    }
}

/*!
    Updates the widget unless updates are disabled or the widget is
    hidden.

    This function does not cause an immediate repaint; instead it
    schedules a paint event for processing when Qt returns to the main
    event loop. This permits Qt to optimize for more speed and less
    flicker than a call to repaint() does.

    Calling update() several times normally results in just one
    paintEvent() call.

    Qt normally erases the widget's area before the paintEvent() call.
    If the Qt::WA_OpaquePaintEvent widget attribute is set, the widget is
    responsible for painting all its pixels with an opaque color.

    \sa repaint() paintEvent(), setUpdatesEnabled(), {Analog Clock Example}
*/
void QWidget::update()
{
    update(rect());
}

/*! \fn void QWidget::update(int x, int y, int w, int h)
    \overload

    This version updates a rectangle (\a x, \a y, \a w, \a h) inside
    the widget.
*/

/*!
    \overload

    This version updates a rectangle \a rect inside the widget.
*/
void QWidget::update(const QRect &rect)
{
    if (!isVisible() || !updatesEnabled())
        return;

    QRect r = rect & QWidget::rect();

    if (r.isEmpty())
        return;

    if (testAttribute(Qt::WA_WState_InPaintEvent)) {
        QApplication::postEvent(this, new QUpdateLaterEvent(r));
        return;
    }

    QTLWExtra *tlwExtra = window()->d_func()->maybeTopData();
    if (tlwExtra && !tlwExtra->inTopLevelResize && tlwExtra->backingStore)
        tlwExtra->backingStore->markDirty(r, this);
}

/*!
    \overload

    This version repaints a region \a rgn inside the widget.
*/
void QWidget::update(const QRegion &rgn)
{
    if (!isVisible() || !updatesEnabled() || rgn.isEmpty())
        return;

    QRegion r = rgn & QWidget::rect();

    if (r.isEmpty())
        return;

    if (testAttribute(Qt::WA_WState_InPaintEvent)) {
        QApplication::postEvent(this, new QUpdateLaterEvent(r));
        return;
    }

    QTLWExtra *tlwExtra = window()->d_func()->maybeTopData();
    if (tlwExtra && !tlwExtra->inTopLevelResize && tlwExtra->backingStore)
        tlwExtra->backingStore->markDirty(r, this);
}

/*!
    Sets the attribute \a attribute on this widget if \a on is true;
    otherwise clears the attribute.

    \sa testAttribute()
*/
void QWidget::setAttribute(Qt::WidgetAttribute attribute, bool on)
{
    if (testAttribute(attribute) == on)
        return;

    Q_D(QWidget);

    d->widget_attributes.set(attribute, on);

    switch (attribute) {

#ifndef QT_NO_DRAGANDDROP
    case Qt::WA_AcceptDrops:  {
        if (on && !testAttribute(Qt::WA_DropSiteRegistered))
            setAttribute(Qt::WA_DropSiteRegistered, true);
        else if (!on && (isWindow() || !parentWidget() || !parentWidget()->testAttribute(Qt::WA_DropSiteRegistered)))
            setAttribute(Qt::WA_DropSiteRegistered, false);
        QEvent e(QEvent::AcceptDropsChange);
        QApplication::sendEvent(this, &e);
        break;
    }
    case Qt::WA_DropSiteRegistered:  {
        for (int i = 0; i < d->children.size(); ++i) {
            QWidget *w = qobject_cast<QWidget *>(d->children.at(i));
            if (w && !w->isWindow() && !w->testAttribute(Qt::WA_AcceptDrops) && w->testAttribute(Qt::WA_DropSiteRegistered) != on)
                w->setAttribute(Qt::WA_DropSiteRegistered, on);
        }
        break;
    }
#endif

    case Qt::WA_NoChildEventsForParent: {
        d->sendChildEvents = !on;
        break;
    }
    case Qt::WA_NoChildEventsFromChildren: {
        d->receiveChildEvents = !on;
        break;
    }
    case Qt::WA_ShowModal: {
        if (!on) {
            if (isVisible())
                QApplicationPrivate::leaveModal(this);
            // reset modality type to Modeless when clearing WA_ShowModal
            data->window_modality = Qt::NonModal;
        } else if (data->window_modality == Qt::NonModal) {
            // determine the modality type if it hasn't been set prior
            // to setting WA_ShowModal. set the default to WindowModal
            // if we are the child of a group leader; otherwise use
            // ApplicationModal.
            QWidget *w = parentWidget();
            if (w)
                w = w->window();
            while (w && !w->testAttribute(Qt::WA_GroupLeader)) {
                w = w->parentWidget();
                if (w)
                    w = w->window();
            }
            data->window_modality = (w && w->testAttribute(Qt::WA_GroupLeader))
                                    ? Qt::WindowModal
                                    : Qt::ApplicationModal;
            // Some window managers does not allow us to enter modal after the
            // window is showing. Therefore, to be consistent, we cannot call
            // QApplicationPrivate::enterModal(this) here. The window must be
            // hidden before changing modality.
        }
        break;
    }
    case Qt::WA_MouseTracking: {
        QEvent e(QEvent::MouseTrackingChange);
        QApplication::sendEvent(this, &e);
        break;
    }
    case Qt::WA_PaintOnScreen: {
        d->updateIsOpaque();
#if defined(Q_WS_X11)
        // Recreate the widget if it's already created as an alien widget and
        // WA_PaintOnScreen is enabled. Paint on screen widgets must have win id.
        // So must their children.
        if (on) {
            setAttribute(Qt::WA_NativeWindow);
            d->enforceNativeChildren();
        }
#endif
        // fall through
    }
    case Qt::WA_OpaquePaintEvent: {
        d->updateIsOpaque();
        break;
    }
    case Qt::WA_NoSystemBackground:
        d->updateIsOpaque();
        // fall through...
    case Qt::WA_UpdatesDisabled: {
        d->updateSystemBackground();
        break;
    }
    case Qt::WA_WindowPropagation: {
        d->resolvePalette();
        d->resolveFont();
        d->resolveLocale();
        break;
    }
#ifdef Q_WS_X11
    case Qt::WA_NoX11EventCompression: {
        if (!d->extra)
            d->createExtra();
        d->extra->compress_events = on;
        break;
    }
    case Qt::WA_X11DoNotAcceptFocus: {
        if (testAttribute(Qt::WA_WState_Created))
            d->updateX11AcceptFocus();
        break;
    }
#endif
    case Qt::WA_DontShowOnScreen: {
        if (on && isVisible()) {
            // Make sure we keep the current state and only hide the widget
            // from the desktop. show_sys will only update platform specific
            // attributes at this point.
            d->hide_sys();
            d->show_sys();
        }
        break;
    }

#ifdef Q_WS_X11
    case Qt::WA_X11NetWmWindowTypeDesktop:
    case Qt::WA_X11NetWmWindowTypeDock:
    case Qt::WA_X11NetWmWindowTypeToolBar:
    case Qt::WA_X11NetWmWindowTypeMenu:
    case Qt::WA_X11NetWmWindowTypeUtility:
    case Qt::WA_X11NetWmWindowTypeSplash:
    case Qt::WA_X11NetWmWindowTypeDialog:
    case Qt::WA_X11NetWmWindowTypeDropDownMenu:
    case Qt::WA_X11NetWmWindowTypePopupMenu:
    case Qt::WA_X11NetWmWindowTypeToolTip:
    case Qt::WA_X11NetWmWindowTypeNotification:
    case Qt::WA_X11NetWmWindowTypeCombo:
    case Qt::WA_X11NetWmWindowTypeDND: {
        if (testAttribute(Qt::WA_WState_Created))
            d->setNetWmWindowTypes();
        break;
    }
#endif

    case Qt::WA_TranslucentBackground: {
        if (on) {
            setAttribute(Qt::WA_NoSystemBackground);
            d->updateIsTranslucent();
        }
        break;
    }
    default:
        break;
    }
}

/*!
  Returns true if attribute \a attribute is set on this widget;
  otherwise returns false.

  \sa setAttribute()
*/
bool QWidget::testAttribute(Qt::WidgetAttribute attribute) const
{
    Q_D(const QWidget);
    return d->widget_attributes.test(attribute);
}

/*!
  \property QWidget::windowOpacity

  \brief The level of opacity for the window.

  The valid range of opacity is from 1.0 (completely opaque) to
  0.0 (completely transparent).

  By default the value of this property is 1.0.

  This feature is available on Embedded Linux, Mac OS X, Windows,
  and X11 platforms that support the Composite extension.

  This feature is not available on Windows CE.

  Note that under X11 you need to have a composite manager running,
  and the X11 specific _NET_WM_WINDOW_OPACITY atom needs to be
  supported by the window manager you are using.

  \warning Changing this property from opaque to transparent might issue a
  paint event that needs to be processed before the window is displayed
  correctly. This affects mainly the use of QPixmap::grabWindow(). Also note
  that semi-transparent windows update and resize significantly slower than
  opaque windows.

  \sa setMask()
*/
qreal QWidget::windowOpacity() const
{
    Q_D(const QWidget);
    return (isWindow() && d->maybeTopData()) ? d->maybeTopData()->opacity / qreal(255.) : qreal(1.0);
}

void QWidget::setWindowOpacity(qreal opacity)
{
    Q_D(QWidget);
    if (!isWindow())
        return;

    opacity = qBound(qreal(0.0), opacity, qreal(1.0));
    QTLWExtra *extra = d->topData();
    extra->opacity = uint(opacity * 255);

    if (!testAttribute(Qt::WA_WState_Created))
        return;

#ifndef QT_NO_GRAPHICSVIEW
    if (QGraphicsProxyWidget *proxy = graphicsProxyWidget()) {
        // Avoid invalidating the cache if set.
        if (proxy->cacheMode() == QGraphicsItem::NoCache)
            proxy->update();
        else if (QGraphicsScene *scene = proxy->scene())
            scene->update(proxy->sceneBoundingRect());
        return;
    }
#endif

    d->setWindowOpacity_sys(opacity);
}

/*!
    \property QWidget::windowModified
    \brief whether the document shown in the window has unsaved changes

    A modified window is a window whose content has changed but has
    not been saved to disk. This flag will have different effects
    varied by the platform. On Mac OS X the close button will have a
    modified look; on other platforms, the window title will have an
    '*' (asterisk).

    The window title must contain a "[*]" placeholder, which
    indicates where the '*' should appear. Normally, it should appear
    right after the file name (e.g., "document1.txt[*] - Text
    Editor"). If the window isn't modified, the placeholder is simply
    removed.

    Note that if a widget is set as modified, all its ancestors will
    also be set as modified. However, if you call \c
    {setWindowModified(false)} on a widget, this will not propagate to
    its parent because other children of the parent might have been
    modified.

    \sa windowTitle, {Application Example}, {SDI Example}, {MDI Example}
*/
bool QWidget::isWindowModified() const
{
    return testAttribute(Qt::WA_WindowModified);
}

void QWidget::setWindowModified(bool mod)
{
    Q_D(QWidget);
    setAttribute(Qt::WA_WindowModified, mod);

    if (Q_UNLIKELY(!windowTitle().contains(QLatin1String("[*]")) && mod))
        qWarning("QWidget::setWindowModified: The window title does not contain a '[*]' placeholder");

    d->setWindowTitle_helper(windowTitle());
    d->setWindowIconText_helper(windowIconText());

    QEvent e(QEvent::ModifiedChange);
    QApplication::sendEvent(this, &e);
}

#ifndef QT_NO_TOOLTIP
/*!
  \property QWidget::toolTip

  \brief the widget's tooltip

  Note that by default tooltips are only shown for widgets that are
  children of the active window. You can change this behavior by
  setting the attribute Qt::WA_AlwaysShowToolTips on the \e window,
  not on the widget with the tooltip.

  If you want to control a tooltip's behavior, you can intercept the
  event() function and catch the QEvent::ToolTip event (e.g., if you
  want to customize the area for which the tooltip should be shown).

  By default, this property contains an empty string.

  \sa QToolTip statusTip whatsThis
*/
void QWidget::setToolTip(const QString &s)
{
    Q_D(QWidget);
    d->toolTip = s;

    QEvent event(QEvent::ToolTipChange);
    QApplication::sendEvent(this, &event);
}

QString QWidget::toolTip() const
{
    Q_D(const QWidget);
    return d->toolTip;
}
#endif // QT_NO_TOOLTIP


#ifndef QT_NO_STATUSTIP
/*!
  \property QWidget::statusTip
  \brief the widget's status tip

  By default, this property contains an empty string.

  \sa toolTip whatsThis
*/
void QWidget::setStatusTip(const QString &s)
{
    Q_D(QWidget);
    d->statusTip = s;
}

QString QWidget::statusTip() const
{
    Q_D(const QWidget);
    return d->statusTip;
}
#endif // QT_NO_STATUSTIP

#ifndef QT_NO_WHATSTHIS
/*!
  \property QWidget::whatsThis

  \brief the widget's What's This help text.

  By default, this property contains an empty string.

  \sa QWhatsThis QWidget::toolTip QWidget::statusTip
*/
void QWidget::setWhatsThis(const QString &s)
{
    Q_D(QWidget);
    d->whatsThis = s;
}

QString QWidget::whatsThis() const
{
    Q_D(const QWidget);
    return d->whatsThis;
}
#endif // QT_NO_WHATSTHIS

#ifndef QT_NO_SHORTCUT
/*!
    Adds a shortcut to Qt's shortcut system that watches for the given
    \a key sequence in the given \a context. If the \a context is
    Qt::ApplicationShortcut, the shortcut applies to the application as a
    whole. Otherwise, it is either local to this widget, Qt::WidgetShortcut,
    or to the window itself, Qt::WindowShortcut.

    If the same \a key sequence has been grabbed by several widgets,
    when the \a key sequence occurs a QEvent::Shortcut event is sent
    to all the widgets to which it applies in a non-deterministic
    order, but with the ``ambiguous'' flag set to true.

    \warning You should not normally need to use this function;
    instead create \l{QAction}s with the shortcut key sequences you
    require (if you also want equivalent menu options and toolbar
    buttons), or create \l{QShortcut}s if you just need key sequences.
    Both QAction and QShortcut handle all the event filtering for you,
    and provide signals which are triggered when the user triggers the
    key sequence, so are much easier to use than this low-level
    function.

    \sa releaseShortcut() setShortcutEnabled()
*/
int QWidget::grabShortcut(const QKeySequence &key, Qt::ShortcutContext context)
{
    Q_ASSERT(qApp);
    if (key.isEmpty())
        return 0;
    setAttribute(Qt::WA_GrabbedShortcut);
    return qApp->d_func()->shortcutMap.addShortcut(this, key, context);
}

/*!
    Removes the shortcut with the given \a id from Qt's shortcut
    system. The widget will no longer receive QEvent::Shortcut events
    for the shortcut's key sequence (unless it has other shortcuts
    with the same key sequence).

    \warning You should not normally need to use this function since
    Qt's shortcut system removes shortcuts automatically when their
    parent widget is destroyed. It is best to use QAction or
    QShortcut to handle shortcuts, since they are easier to use than
    this low-level function. Note also that this is an expensive
    operation.

    \sa grabShortcut() setShortcutEnabled()
*/
void QWidget::releaseShortcut(int id)
{
    Q_ASSERT(qApp);
    if (id)
        qApp->d_func()->shortcutMap.removeShortcut(id, this);
}

/*!
    If \a enable is true, the shortcut with the given \a id is
    enabled; otherwise the shortcut is disabled.

    \warning You should not normally need to use this function since
    Qt's shortcut system enables/disables shortcuts automatically as
    widgets become hidden/visible and gain or lose focus. It is best
    to use QAction or QShortcut to handle shortcuts, since they are
    easier to use than this low-level function.

    \sa grabShortcut() releaseShortcut()
*/
void QWidget::setShortcutEnabled(int id, bool enable)
{
    Q_ASSERT(qApp);
    if (id)
        qApp->d_func()->shortcutMap.setShortcutEnabled(enable, id, this);
}

/*!
    \since 4.2

    If \a enable is true, auto repeat of the shortcut with the
    given \a id is enabled; otherwise it is disabled.

    \sa grabShortcut() releaseShortcut()
*/
void QWidget::setShortcutAutoRepeat(int id, bool enable)
{
    Q_ASSERT(qApp);
    if (id)
        qApp->d_func()->shortcutMap.setShortcutAutoRepeat(enable, id, this);
}
#endif // QT_NO_SHORTCUT

/*!
    Returns the window system handle of the widget, for low-level
    access. Using this function is not portable.

    The HANDLE type varies with platform; see \c qwindowdefs.h for
    details.
*/
Qt::HANDLE QWidget::handle() const
{
    Q_D(const QWidget);
    if (!internalWinId() && testAttribute(Qt::WA_WState_Created))
        (void)winId(); // enforce native window
    return d->hd;
}


/*!
    Raises this widget to the top of the parent widget's stack.

    After this call the widget will be visually in front of any
    overlapping sibling widgets.

    \note When using activateWindow(), you can call this function to
    ensure that the window is stacked on top.

    \sa lower(), stackUnder()
*/

void QWidget::raise()
{
    Q_D(QWidget);
    if (!isWindow()) {
        QWidget *p = parentWidget();
        const int parentChildCount = p->d_func()->children.size();
        if (parentChildCount < 2)
            return;
        const int from = p->d_func()->children.indexOf(this);
        Q_ASSERT(from >= 0);
        // Do nothing if the widget is already in correct stacking order _and_ created.
        if (from != parentChildCount -1)
            p->d_func()->children.move(from, parentChildCount - 1);
        if (!testAttribute(Qt::WA_WState_Created) && p->testAttribute(Qt::WA_WState_Created))
            create();
        else if (from == parentChildCount - 1)
            return;

        QRegion region(rect());
        d->subtractOpaqueSiblings(region);
        d->invalidateBuffer(region);
    }
    if (testAttribute(Qt::WA_WState_Created))
        d->raise_sys();

    QEvent e(QEvent::ZOrderChange);
    QApplication::sendEvent(this, &e);
}

/*!
    Lowers the widget to the bottom of the parent widget's stack.

    After this call the widget will be visually behind (and therefore
    obscured by) any overlapping sibling widgets.

    \sa raise(), stackUnder()
*/

void QWidget::lower()
{
    Q_D(QWidget);
    if (!isWindow()) {
        QWidget *p = parentWidget();
        const int parentChildCount = p->d_func()->children.size();
        if (parentChildCount < 2)
            return;
        const int from = p->d_func()->children.indexOf(this);
        Q_ASSERT(from >= 0);
        // Do nothing if the widget is already in correct stacking order _and_ created.
        if (from != 0)
            p->d_func()->children.move(from, 0);
        if (!testAttribute(Qt::WA_WState_Created) && p->testAttribute(Qt::WA_WState_Created))
            create();
        else if (from == 0)
            return;
    }
    if (testAttribute(Qt::WA_WState_Created))
        d->lower_sys();

    QEvent e(QEvent::ZOrderChange);
    QApplication::sendEvent(this, &e);
}


/*!
    Places the widget under \a w in the parent widget's stack.

    To make this work, the widget itself and \a w must be siblings.

    \sa raise(), lower()
*/
void QWidget::stackUnder(QWidget* w)
{
    Q_D(QWidget);
    QWidget *p = parentWidget();
    if (!w || isWindow() || p != w->parentWidget() || this == w)
        return;
    if (p) {
        int from = p->d_func()->children.indexOf(this);
        int to = p->d_func()->children.indexOf(w);
        Q_ASSERT(from >= 0);
        Q_ASSERT(to >= 0);
        if (from < to)
            --to;
        // Do nothing if the widget is already in correct stacking order _and_ created.
        if (from != to)
            p->d_func()->children.move(from, to);
        if (!testAttribute(Qt::WA_WState_Created) && p->testAttribute(Qt::WA_WState_Created))
            create();
        else if (from == to)
            return;
    }
    if (testAttribute(Qt::WA_WState_Created))
        d->stackUnder_sys(w);

    QEvent e(QEvent::ZOrderChange);
    QApplication::sendEvent(this, &e);
}

void QWidget::styleChange(QStyle&) { }
void QWidget::fontChange(const QFont &) { }  // compat

/*!
    \fn bool QWidget::isVisibleToTLW() const

    Use isVisible() instead.
*/

/*!
    \fn void QWidget::iconify()

    Use showMinimized() instead.
*/

/*!
    \fn void QWidget::constPolish() const

    Use ensurePolished() instead.
*/

/*!
    \fn void QWidget::reparent(QWidget *parent, Qt::WindowFlags f, const QPoint &p, bool showIt)

    Use setParent() to change the parent or the widget's widget flags;
    use move() to move the widget, and use show() to show the widget.
*/

/*!
    \fn void QWidget::reparent(QWidget *parent, const QPoint &p, bool showIt)

    Use setParent() to change the parent; use move() to move the
    widget, and use show() to show the widget.
*/

/*!
    \fn void QWidget::recreate(QWidget *parent, Qt::WindowFlags f, const QPoint & p, bool showIt)

    Use setParent() to change the parent or the widget's widget flags;
    use move() to move the widget, and use show() to show the widget.
*/

/*!
    \fn bool QWidget::hasMouse() const

    Use testAttribute(Qt::WA_UnderMouse) instead.
*/

/*!
    \fn bool QWidget::ownCursor() const

    Use testAttribute(Qt::WA_SetCursor) instead.
*/

/*!
    \fn bool QWidget::ownFont() const

    Use testAttribute(Qt::WA_SetFont) instead.
*/

/*!
    \fn void QWidget::unsetFont()

    Use setFont(QFont()) instead.
*/

/*!
    \fn bool QWidget::ownPalette() const

    Use testAttribute(Qt::WA_SetPalette) instead.
*/

/*!
    \fn void QWidget::unsetPalette()

    Use setPalette(QPalette()) instead.
*/

/*!
    \fn void QWidget::setEraseColor(const QColor &color)

    Use the palette instead.

    \oldcode
    widget->setEraseColor(color);
    \newcode
    QPalette palette;
    palette.setColor(widget->backgroundRole(), color);
    widget->setPalette(palette);
    \endcode
*/

/*!
    \fn void QWidget::setErasePixmap(const QPixmap &pixmap)

    Use the palette instead.

    \oldcode
    widget->setErasePixmap(pixmap);
    \newcode
    QPalette palette;
    palette.setBrush(widget->backgroundRole(), QBrush(pixmap));
    widget->setPalette(palette);
    \endcode
*/

/*!
    \fn void QWidget::setPaletteForegroundColor(const QColor &color)

    Use the palette directly.

    \oldcode
    widget->setPaletteForegroundColor(color);
    \newcode
    QPalette palette;
    palette.setColor(widget->foregroundRole(), color);
    widget->setPalette(palette);
    \endcode
*/

/*!
    \fn void QWidget::setPaletteBackgroundColor(const QColor &color)

    Use the palette directly.

    \oldcode
    widget->setPaletteBackgroundColor(color);
    \newcode
    QPalette palette;
    palette.setColor(widget->backgroundRole(), color);
    widget->setPalette(palette);
    \endcode
*/

/*!
    \fn void QWidget::setPaletteBackgroundPixmap(const QPixmap &pixmap)

    Use the palette directly.

    \oldcode
    widget->setPaletteBackgroundPixmap(pixmap);
    \newcode
    QPalette palette;
    palette.setBrush(widget->backgroundRole(), QBrush(pixmap));
    widget->setPalette(palette);
    \endcode
*/

/*!
    \fn void QWidget::setBackgroundPixmap(const QPixmap &pixmap)

    Use the palette instead.

    \oldcode
    widget->setBackgroundPixmap(pixmap);
    \newcode
    QPalette palette;
    palette.setBrush(widget->backgroundRole(), QBrush(pixmap));
    widget->setPalette(palette);
    \endcode
*/

/*!
    \fn void QWidget::setBackgroundColor(const QColor &color)

    Use the palette instead.

    \oldcode
    widget->setBackgroundColor(color);
    \newcode
    QPalette palette;
    palette.setColor(widget->backgroundRole(), color);
    widget->setPalette(palette);
    \endcode
*/

/*!
    \fn QWidget *QWidget::parentWidget(bool sameWindow) const

    Use the no-argument overload instead.
*/

/*!
    \fn void QWidget::setFont(const QFont &f, bool b)

    Use the single-argument overload instead.
*/

/*!
    \fn void QWidget::setPalette(const QPalette &p, bool b)

    Use the single-argument overload instead.
*/

/*!
    \fn void QWidget::repaint(bool b)

    The boolean parameter \a b is ignored. Use the no-argument overload instead.
*/

/*!
    \fn void QWidget::repaint(int x, int y, int w, int h, bool b)

    The boolean parameter \a b is ignored. Use the four-argument overload instead.
*/

/*!
    \fn void QWidget::repaint(const QRect &r, bool b)

    The boolean parameter \a b is ignored. Use the single rect-argument overload instead.
*/

/*!
    \fn void QWidget::repaint(const QRegion &rgn, bool b)

    The boolean parameter \a b is ignored. Use the single region-argument overload instead.
*/

/*!
    \fn void QWidget::erase()

    Drawing may only take place in a QPaintEvent. Overload
    paintEvent() to do your erasing and call update() to schedule a
    replaint whenever necessary. See also QPainter.
*/

/*!
    \fn void QWidget::erase(int x, int y, int w, int h)

    Drawing may only take place in a QPaintEvent. Overload
    paintEvent() to do your erasing and call update() to schedule a
    replaint whenever necessary. See also QPainter.
*/

/*!
    \fn void QWidget::erase(const QRect &rect)

    Drawing may only take place in a QPaintEvent. Overload
    paintEvent() to do your erasing and call update() to schedule a
    replaint whenever necessary. See also QPainter.
*/

/*!
    \fn void QWidget::drawText(const QPoint &p, const QString &s)

    Drawing may only take place in a QPaintEvent. Overload
    paintEvent() to do your drawing and call update() to schedule a
    replaint whenever necessary. See also QPainter.
*/

/*!
    \fn void QWidget::drawText(int x, int y, const QString &s)

    Drawing may only take place in a QPaintEvent. Overload
    paintEvent() to do your drawing and call update() to schedule a
    replaint whenever necessary. See also QPainter.
*/

/*!
    \fn QWidget *QWidget::childAt(const QPoint &p, bool includeThis) const

    Use the single point argument overload instead.
*/

/*!
    \fn void QWidget::setCaption(const QString &c)

    Use setWindowTitle() instead.
*/

/*!
    \fn void QWidget::setIcon(const QPixmap &i)

    Use setWindowIcon() instead.
*/

/*!
    \fn void QWidget::setIconText(const QString &it)

    Use setWindowIconText() instead.
*/

/*!
    \fn QString QWidget::caption() const

    Use windowTitle() instead.
*/

/*!
    \fn QString QWidget::iconText() const

    Use windowIconText() instead.
*/

/*!
    \fn bool QWidget::isTopLevel() const
    \obsolete

    Use isWindow() instead.
*/

/*!
    \fn bool QWidget::isRightToLeft() const
    \internal
*/

/*!
    \fn bool QWidget::isLeftToRight() const
    \internal
*/

/*!
    \fn void QWidget::setActiveWindow()

    Use activateWindow() instead.
*/

/*!
    \fn bool QWidget::isShown() const

    Use !isHidden() instead (notice the exclamation mark), or use isVisible() to check whether the widget is visible.
*/

/*!
    \fn bool QWidget::isDialog() const

    Use windowType() == Qt::Dialog instead.
*/

/*!
    \fn bool QWidget::isPopup() const

    Use windowType() == Qt::Popup instead.
*/

/*!
    \fn bool QWidget::isDesktop() const

    Use windowType() == Qt::Desktop instead.
*/

/*!
    \fn void QWidget::polish()

    Use ensurePolished() instead.
*/

/*!
    \fn QWidget *QWidget::childAt(int x, int y, bool includeThis) const

    Use the childAt() overload that doesn't have an \a includeThis parameter.

    \oldcode
        return widget->childAt(x, y, true);
    \newcode
        QWidget *child = widget->childAt(x, y, true);
        if (child)
            return child;
        if (widget->rect().contains(x, y))
            return widget;
    \endcode
*/

/*!
    \fn void QWidget::setSizePolicy(QSizePolicy::Policy hor, QSizePolicy::Policy ver, bool hfw)
    \compat

    Use the \l sizePolicy property and heightForWidth() function instead.
*/

/*!
    \fn bool QWidget::isUpdatesEnabled() const
    \compat

    Use the \l updatesEnabled property instead.
*/

/*!
     \macro QWIDGETSIZE_MAX
     \relates QWidget

     Defines the maximum size for a QWidget object.

     The largest allowed size for a widget is QSize(QWIDGETSIZE_MAX,
     QWIDGETSIZE_MAX), i.e. QSize (16777215,16777215).

     \sa QWidget::setMaximumSize()
*/

/*!
    \fn QWidget::setupUi(QWidget *widget)

    Sets up the user interface for the specified \a widget.

    \note This function is available with widgets that derive from user
    interface descriptions created using \l{uic}.

    \sa {Using a Designer UI File in Your Application}
*/

QRect QWidgetPrivate::frameStrut() const
{
    Q_Q(const QWidget);
    if (!q->isWindow() || (q->windowType() == Qt::Desktop) || q->testAttribute(Qt::WA_DontShowOnScreen)) {
        // x2 = x1 + w - 1, so w/h = 1
        return QRect(0, 0, 1, 1);
    }

    if (data.fstrut_dirty
        // ### Fix properly for 4.3
        && q->isVisible()
        && q->testAttribute(Qt::WA_WState_Created))
        const_cast<QWidgetPrivate *>(this)->updateFrameStrut();

    return maybeTopData() ? maybeTopData()->frameStrut : QRect();
}


/*!
    \preliminary
    \since 4.2
    \obsolete

    Sets the window surface to be the \a surface specified.
    The QWidget takes will ownership of the \a surface.
    widget itself is deleted.
*/
void QWidget::setWindowSurface(QWindowSurface *surface)
{
    // ### createWinId() ??

    if (!isTopLevel())
        return;

    Q_D(QWidget);

    QTLWExtra *topData = d->topData();
    if (topData->windowSurface == surface)
        return;

    QWindowSurface *oldSurface = topData->windowSurface;
    delete topData->windowSurface;
    topData->windowSurface = surface;

    QWidgetBackingStore *bs = d->maybeBackingStore();
    if (!bs)
        return;

    if (isTopLevel()) {
        if (bs->windowSurface != oldSurface && bs->windowSurface != surface)
            delete bs->windowSurface;
        bs->windowSurface = surface;
    }
}

/*!
    \preliminary
    \since 4.2

    Returns the QWindowSurface this widget will be drawn into.
*/
QWindowSurface *QWidget::windowSurface() const
{
    Q_D(const QWidget);
    QTLWExtra *extra = d->maybeTopData();
    if (extra && extra->windowSurface)
        return extra->windowSurface;

    QWidgetBackingStore *bs = d->maybeBackingStore();

    return bs ? bs->windowSurface : 0;
}

void QWidgetPrivate::getLayoutItemMargins(int *left, int *top, int *right, int *bottom) const
{
    if (left)
        *left = leftLayoutItemMargin;
    if (top)
        *top = topLayoutItemMargin;
    if (right)
        *right = rightLayoutItemMargin;
    if (bottom)
        *bottom = bottomLayoutItemMargin;
}

void QWidgetPrivate::setLayoutItemMargins(int left, int top, int right, int bottom)
{
    if (leftLayoutItemMargin == left
        && topLayoutItemMargin == top
        && rightLayoutItemMargin == right
        && bottomLayoutItemMargin == bottom)
        return;

    Q_Q(QWidget);
    leftLayoutItemMargin = left;
    topLayoutItemMargin = top;
    rightLayoutItemMargin = right;
    bottomLayoutItemMargin = bottom;
    q->updateGeometry();
}

void QWidgetPrivate::setLayoutItemMargins(QStyle::SubElement element, const QStyleOption *opt)
{
    Q_Q(QWidget);
    QStyleOption myOpt;
    if (!opt) {
        myOpt.initFrom(q);
        myOpt.rect.setRect(0, 0, 32768, 32768);     // arbitrary
        opt = &myOpt;
    }

    QRect liRect = q->style()->subElementRect(element, opt, q);
    if (liRect.isValid()) {
        leftLayoutItemMargin = (opt->rect.left() - liRect.left());
        topLayoutItemMargin = (opt->rect.top() - liRect.top());
        rightLayoutItemMargin = (liRect.right() - opt->rect.right());
        bottomLayoutItemMargin = (liRect.bottom() - opt->rect.bottom());
    } else {
        leftLayoutItemMargin = 0;
        topLayoutItemMargin = 0;
        rightLayoutItemMargin = 0;
        bottomLayoutItemMargin = 0;
    }
}
// resets the Qt::WA_QuitOnClose attribute to the default value for transient widgets.
void QWidgetPrivate::adjustQuitOnCloseAttribute()
{
    Q_Q(QWidget);

    if (!q->parentWidget()) {
        Qt::WindowType type = q->windowType();
        if (type == Qt::Widget || type == Qt::SubWindow)
            type = Qt::Window;
        if (type != Qt::Widget && type != Qt::Window && type != Qt::Dialog)
            q->setAttribute(Qt::WA_QuitOnClose, false);
    }
}



Q_GUI_EXPORT QWidgetData *qt_qwidget_data(QWidget *widget)
{
    return widget->data;
}

Q_GUI_EXPORT QWidgetPrivate *qt_widget_private(QWidget *widget)
{
    return widget->d_func();
}


#ifndef QT_NO_GRAPHICSVIEW
/*!
   \since 4.5

   Returns the proxy widget for the corresponding embedded widget in a graphics
   view; otherwise returns 0.

   \sa QGraphicsProxyWidget::createProxyForChildWidget(),
       QGraphicsScene::addWidget()
 */
QGraphicsProxyWidget *QWidget::graphicsProxyWidget() const
{
    Q_D(const QWidget);
    if (d->extra) {
        return d->extra->proxyWidget;
    }
    return 0;
}
#endif


/*!
    \typedef QWidgetList
    \relates QWidget

    Synonym for QList<QWidget *>.
*/

/*!
    \typedef WId
    \relates QWidget

    Platform dependent window identifier.
*/

/*!
    \fn void QWidget::destroy(bool destroyWindow, bool destroySubWindows)

    Frees up window system resources. Destroys the widget window if \a
    destroyWindow is true.

    destroy() calls itself recursively for all the child widgets,
    passing \a destroySubWindows for the \a destroyWindow parameter.
    To have more control over destruction of subwidgets, destroy
    subwidgets selectively first.

    This function is usually called from the QWidget destructor.
*/

/*!
    \fn QPaintEngine *QWidget::paintEngine() const

    Returns the widget's paint engine.

    Note that this function should not be called explicitly by the
    user, since it's meant for reimplementation purposes only. The
    function is called by Qt internally, and the default
    implementation may not always return a valid pointer.
*/

/*!
    \fn QPoint QWidget::mapToGlobal(const QPoint &pos) const

    Translates the widget coordinate \a pos to global screen
    coordinates. For example, \c{mapToGlobal(QPoint(0,0))} would give
    the global coordinates of the top-left pixel of the widget.

    \sa mapFromGlobal() mapTo() mapToParent()
*/

/*!
    \fn QPoint QWidget::mapFromGlobal(const QPoint &pos) const

    Translates the global screen coordinate \a pos to widget
    coordinates.

    \sa mapToGlobal() mapFrom() mapFromParent()
*/

/*!
    \fn void QWidget::grabMouse()

    Grabs the mouse input.

    This widget receives all mouse events until releaseMouse() is
    called; other widgets get no mouse events at all. Keyboard
    events are not affected. Use grabKeyboard() if you want to grab
    that.

    \warning Bugs in mouse-grabbing applications very often lock the
    terminal. Use this function with extreme caution, and consider
    using the \c -nograb command line option while debugging.

    It is almost never necessary to grab the mouse when using Qt, as
    Qt grabs and releases it sensibly. In particular, Qt grabs the
    mouse when a mouse button is pressed and keeps it until the last
    button is released.

    \note Only visible widgets can grab mouse input. If isVisible()
    returns false for a widget, that widget cannot call grabMouse().

    \sa releaseMouse() grabKeyboard() releaseKeyboard()
*/

/*!
    \fn void QWidget::grabMouse(const QCursor &cursor)
    \overload grabMouse()

    Grabs the mouse input and changes the cursor shape.

    The cursor will assume shape \a cursor (for as long as the mouse
    focus is grabbed) and this widget will be the only one to receive
    mouse events until releaseMouse() is called().

    \warning Grabbing the mouse might lock the terminal.

    \sa releaseMouse(), grabKeyboard(), releaseKeyboard(), setCursor()
*/

/*!
    \fn void QWidget::releaseMouse()

    Releases the mouse grab.

    \sa grabMouse(), grabKeyboard(), releaseKeyboard()
*/

/*!
    \fn void QWidget::grabKeyboard()

    Grabs the keyboard input.

    This widget receives all keyboard events until releaseKeyboard()
    is called; other widgets get no keyboard events at all. Mouse
    events are not affected. Use grabMouse() if you want to grab that.

    The focus widget is not affected, except that it doesn't receive
    any keyboard events. setFocus() moves the focus as usual, but the
    new focus widget receives keyboard events only after
    releaseKeyboard() is called.

    If a different widget is currently grabbing keyboard input, that
    widget's grab is released first.

    \sa releaseKeyboard() grabMouse() releaseMouse() focusWidget()
*/

/*!
    \fn void QWidget::releaseKeyboard()

    Releases the keyboard grab.

    \sa grabKeyboard(), grabMouse(), releaseMouse()
*/

/*!
    \fn QWidget *QWidget::mouseGrabber()

    Returns the widget that is currently grabbing the mouse input.

    If no widget in this application is currently grabbing the mouse,
    0 is returned.

    \sa grabMouse(), keyboardGrabber()
*/

/*!
    \fn QWidget *QWidget::keyboardGrabber()

    Returns the widget that is currently grabbing the keyboard input.

    If no widget in this application is currently grabbing the
    keyboard, 0 is returned.

    \sa grabMouse(), mouseGrabber()
*/

/*!
    \fn void QWidget::activateWindow()

    Sets the top-level widget containing this widget to be the active
    window.

    An active window is a visible top-level window that has the
    keyboard input focus.

    This function performs the same operation as clicking the mouse on
    the title bar of a top-level window. On X11, the result depends on
    the Window Manager. If you want to ensure that the window is
    stacked on top as well you should also call raise(). Note that the
    window must be visible, otherwise activateWindow() has no effect.

    On Windows, if you are calling this when the application is not
    currently the active one then it will not make it the active
    window.  It will change the color of the taskbar entry to indicate
    that the window has changed in some way. This is because Microsoft
    does not allow an application to interrupt what the user is currently
    doing in another application.

    \sa isActiveWindow(), window(), show()
*/

/*!
    \fn int QWidget::metric(PaintDeviceMetric m) const

    Internal implementation of the virtual QPaintDevice::metric()
    function.

    \a m is the metric to get.
*/

/*!
    \fn void QWidget::setMask(const QRegion &region)
    \overload

    Causes only the parts of the widget which overlap \a region to be
    visible. If the region includes pixels outside the rect() of the
    widget, window system controls in that area may or may not be
    visible, depending on the platform.

    Note that this effect can be slow if the region is particularly
    complex.

    \sa windowOpacity
*/
void QWidget::setMask(const QRegion &newMask)
{
    Q_D(QWidget);

    d->createExtra();
    if (newMask == d->extra->mask)
        return;

#ifndef QT_NO_BACKINGSTORE
    const QRegion oldMask(d->extra->mask);
#endif

    d->extra->mask = newMask;
    d->extra->hasMask = !newMask.isEmpty();

    if (!testAttribute(Qt::WA_WState_Created))
        return;

    d->setMask_sys(newMask);

#ifndef QT_NO_BACKINGSTORE
    if (!isVisible())
        return;

    if (!d->extra->hasMask) {
        // Mask was cleared; update newly exposed area.
        QRegion expose(rect());
        expose -= oldMask;
        if (!expose.isEmpty()) {
            d->setDirtyOpaqueRegion();
            update(expose);
        }
        return;
    }

    if (!isWindow()) {
        // Update newly exposed area on the parent widget.
        QRegion parentExpose(rect());
        parentExpose -= newMask;
        if (!parentExpose.isEmpty()) {
            d->setDirtyOpaqueRegion();
            parentExpose.translate(data->crect.topLeft());
            parentWidget()->update(parentExpose);
        }

        // Update newly exposed area on this widget
        if (!oldMask.isEmpty())
            update(newMask - oldMask);
    }
#endif
}

/*!
    \fn void QWidget::setMask(const QBitmap &bitmap)

    Causes only the pixels of the widget for which \a bitmap has a
    corresponding 1 bit to be visible. If the region includes pixels
    outside the rect() of the widget, window system controls in that
    area may or may not be visible, depending on the platform.

    Note that this effect can be slow if the region is particularly
    complex.

    The following code shows how an image with an alpha channel can be
    used to generate a mask for a widget:

    \snippet doc/src/snippets/widget-mask/main.cpp 0

    The label shown by this code is masked using the image it contains,
    giving the appearance that an irregularly-shaped image is being drawn
    directly onto the screen.

    Masked widgets receive mouse events only on their visible
    portions.

    \sa clearMask(), windowOpacity(), {Shaped Clock Example}
*/
void QWidget::setMask(const QBitmap &bitmap)
{
    setMask(QRegion(bitmap));
}

/*!
    \fn void QWidget::clearMask()

    Removes any mask set by setMask().

    \sa setMask()
*/
void QWidget::clearMask()
{
    setMask(QRegion());
}

/*! \fn const QX11Info &QWidget::x11Info() const
    Returns information about the configuration of the X display used to display
    the widget.

    \warning This function is only available on X11.
*/

/*! \fn Qt::HANDLE QWidget::x11PictureHandle() const
    Returns the X11 Picture handle of the widget for XRender
    support. Use of this function is not portable. This function will
    return 0 if XRender support is not compiled into Qt, if the
    XRender extension is not supported on the X11 display, or if the
    handle could not be created.
*/

QT_END_NAMESPACE

#include "moc_qwidget.h"


