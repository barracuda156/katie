/****************************************************************************
**
** Copyright (C) 2015 The Qt Company Ltd.
** Copyright (C) 2016-2021 Ivailo Monev
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

#include <QtCore/qglobal.h>


QT_BEGIN_NAMESPACE

#if !defined(QDBUS_THREAD_DEBUG) && defined(QT_BUILD_INTERNAL)
# define QDBUS_THREAD_DEBUG 1
#else
# define QDBUS_THREAD_DEBUG 0
#endif

#if QDBUS_THREAD_DEBUG
typedef void (*qdbusThreadDebugFunc)(int, int, QDBusConnectionPrivate *);
Q_DBUS_EXPORT void qdbusDefaultThreadDebug(int, int, QDBusConnectionPrivate *);
extern Q_DBUS_EXPORT qdbusThreadDebugFunc qdbusThreadDebug;
#endif

enum ThreadAction {
    ConnectAction = 0,
    DisconnectAction = 1,
    RegisterObjectAction = 2,
    UnregisterObjectAction = 3,
    ObjectRegisteredAtAction = 4,

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

#if QDBUS_THREAD_DEBUG
    static inline void reportThreadAction(int action, int condition, QDBusConnectionPrivate *ptr)
    { if (qdbusThreadDebug) qdbusThreadDebug(action, condition, ptr); }
#else
    static inline void reportThreadAction(int, int, QDBusConnectionPrivate *) { }
#endif
};

struct QDBusReadLocker: QDBusLockerBase
{
    QDBusConnectionPrivate *self;
    ThreadAction action;
    inline QDBusReadLocker(ThreadAction a, QDBusConnectionPrivate *s)
        : self(s), action(a)
    {
        reportThreadAction(action, BeforeLock, self);
        self->lock.lockForRead();
        reportThreadAction(action, AfterLock, self);
    }

    inline ~QDBusReadLocker()
    {
        reportThreadAction(action, BeforeUnlock, self);
        self->lock.unlock();
        reportThreadAction(action, AfterUnlock, self);
    }
};

struct QDBusWriteLocker: QDBusLockerBase
{
    QDBusConnectionPrivate *self;
    ThreadAction action;
    inline QDBusWriteLocker(ThreadAction a, QDBusConnectionPrivate *s)
        : self(s), action(a)
    {
        reportThreadAction(action, BeforeLock, self);
        self->lock.lockForWrite();
        reportThreadAction(action, AfterLock, self);
    }

    inline ~QDBusWriteLocker()
    {
        reportThreadAction(action, BeforeUnlock, self);
        self->lock.unlock();
        reportThreadAction(action, AfterUnlock, self);
    }
};

struct QDBusMutexLocker: QDBusLockerBase
{
    QDBusConnectionPrivate *self;
    QMutex *mutex;
    ThreadAction action;
    inline QDBusMutexLocker(ThreadAction a, QDBusConnectionPrivate *s,
                            QMutex *m)
        : self(s), mutex(m), action(a)
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

struct QDBusDispatchLocker: QDBusMutexLocker
{
    inline QDBusDispatchLocker(ThreadAction a, QDBusConnectionPrivate *s)
        : QDBusMutexLocker(a, s, &s->dispatchLock)
    { }
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
