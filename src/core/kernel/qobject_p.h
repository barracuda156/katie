/****************************************************************************
**
** Copyright (C) 2015 The Qt Company Ltd.
** Copyright (C) 2016-2020 Ivailo Monev
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

#ifndef QOBJECT_P_H
#define QOBJECT_P_H

//
//  W A R N I N G
//  -------------
//
// This file is not part of the Katie API.  It exists for the convenience
// of qapplication_*.cpp, qwidget*.cpp and qfiledialog.cpp.  This header
// file may change from version to version without notice, or even be removed.
//
// We mean it.
//

#include "QtCore/qpointer.h"
#include "QtCore/qsharedpointer.h"
#include "QtCore/qcoreevent.h"
#include "QtCore/qreadwritelock.h"
#include "QtCore/qmetaobject.h"
#include "QtCore/qvarlengtharray.h"

QT_BEGIN_NAMESPACE

class QVariant;
class QThreadData;
class QObjectConnectionListVector;
namespace QtSharedPointer { struct ExternalRefCountData; }

/* used in QtTestLib, DON'T CHANGE without prior warning */
struct QSignalSpyCallbackSet
{
    typedef void (*BeginCallback)(QObject *caller, int method_index, void **argv);
    typedef void (*EndCallback)(QObject *caller, int method_index);
    BeginCallback signal_begin_callback,
                    slot_begin_callback;
    EndCallback signal_end_callback;
};
void Q_CORE_EXPORT qt_register_signal_spy_callbacks(const QSignalSpyCallbackSet &callback_set);

extern QSignalSpyCallbackSet Q_CORE_EXPORT qt_signal_spy_callback_set;

enum { QObjectPrivateVersion = QT_VERSION };

class Q_CORE_EXPORT QAbstractDeclarativeData
{
public:
    static void (*destroyed)(QAbstractDeclarativeData *, QObject *);
    static void (*parentChanged)(QAbstractDeclarativeData *, QObject *, QObject *);
    static void (*objectNameChanged)(QAbstractDeclarativeData *, QObject *);
};

class Q_CORE_EXPORT QObjectPrivate : public QObjectData
{
    Q_DECLARE_PUBLIC(QObject)

public:
    struct ExtraData
    {
        ExtraData() {}
        QList<QByteArray> propertyNames;
        QList<QVariant> propertyValues;
    };

    typedef void (*StaticMetaCallFunction)(QObject *, QMetaObject::Call, int, void **);
    struct Connection
    {
        QObject *sender;
        QObject *receiver;
        StaticMetaCallFunction callFunction;
        // The next pointer for the singly-linked ConnectionList
        Connection *nextConnectionList;
        //senders linked list
        Connection *next;
        Connection **prev;
        QAtomicPointer<int> argumentTypes;
        ushort method_offset;
        ushort method_relative;
        ushort connectionType : 3; // 0 == auto, 1 == direct, 2 == queued, 4 == blocking
        ~Connection();
        int method() const { return method_offset + method_relative; }
    };
    // ConnectionList is a singly-linked list
    struct ConnectionList {
        ConnectionList() : first(Q_NULLPTR), last(Q_NULLPTR) {}
        Connection *first;
        Connection *last;
    };

    struct Sender
    {
        QObject *sender;
        int signal;
        QAtomicInt ref;
    };


    QObjectPrivate(int version = QObjectPrivateVersion);
    virtual ~QObjectPrivate();
    void deleteChildren();

    void setParent_helper(QObject *);
    void moveToThread_helper();
    void setThreadData_helper(QThreadData *currentData, QThreadData *targetData);
    void _q_reregisterTimers(void *pointer);

    bool isSender(const QObject *receiver, const char *signal) const;
    QObjectList receiverList(const char *signal) const;
    QObjectList senderList() const;

    void addConnection(int signal, Connection *c);
    void cleanConnectionLists();


    static inline Sender *setCurrentSender(QObject *receiver,
                                    Sender *sender);
    static inline void resetCurrentSender(QObject *receiver,
                                   Sender *currentSender,
                                   Sender *previousSender);

    static QObjectPrivate *get(QObject *o) {
        return o->d_func();
    }

    int signalIndex(const char *signalName) const;
    inline bool isSignalConnected(uint signalIdx) const;

    // To allow arbitrary objects to call connectNotify()/disconnectNotify() without making
    // the API public in QObject. This is used by QDeclarativeNotifierEndpoint.
    inline void connectNotify(const char *signal);
    inline void disconnectNotify(const char *signal);

    static inline void signalSignature(const QMetaMethod &signal,
                                       QVarLengthArray<char> *result);

public:
    QString objectName;
    ExtraData *extraData;    // extra data set by the user
    QThreadData *threadData; // id of the thread that owns the object

