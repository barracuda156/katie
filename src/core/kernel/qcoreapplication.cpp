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

#include "qcoreapplication.h"
#include "qcoreapplication_p.h"

#include "qabstracteventdispatcher.h"
#include "qcoreevent.h"
#include "qeventloop.h"
#include "qdatastream.h"
#include "qdebug.h"
#include "qdir.h"
#include "qfile.h"
#include "qfileinfo.h"
#include "qhash.h"
#include "qtextcodec.h"
#include "qthread.h"
#include "qstandardpaths.h"
#include "qelapsedtimer.h"
#include "qscopedpointer.h"
#include "qlibraryinfo.h"
#include "qlocale_tools_p.h"
#include "qthread_p.h"
#include "qprocess_p.h"
#include "qfactoryloader_p.h"
#include "qeventdispatcher_unix_p.h"
#include "qstdcontainers_p.h"
#include "qcorecommon_p.h"

#include <stdlib.h>
#include <errno.h>

QT_BEGIN_NAMESPACE

class QMutexUnlocker
{
public:
    inline explicit QMutexUnlocker(QMutex *m) : mtx(m) { }
    inline ~QMutexUnlocker() { unlock(); }
    inline void unlock() { if (mtx) mtx->unlock(); mtx = 0; }

private:
    Q_DISABLE_COPY(QMutexUnlocker)

    QMutex *mtx;
};

bool QCoreApplicationPrivate::checkInstance(const char *function)
{
    if (Q_UNLIKELY(!QCoreApplication::self)) {
        qWarning("QCoreApplication::%s: Please instantiate the QApplication object first", function);
        return false;
    }
    return true;
}

typedef QStdVector<QtCleanUpFunction> QVFuncList;
Q_GLOBAL_STATIC(QVFuncList, qGlobalCleanupList)
static std::recursive_mutex qGlobalApplicationMutex;

void qAddPostRoutine(QtCleanUpFunction p)
{
    std::lock_guard<std::recursive_mutex> locker(qGlobalApplicationMutex);
    QVFuncList *list = qGlobalCleanupList();
    list->prepend(p);
}

void qRemovePostRoutine(QtCleanUpFunction p)
{
    std::lock_guard<std::recursive_mutex> locker(qGlobalApplicationMutex);
    QVFuncList *list = qGlobalCleanupList();
    list->removeAll(p);
}

void Q_CORE_EXPORT qt_call_post_routines()
{
    std::lock_guard<std::recursive_mutex> locker(qGlobalApplicationMutex);
    QVFuncList *list = qGlobalCleanupList();
    while (!list->isEmpty()) {
        QtCleanUpFunction vfunc = list->takeFirst();
        vfunc();
    }
}

Q_AUTOTEST_EXPORT uint qGlobalPostedEventsCount()
{
    QThreadData *currentThreadData = QThreadData::current();
    return currentThreadData->postEventList.size() - currentThreadData->postEventList.startOffset;
}

// app starting up if false
bool QCoreApplicationPrivate::is_app_running = false;
 // app closing down if true
bool QCoreApplicationPrivate::is_app_closing = false;

QCoreApplication *QCoreApplication::self = 0;
std::bitset<Qt::AA_AttributeCount> QCoreApplicationPrivate::attribs;
QCoreApplication::Type QCoreApplicationPrivate::app_type = QCoreApplication::Tty;

struct QCoreApplicationData
{
    QString orgName, orgDomain, application;
    QString applicationVersion;

#ifndef QT_NO_LIBRARY
    QStringList app_librarypaths;
    QStringList app_pluginpaths;
#endif
};

Q_GLOBAL_STATIC(QCoreApplicationData, coreappdata)

QCoreApplicationPrivate::QCoreApplicationPrivate(int &aargc, char **aargv)
    : QObjectPrivate(), argc(aargc), argv(aargv), eventFilter(0),
      in_exec(false)
{
    QCoreApplicationPrivate::is_app_closing = false;
}

QCoreApplicationPrivate::~QCoreApplicationPrivate()
{
    if (threadData) {
        // need to clear the state of the mainData, just in case a new QCoreApplication comes along.
        QMutexLocker locker(&threadData->postEventList.mutex);
        for (int i = 0; i < threadData->postEventList.size(); ++i) {
            const QPostEvent &pe = threadData->postEventList.at(i);
            if (pe.event) {
                --pe.receiver->d_func()->postedEvents;
                pe.event->posted = false;
                delete pe.event;
            }
        }
        threadData->postEventList.clear();
        threadData->postEventList.recursion = 0;
        threadData->quitNow = false;
    }
}

QAbstractEventDispatcher* QCoreApplicationPrivate::createEventDispatcher()
{
    Q_Q(QCoreApplication);
    return new QEventDispatcherUNIX(q);
}

#if !defined (QT_NO_DEBUG)
void QCoreApplicationPrivate::checkReceiverThread(QObject *receiver)
{
    QThread *currentThread = QThread::currentThread();
    QThread *thr = receiver->thread();
    Q_ASSERT_X(currentThread == thr || !thr,
               "QCoreApplication::sendEvent",
               QString::fromLatin1("Cannot send events to objects owned by a different thread. "
                                   "Current thread %1. Receiver '%2' (of type '%3') was created in thread %4")
               .arg(QString::number(quintptr(currentThread), 16))
               .arg(receiver->objectName())
               .arg(QString::fromLatin1(receiver->metaObject()->className()))
               .arg(QString::number(quintptr(thr), 16))
               .toLocal8Bit().data());
    Q_UNUSED(currentThread);
    Q_UNUSED(thr);
}
#endif

/*!
    \class QCoreApplication
    \brief The QCoreApplication class provides an event loop for console Qt
    applications.

    This class is used by non-GUI applications to provide their event
    loop. For non-GUI application that uses Qt, there should be exactly
    one QCoreApplication object. For GUI applications, see
    QApplication.

    QCoreApplication contains the main event loop, where all events
    from the operating system (e.g., timer and network events) and
    other sources are processed and dispatched. It also handles the
    application's initialization and finalization, as well as
    system-wide and application-wide settings.

    \section1 The Event Loop and Event Handling

    The event loop is started with a call to exec(). Long running
    operations can call processEvents() to keep the application
    responsive.

    In general, we recommend that you create a QCoreApplication or a
    QApplication object in your \c main() function as early as
    possible. exec() will not return until the event loop exits; e.g.,
    when quit() is called.

    Several static convenience functions are also provided. The
    QCoreApplication object is available from instance(). Events can
    be sent or posted using sendEvent(), postEvent(), and
    sendPostedEvents(). Pending events can be removed with
    removePostedEvents() or flushed with flush().

    The class provides a quit() slot and an aboutToQuit() signal.

    \section1 Application and Library Paths

    An application has an applicationDirPath() and an
    applicationFilePath(). Library paths (see QLibrary) can be retrieved
    with libraryPaths() and manipulated by setLibraryPaths(), addLibraryPath(),
    and removeLibraryPath().

    \section1 Internationalization and Translations

    Translation files can be added or removed
    using installTranslator() and removeTranslator(). Application
    strings can be translated using translate(). The QObject::tr()
    and QObject::trUtf8() functions are implemented in terms of
    translate().

    \section1 Accessing Command Line Arguments

    The command line arguments which are passed to QCoreApplication's
    constructor should be accessed using the arguments() function.
    Note that some arguments supplied by the user may have been
    processed and removed by QCoreApplication.

    In cases where command line arguments need to be obtained using the
    argv() function, you must convert them from the local string encoding
    using QString::fromLocal8Bit().

    \section1 Locale Settings

    On Unix/Linux Qt is configured to use the system locale settings by
    default. This can cause a conflict when using POSIX functions, for
    instance, when converting between data types such as floats and
    strings, since the notation may differ between locales. To get
    around this problem, call the POSIX function \c{setlocale(LC_NUMERIC,"C")}
    right after initializing QApplication or QCoreApplication to reset
    the locale that is used for number formatting to "C"-locale.

    \sa QApplication, QAbstractEventDispatcher, QEventLoop,
    {Semaphores Example}, {Wait Conditions Example}
*/

