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

#include "qdbusabstractadaptor.h"
#include "qcoreapplication.h"
#include "qmetaobject.h"
#include "qset.h"
#include "qtimer.h"
#include "qthread.h"
#include "qdbusconnection.h"
#include "qdbusconnection_p.h"  // for qDBusParametersForMethod
#include "qdbusabstractadaptor_p.h"
#include "qdbusmetatype_p.h"


QT_BEGIN_NAMESPACE

static int cachedRelaySlotMethodIndex = -1;

int QDBusAdaptorConnector::relaySlotMethodIndex()
{
    if (cachedRelaySlotMethodIndex == -1) {
        cachedRelaySlotMethodIndex = staticMetaObject.indexOfMethod("relaySlot()");
        Q_ASSERT(cachedRelaySlotMethodIndex != -1);
    }
    return cachedRelaySlotMethodIndex;
}

QDBusAdaptorConnector *qDBusFindAdaptorConnector(QObject *obj)
{
    if (!obj)
        return 0;
    foreach (QObject *it, obj->children()) {
        QDBusAdaptorConnector *connector = qobject_cast<QDBusAdaptorConnector *>(it);
        if (connector) {
            connector->polish();
            return connector;
        }
    }
    return 0;
}

QDBusAdaptorConnector *qDBusCreateAdaptorConnector(QObject *obj)
{
    QDBusAdaptorConnector *connector = qDBusFindAdaptorConnector(obj);
    if (connector)
        return connector;
    return new QDBusAdaptorConnector(obj);
}

QString QDBusAbstractAdaptorPrivate::retrieveIntrospectionXml(QDBusAbstractAdaptor *adaptor)
{
    return adaptor->d_func()->xml;
}

void QDBusAbstractAdaptorPrivate::saveIntrospectionXml(QDBusAbstractAdaptor *adaptor,
                                                       const QString &xml)
{
    adaptor->d_func()->xml = xml;
}

/*!
    \class QDBusAbstractAdaptor
    \inmodule QtDBus
    \since 4.2

    \brief The QDBusAbstractAdaptor class is the base class of D-Bus adaptor classes.

    The QDBusAbstractAdaptor class is the starting point for all objects intending to provide
    interfaces to the external world using D-Bus. This is accomplished by attaching a one or more
    classes derived from QDBusAbstractAdaptor to a normal QObject and then registering that QObject
    with QDBusConnection::registerObject. QDBusAbstractAdaptor objects are intended to be
    light-weight wrappers, mostly just relaying calls into the real object (its parent) and the
    signals from it.

    Each QDBusAbstractAdaptor-derived class should define the D-Bus interface it is implementing
    using the Q_CLASSINFO macro in the class definition. Note that only one interface can be
    exposed in this way.

    QDBusAbstractAdaptor uses the standard QObject mechanism of signals, slots and properties to
    determine what signals, methods and properties to export to the bus. Any signal emitted by
    QDBusAbstractAdaptor-derived classes will be automatically be relayed through any D-Bus
    connections the object is registered on.

    Classes derived from QDBusAbstractAdaptor must be created on the heap using the \a new operator
    and must not be deleted by the user (they will be deleted automatically when the object they are
    connected to is also deleted).

    \sa {usingadaptors.html}{Using adaptors}, QDBusConnection
*/

/*!
    Constructs a QDBusAbstractAdaptor with \a obj as the parent object.
*/
QDBusAbstractAdaptor::QDBusAbstractAdaptor(QObject* obj)
    : QObject(*new QDBusAbstractAdaptorPrivate, obj)
{
    QDBusAdaptorConnector *connector = qDBusCreateAdaptorConnector(obj);

    connector->waitingForPolish = true;
    QMetaObject::invokeMethod(connector, "polish", Qt::QueuedConnection);
}

/*!
    Destroys the adaptor.

    \warning Adaptors are destroyed automatically when the real object they refer to is
             destroyed. Do not delete the adaptors yourself.
*/
QDBusAbstractAdaptor::~QDBusAbstractAdaptor()
{
}

/*!
    Toggles automatic signal relaying from the real object (see object()).

    Automatic signal relaying consists of signal-to-signal connection of the signals on the parent
    that have the exact same method signatue in both classes.

    If \a enable is set to true, connect the signals; if set to false, disconnect all signals.
*/
void QDBusAbstractAdaptor::setAutoRelaySignals(bool enable)
{
    const QMetaObject *us = metaObject();
    const QMetaObject *them = parent()->metaObject();
    bool connected = false;
    for (int idx = staticMetaObject.methodCount(); idx < us->methodCount(); ++idx) {
        QMetaMethod mm = us->method(idx);

        if (mm.methodType() != QMetaMethod::Signal)
            continue;

        // try to connect/disconnect to a signal on the parent that has the same method signature
        QByteArray sig = QMetaObject::normalizedSignature(mm.signature());
        if (them->indexOfSignal(sig) == -1)
            continue;
        sig.prepend(QSIGNAL_CODE + '0');
        parent()->disconnect(sig, this, sig);
        if (enable)
            connected = connect(parent(), sig, sig) || connected;
    }
    d_func()->autoRelaySignals = connected;
}

