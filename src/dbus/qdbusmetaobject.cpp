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

#include "qdbusmetaobject_p.h"
#include "qbytearray.h"
#include "qhash.h"
#include "qstring.h"
#include "qdbusutil_p.h"
#include "qdbuserror.h"
#include "qdbusmetatype.h"
#include "qdbusargument.h"
#include "qdbusintrospection_p.h"
#include "qdbusabstractinterface_p.h"
#include "qmetaobject_p.h"
#include "qstdcontainers_p.h"


QT_BEGIN_NAMESPACE

// must be kept in sync with the moc
static const int qmetaobjectrevision = 6;

class QDBusMetaObjectGenerator
{
public:
    QDBusMetaObjectGenerator(const QString &interface,
                             const QDBusIntrospection::Interface *parsedData);
    void write(QDBusMetaObject *obj);

private:
    struct Method {
        QByteArray parameters;
        QByteArray typeName;
        QByteArray name;
        QByteArray inputSignature;
        QByteArray outputSignature;
        QStdVector<int> inputTypes;
        QStdVector<int> outputTypes;
        int flags;
    };
    
    struct Property {
        QByteArray typeName;
        QByteArray signature;
        int type;
        int flags;
    };
    struct Type {
        int id;
        QByteArray name;
    };

    QMap<QByteArray, Method> signals_;
    QMap<QByteArray, Method> methods;
    QMap<QByteArray, Property> properties;
    
    const QDBusIntrospection::Interface *data;
    QString interface;

    Type findType(const QByteArray &signature,
                  const QDBusIntrospection::Annotations &annotations,
                  const char *direction = "Out", int id = -1);
    
    void parseMethods();
    void parseSignals();
    void parseProperties();
};

static const int intsPerProperty = 2;
static const int intsPerMethod = 5;

struct QDBusMetaObjectPrivate : public QMetaObjectPrivate
{
    int propertyDBusData;
    int methodDBusData;
};

QDBusMetaObjectGenerator::QDBusMetaObjectGenerator(const QString &interfaceName,
                                                   const QDBusIntrospection::Interface *parsedData)
    : data(parsedData), interface(interfaceName)
{
    if (data) {
        parseProperties();
        parseSignals();             // call parseSignals first so that slots override signals
        parseMethods();
    }
}

Q_DBUS_EXPORT bool qt_dbus_metaobject_skip_annotations = false;

QDBusMetaObjectGenerator::Type
QDBusMetaObjectGenerator::findType(const QByteArray &signature,
                                   const QDBusIntrospection::Annotations &annotations,
                                   const char *direction, int id)
{
    Type result;
    result.id = QVariant::Invalid;

    int type = QDBusMetaType::signatureToType(signature);
    if (type == QVariant::Invalid && !qt_dbus_metaobject_skip_annotations) {
        // it's not a type normally handled by our meta type system
        // it must contain an annotation
        QString annotationName = QString::fromLatin1("org.qtproject.QtDBus.QtTypeName");
        if (id >= 0)
            annotationName += QString::fromLatin1(".%1%2")
                              .arg(QLatin1String(direction))
                              .arg(id);

        // extract from annotations:
        QByteArray typeName = annotations.value(annotationName).toLatin1();

        if (!typeName.isEmpty()) {
            // type name found
            type = QVariant::nameToType(typeName);
            if (type == QVariant::UserType)
                type = QMetaType::type(typeName);
        }

        if (type == QVariant::Invalid || signature != QDBusMetaType::typeToSignature(type)) {
            // type is still unknown or doesn't match back to the signature that it
            // was expected to, so synthesize a fake type
            type = QMetaType::VoidStar;
            typeName = "QDBusRawType<0x" + signature.toHex() + ">*";
        }

        result.name = typeName;
    } else if (type == QVariant::Invalid) {
        // this case is used only by the qdbus command-line tool
        // invalid, let's create an impossible type that contains the signature

        if (signature == "av") {
            result.name = "QVariantList";
            type = QVariant::List;
        } else if (signature == "a{sv}") {
            result.name = "QVariantMap";
            type = QVariant::Map;
        } else {
            result.name = "QDBusRawType::" + signature;
            type = -1;
        }
    } else {
        result.name = QVariant::typeToName( QVariant::Type(type) );
    }

    result.id = type;
    return result;              // success
}