/*!
    \fn static QCoreApplication *QCoreApplication::instance()

    Returns a pointer to the application's QCoreApplication (or
    QApplication) instance.

    If no instance has been allocated, \c null is returned.
*/

/*!\internal
 */
QCoreApplication::QCoreApplication(QCoreApplicationPrivate &p)
    : QObject(p, 0)
{
    init();
}

/*!
    Flushes the platform specific event queues.

    If you are doing graphical changes inside a loop that does not
    return to the event loop on asynchronous window systems like X11
    or double buffered window systems like Mac OS X, and you want to
    visualize these changes immediately (e.g. Splash Screens), call
    this function.

    \sa sendPostedEvents()
*/
void QCoreApplication::flush()
{
    if (self && self->d_func()->threadData->eventDispatcher) {
        self->d_func()->threadData->eventDispatcher->flush();
    }
}

/*!
    Constructs a Qt kernel application. Kernel applications are
    applications without a graphical user interface. These type of
    applications are used at the console or as server processes.

    The \a argc and \a argv arguments are processed by the application,
    and made available in a more convenient form by the arguments()
    function.

    \warning The data referred to by \a argc and \a argv must stay valid
    for the entire lifetime of the QCoreApplication object. In addition,
    \a argc must be greater than zero and \a argv must contain at least
    one valid character string.
*/
QCoreApplication::QCoreApplication(int &argc, char **argv)
: QObject(*new QCoreApplicationPrivate(argc, argv))
{
    init();
}


// ### move to QCoreApplicationPrivate constructor?
void QCoreApplication::init()
{
    Q_D(QCoreApplication);

    Q_ASSERT_X(!self, "QCoreApplication", "there should be only one application object");
    QCoreApplication::self = this;

    // if there is no application instance then the locale-aware comparison
    // will use fallback method (see QString::localeAwareCompare_helper())
    qt_initLocale(qGetLang());

    // use the event dispatcher created by the app programmer (if any),
    // otherwise we create one
    if (!d->threadData->eventDispatcher)
        d->threadData->eventDispatcher = d->createEventDispatcher();
    Q_ASSERT(d->threadData->eventDispatcher);

    if (!d->threadData->eventDispatcher->parent())
        d->threadData->eventDispatcher->moveToThread(d->threadData->thread);

#if !defined(QT_NO_LIBRARY) && !defined(QT_NO_SETTINGS)
    // make sure that library and plugin paths are initialized
    if (coreappdata()->app_librarypaths.isEmpty()) {
        libraryPaths();
    }
    if (coreappdata()->app_pluginpaths.isEmpty()) {
        pluginPaths();
    }
#endif

#if !defined(QT_NO_PROCESS)
    // Make sure the process manager thread object is created in the main
    // thread.
    QProcessPrivate::initializeProcessManager();
#endif
}


/*!
    Destroys the QCoreApplication object.
*/
QCoreApplication::~QCoreApplication()
{
    qt_call_post_routines();

    self = 0;
    QCoreApplicationPrivate::is_app_closing = true;
    QCoreApplicationPrivate::is_app_running = false;

#ifndef QT_NO_LIBRARY
    coreappdata()->app_librarypaths.clear();
    coreappdata()->app_pluginpaths.clear();
#endif
}

/*!
    Returns the type of application (\l Tty or Gui). The type is set depending
    on the application instance - if it is QCoreApplication then type is Tty,
    if it is QApplication then it is Gui.
*/
QCoreApplication::Type QCoreApplication::type()
{
    return QCoreApplicationPrivate::app_type;
}

/*!
    Sets the attribute \a attribute if \a on is true;
    otherwise clears the attribute.

    One of the attributes that can be set with this method is
    Qt::AA_ImmediateWidgetCreation. It tells Qt to create toplevel
    windows immediately. Normally, resources for widgets are allocated
    on demand to improve efficiency and minimize resource usage.
    Therefore, if it is important to minimize resource consumption, do
    not set this attribute.

    \sa testAttribute()
*/
void QCoreApplication::setAttribute(Qt::ApplicationAttribute attribute, bool on)
{
    QCoreApplicationPrivate::attribs.set(attribute, on);
}

/*!
  Returns true if attribute \a attribute is set;
  otherwise returns false.

  \sa setAttribute()
 */
bool QCoreApplication::testAttribute(Qt::ApplicationAttribute attribute)
{
    return QCoreApplicationPrivate::attribs.test(attribute);
}


/*!
  \internal

  This function is here to make it possible for Qt extensions to
  hook into event notification without subclassing QApplication
*/
bool QCoreApplication::notifyInternal(QObject *receiver, QEvent *event)
{
    // Qt enforces the rule that events can only be sent to objects in
    // the current thread, so receiver->d_func()->threadData is
    // equivalent to QThreadData::current(), just without the function
    // call overhead.
    QObjectPrivate *d = receiver->d_func();
    QThreadData *threadData = d->threadData;
    ++threadData->loopLevel;

    bool returnValue = notify(receiver, event);

    --threadData->loopLevel;
    return returnValue;
}


/*!
  Sends \a event to \a receiver: \a {receiver}->event(\a event).
  Returns the value that is returned from the receiver's event
  handler. Note that this function is called for all events sent to
  any object in any thread.

  For certain types of events (e.g. mouse and key events),
  the event will be propagated to the receiver's parent and so on up to
  the top-level object if the receiver is not interested in the event
  (i.e., it returns false).

  There are five different ways that events can be processed;
  reimplementing this virtual function is just one of them. All five
  approaches are listed below:
  \list 1
  \i Reimplementing paintEvent(), mousePressEvent() and so
  on. This is the commonest, easiest and least powerful way.

  \i Reimplementing this function. This is very powerful, providing
  complete control; but only one subclass can be active at a time.

  \i Installing an event filter on QCoreApplication::instance(). Such
  an event filter is able to process all events for all widgets, so
  it's just as powerful as reimplementing notify(); furthermore, it's
  possible to have more than one application-global event filter.
  Global event filters even see mouse events for
  \l{QWidget::isEnabled()}{disabled widgets}. Note that application
  event filters are only called for objects that live in the main
  thread.

  \i Reimplementing QObject::event() (as QWidget does). If you do
  this you get Tab key presses, and you get to see the events before
  any widget-specific event filters.

  \i Installing an event filter on the object. Such an event filter gets all
  the events, including Tab and Shift+Tab key press events, as long as they
  do not change the focus widget.
  \endlist

  \sa QObject::event(), installEventFilter()
*/

