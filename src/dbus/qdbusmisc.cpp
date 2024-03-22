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

#include <string.h>

#include <QtCore/qcoreapplication.h>
#include <QtCore/qvariant.h>
#include <QtCore/qmetaobject.h>

#include "qdbusutil_p.h"
#include "qdbusconnection_p.h"
#include "qdbusmetatype_p.h"
#include "qdbusabstractadaptor_p.h" // for QCLASSINFO_DBUS_*


QT_BEGIN_NAMESPACE

QString qDBusInterfaceFromMetaObject(const QMetaObject *mo)
{
    QString interface;

    int idx = mo->indexOfClassInfo(QCLASSINFO_DBUS_INTERFACE);
    if (idx >= mo->classInfoOffset()) {
        interface = QString::fromLatin1(mo->classInfo(idx).value());
    } else {
        interface = QString::fromLatin1(mo->className());
        interface.replace(QLatin1String("::"), QLatin1String("."));

        if (interface.startsWith(QLatin1String("QDBus"))) {
            interface.prepend(QLatin1String("org.qtproject.QtDBus."));
        } else if (interface.startsWith(QLatin1Char('Q')) &&
                   interface.length() >= 2 && interface.at(1).isUpper()) {
            // assume it's Qt
            interface.prepend(QLatin1String("org.qtproject.Qt."));
        } else if (!QCoreApplication::instance()||
                   QCoreApplication::instance()->applicationName().isEmpty()) {
            interface.prepend(QLatin1String("local."));
         } else {
            interface.prepend(QLatin1Char('.')).prepend(QCoreApplication::instance()->applicationName());
            QStringList domainName =
                QCoreApplication::instance()->organizationDomain().split(QLatin1Char('.'),
                                                                         QString::SkipEmptyParts);
            if (domainName.isEmpty())
                 interface.prepend(QLatin1String("local."));
            else
                for (int i = 0; i < domainName.count(); ++i)
                    interface.prepend(QLatin1Char('.')).prepend(domainName.at(i));
         }
     }

    return interface;
}

bool qDBusInterfaceInObject(QObject *obj, const QString &interface_name)
{
    const QMetaObject *mo = obj->metaObject();
    for ( ; mo != &QObject::staticMetaObject; mo = mo->superClass())
        if (interface_name == qDBusInterfaceFromMetaObject(mo))
            return true;
    return false;
}

// calculates the metatypes for the method
// the slot must have the parameters in the following form:
//  - zero or more value or const-ref parameters of any kind
//  - zero or one const ref of QDBusMessage
//  - zero or more non-const ref parameters
// No parameter may be a template.
// this function returns -1 if the parameters don't match the above form
// this function returns the number of *input* parameters, including the QDBusMessage one if any
// this function does not check the return type, so metaTypes[0] is always 0 and always present
// metaTypes.count() >= retval + 1 in all cases
//
// sig must be the normalised signature for the method
int qDBusParametersForMethod(const QMetaMethod &mm, QList<int>& metaTypes)
{
    metaTypes.clear();
    metaTypes.append(0);        // return type
    int inputCount = 0;
    bool seenMessage = false;
    foreach (const QByteArray &type, mm.parameterTypes()) {
        if (type.endsWith('*')) {
            //qWarning("Could not parse the method '%s'", mm.signature());
            // pointer?
            return -1;
        }

        if (type.endsWith('&')) {
            QByteArray basictype = type;
            basictype.truncate(type.length() - 1);

            int id = QMetaType::type(basictype);
            if (id == 0) {
                //qWarning("Could not parse the method '%s'", mm.signature());
                // invalid type in method parameter list
                return -1;
            } else if (QDBusMetaType::typeToSignature(id) == 0)
                return -1;

            metaTypes.append( id );
            seenMessage = true; // it cannot appear anymore anyways
            continue;
        }

        if (seenMessage) {      // && !type.endsWith('&')
            //qWarning("Could not parse the method '%s'", mm.signature());
            // non-output parameters after message or after output params
            return -1;          // not allowed
        }

        int id = QMetaType::type(type);
        if (id == 0) {
            //qWarning("Could not parse the method '%s'", mm.signature());
            // invalid type in method parameter list
            return -1;
        }

        if (id == QDBusMetaTypeId::message)
            seenMessage = true;
        else if (QDBusMetaType::typeToSignature(id) == 0)
            return -1;

        metaTypes.append(id);
        ++inputCount;
    }

    return inputCount;
}

QT_END_NAMESPACE