void QDBusMetaObjectGenerator::parseMethods()
{
    //
    // TODO:
    //  Add cloned methods when the remote object has return types
    //

    QDBusIntrospection::Methods::ConstIterator method_it = data->methods.constBegin();
    QDBusIntrospection::Methods::ConstIterator method_end = data->methods.constEnd();
    for ( ; method_it != method_end; ++method_it) {
        const QDBusIntrospection::Method &m = *method_it;
        Method mm;

        mm.name = m.name.toLatin1();
        QByteArray prototype = mm.name;
        prototype += '(';

        bool ok = true;

        // build the input argument list
        for (int i = 0; i < m.inputArgs.count(); ++i) {
            const QDBusIntrospection::Argument &arg = m.inputArgs.at(i);

            Type type = findType(arg.type.toLatin1(), m.annotations, "In", i);
            if (type.id == QVariant::Invalid) {
                ok = false;
                break;
            }

            mm.inputSignature += arg.type.toLatin1();
            mm.inputTypes.append(type.id);

            mm.parameters.append(arg.name.toLatin1());
            mm.parameters.append(',');
            
            prototype.append(type.name);
            prototype.append(',');
        }
        if (!ok) continue;

        // build the output argument list:
        for (int i = 0; i < m.outputArgs.count(); ++i) {
            const QDBusIntrospection::Argument &arg = m.outputArgs.at(i);

            Type type = findType(arg.type.toLatin1(), m.annotations, "Out", i);
            if (type.id == QVariant::Invalid) {
                ok = false;
                break;
            }

            mm.outputSignature += arg.type.toLatin1();
            mm.outputTypes.append(type.id);

            if (i == 0) {
                // return value
                mm.typeName = type.name;
            } else {
                // non-const ref parameter
                mm.parameters.append(arg.name.toLatin1());
                mm.parameters.append(',');

                prototype.append(type.name);
                prototype.append("&,");
            }
        }
        if (!ok) continue;

        // convert the last commas:
        if (!mm.parameters.isEmpty()) {
            mm.parameters.truncate(mm.parameters.length() - 1);
            prototype[prototype.length() - 1] = ')';
        } else {
            prototype.append(')');
        }

        // meta method flags
        mm.flags = AccessPublic | MethodSlot | MethodScriptable;

        // add
        methods.insert(QMetaObject::normalizedSignature(prototype), mm);
    }
}

void QDBusMetaObjectGenerator::parseSignals()
{
    QDBusIntrospection::Signals::ConstIterator signal_it = data->signals_.constBegin();
    QDBusIntrospection::Signals::ConstIterator signal_end = data->signals_.constEnd();
    for ( ; signal_it != signal_end; ++signal_it) {
        const QDBusIntrospection::Signal &s = *signal_it;
        Method mm;

        mm.name = s.name.toLatin1();
        QByteArray prototype = mm.name;
        prototype += '(';

        bool ok = true;

        // build the output argument list
        for (int i = 0; i < s.outputArgs.count(); ++i) {
            const QDBusIntrospection::Argument &arg = s.outputArgs.at(i);

            Type type = findType(arg.type.toLatin1(), s.annotations, "Out", i);
            if (type.id == QVariant::Invalid) {
                ok = false;
                break;
            }

            mm.inputSignature += arg.type.toLatin1();
            mm.inputTypes.append(type.id);

            mm.parameters.append(arg.name.toLatin1());
            mm.parameters.append(',');
            
            prototype.append(type.name);
            prototype.append(',');
        }
        if (!ok) continue;

        // convert the last commas:
        if (!mm.parameters.isEmpty()) {
            mm.parameters.truncate(mm.parameters.length() - 1);
            prototype[prototype.length() - 1] = ')';
        } else {
            prototype.append(')');
        }

        // meta method flags
        mm.flags = AccessProtected | MethodSignal | MethodScriptable;

        // add
        signals_.insert(QMetaObject::normalizedSignature(prototype), mm);
    }
}