bool QCoreApplication::notify(QObject *receiver, QEvent *event)
{
    Q_D(QCoreApplication);
    // no events are delivered after ~QCoreApplication() has started
    if (QCoreApplicationPrivate::is_app_closing)
        return true;

    if (Q_UNLIKELY(!receiver)) {                        // serious error
        qWarning("QCoreApplication::notify: Unexpected null receiver");
        return true;
    }

#ifndef QT_NO_DEBUG
    d->checkReceiverThread(receiver);
#endif

    return receiver->isWidgetType() ? false : d->notify_helper(receiver, event);
}

bool QCoreApplicationPrivate::sendThroughApplicationEventFilters(QObject *receiver, QEvent *event)
{
    if (receiver->d_func()->threadData == this->threadData) {
        // application event filters are only called for objects in the GUI thread
        for (int i = 0; i < eventFilters.size(); ++i) {
            QObject *obj = eventFilters.at(i);
            if (!obj)
                continue;
            if (Q_UNLIKELY(obj->d_func()->threadData != threadData)) {
                qWarning("QCoreApplication: Application event filter cannot be in a different thread.");
                continue;
            }
            if (obj->eventFilter(receiver, event))
                return true;
        }
    }
    return false;
}

bool QCoreApplicationPrivate::sendThroughObjectEventFilters(QObject *receiver, QEvent *event)
{
    Q_Q(QCoreApplication);
    if (receiver != q) {
        for (int i = 0; i < receiver->d_func()->eventFilters.size(); ++i) {
            QObject *obj = receiver->d_func()->eventFilters.at(i);
            if (!obj)
                continue;
            if (Q_UNLIKELY(obj->d_func()->threadData != receiver->d_func()->threadData)) {
                qWarning("QCoreApplication: Object event filter cannot be in a different thread.");
                continue;
            }
            if (obj->eventFilter(receiver, event))
                return true;
        }
    }
    return false;
}

/*!\internal

  Helper function called by notify()
 */
bool QCoreApplicationPrivate::notify_helper(QObject *receiver, QEvent * event)
{
    // send to all application event filters
    if (sendThroughApplicationEventFilters(receiver, event))
        return true;
    // send to all receiver event filters
    if (sendThroughObjectEventFilters(receiver, event))
        return true;
    // deliver the event
    return receiver->event(event);
}

/*!
  Returns true if an application object has not been created yet;
  otherwise returns false.

  \sa closingDown()
*/

bool QCoreApplication::startingUp()
{
    return !QCoreApplicationPrivate::is_app_running;
}

/*!
  Returns true if the application objects are being destroyed;
  otherwise returns false.

  \sa startingUp()
*/

bool QCoreApplication::closingDown()
{
    return QCoreApplicationPrivate::is_app_closing;
}


/*!
    Processes all pending events for the calling thread according to
    the specified \a flags until there are no more events to process.

    You can call this function occasionally when your program is busy
    performing a long operation (e.g. copying a file).

    In event you are running a local loop which calls this function
    continuously, without an event loop, the
    \l{QEvent::DeferredDelete}{DeferredDelete} events will
    not be processed. This can affect the behaviour of widgets,
    e.g. QToolTip, that rely on \l{QEvent::DeferredDelete}{DeferredDelete}
    events to function properly. An alternative would be to call
    \l{QCoreApplication::sendPostedEvents()}{sendPostedEvents()} from
    within that local loop.

    Calling this function processes events only for the calling thread.

    \threadsafe

    \sa exec(), QTimer, QEventLoop::processEvents(), flush(), sendPostedEvents()
*/
void QCoreApplication::processEvents(QEventLoop::ProcessEventsFlags flags)
{
    QThreadData *data = QThreadData::current();
    if (!data->eventDispatcher)
        return;
    data->eventDispatcher->processEvents(flags);
}

/*!
    \overload processEvents()

    Processes pending events for the calling thread for \a maxtime
    milliseconds or until there are no more events to process,
    whichever is shorter.

    You can call this function occasionally when you program is busy
    doing a long operation (e.g. copying a file).

    Calling this function processes events only for the calling thread.

    \threadsafe

    \sa exec(), QTimer, QEventLoop::processEvents()
*/
void QCoreApplication::processEvents(QEventLoop::ProcessEventsFlags flags, int maxtime)
{
    QThreadData *data = QThreadData::current();
    if (!data->eventDispatcher)
        return;
    QElapsedTimer start;
    start.start();
    while (data->eventDispatcher->processEvents(flags & ~QEventLoop::WaitForMoreEvents)) {
        if (start.elapsed() > maxtime)
            break;
    }
}

/*****************************************************************************
  Main event loop wrappers
 *****************************************************************************/

/*!
    Enters the main event loop and waits until exit() is called.
    Returns the value that was set to exit() (which is 0 if exit() is
    called via quit()).

    It is necessary to call this function to start event handling. The
    main event loop receives events from the window system and
    dispatches these to the application widgets.

    To make your application perform idle processing (i.e. executing a
    special function whenever there are no pending events), use a
    QTimer with 0 timeout. More advanced idle processing schemes can
    be achieved using processEvents().

    We recommend that you connect clean-up code to the
    \l{QCoreApplication::}{aboutToQuit()} signal.

    \sa quit(), exit(), processEvents(), QApplication::exec()
*/
int QCoreApplication::exec()
{
    if (!QCoreApplicationPrivate::checkInstance("exec"))
        return -1;

    QThreadData *threadData = self->d_func()->threadData;
    if (Q_UNLIKELY(threadData != QThreadData::current())) {
        qWarning("%s::exec: Must be called from the main thread", self->metaObject()->className());
        return -1;
    } else if (Q_UNLIKELY(!threadData->eventLoops.isEmpty())) {
        qWarning("QCoreApplication::exec: The event loop is already running");
        return -1;
    }

    threadData->quitNow = false;
    QEventLoop eventLoop;
    self->d_func()->in_exec = true;
    int returnCode = eventLoop.exec();
    threadData->quitNow = false;
    if (self) {
        self->d_func()->in_exec = false;
        emit self->aboutToQuit();
        sendPostedEvents(0, QEvent::DeferredDelete);
    }

    return returnCode;
}


/*!
  Tells the application to exit with a return code.

    After this function has been called, the application leaves the
    main event loop and returns from the call to exec(). The exec()
    function returns \a returnCode. If the event loop is not running,
    this function does nothing.

  By convention, a \a returnCode of 0 means success, and any non-zero
  value indicates an error.

  Note that unlike the C library function of the same name, this
  function \e does return to the caller -- it is event processing that
  stops.

  \sa quit(), exec()
*/
void QCoreApplication::exit(int returnCode)
{
    if (!self)
        return;
    QThreadData *data = self->d_func()->threadData;
    data->quitNow = true;
    for (int i = 0; i < data->eventLoops.size(); ++i) {
        QEventLoop *eventLoop = data->eventLoops.at(i);
        eventLoop->exit(returnCode);
    }
}

/*****************************************************************************
  QCoreApplication management of posted events
 *****************************************************************************/

/*!
    \fn bool QCoreApplication::sendEvent(QObject *receiver, QEvent *event)

    Sends event \a event directly to receiver \a receiver, using the
    notify() function. Returns the value that was returned from the
    event handler.

    The event is \e not deleted when the event has been sent. The normal
    approach is to create the event on the stack, for example:

    \snippet doc/src/snippets/code/src_corelib_kernel_qcoreapplication.cpp 0

    \sa postEvent(), notify()
*/