/*!
    Returns true if automatic signal relaying from the real object (see object()) is enabled,
    otherwiser returns false.

    \sa setAutoRelaySignals()
*/
bool QDBusAbstractAdaptor::autoRelaySignals() const
{
    return d_func()->autoRelaySignals;
}

QDBusAdaptorConnector::QDBusAdaptorConnector(QObject *obj)
    : QObject(obj), waitingForPolish(false)
{
}

QDBusAdaptorConnector::~QDBusAdaptorConnector()
{
}

void QDBusAdaptorConnector::addAdaptor(QDBusAbstractAdaptor *adaptor)
{
    // find the interface name
    const QMetaObject *mo = adaptor->metaObject();
    int ciid = mo->indexOfClassInfo(QCLASSINFO_DBUS_INTERFACE);
    if (ciid != -1) {
        QMetaClassInfo mci = mo->classInfo(ciid);
        if (*mci.value()) {
            // find out if this interface exists first
            const char *interface = mci.value();
            AdaptorMap::Iterator it = qLowerBound(adaptors.begin(), adaptors.end(),
                                                  QByteArray(interface));
            if (it != adaptors.end() && qstrcmp(interface, it->interface) == 0) {
                // exists. Replace it (though it's probably the same)
                if (it->adaptor != adaptor) {
                    // reconnect the signals
                    disconnectAllSignals(it->adaptor);
                    connectAllSignals(adaptor);
                }
                it->adaptor = adaptor;
            } else {
                // create a new one
                AdaptorData entry;
                entry.interface = interface;
                entry.adaptor = adaptor;
                adaptors << entry;

                // connect the adaptor's signals to our relaySlot slot
                connectAllSignals(adaptor);
            }
        }
    }
}

void QDBusAdaptorConnector::disconnectAllSignals(QObject *obj)
{
    QMetaObject::disconnect(obj, -1, this, relaySlotMethodIndex());
}

void QDBusAdaptorConnector::connectAllSignals(QObject *obj)
{
    QMetaObject::connect(obj, -1, this, relaySlotMethodIndex(), Qt::DirectConnection);
}

void QDBusAdaptorConnector::polish()
{
    if (!waitingForPolish)
        return;                 // avoid working multiple times if multiple adaptors were added

    waitingForPolish = false;
    foreach (QObject *obj, parent()->children()) {
        QDBusAbstractAdaptor *adaptor = qobject_cast<QDBusAbstractAdaptor *>(obj);
        if (adaptor)
            addAdaptor(adaptor);
    }

    // sort the adaptor list
    qSort(adaptors);
}

void QDBusAdaptorConnector::relaySlot(void **argv)
{
    QObjectPrivate *d = static_cast<QObjectPrivate *>(d_ptr);
    if (Q_LIKELY(d->currentSender)) {
        relay(d->currentSender->sender, d->currentSender->signal, argv);
    } else {
        qWarning("QtDBus: cannot relay signals from parent %s(%p \"%s\") unless they are emitted in the object's thread %s(%p \"%s\"). "
                 "Current thread is %s(%p \"%s\").",
                 parent()->metaObject()->className(), parent(), qPrintable(parent()->objectName()),
                 parent()->thread()->metaObject()->className(), parent()->thread(), qPrintable(parent()->thread()->objectName()),
                 QThread::currentThread()->metaObject()->className(), QThread::currentThread(), qPrintable(QThread::currentThread()->objectName()));
    }
}

void QDBusAdaptorConnector::relay(QObject *senderObj, int lastSignalIdx, void **argv)
{
    if (lastSignalIdx < QObject::staticMetaObject.methodCount())
        // QObject signal (destroyed(QObject *)) -- ignore
        return;

    const QMetaObject *senderMetaObject = senderObj->metaObject();
    QMetaMethod mm = senderMetaObject->method(lastSignalIdx);

    QObject *realObject = senderObj;
    if (qobject_cast<QDBusAbstractAdaptor *>(senderObj))
        // it's an adaptor, so the real object is in fact its parent
        realObject = realObject->parent();

    // break down the parameter list
    QList<int> types;
    int inputCount = qDBusParametersForMethod(mm, types);
    if (inputCount == -1)
        // invalid signal signature
        // qDBusParametersForMethod has already complained
        return;
    if (Q_UNLIKELY(inputCount + 1 != types.count() ||
        types.at(inputCount) == QDBusMetaTypeId::message)) {
        // invalid signal signature
        // qDBusParametersForMethod has not yet complained about this one
        qWarning("QDBusAbstractAdaptor: Cannot relay signal %s::%s",
                 senderMetaObject->className(), mm.signature());
        return;
    }

    QVariantList args;
    for (int i = 1; i < types.count(); ++i)
        args << QVariant(types.at(i), argv[i]);

    // now emit the signal with all the information
    emit relaySignal(realObject, senderMetaObject, lastSignalIdx, args);
}