void QDBusMetaObjectGenerator::parseProperties()
{
    foreach (const QDBusIntrospection::Property &p, data->properties) {
        Property mp;
        Type type = findType(p.type.toLatin1(), p.annotations);
        if (type.id == QVariant::Invalid)
            continue;
        
        QByteArray name = p.name.toLatin1();
        mp.signature = p.type.toLatin1();
        mp.type = type.id;
        mp.typeName = type.name;

        // build the flags:
        mp.flags = StdCppSet | Scriptable;
        if (p.access != QDBusIntrospection::Property::Write)
            mp.flags |= Readable;
        if (p.access != QDBusIntrospection::Property::Read)
            mp.flags |= Writable;

        if (mp.typeName == "QDBusVariant")
            mp.flags |= 0xff << 24;
        else if (mp.type < 0xff)
            // encode the type in the flags
            mp.flags |= mp.type << 24;

        // add the property:
        properties.insert(name, mp);
    }
}

void QDBusMetaObjectGenerator::write(QDBusMetaObject *obj)
{
    // this code here is mostly copied from qaxbase.cpp
    // with a few modifications to make it cleaner
    
    QString className = interface;
    className.replace(QLatin1Char('.'), QLatin1String("::"));
    if (className.isEmpty())
        className = QLatin1String("QDBusInterface");

    static const int QDBusMetaObjectPrivateSize = (sizeof(QDBusMetaObjectPrivate) / sizeof(int));

    const int methodCount = (signals_.count() + methods.count());
    const int propertyCount = properties.count();
    int data_size = QDBusMetaObjectPrivateSize +
                    (methodCount * (5+intsPerMethod)) +
                    (propertyCount * (3+intsPerProperty));
    foreach (const Method &mm, signals_)
        data_size += 2 + mm.inputTypes.count() + mm.outputTypes.count();
    foreach (const Method &mm, methods)
        data_size += 2 + mm.inputTypes.count() + mm.outputTypes.count();

    QStdVector<int> idata(data_size + 1);

    QDBusMetaObjectPrivate *header = reinterpret_cast<QDBusMetaObjectPrivate *>(idata.data());
    header->revision = qmetaobjectrevision;
    header->className = 0;
    header->classInfoCount = 0;
    header->classInfoData = 0;
    header->methodCount = methodCount;
    header->methodData = QDBusMetaObjectPrivateSize;
    header->propertyCount = propertyCount;
    header->propertyData = header->methodData + header->methodCount * 5;
    header->enumeratorCount = 0;
    header->enumeratorData = 0;
    header->constructorCount = 0;
    header->constructorData = 0;
    header->flags = RequiresVariantMetaObject;
    header->signalCount = signals_.count();
    // These are specific to QDBusMetaObject:
    header->propertyDBusData = header->propertyData + header->propertyCount * 3;
    header->methodDBusData = header->propertyDBusData + header->propertyCount * intsPerProperty;

    char null('\0');
    QByteArray stringdata = className.toLatin1();
    stringdata += null;
    stringdata.reserve(8192);

    int offset = header->methodData;
    int signatureOffset = header->methodDBusData;
    int typeidOffset = header->methodDBusData + header->methodCount * intsPerMethod;
    idata[typeidOffset++] = 0;                           // eod

    // add each method:
    for (int x = 0; x < 2; ++x) {
        // Signals must be added before other methods, to match moc.
        QMap<QByteArray, Method> &map = (x == 0) ? signals_ : methods;
        for (QMap<QByteArray, Method>::ConstIterator it = map.constBegin();
             it != map.constEnd(); ++it) {
            // form "prototype\0parameters\0typeName\0methodname\0inputSignature\0outputSignature"
            const Method &mm = it.value();

            idata[offset++] = stringdata.length();
            stringdata += it.key();                 // prototype
            stringdata += null;
            idata[offset++] = stringdata.length();
            stringdata += mm.parameters;
            stringdata += null;
            idata[offset++] = stringdata.length();
            stringdata += mm.typeName;
            stringdata += null;
            idata[offset++] = stringdata.length();
            stringdata += null;
            idata[offset++] = mm.flags;

            idata[signatureOffset++] = stringdata.length();
            stringdata += mm.name;
            stringdata += null;
            idata[signatureOffset++] = stringdata.length();
            stringdata += mm.inputSignature;
            stringdata += null;
            idata[signatureOffset++] = stringdata.length();
            stringdata += mm.outputSignature;
            stringdata += null;

            idata[signatureOffset++] = typeidOffset;
            idata[typeidOffset++] = mm.inputTypes.count();
            memcpy(idata.data() + typeidOffset, mm.inputTypes.data(), mm.inputTypes.count() * sizeof(int));
            typeidOffset += mm.inputTypes.count();

            idata[signatureOffset++] = typeidOffset;
            idata[typeidOffset++] = mm.outputTypes.count();
            memcpy(idata.data() + typeidOffset, mm.outputTypes.data(), mm.outputTypes.count() * sizeof(int));
            typeidOffset += mm.outputTypes.count();
        }
    }

    Q_ASSERT(offset == header->propertyData);
    Q_ASSERT(signatureOffset == header->methodDBusData + header->methodCount * intsPerMethod);
    Q_ASSERT(typeidOffset == idata.size());

    // add each property
    signatureOffset = header->propertyDBusData;
    for (QMap<QByteArray, Property>::ConstIterator it = properties.constBegin();
         it != properties.constEnd(); ++it) {
        const Property &mp = it.value();

        // form is "name\0typeName\0signature\0"
        idata[offset++] = stringdata.length();
        stringdata += it.key();                 // name
        stringdata += null;
        idata[offset++] = stringdata.length();
        stringdata += mp.typeName;
        stringdata += null;
        idata[offset++] = mp.flags;

        idata[signatureOffset++] = stringdata.length();
        stringdata += mp.signature;
        stringdata += null;
        idata[signatureOffset++] = mp.type;
    }

    Q_ASSERT(offset == header->propertyDBusData);
    Q_ASSERT(signatureOffset == header->methodDBusData);

    char *string_data = new char[stringdata.length()];
    memcpy(string_data, stringdata, stringdata.length());

    uint *uint_data = new uint[idata.size()];
    memcpy(uint_data, idata.data(), idata.size() * sizeof(int));

    // put the metaobject together
    obj->d.data = uint_data;
    obj->d.relatedMetaObjects = 0;
    obj->d.static_metacall = 0;
    obj->d.stringdata = string_data;
    obj->d.superdata = &QDBusAbstractInterface::staticMetaObject;
}