/*!
    Adds the event \a event, with the object \a receiver as the
    receiver of the event, to an event queue and returns immediately.

    The event must be allocated on the heap since the post event queue
    will take ownership of the event and delete it once it has been
    posted.  It is \e {not safe} to access the event after
    it has been posted.

    When control returns to the main event loop, all events that are
    stored in the queue will be sent using the notify() function.

    Events are processed in the order posted. For more control over
    the processing order, use the postEvent() overload below, which
    takes a priority argument. This function posts all event with a
    Qt::NormalEventPriority.

    \threadsafe

    \sa sendEvent(), notify(), sendPostedEvents()
*/

void QCoreApplication::postEvent(QObject *receiver, QEvent *event)
{
    postEvent(receiver, event, Qt::NormalEventPriority);
}


/*!
    \overload postEvent()
    \since 4.3

    Adds the event \a event, with the object \a receiver as the
    receiver of the event, to an event queue and returns immediately.

    The event must be allocated on the heap since the post event queue
    will take ownership of the event and delete it once it has been
    posted.  It is \e {not safe} to access the event after
    it has been posted.

    When control returns to the main event loop, all events that are
    stored in the queue will be sent using the notify() function.

    Events are sorted in descending \a priority order, i.e. events
    with a high \a priority are queued before events with a lower \a
    priority. The \a priority can be any integer value, i.e. between
    INT_MAX and INT_MIN, inclusive; see Qt::EventPriority for more
    details. Events with equal \a priority will be processed in the
    order posted.

    \threadsafe

    \sa sendEvent(), notify(), sendPostedEvents(), Qt::EventPriority
*/
void QCoreApplication::postEvent(QObject *receiver, QEvent *event, int priority)
{
    if (Q_UNLIKELY(!receiver)) {
        qWarning("QCoreApplication::postEvent: Unexpected null receiver");
        delete event;
        return;
    }

    QThreadData * volatile * pdata = &receiver->d_func()->threadData;
    QThreadData *data = *pdata;
    if (!data) {
        // posting during destruction? just delete the event to prevent a leak
        delete event;
        return;
    }

    // lock the post event mutex
    data->postEventList.mutex.lock();

    // if object has moved to another thread, follow it
    while (data != *pdata) {
        data->postEventList.mutex.unlock();

        data = *pdata;
        if (!data) {
            // posting during destruction? just delete the event to prevent a leak
            delete event;
            return;
        }

        data->postEventList.mutex.lock();
    }

    QMutexUnlocker locker(&data->postEventList.mutex);

    // if this is one of the compressible events, do compression
    if (receiver->d_func()->postedEvents
        && self && self->compressEvent(event, receiver, &data->postEventList)) {
        return;
    }

    if (event->type() == QEvent::DeferredDelete && data == QThreadData::current()) {
        // remember the current running eventloop for DeferredDelete
        // events posted in the receiver's thread
        event->looplevel = data->loopLevel;
    }

    // delete the event on exceptions to protect against memory leaks till the event is
    // properly owned in the postEventList
    QScopedPointer<QEvent> eventDeleter(event);
    data->postEventList.addEvent(QPostEvent(receiver, event, priority));
    eventDeleter.take();
    event->posted = true;
    ++receiver->d_func()->postedEvents;
    data->canWait = false;
    locker.unlock();

    if (data->eventDispatcher)
        data->eventDispatcher->wakeUp();
}

/*!
  \internal
  Returns true if \a event was compressed away (possibly deleted) and should not be added to the list.
*/
bool QCoreApplication::compressEvent(QEvent *event, QObject *receiver, QPostEventList *postedEvents)
{
    if ((event->type() == QEvent::DeferredDelete || event->type() == QEvent::Quit)
        && receiver->d_func()->postedEvents > 0) {
        for (int i = 0; i < postedEvents->size(); ++i) {
            const QPostEvent &cur = postedEvents->at(i);
            if (cur.receiver != receiver
                || cur.event == 0
                || cur.event->type() != event->type())
                continue;
            // found an event for this receiver
            delete event;
            return true;
        }
    }
    return false;
}

/*!
  \fn void QCoreApplication::sendPostedEvents()
  \overload sendPostedEvents()

    Dispatches all posted events, i.e. empties the event queue.
*/

/*!
  Immediately dispatches all events which have been previously queued
  with QCoreApplication::postEvent() and which are for the object \a receiver
  and have the event type \a event_type.

  Events from the window system are \e not dispatched by this
  function, but by processEvents().

  If \a receiver is null, the events of \a event_type are sent for all
  objects. If \a event_type is 0, all the events are sent for \a receiver.

  \note This method must be called from the same thread as its QObject parameter, \a receiver.

  \sa flush(), postEvent()
*/

void QCoreApplication::sendPostedEvents(QObject *receiver, int event_type)
{
    QThreadData *data = QThreadData::current();

    QCoreApplicationPrivate::sendPostedEvents(receiver, event_type, data);
}

void QCoreApplicationPrivate::sendPostedEvents(QObject *receiver, int event_type,
                                               QThreadData *data)
{
    if (event_type == -1) {
        // we were called by an obsolete event dispatcher.
        event_type = 0;
    }

    if (Q_UNLIKELY(receiver && receiver->d_func()->threadData != data)) {
        qWarning("QCoreApplication::sendPostedEvents: Cannot send "
                 "posted events for objects in another thread");
        return;
    }

    ++data->postEventList.recursion;


    QMutexLocker locker(&data->postEventList.mutex);

    // by default, we assume that the event dispatcher can go to sleep after
    // processing all events. if any new events are posted while we send
    // events, canWait will be set to false.
    data->canWait = (data->postEventList.size() == 0);

    if (data->postEventList.size() == 0 || (receiver && !receiver->d_func()->postedEvents)) {
        --data->postEventList.recursion;
        return;
    }

    data->canWait = true;

    // okay. here is the tricky loop. be careful about optimizing
    // this, it looks the way it does for good reasons.
    int startOffset = data->postEventList.startOffset;
    int &i = (!event_type && !receiver) ? data->postEventList.startOffset : startOffset;
    data->postEventList.insertionOffset = data->postEventList.size();

    while (i < data->postEventList.size()) {
        // avoid live-lock
        if (i >= data->postEventList.insertionOffset)
            break;

        const QPostEvent &pe = data->postEventList.at(i);
        ++i;

        if (!pe.event)
            continue;
        if ((receiver && receiver != pe.receiver) || (event_type && event_type != pe.event->type())) {
            data->canWait = false;
            continue;
        }

        if (pe.event->type() == QEvent::DeferredDelete) {
            // DeferredDelete events are only sent when we are explicitly asked to
            // (s.a. QEvent::DeferredDelete), and then only if the event loop that
            // posted the event has returned.
            const bool allowDeferredDelete =
                (pe.event->looplevel > data->loopLevel
                 || (!pe.event->looplevel && data->loopLevel > 0)
                 || (event_type == QEvent::DeferredDelete && pe.event->looplevel == data->loopLevel));
            if (!allowDeferredDelete) {
                // cannot send deferred delete
                if (!event_type && !receiver) {
                    // don't lose the event
                    data->postEventList.addEvent(pe);
                    const_cast<QPostEvent &>(pe).event = 0;
                }
                continue;
            }
        }

        // first, we diddle the event so that we can deliver
        // it, and that no one will try to touch it later.
        pe.event->posted = false;
        QEvent * e = pe.event;
        QObject * r = pe.receiver;

        --r->d_func()->postedEvents;
        Q_ASSERT(r->d_func()->postedEvents >= 0);

        // next, update the data structure so that we're ready
        // for the next event.
        const_cast<QPostEvent &>(pe).event = 0;

        locker.unlock();
        // after all that work, it's time to deliver the event.
        QCoreApplication::sendEvent(r, e);

        delete e;
        locker.relock();

        // careful when adding anything below this point - the
        // sendEvent() call might invalidate any invariants this
        // function depends on.
    }

    --data->postEventList.recursion;
    if (!data->postEventList.recursion && !data->canWait && data->eventDispatcher)
        data->eventDispatcher->wakeUp();

    // clear the global list, i.e. remove everything that was
    // delivered.
    if (!event_type && !receiver && data->postEventList.startOffset >= 0) {
        const QPostEventList::iterator it = data->postEventList.begin();
        data->postEventList.erase(it, it + data->postEventList.startOffset);
        data->postEventList.insertionOffset -= data->postEventList.startOffset;
        Q_ASSERT(data->postEventList.insertionOffset >= 0);
        data->postEventList.startOffset = 0;
    }
}