    QObjectConnectionListVector *connectionLists;

    Connection *senders;     // linked list of connections connected to this object
    Sender *currentSender;   // object currently activating the object
    mutable quint32 connectedSignals[2];

    QList<QPointer<QObject> > eventFilters;
    union {
        QObject *currentChildBeingDeleted;
        QAbstractDeclarativeData *declarativeData; //extra data used by the declarative module
    };

    // these objects are all used to indicate that a QObject was deleted
    // plus QPointer, which keeps a separate list
    QAtomicPointer<QtSharedPointer::ExternalRefCountData> sharedRefcount;
};


/*! \internal

  Returns true if the signal with index \a signal_index from object \a sender is connected.
  Signals with indices above a certain range are always considered connected (see connectedSignals
  in QObjectPrivate). If a signal spy is installed, all signals are considered connected.

  \a signal_index must be the index returned by QObjectPrivate::signalIndex;
*/
inline bool QObjectPrivate::isSignalConnected(uint signal_index) const
{
    return signal_index >= sizeof(connectedSignals) * 8
        || (connectedSignals[signal_index >> 5] & (1 << (signal_index & 0x1f))
        || qt_signal_spy_callback_set.signal_begin_callback
        || qt_signal_spy_callback_set.signal_end_callback);
}

inline void QObjectPrivate::connectNotify(const char *signal)
{
    q_ptr->connectNotify(signal);
}

inline void QObjectPrivate::disconnectNotify(const char *signal)
{
    q_ptr->disconnectNotify(signal);
}

inline void QObjectPrivate::signalSignature(const QMetaMethod &signal,
                                                  QVarLengthArray<char> *result)
{
    Q_ASSERT(result);
    const int signatureLength = qstrlen(signal.signature());
    if (signatureLength == 0) {
        result->append((char)0);
        return;
    }
    result->reserve(signatureLength + 2);
    result->append((char)(QSIGNAL_CODE + '0'));
    result->append(signal.signature(), signatureLength + 1);
}

inline QObjectPrivate::Sender *QObjectPrivate::setCurrentSender(QObject *receiver,
                                                         Sender *sender)
{
    Sender *previousSender = receiver->d_func()->currentSender;
    receiver->d_func()->currentSender = sender;
    return previousSender;
}

inline void QObjectPrivate::resetCurrentSender(QObject *receiver,
                                        Sender *currentSender,
                                        Sender *previousSender)
{
    // ref is set to zero when this object is deleted during the metacall
    if (currentSender->ref == 1)
        receiver->d_func()->currentSender = previousSender;
    // if we've recursed, we need to tell the caller about the objects deletion
    if (previousSender)
        previousSender->ref = currentSender->ref;
}


Q_DECLARE_TYPEINFO(QObjectPrivate::Connection, Q_MOVABLE_TYPE);
Q_DECLARE_TYPEINFO(QObjectPrivate::Sender, Q_MOVABLE_TYPE);

class QSemaphore;
class Q_CORE_EXPORT QMetaCallEvent : public QEvent
{
public:
    QMetaCallEvent(ushort method_offset, ushort method_relative,
                   QObjectPrivate::StaticMetaCallFunction callFunction,
                   const QObject *sender, int signalId,
                   int nargs = 0, int *types = Q_NULLPTR, void **args = Q_NULLPTR,
                   QSemaphore *semaphore = Q_NULLPTR);
    ~QMetaCallEvent();

    inline int id() const { return method_offset_ + method_relative_; }
    inline const QObject *sender() const { return sender_; }
    inline int signalId() const { return signalId_; }
    inline void **args() const { return args_; }

    virtual void placeMetaCall(QObject *object);

private:
    const QObject *sender_;
    int signalId_;
    int nargs_;
    int *types_;
    void **args_;
    QSemaphore *semaphore_;
    QObjectPrivate::StaticMetaCallFunction callFunction_;
    ushort method_offset_;
    ushort method_relative_;
};

class QBoolBlocker
{
public:
    inline QBoolBlocker(bool &b, bool value=true):block(b), reset(b){block = value;}
    inline ~QBoolBlocker(){block = reset; }
private:
    bool &block;
    bool reset;
};

struct Q_CORE_EXPORT QAbstractDynamicMetaObject : public QMetaObject
{
    virtual ~QAbstractDynamicMetaObject() {}
    virtual int metaCall(QMetaObject::Call, int _id, void **) { return _id; }
    virtual int createProperty(const char *, const char *) { return -1; }
};

QT_END_NAMESPACE

#endif // QOBJECT_P_H
