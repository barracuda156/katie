/****************************************************************************
**
** Copyright (C) 2015 The Qt Company Ltd.
** Copyright (C) 2016 Ivailo Monev
**
** This file is part of the QtDBus module of the Katie Toolkit.
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
#ifndef QDBUSTHREADDEBUG_P_H
#define QDBUSTHREADDEBUG_P_H

//
//  W A R N I N G
//  -------------
//
// This file is not part of the Katie API.  It exists purely as an
// implementation detail.  This header file may change from version to
// version without notice, or even be removed.
//
// We mean it.
//

#include <QtCore/qdebug.h>


QT_BEGIN_NAMESPACE

#if !defined(QDBUS_THREAD_DEBUG) && defined(QT_BUILD_INTERNAL)
# define QDBUS_THREAD_DEBUG 1
#else
# define QDBUS_THREAD_DEBUG 0
#endif

static inline QDebug operator<<(QDebug dbg, const QThread *th)
{
    dbg.nospace() << "QThread(ptr=" << (void*)th;
    if (th && !th->objectName().isEmpty())
        dbg.nospace() << ", name=" << th->objectName();
    dbg.nospace() << ')';
    return dbg.space();
}

#if QDBUS_THREAD_DEBUG
static inline QDebug operator<<(QDebug dbg, const QDBusConnectionPrivate *conn)
{
    dbg.nospace() << "QDBusConnection("
                  << "ptr=" << (void*)conn
                  << ", name=" << conn->name
                  << ", baseService=" << conn->baseService
                  << ", thread=";
    if (conn->thread() == QThread::currentThread())
        dbg.nospace() << "same thread";
    else
        dbg.nospace() << conn->thread();
    dbg.nospace() << ')';
    return dbg.space();
}
#endif


enum ThreadAction {
    ConnectAction = 0,
    DisconnectAction = 1,
    RegisterObjectAction = 2,
    UnregisterObjectAction = 3,
    ObjectRegisteredAtAction = 4,
    ServiceRegisteredAction = 5,

    CloseConnectionAction = 10,
    ObjectDestroyedAction = 11,
    RelaySignalAction = 12,
    HandleObjectCallAction = 13,
    HandleSignalAction = 14,
    ConnectRelayAction = 15,
    DisconnectRelayAction = 16,
    FindMetaObject1Action = 17,
    FindMetaObject2Action = 18,
    RegisterServiceAction = 19,
    UnregisterServiceAction = 20,
    UpdateSignalHookOwnerAction = 21,
    HandleObjectCallPostEventAction = 22,
    HandleObjectCallSemaphoreAction = 23,
    DoDispatchAction = 24,
    SendWithReplyAsyncAction = 25,
    MessageResultReceivedAction = 26,
    ActivateSignalAction = 27,
    PendingCallBlockAction = 28,
    SendMessageAction = 29,
    SendWithReplyAndBlockAction = 30,
    HuntAndEmitAction = 31,

    AddTimeoutAction = 50,
    RealAddTimeoutAction = 51,
    RemoveTimeoutAction = 52,
    KillTimerAction = 58,
    TimerEventAction = 59,
    AddWatchAction = 60,
    RemoveWatchAction = 61,
    ToggleWatchAction = 62,
    SocketReadAction = 63,
    SocketWriteAction = 64
};

struct QDBusLockerBase
{
    enum Condition
    {
        BeforeLock,
        AfterLock,
        BeforeUnlock,
        AfterUnlock,

        BeforePost,
        AfterPost,
        BeforeDeliver,
        AfterDeliver,

        BeforeAcquire,
        AfterAcquire,
        BeforeRelease,
        AfterRelease
    };

    static const bool dbusThreadDebug;
#if QDBUS_THREAD_DEBUG
    static inline void reportThreadAction(int action, int condition, QDBusConnectionPrivate *conn)
    {
        if (dbusThreadDebug) {
            qDebug() << QThread::currentThread()
                 << "QtDBus threading action" << action
                 << (condition == QDBusLockerBase::BeforeLock ? "before lock" :
                     condition == QDBusLockerBase::AfterLock ? "after lock" :
                     condition == QDBusLockerBase::BeforeUnlock ? "before unlock" :
                     condition == QDBusLockerBase::AfterUnlock ? "after unlock" :
                     condition == QDBusLockerBase::BeforePost ? "before event posting" :
                     condition == QDBusLockerBase::AfterPost ? "after event posting" :
                     condition == QDBusLockerBase::BeforeDeliver ? "before event delivery" :
                     condition == QDBusLockerBase::AfterDeliver ? "after event delivery" :
                     condition == QDBusLockerBase::BeforeAcquire ? "before acquire" :
                     condition == QDBusLockerBase::AfterAcquire ? "after acquire" :
                     condition == QDBusLockerBase::BeforeRelease ? "before release" :
                     condition == QDBusLockerBase::AfterRelease ? "after release" :
                     "condition unknown")
                 << "in connection" << conn;
        }
    }
#else
    static inline void reportThreadAction(int, int, QDBusConnectionPrivate *) { }
#endif
};

struct QDBusLocker: QDBusLockerBase
{
    QDBusConnectionPrivate *self;
    ThreadAction action;
    inline QDBusLocker(ThreadAction a, QDBusConnectionPrivate *s)
        : self(s), action(a)
    {
        reportThreadAction(action, BeforeLock, self);
        self->lock.lock();
        reportThreadAction(action, AfterLock, self);
    }

    inline ~QDBusLocker()
    {
        reportThreadAction(action, BeforeUnlock, self);
        self->lock.unlock();
        reportThreadAction(action, AfterUnlock, self);
    }
};

struct QDBusMutexLocker: QDBusLockerBase
{
    QDBusConnectionPrivate *self;
    std::recursive_mutex *mutex;
    ThreadAction action;
    inline QDBusMutexLocker(ThreadAction a, QDBusConnectionPrivate *s)
        : self(s), mutex(&s->dispatchLock), action(a)
    {
        reportThreadAction(action, BeforeLock, self);
        mutex->lock();
        reportThreadAction(action, AfterLock, self);
    }

    inline ~QDBusMutexLocker()
    {
        reportThreadAction(action, BeforeUnlock, self);
        mutex->unlock();
        reportThreadAction(action, AfterUnlock, self);
    }
};

#if QDBUS_THREAD_DEBUG
# define SEM_ACQUIRE(action, sem)                                       \
    QDBusLockerBase::reportThreadAction(action, QDBusLockerBase::BeforeAcquire, this); \
    sem.acquire();                                                      \
    QDBusLockerBase::reportThreadAction(action, QDBusLockerBase::AfterAcquire, this);

# define SEM_RELEASE(action, sem)                                       \
    QDBusLockerBase::reportThreadAction(action, QDBusLockerBase::BeforeRelease, that); \
    sem.release();                                                      \
    QDBusLockerBase::reportThreadAction(action, QDBusLockerBase::AfterRelease, that);

#else
# define SEM_ACQUIRE(action, sem)       sem.acquire()
# define SEM_RELEASE(action, sem)       sem.release()
#endif

QT_END_NAMESPACE

#endif