/*!
  Removes all events posted using postEvent() for \a receiver.

  The events are \e not dispatched, instead they are removed from the
  queue. You should never need to call this function. If you do call it,
  be aware that killing events may cause \a receiver to break one or
  more invariants.

  \threadsafe
*/

void QCoreApplication::removePostedEvents(QObject *receiver)
{
    removePostedEvents(receiver, 0);
}

/*!
    \overload removePostedEvents()
    \since 4.3

    Removes all events of the given \a eventType that were posted
    using postEvent() for \a receiver.

    The events are \e not dispatched, instead they are removed from
    the queue. You should never need to call this function. If you do
    call it, be aware that killing events may cause \a receiver to
    break one or more invariants.

    If \a receiver is null, the events of \a eventType are removed for
    all objects. If \a eventType is 0, all the events are removed for
    \a receiver.

    \threadsafe
*/

void QCoreApplication::removePostedEvents(QObject *receiver, int eventType)
{

    QThreadData *data = receiver ? receiver->d_func()->threadData : QThreadData::current();
    QMutexLocker locker(&data->postEventList.mutex);

    // the QObject destructor calls this function directly.  this can
    // happen while the event loop is in the middle of posting events,
    // and when we get here, we may not have any more posted events
    // for this object.
    if (receiver && !receiver->d_func()->postedEvents)
        return;

    //we will collect all the posted events for the QObject
    //and we'll delete after the mutex was unlocked
    QStdVector<QEvent*> events;
    int n = data->postEventList.size();
    int j = 0;

    for (int i = 0; i < n; ++i) {
        const QPostEvent &pe = data->postEventList.at(i);

        if ((!receiver || pe.receiver == receiver)
            && (pe.event && (eventType == 0 || pe.event->type() == eventType))) {
            --pe.receiver->d_func()->postedEvents;
            pe.event->posted = false;
            events.append(pe.event);
            const_cast<QPostEvent &>(pe).event = 0;
        } else if (!data->postEventList.recursion) {
            if (i != j)
                data->postEventList.swap(i, j);
            ++j;
        }
    }

#ifndef QT_NO_DEBUG
    if (receiver && eventType == 0) {
        Q_ASSERT(!receiver->d_func()->postedEvents);
    }
#endif

    if (!data->postEventList.recursion) {
        // truncate list
        data->postEventList.erase(data->postEventList.begin() + j, data->postEventList.end());
    }

    locker.unlock();
    for (int i = 0; i < events.count(); ++i) {
        delete events[i];
    }
}

/*!
  Removes \a event from the queue of posted events, and emits a
  warning message if appropriate.

  \warning This function can be \e really slow. Avoid using it, if
  possible.

  \threadsafe
*/

void QCoreApplicationPrivate::removePostedEvent(QEvent * event)
{
    if (!event || !event->posted)
        return;

    QThreadData *data = QThreadData::current();

    QMutexLocker locker(&data->postEventList.mutex);

    if (data->postEventList.size() == 0) {
#ifndef QT_NO_DEBUG
        qDebug("QCoreApplication::removePostedEvent: Internal error: %p %d is posted",
                (void*)event, event->type());
        return;
#endif
    }

    for (int i = 0; i < data->postEventList.size(); ++i) {
        const QPostEvent & pe = data->postEventList.at(i);
        if (pe.event == event) {
#ifndef QT_NO_DEBUG
            qWarning("QCoreApplication::removePostedEvent: Event of type %d deleted while posted to %s %s",
                     event->type(),
                     pe.receiver->metaObject()->className(),
                     pe.receiver->objectName().toLocal8Bit().data());
#endif
            --pe.receiver->d_func()->postedEvents;
            pe.event->posted = false;
            delete pe.event;
            const_cast<QPostEvent &>(pe).event = 0;
            return;
        }
    }
}

/*!\reimp

*/
bool QCoreApplication::event(QEvent *e)
{
    if (e->type() == QEvent::Quit) {
        quit();
        return true;
    }
    return QObject::event(e);
}

/*! \enum QCoreApplication::Encoding

    This enum type defines the 8-bit encoding of character string
    arguments to translate():

    \value CodecForTr  US-ASCII.
    \value UnicodeUTF8  UTF-8.
    \value DefaultCodec  (Obsolete) Use CodecForTr instead.

    \sa QObject::tr(), QObject::trUtf8(), QString::fromUtf8()
*/

/*!
    Tells the application to exit with return code 0 (success).
    Equivalent to calling QCoreApplication::exit(0).

    It's common to connect the QApplication::lastWindowClosed() signal
    to quit(), and you also often connect e.g. QAbstractButton::clicked() or
    signals in QAction, QMenu, or QMenuBar to it.

    Example:

    \snippet doc/src/snippets/code/src_corelib_kernel_qcoreapplication.cpp 1

    \sa exit(), aboutToQuit(), QApplication::lastWindowClosed()
*/

void QCoreApplication::quit()
{
    QCoreApplication::exit(0);
}

/*!
  \fn void QCoreApplication::aboutToQuit()

  This signal is emitted when the application is about to quit the
  main event loop, e.g. when the event loop level drops to zero.
  This may happen either after a call to quit() from inside the
  application or when the users shuts down the entire desktop session.

  The signal is particularly useful if your application has to do some
  last-second cleanup. Note that no user interaction is possible in
  this state.

  \sa quit()
*/

#ifndef QT_NO_TRANSLATION
/*!
    Adds the translation file \a translationFile to the list of
    translation files to be used for translations.

    Multiple translation files can be installed. Translations are
    searched for in the reverse order in which they were installed,
    so the most recently installed translation file is searched first
    and the first translation file installed is searched last.
    The search stops as soon as a translation containing a matching
    string is found.

    Installing or removing a QTranslator, or changing an installed QTranslator
    generates a \l{QEvent::LanguageChange}{LanguageChange} event for the
    QCoreApplication instance. A QApplication instance will propagate the event
    to all toplevel windows, where a reimplementation of changeEvent can
    re-translate the user interface by passing user-visible strings via the
    tr() function to the respective property setters. User-interface classes
    generated by \l{Katie Designer} provide a \c retranslateUi() function that can be
    called.

    \sa removeTranslator() translate() QTranslator::load() {Dynamic Translation}
*/

void QCoreApplication::installTranslator(QTranslator *translationFile)
{
    if (!translationFile || translationFile->isEmpty()) {
        return;
    }

    if (!QCoreApplicationPrivate::checkInstance("installTranslator")) {
        return;
    }

    std::lock_guard<std::recursive_mutex> locker(qGlobalApplicationMutex);
    QCoreApplicationPrivate *d = self->d_func();
    d->translators.prepend(translationFile);

    QEvent ev(QEvent::LanguageChange);
    QCoreApplication::sendEvent(self, &ev);
}

