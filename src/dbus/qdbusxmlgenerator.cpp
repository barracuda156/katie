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

#include <QtCore/qmetaobject.h>
#include <QtCore/qstringlist.h>

#include "qdbusinterface_p.h"   // for ANNOTATION_NO_WAIT
#include "qdbusabstractadaptor_p.h" // for QCLASSINFO_DBUS_*
#include "qdbusconnection_p.h"  // for the flags
#include "qdbusmetatype_p.h"
#include "qdbusmetatype.h"
#include "qdbusutil_p.h"


QT_BEGIN_NAMESPACE

extern Q_DBUS_EXPORT QString qDBusGenerateMetaObjectXml(QString interface, const QMetaObject *mo,
                                                       const QMetaObject *base, int flags);

// implement the D-Bus org.freedesktop.DBus.Introspectable interface
// we do that by analysing the metaObject of all the adaptor interfaces

static QString generateInterfaceXml(const QMetaObject *mo, int flags, int methodOffset, int propOffset)
{
    QString retval;

    // start with properties:
    if (flags & (QDBusConnection::ExportScriptableProperties |
                 QDBusConnection::ExportNonScriptableProperties)) {
        for (int i = propOffset; i < mo->propertyCount(); ++i) {
            static const char *accessvalues[] = {0, "read", "write", "readwrite"};

            QMetaProperty mp = mo->property(i);

            if (!((mp.isScriptable() && (flags & QDBusConnection::ExportScriptableProperties)) ||
                  (!mp.isScriptable() && (flags & QDBusConnection::ExportNonScriptableProperties))))
                continue;

            int access = 0;
            if (mp.isReadable())
                access |= 1;
            if (mp.isWritable())
                access |= 2;

            int typeId = QMetaType::type(mp.typeName());
            if (!typeId)
                continue;
            const char *signature = QDBusMetaType::typeToSignature(typeId);
            if (!signature)
                continue;

            retval += QString::fromLatin1("    <property name=\"%1\" type=\"%2\" access=\"%3\"")
                      .arg(QString::fromLatin1(mp.name()))
                      .arg(QString::fromLatin1(signature))
                      .arg(QString::fromLatin1(accessvalues[access]));

            if (QDBusMetaType::signatureToType(signature) == QVariant::Invalid) {
                const char *typeName = QVariant::typeToName(QVariant::Type(typeId));
                retval += QString::fromLatin1(">\n      <annotation name=\"org.qtproject.QtDBus.QtTypeName\" value=\"%3\"/>\n    </property>\n")
                          .arg(Qt::escape(QString::fromLatin1(typeName)));
            } else {
                retval += QLatin1String("/>\n");
            }
        }
    }

    // now add methods:
    for (int i = methodOffset; i < mo->methodCount(); ++i) {
        QMetaMethod mm = mo->method(i);
        QByteArray signature = mm.signature();
        int paren = signature.indexOf('(');

        bool isSignal;
        if (mm.methodType() == QMetaMethod::Signal)
            // adding a signal
            isSignal = true;
        else if (mm.access() == QMetaMethod::Public && (mm.methodType() == QMetaMethod::Slot || mm.methodType() == QMetaMethod::Method))
            isSignal = false;
        else
            continue;           // neither signal nor public slot

        if (isSignal && !(flags & (QDBusConnection::ExportScriptableSignals |
                                   QDBusConnection::ExportNonScriptableSignals)))
            continue;           // we're not exporting any signals
        if (!isSignal && (!(flags & (QDBusConnection::ExportScriptableSlots | QDBusConnection::ExportNonScriptableSlots)) &&
                          !(flags & (QDBusConnection::ExportScriptableInvokables | QDBusConnection::ExportNonScriptableInvokables))))
            continue;           // we're not exporting any slots or invokables

        QString xml = QString::fromLatin1("    <%1 name=\"%2\">\n")
                      .arg(isSignal ? QLatin1String("signal") : QLatin1String("method"))
                      .arg(QString::fromLatin1(signature.left(paren)));

        // check the return type first
        int typeId = QMetaType::type(mm.typeName());
        if (typeId) {
            const char *typeName = QDBusMetaType::typeToSignature(typeId);
            if (typeName) {
                xml += QString::fromLatin1("      <arg type=\"%1\" direction=\"out\"/>\n")
                       .arg(Qt::escape(QString::fromLatin1(typeName)));

                // do we need to describe this argument?
                if (QDBusMetaType::signatureToType(typeName) == QVariant::Invalid)
                    xml += QString::fromLatin1("      <annotation name=\"org.qtproject.QtDBus.QtTypeName.Out0\" value=\"%1\"/>\n")
                        .arg(Qt::escape(QString::fromLatin1(QVariant::typeToName(QVariant::Type(typeId)))));
            } else
                continue;
        }
        else if (*mm.typeName())
            continue;           // wasn't a valid type

        QList<QByteArray> names = mm.parameterNames();
        QList<int> types;
        int inputCount = qDBusParametersForMethod(mm, types);
        if (inputCount == -1)
            continue;           // invalid form
        if (isSignal && inputCount + 1 != types.count())
            continue;           // signal with output arguments?
        if (isSignal && types.at(inputCount) == QDBusMetaTypeId::message)
            continue;           // signal with QDBusMessage argument?
        if (isSignal && mm.attributes() & QMetaMethod::Cloned)
            continue;           // cloned signal?

        int j;
        bool isScriptable = mm.attributes() & QMetaMethod::Scriptable;
        for (j = 1; j < types.count(); ++j) {
            // input parameter for a slot or output for a signal
            if (types.at(j) == QDBusMetaTypeId::message) {
                isScriptable = true;
                continue;
            }

            QString name;
            if (!names.at(j - 1).isEmpty())
                name = QString::fromLatin1("name=\"%1\" ").arg(QString::fromLatin1(names.at(j - 1)));

            bool isOutput = isSignal || j > inputCount;

            const char *signature = QDBusMetaType::typeToSignature(types.at(j));
            xml += QString::fromLatin1("      <arg %1type=\"%2\" direction=\"%3\"/>\n")
                   .arg(name)
                   .arg(QString::fromLatin1(signature))
                   .arg(isOutput ? QLatin1String("out") : QLatin1String("in"));

            // do we need to describe this argument?
            if (QDBusMetaType::signatureToType(signature) == QVariant::Invalid) {
                const char *typeName = QVariant::typeToName( QVariant::Type(types.at(j)) );
                xml += QString::fromLatin1("      <annotation name=\"org.qtproject.QtDBus.QtTypeName.%1%2\" value=\"%3\"/>\n")
                       .arg(isOutput ? QLatin1String("Out") : QLatin1String("In"))
                       .arg(isOutput && !isSignal ? j - inputCount : j - 1)
                       .arg(Qt::escape(QString::fromLatin1(typeName)));
            }
        }

        int wantedMask;
        if (isScriptable)
            wantedMask = isSignal ? QDBusConnection::ExportScriptableSignals
                                  : QDBusConnection::ExportScriptableSlots;
        else
            wantedMask = isSignal ? QDBusConnection::ExportNonScriptableSignals
                                  : QDBusConnection::ExportNonScriptableSlots;
        if ((flags & wantedMask) != wantedMask)
            continue;

        retval += xml + QString::fromLatin1("    </%1>\n")
                        .arg(isSignal ? QLatin1String("signal") : QLatin1String("method"));
    }

    return retval;
}

QString qDBusGenerateMetaObjectXml(QString interface, const QMetaObject *mo,
                                   const QMetaObject *base, int flags)
{
    if (interface.isEmpty())
        // generate the interface name from the meta object
        interface = qDBusInterfaceFromMetaObject(mo);

    int idx = mo->indexOfClassInfo(QCLASSINFO_DBUS_INTROSPECTION);
    if (idx >= mo->classInfoOffset())
        return QString::fromUtf8(mo->classInfo(idx).value());

    QString xml = generateInterfaceXml(mo, flags, base->methodCount(), base->propertyCount());

    if (xml.isEmpty())
        return QString();       // don't add an empty interface
    return QString::fromLatin1("  <interface name=\"%1\">\n%2  </interface>\n")
        .arg(interface, xml);
}

QT_END_NAMESPACE