// our Meta Object
// modify carefully: this has been hand-edited!
// the relaySlot slot gets called with the void** array

static const uint qt_meta_data_QDBusAdaptorConnector[] = {

 // content:
       6,       // revision
       0,       // classname
       0,    0, // classinfo
       3,   14, // methods
       0,    0, // properties
       0,    0, // enums/sets
       0,    0, // constructors
       0,       // flags
       1,       // signalCount

 // signals: signature, parameters, type, tag, flags
      47,   23,   22,   22, 0x05,

 // slots: signature, parameters, type, tag, flags
     105,   22,   22,   22, 0x0a,
     117,   22,   22,   22, 0x0a,

       0        // eod
};

static const char qt_meta_stringdata_QDBusAdaptorConnector[] = {
    "QDBusAdaptorConnector\0\0obj,metaObject,sid,args\0"
    "relaySignal(QObject*,const QMetaObject*,int,QVariantList)\0"
    "relaySlot()\0polish()\0"
};

void QDBusAdaptorConnector::qt_static_metacall(QObject *_o, QMetaObject::Call _c, int _id, void **_a)
{
    if (_c == QMetaObject::InvokeMetaMethod) {
        Q_ASSERT(staticMetaObject.cast(_o));
        QDBusAdaptorConnector *_t = static_cast<QDBusAdaptorConnector *>(_o);
        switch (_id) {
        case 0: _t->relaySignal((*reinterpret_cast< QObject*(*)>(_a[1])),(*reinterpret_cast< const QMetaObject*(*)>(_a[2])),(*reinterpret_cast< int(*)>(_a[3])),(*reinterpret_cast< const QVariantList(*)>(_a[4]))); break;
        case 1: _t->relaySlot(_a); break; // HAND EDIT: add the _a parameter
        case 2: _t->polish(); break;
        default: break;
        }
    }
}

const QMetaObject QDBusAdaptorConnector::staticMetaObject = {
    { &QObject::staticMetaObject, qt_meta_stringdata_QDBusAdaptorConnector,
      qt_meta_data_QDBusAdaptorConnector, qt_static_metacall, nullptr }
};

#ifdef Q_NO_DATA_RELOCATION
const QMetaObject &QDBusAdaptorConnector::getStaticMetaObject() { return staticMetaObject; }
#endif //Q_NO_DATA_RELOCATION

const QMetaObject *QDBusAdaptorConnector::metaObject() const
{
    return QObject::d_ptr->metaObject ? QObject::d_ptr->metaObject : &staticMetaObject;
}

void *QDBusAdaptorConnector::qt_metacast(const char *_clname)
{
    if (!_clname) return nullptr;
    if (strcmp(_clname, qt_meta_stringdata_QDBusAdaptorConnector) == 0)
        return static_cast<void*>(const_cast< QDBusAdaptorConnector*>(this));
    return QObject::qt_metacast(_clname);
}

int QDBusAdaptorConnector::qt_metacall(QMetaObject::Call _c, int _id, void **_a)
{
    _id = QObject::qt_metacall(_c, _id, _a);
    if (_id < 0)
        return _id;
    if (_c == QMetaObject::InvokeMetaMethod) {
        if (_id < 3)
            qt_static_metacall(this, _c, _id, _a);
        _id -= 3;
    }
    return _id;
}

// SIGNAL 0
void QDBusAdaptorConnector::relaySignal(QObject * _t1, const QMetaObject * _t2, int _t3, const QVariantList & _t4)
{
    void *_a[] = { nullptr, const_cast<void*>(reinterpret_cast<const void*>(&_t1)), const_cast<void*>(reinterpret_cast<const void*>(&_t2)), const_cast<void*>(reinterpret_cast<const void*>(&_t3)), const_cast<void*>(reinterpret_cast<const void*>(&_t4)) };
    QMetaObject::activate(this, &staticMetaObject, 0, _a);
}

QT_END_NAMESPACE


#include "moc_qdbusabstractadaptor.h"