/*!
    Removes the translation file \a translationFile from the list of
    translation files used by this application. (It does not delete the
    translation file from the file system.)

    \sa installTranslator() translate(), QObject::tr()
*/

void QCoreApplication::removeTranslator(QTranslator *translationFile)
{
    if (!translationFile) {
        return;
    }

    if (!QCoreApplicationPrivate::checkInstance("removeTranslator")) {
        return;
    }

    std::lock_guard<std::recursive_mutex> locker(qGlobalApplicationMutex);
    QCoreApplicationPrivate *d = self->d_func();
    if (d->translators.removeAll(translationFile) && !self->closingDown()) {
        QEvent ev(QEvent::LanguageChange);
        QCoreApplication::sendEvent(self, &ev);
    }
}

/*!
    \reentrant
    \since 4.5

    Returns the translation text for \a sourceText, by querying the
    installed translation files. The translation files are searched
    from the most recently installed file back to the first
    installed file.

    QObject::tr() and QObject::trUtf8() provide this functionality
    more conveniently.

    \a context is typically a class name (e.g., "MyDialog") and \a
    sourceText is either English text or a short identifying text.

    See the \l QTranslator and \l QObject::tr() documentation for
    more information.

    \a encoding indicates the 8-bit encoding of character strings.

    If none of the translation files contain a translation for \a
    sourceText in \a context, this function returns a QString
    equivalent of \a sourceText. The encoding of \a sourceText is
    specified by \e encoding; it defaults to CodecForTr.

    This function is not virtual. You can use alternative translation
    techniques by subclassing \l QTranslator.

    \warning This method is reentrant only if all translators are
    installed \e before calling this method. Installing or removing
    translators while performing translations is not supported. Doing
    so will most likely result in crashes or other undesirable
    behavior.

    \sa QObject::tr() installTranslator()
*/


QString QCoreApplication::translate(const char *context, const char *sourceText, Encoding encoding)
{
    if (Q_UNLIKELY(!sourceText))
        return QString();

    if (self && !self->d_func()->translators.isEmpty()) {
        foreach (const QTranslator *translationFile, self->d_func()->translators) {
            QString result = translationFile->translate(context, sourceText);
            if (!result.isEmpty())
                return result;
        }
    }

    if (encoding == UnicodeUTF8) {
        return QString::fromUtf8(sourceText);
    }
    return QString::fromAscii(sourceText);
}
#endif // QT_NO_TRANSLATION

/*!
    Returns the directory that contains the application executable.

    For example, if you have installed Qt in the \c{C:\Trolltech\Qt}
    directory, and you run the \c{regexp} example, this function will
    return "C:/Trolltech/Qt/examples/tools/regexp".

    \warning On Linux, this function will try to get the path from the
    \c {/proc} file system. If that fails, it assumes that \c
    {argv[0]} contains the absolute file name of the executable. The
    function also assumes that the current directory has not been
    changed by the application.

    In Symbian this function will return the application private directory,
    not the path to executable itself, as those are always in \c {/sys/bin}.
    If the application is in a read only drive, i.e. ROM, then the private path
    on the system drive will be returned.

    \sa applicationFilePath()
*/
QString QCoreApplication::applicationDirPath()
{
    if (Q_UNLIKELY(!self)) {
        qWarning("QCoreApplication::applicationDirPath: Please instantiate the QApplication object first");
        return QString();
    }

    QCoreApplicationPrivate *d = self->d_func();
    if (d->cachedApplicationDirPath.isNull())
        d->cachedApplicationDirPath = QFileInfo(applicationFilePath()).path();
    return d->cachedApplicationDirPath;
}

/*!
    Returns the file path of the application executable.

    For example, if you have installed Qt in the \c{/usr/local/qt}
    directory, and you run the \c{regexp} example, this function will
    return "/usr/local/qt/examples/tools/regexp/regexp".

    \warning On Linux, this function will try to get the path from the
    \c {/proc} file system. If that fails, it assumes that \c
    {argv[0]} contains the absolute file name of the executable. The
    function also assumes that the current directory has not been
    changed by the application.

    \sa applicationDirPath()
*/
QString QCoreApplication::applicationFilePath()
{
    if (Q_UNLIKELY(!self)) {
        qWarning("QCoreApplication::applicationFilePath: Please instantiate the QApplication object first");
        return QString();
    }

    QCoreApplicationPrivate *d = self->d_func();
    if (!d->cachedApplicationFilePath.isNull())
        return d->cachedApplicationFilePath;

#if defined(QT_HAVE_PROC_EXE)
    // Try looking for a /proc/<pid>/exe symlink first which points to
    // the absolute path of the executable
    QFileInfo pfi(QString::fromLatin1("/proc/%1/exe").arg(::getpid()));
    if (pfi.exists() && pfi.isSymLink()) {
        d->cachedApplicationFilePath = pfi.canonicalFilePath();
        return d->cachedApplicationFilePath;
    }
#endif

    QString argv0;

    char ** const argv = d->argv;
    if (argv && argv[0]) {
        argv0 = QString::fromLocal8Bit(argv[0]);
    }

#ifdef QT_HAVE_GETPROGNAME
    if (argv0.isEmpty()) {
        argv0 = QString::fromLocal8Bit(::getprogname());
    }
#endif // QT_HAVE_GETPROGNAME

    QString absPath;

    if (!argv0.isEmpty() && argv0.at(0) == QLatin1Char('/')) {
        /*
          If argv0 starts with a slash, it is already an absolute
          file path.
        */
        absPath = argv0;
    } else if (argv0.contains(QLatin1Char('/'))) {
        /*
          If argv0 contains one or more slashes, it is a file path
          relative to the current directory.
        */
        absPath = QDir::current().absoluteFilePath(argv0);
    } else {
        /*
          Otherwise, the file path has to be determined using the
          PATH environment variable.
        */
        absPath = QStandardPaths::findExecutable(argv0);
    }

    absPath = QDir::cleanPath(absPath);

    QFileInfo fi(absPath);
    d->cachedApplicationFilePath = fi.exists() ? fi.canonicalFilePath() : QString();
    return d->cachedApplicationFilePath;
}

/*!
    \since 4.4

    Returns the current process ID for the application.
*/
qint64 QCoreApplication::applicationPid()
{
    return ::getpid();
}

/*!
    \since 4.1

    Returns the list of command-line arguments.

    Usually arguments().at(0) is the program name, arguments().at(1)
    is the first argument, and arguments().last() is the last
    argument. See the note below about Windows.

    Calling this function is slow - you should store the result in a variable
    when parsing the command line.

    \warning On Unix, this list is built from the argc and argv parameters passed
    to the constructor in the main() function. The string-data in argv is
    interpreted using QString::fromLocal8Bit(); hence it is not possible to
    pass, for example, Japanese command line arguments on a system that runs in a
    Latin1 locale. Most modern Unix systems do not have this limitation, as they are
    Unicode-based.

    \sa applicationFilePath()
*/

QStringList QCoreApplication::arguments()
{
    QStringList list;

    if (Q_UNLIKELY(!self)) {
        qWarning("QCoreApplication::arguments: Please instantiate the QApplication object first");
        return list;
    }
    const int ac = self->d_func()->argc;
    char ** const av = self->d_func()->argv;
    for (int a = 0; a < ac; ++a) {
        list << QString::fromLocal8Bit(av[a]);
    }

    return list;
}