/////////
// class QDBusMetaObject

QDBusMetaObject *QDBusMetaObject::createMetaObject(const QString &interface, const QString &xml,
                                                   QHash<QString, QDBusMetaObject *> &cache,
                                                   QDBusError &error)
{
    error = QDBusError();
    QDBusIntrospection::Interfaces parsed = QDBusIntrospection::parseInterfaces(xml);

    QDBusMetaObject *we = 0;
    QDBusIntrospection::Interfaces::ConstIterator it = parsed.constBegin();
    QDBusIntrospection::Interfaces::ConstIterator end = parsed.constEnd();
    for ( ; it != end; ++it) {
        // check if it's in the cache
        bool us = it.key() == interface;

        QDBusMetaObject *obj = cache.value(it.key(), 0);
        if ( !obj && ( us || !interface.startsWith( QLatin1String("local.") ) ) ) {
            // not in cache; create
            obj = new QDBusMetaObject();
            QDBusMetaObjectGenerator generator(it.key(), it.value().constData());
            generator.write(obj);

            if ( (obj->cached = !it.key().startsWith( QLatin1String("local.") )) )
                // cache it
                cache.insert(it.key(), obj);
            else if (!us)
                delete obj;

        }

        if (us)
            // it's us
            we = obj;
    }

    if (we)
        return we;
    // still nothing?
    
    if (parsed.isEmpty()) {
        // object didn't return introspection
        we = new QDBusMetaObject();
        QDBusMetaObjectGenerator generator(interface, 0);
        generator.write(we);
        return we;
    } else if (interface.isEmpty()) {
        // merge all interfaces
        it = parsed.constBegin();
        QDBusIntrospection::Interface merged = *it.value().constData();
 
        for (++it; it != end; ++it) {
            merged.annotations.unite(it.value()->annotations);
            merged.methods.unite(it.value()->methods);
            merged.signals_.unite(it.value()->signals_);
            merged.properties.unite(it.value()->properties);
        }

        merged.name = QLatin1String("local.Merged");
        merged.introspection.clear();

        we = new QDBusMetaObject();
        QDBusMetaObjectGenerator generator(merged.name, &merged);
        generator.write(we);
        return we;
    }

    // mark as an error
    error = QDBusError(QDBusError::UnknownInterface,
        QString::fromLatin1("Interface '%1' was not found")
                       .arg(interface));
    return 0;
}