/*!
    \property QCoreApplication::organizationName
    \brief the name of the organization that wrote this application

    The value is used by the QSettings class when it is constructed
    using the empty constructor. This saves having to repeat this
    information each time a QSettings object is created.

    \sa organizationDomain applicationName
*/

void QCoreApplication::setOrganizationName(const QString &orgName)
{
    coreappdata()->orgName = orgName;
}

QString QCoreApplication::organizationName()
{
    return coreappdata()->orgName;
}

/*!
    \property QCoreApplication::organizationDomain
    \brief the Internet domain of the organization that wrote this application

    The value is used by the QSettings class when it is constructed
    using the empty constructor. This saves having to repeat this
    information each time a QSettings object is created.

    On Mac, QSettings uses organizationDomain() as the organization
    if it's not an empty string; otherwise it uses organizationName().
    On all other platforms, QSettings uses organizationName() as the
    organization.

    \sa organizationName applicationName applicationVersion
*/
void QCoreApplication::setOrganizationDomain(const QString &orgDomain)
{
    coreappdata()->orgDomain = orgDomain;
}

QString QCoreApplication::organizationDomain()
{
    return coreappdata()->orgDomain;
}

/*!
    \property QCoreApplication::applicationName
    \brief the name of this application

    The value is used by the QSettings class when it is constructed
    using the empty constructor. This saves having to repeat this
    information each time a QSettings object is created.

    \sa organizationName organizationDomain applicationVersion
*/
void QCoreApplication::setApplicationName(const QString &application)
{
    coreappdata()->application = application;
}

QString QCoreApplication::applicationName()
{
    QString name = coreappdata()->application;

#ifdef QT_HAVE_PROGRAM_INVOCATION_SHORT_NAME
    if (name.isEmpty()) {
        name = QString::fromLocal8Bit(program_invocation_short_name);
    }
#endif

#ifdef QT_HAVE_GETPROGNAME
    if (name.isEmpty()) {
        name = QString::fromLocal8Bit(::getprogname());
    }
#endif // QT_HAVE_GETPROGNAME

#if defined(QT_HAVE_PROC_EXE)
    if (name.isEmpty()) {
        // Try looking for a /proc/<pid>/exe symlink first which points to
        // the absolute path of the executable
        QFileInfo pfi(QString::fromLatin1("/proc/%1/exe").arg(::getpid()));
        if (pfi.exists() && pfi.isSymLink()) {
            pfi.setFile(pfi.canonicalFilePath());
            name = pfi.baseName();
        }
    }
#endif

    if (name.isEmpty() && self) {
        char ** const argv = self->d_func()->argv;
        if (argv && argv[0]) {
            const char *p = ::strrchr(argv[0], '/');
            name = QString::fromLocal8Bit(p ? p + 1 : argv[0]);
        }
    }

    return name;
}

/*!
    \property QCoreApplication::applicationVersion
    \since 4.4
    \brief the version of this application

    \sa applicationName organizationName organizationDomain
*/
void QCoreApplication::setApplicationVersion(const QString &version)
{
    coreappdata()->applicationVersion = version;
}

QString QCoreApplication::applicationVersion()
{
    return coreappdata()->applicationVersion;
}

#ifndef QT_NO_LIBRARY
/*!
    Returns a list of paths that the application will search when
    dynamically loading libraries.

    Katie provides default library paths, but they can also be set using
    a \l{Using Katie.json}{Katie.json} file. Paths specified in this file
    will override default values.

    This list will include the installation directory for libraries if
    it exists (the default installation directory for libraries is \c
    INSTALL/lib, where \c INSTALL is the directory where Katie was
    installed). The colon separated entries of the LD_LIBRARY_PATH
    environment variable are also added.

    \sa setLibraryPaths(), addLibraryPath(), removeLibraryPath(), QLibrary
*/
QStringList QCoreApplication::libraryPaths()
{
    std::lock_guard<std::recursive_mutex> locker(qGlobalApplicationMutex);

    if (coreappdata()->app_librarypaths.isEmpty()) {
        const QString installPathLibraries = QLibraryInfo::location(QLibraryInfo::LibrariesPath);
        if (QDir(installPathLibraries).exists()
            && !coreappdata()->app_librarypaths.contains(installPathLibraries)) {
            coreappdata()->app_librarypaths.append(installPathLibraries);
        }
    }

    foreach (const QString &it, qGetEnvList("LD_LIBRARY_PATH")) {
        const QString canonicalPath = QDir(it).canonicalPath();
        if (canonicalPath.isEmpty()) {
            continue;
        }
        if (!coreappdata()->app_librarypaths.contains(canonicalPath)) {
            coreappdata()->app_librarypaths.append(canonicalPath);
        }
    }

    return coreappdata()->app_librarypaths;
}

/*!

    Sets the list of directories to search when loading libraries to
    \a paths. All existing paths will be deleted and the path list
    will consist of the paths given in \a paths.

    \sa libraryPaths(), addLibraryPath(), removeLibraryPath(), QLibrary
 */
void QCoreApplication::setLibraryPaths(const QStringList &paths)
{
    std::lock_guard<std::recursive_mutex> locker(qGlobalApplicationMutex);
    coreappdata()->app_librarypaths = paths;
}

/*!
  Prepends \a path to the beginning of the library path list, ensuring that
  it is searched for libraries first. If \a path is empty or already in the
  path list, the path list is not changed.

  \sa removeLibraryPath(), libraryPaths(), setLibraryPaths()
 */
void QCoreApplication::addLibraryPath(const QString &path)
{
    const QString canonicalPath = QDir(path).canonicalPath();
    if (canonicalPath.isEmpty()) {
        return;
    }

    // make sure that library paths is initialized
    libraryPaths();

    std::lock_guard<std::recursive_mutex> locker(qGlobalApplicationMutex);
    if (!coreappdata()->app_librarypaths.contains(canonicalPath)) {
        coreappdata()->app_librarypaths.prepend(canonicalPath);
    }
}

/*!
    Removes \a path from the library path list. If \a path is empty or not
    in the path list, the list is not changed.

    \sa addLibraryPath(), libraryPaths(), setLibraryPaths()
*/
void QCoreApplication::removeLibraryPath(const QString &path)
{
    const QString canonicalPath = QDir(path).canonicalPath();
    if (canonicalPath.isEmpty()) {
        return;
    }

    // make sure that library paths is initialized
    libraryPaths();

    std::lock_guard<std::recursive_mutex> locker(qGlobalApplicationMutex);
    coreappdata()->app_librarypaths.removeAll(canonicalPath);
}

/*!
    Returns a list of paths that the application will search when
    dynamically loading plugins.

    This list will include the installation directory for plugins if
    it exists (the default installation directory for plugins is \c
    INSTALL/plugins, where \c INSTALL is the directory where Katie was
    installed). The colon separated entries of the QT_PLUGIN_PATH
    environment variable are also added.

    \sa setPluginPaths(), addPluginPath(), removePluginPath(),
        QPluginLoader, {How to Create Katie Plugins}
*/
QStringList QCoreApplication::pluginPaths()
{
    std::lock_guard<std::recursive_mutex> locker(qGlobalApplicationMutex);

    if (coreappdata()->app_pluginpaths.isEmpty()) {
        const QString installPathPlugins = QLibraryInfo::location(QLibraryInfo::PluginsPath);
        if (QDir(installPathPlugins).exists()
            && !coreappdata()->app_pluginpaths.contains(installPathPlugins)) {
            coreappdata()->app_pluginpaths.append(installPathPlugins);
        }
    }

    foreach (const QString &it, qGetEnvList("QT_PLUGIN_PATH")) {
        const QString canonicalPath = QDir(it).canonicalPath();
        if (canonicalPath.isEmpty()) {
            continue;
        }
        if (!coreappdata()->app_pluginpaths.contains(canonicalPath)) {
            coreappdata()->app_pluginpaths.append(canonicalPath);
        }
    }

    return coreappdata()->app_pluginpaths;
}

/*!

    Sets the list of directories to search when loading plugins to
    \a paths. All existing paths will be deleted and the path list
    will consist of the paths given in \a paths.

    \sa pluginPaths(), addPluginPath(), removePluginPath(), QLibrary
 */
void QCoreApplication::setPluginPaths(const QStringList &paths)
{
    {
        std::lock_guard<std::recursive_mutex> locker(qGlobalApplicationMutex);
        coreappdata()->app_pluginpaths = paths;
    }
    QFactoryLoader::refreshAll();
}

/*!
  Prepends \a path to the beginning of the plugin path list, ensuring that
  it is searched for plugins first. If \a path is empty or already in the
  path list, the path list is not changed.

  \sa removePluginPath(), pluginPaths(), setPluginPaths()
 */
void QCoreApplication::addPluginPath(const QString &path)
{
    const QString canonicalPath = QDir(path).canonicalPath();
    if (canonicalPath.isEmpty()) {
        return;
    }

    // make sure that plugin paths is initialized
    pluginPaths();

    std::lock_guard<std::recursive_mutex> locker(qGlobalApplicationMutex);
    if (!coreappdata()->app_pluginpaths.contains(canonicalPath)) {
        coreappdata()->app_pluginpaths.prepend(canonicalPath);
        QFactoryLoader::refreshAll();
    }
}

/*!
    Removes \a path from the plugin path list. If \a path is empty or not
    in the path list, the list is not changed.

    \sa addPluginPath(), pluginPaths(), setPluginPaths()
*/
void QCoreApplication::removePluginPath(const QString &path)
{
    const QString canonicalPath = QDir(path).canonicalPath();
    if (canonicalPath.isEmpty()) {
        return;
    }

    // make sure that plugin paths is initialized
    pluginPaths();

    std::lock_guard<std::recursive_mutex> locker(qGlobalApplicationMutex);
    coreappdata()->app_pluginpaths.removeAll(canonicalPath);
    QFactoryLoader::refreshAll();
}
#endif //QT_NO_LIBRARY

/*!
    \typedef QCoreApplication::EventFilter

    A function with the following signature that can be used as an
    event filter:

    \snippet doc/src/snippets/code/src_corelib_kernel_qcoreapplication.cpp 3

    \sa setEventFilter()
*/

/*!
    \fn EventFilter QCoreApplication::setEventFilter(EventFilter filter)

    Replaces the event filter function for the QCoreApplication with
    \a filter and returns the pointer to the replaced event filter
    function. Only the current event filter function is called. If you
    want to use both filter functions, save the replaced EventFilter
    in a place where yours can call it.

    The event filter function set here is called for all messages
    received by all threads meant for all Qt objects. It is \e not
    called for messages that are not meant for Qt objects.

    The event filter function should return true if the message should
    be filtered, (i.e. stopped). It should return false to allow
    processing the message to continue.

    By default, no event filter function is set (i.e., this function
    returns a null EventFilter the first time it is called).

    \note The filter function set here receives native messages,
    i.e. MSG or XEvent structs, that are going to Qt objects. It is
    called by QCoreApplication::filterEvent(). If the filter function
    returns false to indicate the message should be processed further,
    the native message can then be translated into a QEvent and
    handled by the standard Qt \l{QEvent} {event} filering, e.g.
    QObject::installEventFilter().

    \note The filter function set here is different form the filter
    function set via QAbstractEventDispatcher::setEventFilter(), which
    gets all messages received by its thread, even messages meant for
    objects that are not handled by Qt.

    \sa QObject::installEventFilter(), QAbstractEventDispatcher::setEventFilter()
*/
QCoreApplication::EventFilter
QCoreApplication::setEventFilter(QCoreApplication::EventFilter filter)
{
    Q_D(QCoreApplication);
    EventFilter old = d->eventFilter;
    d->eventFilter = filter;
    return old;
}

/*!
    Sends \a message through the event filter that was set by
    setEventFilter(). If no event filter has been set, this function
    returns false; otherwise, this function returns the result of the
    event filter function in the \a result parameter.

    \sa setEventFilter()
*/
bool QCoreApplication::filterEvent(void *message, long *result)
{
    Q_D(QCoreApplication);
    if (result)
        *result = 0;
    if (d->eventFilter)
        return d->eventFilter(message, result);
    return false;
}

/*!
    This function returns true if there are pending events; otherwise
    returns false. Pending events can be either from the window
    system or posted events using postEvent().

    \sa QAbstractEventDispatcher::hasPendingEvents()
*/
bool QCoreApplication::hasPendingEvents()
{
    QAbstractEventDispatcher *eventDispatcher = QAbstractEventDispatcher::instance();
    if (eventDispatcher)
        return eventDispatcher->hasPendingEvents();
    return false;
}

/*!
    \fn void qAddPostRoutine(QtCleanUpFunction ptr)
    \relates QCoreApplication

    Adds a global routine that will be called from the QApplication
    destructor. This function is normally used to add cleanup routines
    for program-wide functionality.

    The function specified by \a ptr should take no arguments and should
    return nothing. For example:

    \snippet doc/src/snippets/code/src_corelib_kernel_qcoreapplication.cpp 4

    Note that for an application- or module-wide cleanup,
    qAddPostRoutine() is often not suitable. For example, if the
    program is split into dynamically loaded modules, the relevant
    module may be unloaded long before the QApplication destructor is
    called.

    For modules and libraries, using a reference-counted
    initialization manager or Qt's parent-child deletion mechanism may
    be better. Here is an example of a private class that uses the
    parent-child mechanism to call a cleanup function at the right
    time:

    \snippet doc/src/snippets/code/src_corelib_kernel_qcoreapplication.cpp 5

    By selecting the right parent object, this can often be made to
    clean up the module's data at the right moment.
*/

/*!
    \macro Q_DECLARE_TR_FUNCTIONS(context)
    \relates QCoreApplication

    The Q_DECLARE_TR_FUNCTIONS() macro declares and implements two
    translation functions, \c tr() and \c trUtf8(), with these
    signatures:

    \snippet doc/src/snippets/code/src_corelib_kernel_qcoreapplication.cpp 6

    This macro is useful if you want to use QObject::tr() or
    QObject::trUtf8() in classes that don't inherit from QObject.

    Q_DECLARE_TR_FUNCTIONS() must appear at the very top of the
    class definition (before the first \c{public:} or \c{protected:}).
    For example:

    \snippet doc/src/snippets/code/src_corelib_kernel_qcoreapplication.cpp 7

    The \a context parameter is normally the class name, but it can
    be any string.

    \sa Q_OBJECT, QObject::tr(), QObject::trUtf8()
*/

#include "moc_qcoreapplication.h"

QT_END_NAMESPACE