QDBusMetaObject::QDBusMetaObject()
    : cached(false)
{
}

static inline const QDBusMetaObjectPrivate *priv(const uint* data)
{
    return reinterpret_cast<const QDBusMetaObjectPrivate *>(data);
}

const char *QDBusMetaObject::dbusNameForMethod(int id) const
{
    //id -= methodOffset();
    if (id >= 0 && id < priv(d.data)->methodCount) {
        int handle = priv(d.data)->methodDBusData + id*intsPerMethod;
        return d.stringdata + d.data[handle];
    }
    return 0;
}

const char *QDBusMetaObject::inputSignatureForMethod(int id) const
{
    //id -= methodOffset();
    if (id >= 0 && id < priv(d.data)->methodCount) {
        int handle = priv(d.data)->methodDBusData + id*intsPerMethod;
        return d.stringdata + d.data[handle + 1];
    }
    return 0;
}

const char *QDBusMetaObject::outputSignatureForMethod(int id) const
{
    //id -= methodOffset();
    if (id >= 0 && id < priv(d.data)->methodCount) {
        int handle = priv(d.data)->methodDBusData + id*intsPerMethod;
        return d.stringdata + d.data[handle + 2];
    }
    return 0;
}

const int *QDBusMetaObject::inputTypesForMethod(int id) const
{
    //id -= methodOffset();
    if (id >= 0 && id < priv(d.data)->methodCount) {
        int handle = priv(d.data)->methodDBusData + id*intsPerMethod;
        return reinterpret_cast<const int*>(d.data + d.data[handle + 3]);
    }
    return 0;
}

const int *QDBusMetaObject::outputTypesForMethod(int id) const
{
    //id -= methodOffset();
    if (id >= 0 && id < priv(d.data)->methodCount) {
        int handle = priv(d.data)->methodDBusData + id*intsPerMethod;
        return reinterpret_cast<const int*>(d.data + d.data[handle + 4]);
    }
    return 0;
}

int QDBusMetaObject::propertyMetaType(int id) const
{
    //id -= propertyOffset();
    if (id >= 0 && id < priv(d.data)->propertyCount) {
        int handle = priv(d.data)->propertyDBusData + id*intsPerProperty;
        return d.data[handle + 1];
    }
    return 0;
}

QT_END_NAMESPACE

