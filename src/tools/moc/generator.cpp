/****************************************************************************
**
** Copyright (C) 2015 The Qt Company Ltd.
** Copyright (C) 2016 Ivailo Monev
**
** This file is part of the tools applications of the Katie Toolkit.
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

#include "generator.h"
#include "utils.h"
#include "qvariant.h"
#include "qmetatype.h"
#include "qmetaobject_p.h" //for the flags.

#include <stdio.h>

QT_BEGIN_NAMESPACE

static uint qvariant_nameToType(const char* name)
{
    if (!name)
        return 0;

    if (strcmp(name, "QVariant") == 0)
        return QVariant::LastType;

    uint tp = QMetaType::type(name);
    return tp < QMetaType::User ? tp : 0;
}

/*
  Returns true if the type is a QVariant types.
*/
static bool isVariantType(const char* type)
{
    return qvariant_nameToType(type) != 0;
}

/*!
  Returns true if the type is qreal.
*/
static bool isQRealType(const char *type)
{
    return strcmp(type, "qreal") == 0;
}

Generator::Generator(ClassDef *classDef, const QList<QByteArray> &metaTypes, FILE *outfile)
    : out(outfile), cdef(classDef), metaTypes(metaTypes)
{
    if (cdef->superclassList.size())
        purestSuperClass = cdef->superclassList.first().first;
}

static inline int lengthOfEscapeSequence(const QByteArray &s, int i)
{
    if (s.at(i) != '\\' || i >= s.length() - 1)
        return 1;
    const int startPos = i;
    ++i;
    char ch = s.at(i);
    if (ch == 'x') {
        ++i;
        while (i < s.length() && is_hex_char(s.at(i)))
            ++i;
    } else if (is_octal_char(ch)) {
        while (i < startPos + 4
               && i < s.length()
               && is_octal_char(s.at(i))) {
            ++i;
        }
    } else { // single character escape sequence
        i = qMin(i + 1, s.length());
    }
    return i - startPos;
}

int Generator::strreg(const char *s)
{
    int idx = 0;
    if (!s)
        s = "";
    for (int i = 0; i < strings.size(); ++i) {
        const QByteArray &str = strings.at(i);
        if (str == s)
            return idx;
        idx += str.length() + 1;
        for (int i = 0; i < str.length(); ++i) {
            if (str.at(i) == '\\') {
                int cnt = lengthOfEscapeSequence(str, i) - 1;
                idx -= cnt;
                i += cnt;
            }
        }
    }
    strings.append(s);
    return idx;
}

void Generator::generateCode()
{
    bool isQt = (cdef->classname == "Qt");
    bool isQObject = (cdef->classname == "QObject");
    bool isConstructible = !cdef->constructorList.isEmpty();

//
// build the data array
//
    int i = 0;


    // filter out undeclared enumerators and sets
    {
        QList<EnumDef> enumList;
        for (i = 0; i < cdef->enumList.count(); ++i) {
            EnumDef def = cdef->enumList.at(i);
            if (cdef->enumDeclarations.contains(def.name)) {
                enumList += def;
            }
            QByteArray alias = cdef->flagAliases.value(def.name);
            if (cdef->enumDeclarations.contains(alias)) {
                def.name = alias;
                enumList += def;
            }
        }
        cdef->enumList = enumList;
    }


    QByteArray qualifiedClassNameIdentifier = cdef->qualified;
    qualifiedClassNameIdentifier.replace(':', '_');

    int index = 14;
    fprintf(out, "static const uint qt_meta_data_%s[] = {\n", qualifiedClassNameIdentifier.constData());
    fprintf(out, "\n // content:\n");
    fprintf(out, "    %4d,       // revision\n", 6);
    fprintf(out, "    %4d,       // classname\n", strreg(cdef->qualified.constData()));
    fprintf(out, "    %4d, %4d, // classinfo\n", cdef->classInfoList.count(), cdef->classInfoList.count() ? index : 0);
    index += cdef->classInfoList.count() * 2;

    int methodCount = cdef->signalList.count() + cdef->slotList.count() + cdef->methodList.count();
    fprintf(out, "    %4d, %4d, // methods\n", methodCount, methodCount ? index : 0);
    index += methodCount * 5;
    fprintf(out, "    %4d, %4d, // properties\n", cdef->propertyList.count(), cdef->propertyList.count() ? index : 0);
    index += cdef->propertyList.count() * 3;
    if(cdef->notifyableProperties)
        index += cdef->propertyList.count();
    fprintf(out, "    %4d, %4d, // enums/sets\n", cdef->enumList.count(), cdef->enumList.count() ? index : 0);

    int enumsIndex = index;
    for (i = 0; i < cdef->enumList.count(); ++i)
        index += 4 + (cdef->enumList.at(i).values.count() * 2);
    fprintf(out, "    %4d, %4d, // constructors\n", isConstructible ? cdef->constructorList.count() : 0,
            isConstructible ? index : 0);

    fprintf(out, "    %4d,       // flags\n", 0);
    fprintf(out, "    %4d,       // signalCount\n", cdef->signalList.count());


//
// Build classinfo array
//
    generateClassInfos();

//
// Build signals array first, otherwise the signal indices would be wrong
//
    generateFunctions(cdef->signalList, "signal", MethodSignal);

//
// Build slots array
//
    generateFunctions(cdef->slotList, "slot", MethodSlot);

//
// Build method array
//
    generateFunctions(cdef->methodList, "method", MethodMethod);

//
// Build property array
//
    generateProperties();

//
// Build enums array
//
    generateEnums(enumsIndex);

//
// Build constructors array
//
    if (isConstructible)
        generateFunctions(cdef->constructorList, "constructor", MethodConstructor);

//
// Terminate data array
//
    fprintf(out, "\n       0        // eod\n};\n\n");

//
// Build stringdata array
//
    fprintf(out, "static const char qt_meta_stringdata_%s[] = {\n", qualifiedClassNameIdentifier.constData());
    fprintf(out, "    \"");
    int col = 0;
    int len = 0;
    for (i = 0; i < strings.size(); ++i) {
        QByteArray s = strings.at(i);
        len = s.length();
        if (col && col + len >= 72) {
            fprintf(out, "\"\n    \"");
            col = 0;
        } else if (len && s.at(0) >= '0' && s.at(0) <= '9') {
            fprintf(out, "\"\"");
            len += 2;
        }
        int idx = 0;
        while (idx < s.length()) {
            if (idx > 0) {
                col = 0;
                fprintf(out, "\"\n    \"");
            }
            int spanLen = qMin(70, s.length() - idx);
            // don't cut escape sequences at the end of a line
            int backSlashPos = s.lastIndexOf('\\', idx + spanLen - 1);
            if (backSlashPos >= idx) {
                int escapeLen = lengthOfEscapeSequence(s, backSlashPos);
                spanLen = qBound(spanLen, backSlashPos + escapeLen - idx, s.length() - idx);
            }
            fwrite(s.constData() + idx, 1, spanLen, out);
            idx += spanLen;
            col += spanLen;
        }

        fputs("\\0", out);
        col += len + 2;
    }
    fprintf(out, "\"\n};\n\n");

//
// Generate internal qt_static_metacall() function
//
    if (cdef->hasQObject && !isQt)
        generateStaticMetacall();

//
// Finally create and initialize the static meta object
//
    if (isQt)
        fprintf(out, "const QMetaObject QObject::staticQtMetaObject = {\n");
    else
        fprintf(out, "const QMetaObject %s::staticMetaObject = {\n", cdef->qualified.constData());

    if (isQObject)
        fprintf(out, "    { nullptr, ");
    else if (cdef->superclassList.size())
        fprintf(out, "    { &%s::staticMetaObject, ", purestSuperClass.constData());
    else
        fprintf(out, "    { nullptr, ");
    fprintf(out, "qt_meta_stringdata_%s,\n      qt_meta_data_%s, ",
             qualifiedClassNameIdentifier.constData(), qualifiedClassNameIdentifier.constData());
    if (cdef->hasQObject && !isQt)
        fprintf(out, " qt_static_metacall, ");
    else
        fprintf(out, " nullptr, ");

    fprintf(out, "nullptr}\n};\n\n");

    if(isQt)
        return;

//
// Generate static meta object accessor (needed for symbian, because DLLs do not support data imports.
//
    fprintf(out, "\n#ifdef Q_NO_DATA_RELOCATION\n");
    fprintf(out, "const QMetaObject &%s::getStaticMetaObject() { return staticMetaObject; }\n", cdef->qualified.constData());
    fprintf(out, "#endif //Q_NO_DATA_RELOCATION\n");

    if (!cdef->hasQObject)
        return;

    fprintf(out, "\nconst QMetaObject *%s::metaObject() const\n{\n    return QObject::d_ptr->metaObject ? QObject::d_ptr->metaObject : &staticMetaObject;\n}\n",
            cdef->qualified.constData());

//
// Generate smart cast function
//
    fprintf(out, "\nvoid *%s::qt_metacast(const char *_clname)\n{\n", cdef->qualified.constData());
    fprintf(out, "    if (!_clname) return nullptr;\n");
    fprintf(out, "    if (strcmp(_clname, qt_meta_stringdata_%s) == 0)\n"
                  "        return static_cast<void*>(this);\n",
            qualifiedClassNameIdentifier.constData());
    for (int i = 1; i < cdef->superclassList.size(); ++i) { // for all superclasses but the first one
        if (cdef->superclassList.at(i).second == FunctionDef::Private)
            continue;
        const char *cname = cdef->superclassList.at(i).first.constData();
        fprintf(out, "    if (strcmp(_clname, \"%s\") == 0)\n        return static_cast<%s*>(const_cast<%s*>(this));\n",
                cname, cname, cdef->classname.constData());
    }
    for (int i = 0; i < cdef->interfaceList.size(); ++i) {
        const QList<ClassDef::Interface> &iface = cdef->interfaceList.at(i);
        for (int j = 0; j < iface.size(); ++j) {
            fprintf(out, "    if (strcmp(_clname, %s) == 0)\n        return ", iface.at(j).interfaceId.constData());
            for (int k = j; k >= 0; --k)
                fprintf(out, "static_cast<%s*>(", iface.at(k).className.constData());
            fprintf(out, "const_cast<%s*>(this)%s;\n",
                    cdef->classname.constData(), QByteArray(j+1, ')').constData());
        }
    }
    if (!purestSuperClass.isEmpty() && !isQObject) {
        fprintf(out, "    return %s::qt_metacast(_clname);\n", purestSuperClass.constData());
    } else {
        fprintf(out, "    return nullptr;\n");
    }
    fprintf(out, "}\n");

//
// Generate internal qt_metacall()  function
//
    generateMetacall();

//
// Generate internal signal functions
//
    for (int signalindex = 0; signalindex < cdef->signalList.size(); ++signalindex)
        generateSignal(&cdef->signalList[signalindex], signalindex);
}


void Generator::generateClassInfos()
{
    if (cdef->classInfoList.isEmpty())
        return;

    fprintf(out, "\n // classinfo: key, value\n");

    for (int i = 0; i < cdef->classInfoList.size(); ++i) {
        const ClassInfoDef &c = cdef->classInfoList.at(i);
        fprintf(out, "    %4d, %4d,\n", strreg(c.name.constData()), strreg(c.value.constData()));
    }
}

void Generator::generateFunctions(QList<FunctionDef>& list, const char *functype, int type)
{
    if (list.isEmpty())
        return;
    fprintf(out, "\n // %ss: signature, parameters, type, tag, flags\n", functype);

    for (int i = 0; i < list.count(); ++i) {
        const FunctionDef &f = list.at(i);

        QByteArray sig = f.name + '(';
        QByteArray arguments;

        for (int j = 0; j < f.arguments.count(); ++j) {
            const ArgumentDef &a = f.arguments.at(j);
            if (j) {
                sig += ",";
                arguments += ",";
            }
            sig += a.normalizedType;
            arguments += a.name;
        }
        sig += ')';

        unsigned char flags = type;
        if (f.access == FunctionDef::Private)
            flags |= AccessPrivate;
        else if (f.access == FunctionDef::Public)
            flags |= AccessPublic;
        else if (f.access == FunctionDef::Protected)
            flags |= AccessProtected;
        if (f.wasCloned)
            flags |= MethodCloned;
        if (f.isScriptable)
            flags |= MethodScriptable;
        fprintf(out, "    %4d, %4d, %4d, %4d, 0x%02x,\n",
                strreg(sig.constData()),
                strreg(arguments.constData()),
                strreg(f.normalizedType.constData()),
                strreg(f.tag.constData()), flags);
    }
}

void Generator::generateProperties()
{
    //
    // Create meta data
    //

    if (cdef->propertyList.count())
        fprintf(out, "\n // properties: name, type, flags\n");
    for (int i = 0; i < cdef->propertyList.count(); ++i) {
        const PropertyDef &p = cdef->propertyList.at(i);
        uint flags = Invalid;
        if (!isVariantType(p.type.constData())) {
            flags |= EnumOrFlag;
        } else if (!isQRealType(p.type.constData())) {
            flags |= qvariant_nameToType(p.type.constData()) << 24;
        }
        if (!p.read.isEmpty())
            flags |= Readable;
        if (!p.write.isEmpty()) {
            flags |= Writable;
            if (p.stdCppSet())
                flags |= StdCppSet;
        }
        if (!p.reset.isEmpty())
            flags |= Resettable;

//         if (p.override)
//             flags |= Override;

        if (p.scriptable.isEmpty())
            flags |= ResolveScriptable;
        else if (p.scriptable != "false")
            flags |= Scriptable;

        if (p.notifyId != -1)
            flags |= Notify;

        fprintf(out, "    %4d, %4d, ",
                strreg(p.name.constData()),
                strreg(p.type.constData()));
        if (!(flags >> 24) && isQRealType(p.type.constData()))
            fprintf(out, "((uint)QMetaType::QReal << 24) | ");
        fprintf(out, "0x%.8x,\n", flags);
    }

    if(cdef->notifyableProperties) {
        fprintf(out, "\n // properties: notify_signal_id\n");
        for (int i = 0; i < cdef->propertyList.count(); ++i) {
            const PropertyDef &p = cdef->propertyList.at(i);
            if(p.notifyId == -1)
                fprintf(out, "    %4d,\n",
                        0);
            else
                fprintf(out, "    %4d,\n",
                        p.notifyId);
        }
    }
}

void Generator::generateEnums(int index)
{
    if (cdef->enumDeclarations.isEmpty())
        return;

    fprintf(out, "\n // enums: name, flags, count, data\n");
    index += 4 * cdef->enumList.count();
    int i;
    for (i = 0; i < cdef->enumList.count(); ++i) {
        const EnumDef &e = cdef->enumList.at(i);
        fprintf(out, "    %4d, 0x%.1x, %4d, %4d,\n",
                 strreg(e.name.constData()),
                 cdef->enumDeclarations.value(e.name) ? 1 : 0,
                 e.values.count(),
                 index);
        index += e.values.count() * 2;
    }

    fprintf(out, "\n // enum data: key, value\n");
    for (i = 0; i < cdef->enumList.count(); ++i) {
        const EnumDef &e = cdef->enumList.at(i);
        for (int j = 0; j < e.values.count(); ++j) {
            const QByteArray &val = e.values.at(j);
            fprintf(out, "    %4d, uint(%s::%s),\n",
                    strreg(val.constData()),
                    cdef->qualified.constData(),
                    val.constData());
        }
    }
}

void Generator::generateMetacall()
{
    bool isQObject = (cdef->classname == "QObject");

    fprintf(out, "\nint %s::qt_metacall(QMetaObject::Call _c, int _id, void **_a)\n{\n",
             cdef->qualified.constData());

    if (!purestSuperClass.isEmpty() && !isQObject) {
        QByteArray superClass = purestSuperClass;
        // workaround for VC6
        if (superClass.contains("::")) {
            fprintf(out, "    typedef %s QMocSuperClass;\n", superClass.constData());
            superClass = "QMocSuperClass";
        }
        fprintf(out, "    _id = %s::qt_metacall(_c, _id, _a);\n", superClass.constData());
    }

    fprintf(out, "    if (_id < 0)\n        return _id;\n");
    fprintf(out, "    ");

    bool needElse = false;
    QList<FunctionDef> methodList;
    methodList += cdef->signalList;
    methodList += cdef->slotList;
    methodList += cdef->methodList;

    if (methodList.size()) {
        needElse = true;
        fprintf(out, "if (_c == QMetaObject::InvokeMetaMethod) {\n");
        fprintf(out, "        if (_id < %d)\n", methodList.size());
        fprintf(out, "            qt_static_metacall(this, _c, _id, _a);\n");
        fprintf(out, "        _id -= %d;\n    }", methodList.size());
    }

    if (cdef->propertyList.size()) {
        bool needGet = false;
        bool needSet = false;
        bool needReset = false;
        bool needScriptable = false;
        for (int i = 0; i < cdef->propertyList.size(); ++i) {
            const PropertyDef &p = cdef->propertyList.at(i);
            needGet |= !p.read.isEmpty();
            needSet |= !p.write.isEmpty();
            needReset |= !p.reset.isEmpty();
            needScriptable |= p.scriptable.endsWith(')');
        }
        fprintf(out, "\n#ifndef QT_NO_PROPERTIES\n     ");

        if (needElse)
            fprintf(out, " else ");
        fprintf(out, "if (_c == QMetaObject::ReadProperty) {\n");
        if (needGet) {
            fprintf(out, "        switch (_id) {\n");
            for (int propindex = 0; propindex < cdef->propertyList.size(); ++propindex) {
                const PropertyDef &p = cdef->propertyList.at(propindex);
                if (p.read.isEmpty())
                    continue;
                QByteArray prefix;
                if (p.inPrivateClass.size()) {
                    prefix = p.inPrivateClass;
                    prefix.append("->");
                }
                if (p.gspec == PropertyDef::PointerSpec)
                    fprintf(out, "        case %d: _a[0] = const_cast<void*>(static_cast<const void*>(%s%s())); break;\n",
                            propindex, prefix.constData(), p.read.constData());
                else if (p.gspec == PropertyDef::ReferenceSpec)
                    fprintf(out, "        case %d: _a[0] = const_cast<void*>(static_cast<const void*>(&%s%s())); break;\n",
                            propindex, prefix.constData(), p.read.constData());
                else if (cdef->enumDeclarations.value(p.type, false))
                    fprintf(out, "        case %d: *static_cast<int*>(_a[0]) = QFlag(%s%s()); break;\n",
                            propindex, prefix.constData(), p.read.constData());
                else
                    fprintf(out, "        case %d: *static_cast<%s*>(_a[0]) = %s%s(); break;\n",
                            propindex, p.type.constData(), prefix.constData(), p.read.constData());
            }
            fprintf(out, "        }\n");
        }

        fprintf(out,
                "        _id -= %d;\n"
                "    }", cdef->propertyList.count());

        fprintf(out, " else ");
        fprintf(out, "if (_c == QMetaObject::WriteProperty) {\n");

        if (needSet) {
            fprintf(out, "        switch (_id) {\n");
            for (int propindex = 0; propindex < cdef->propertyList.size(); ++propindex) {
                const PropertyDef &p = cdef->propertyList.at(propindex);
                if (p.write.isEmpty())
                    continue;
                QByteArray prefix;
                if (p.inPrivateClass.size()) {
                    prefix = p.inPrivateClass;
                    prefix.append("->");
                }
                if (cdef->enumDeclarations.value(p.type, false)) {
                    fprintf(out, "        case %d: %s%s(QFlag(*static_cast<int*>(_a[0]))); break;\n",
                            propindex, prefix.constData(), p.write.constData());
                } else {
                    fprintf(out, "        case %d: %s%s(*static_cast<%s*>(_a[0])); break;\n",
                            propindex, prefix.constData(), p.write.constData(), p.type.constData());
                }
            }
            fprintf(out, "        }\n");
        }

        fprintf(out,
                "        _id -= %d;\n"
                "    }", cdef->propertyList.count());

        fprintf(out, " else ");
        fprintf(out, "if (_c == QMetaObject::ResetProperty) {\n");
        if (needReset) {
            fprintf(out, "        switch (_id) {\n");
            for (int propindex = 0; propindex < cdef->propertyList.size(); ++propindex) {
                const PropertyDef &p = cdef->propertyList.at(propindex);
                if (!p.reset.endsWith(')'))
                    continue;
                QByteArray prefix;
                if (p.inPrivateClass.size()) {
                    prefix = p.inPrivateClass;
                    prefix.append("->");
                }
                fprintf(out, "        case %d: %s%s; break;\n",
                        propindex, prefix.constData(), p.reset.constData());
            }
            fprintf(out, "        }\n");
        }
        fprintf(out,
                "        _id -= %d;\n"
                "    }", cdef->propertyList.count());

        fprintf(out, " else if (_c == QMetaObject::QueryPropertyScriptable) {\n");
        if (needScriptable) {
            fprintf(out, "        switch (_id) {\n");
            for (int propindex = 0; propindex < cdef->propertyList.size(); ++propindex) {
                const PropertyDef &p = cdef->propertyList.at(propindex);
                if (!p.scriptable.endsWith(')'))
                    continue;
                fprintf(out, "        case %d: *static_cast<bool*>(_a[0]) = %s; break;\n",
                         propindex, p.scriptable.constData());
            }
            fprintf(out, "        }\n");
        }
        fprintf(out,
                "        _id -= %d;\n"
                "    }", cdef->propertyList.count());

        fprintf(out, "\n#endif // QT_NO_PROPERTIES");
    }
    if (methodList.size() || cdef->signalList.size() || cdef->propertyList.size())
        fprintf(out, "\n    ");
    fprintf(out,"return _id;\n}\n");
}

void Generator::generateStaticMetacall()
{
    fprintf(out, "void %s::qt_static_metacall(QObject *_o, QMetaObject::Call _c, int _id, void **_a)\n{\n",
            cdef->qualified.constData());

    bool needElse = false;
    bool isUsed_a = false;

    if (!cdef->constructorList.isEmpty()) {
        fprintf(out, "    if (_c == QMetaObject::CreateInstance) {\n");
        fprintf(out, "        switch (_id) {\n");
        for (int ctorindex = 0; ctorindex < cdef->constructorList.count(); ++ctorindex) {
            fprintf(out, "        case %d: { %s *_r = new %s(", ctorindex,
                    cdef->classname.constData(), cdef->classname.constData());
            const FunctionDef &f = cdef->constructorList.at(ctorindex);
            int offset = 1;
            for (int j = 0; j < f.arguments.count(); ++j) {
                const ArgumentDef &a = f.arguments.at(j);
                if (j)
                    fprintf(out, ",");
                fprintf(out, "*static_cast<%s>(_a[%d])", a.typeNameForCast.constData(), offset++);
            }
            fprintf(out, ");\n");
            fprintf(out, "            if (_a[0]) *static_cast<QObject**>(_a[0]) = _r; } break;\n");
        }
        fprintf(out, "        }\n");
        fprintf(out, "    }");
        needElse = true;
        isUsed_a = true;
    }

    QList<FunctionDef> methodList;
    methodList += cdef->signalList;
    methodList += cdef->slotList;
    methodList += cdef->methodList;

    if (!methodList.isEmpty()) {
        if (needElse)
            fprintf(out, " else ");
        else
            fprintf(out, "    ");
        fprintf(out, "if (_c == QMetaObject::InvokeMetaMethod) {\n");
#ifndef QT_NO_DEBUG
        fprintf(out, "        Q_ASSERT(staticMetaObject.cast(_o));\n");
#endif
        fprintf(out, "        %s *_t = static_cast<%s *>(_o);\n", cdef->classname.constData(), cdef->classname.constData());
        fprintf(out, "        switch (_id) {\n");
        for (int methodindex = 0; methodindex < methodList.size(); ++methodindex) {
            const FunctionDef &f = methodList.at(methodindex);
            fprintf(out, "        case %d: ", methodindex);
            if (f.normalizedType.size())
                fprintf(out, "{ %s _r = ", noRef(f.normalizedType).constData());
            fprintf(out, "_t->");
            if (f.inPrivateClass.size())
                fprintf(out, "%s->", f.inPrivateClass.constData());
            fprintf(out, "%s(", f.name.constData());
            int offset = 1;
            for (int j = 0; j < f.arguments.count(); ++j) {
                const ArgumentDef &a = f.arguments.at(j);
                if (j)
                    fprintf(out, ",");
                fprintf(out, "*static_cast<%s>(_a[%d])",a.typeNameForCast.constData(), offset++);
                isUsed_a = true;
            }
            fprintf(out, ");");
            if (f.normalizedType.size()) {
                fprintf(out, "\n            if (_a[0]) *static_cast<%s*>(_a[0]) = _r; } ",
                        noRef(f.normalizedType).constData());
                isUsed_a = true;
            }
            fprintf(out, " break;\n");
        }
        fprintf(out, "        default: break;\n");
        fprintf(out, "        }\n");
        fprintf(out, "    }");
        needElse = true;
    }

    if (needElse)
        fprintf(out, "\n");

    if (methodList.isEmpty()) {
        fprintf(out, "    Q_UNUSED(_o);\n");
        if (cdef->constructorList.isEmpty()) {
            fprintf(out, "    Q_UNUSED(_id);\n");
            fprintf(out, "    Q_UNUSED(_c);\n");
        }
    }
    if (!isUsed_a)
        fprintf(out, "    Q_UNUSED(_a);\n");

    fprintf(out, "}\n\n");
}

void Generator::generateSignal(FunctionDef *def,int index)
{
    if (def->wasCloned || def->isAbstract)
        return;
    fprintf(out, "\n// SIGNAL %d\n%s %s::%s(",
            index, def->type.name.constData(), cdef->qualified.constData(), def->name.constData());

    QByteArray thisPtr = "this";
    const char *constQualifier = "";

    if (def->isConst) {
        thisPtr = "const_cast<";
        thisPtr += cdef->qualified;
        thisPtr += " *>(this)";
        constQualifier = "const";
    }

    if (def->arguments.isEmpty() && def->normalizedType.isEmpty()) {
        fprintf(out, ")%s\n{\n"
                "    QMetaObject::activate(%s, &staticMetaObject, %d, nullptr);\n"
                "}\n", constQualifier, thisPtr.constData(), index);
        return;
    }

    int offset = 1;
    for (int j = 0; j < def->arguments.count(); ++j) {
        const ArgumentDef &a = def->arguments.at(j);
        if (j)
            fprintf(out, ", ");
        fprintf(out, "%s _t%d%s", a.type.name.constData(), offset++, a.rightType.constData());
    }
    fprintf(out, ")%s\n{\n", constQualifier);
    if (def->type.name.size() && def->normalizedType.size())
        fprintf(out, "    %s _t0;\n", noRef(def->normalizedType).constData());

    fprintf(out, "    void *_a[] = { ");
    if (def->normalizedType.isEmpty()) {
        fprintf(out, "nullptr");
    } else {
        if (def->returnTypeIsVolatile)
             fprintf(out, "const_cast<void*>(static_cast<const volatile void*>(&_t0))");
        else
             fprintf(out, "const_cast<void*>(static_cast<const void*>(&_t0))");
    }
    int i;
    for (i = 1; i < offset; ++i)
        if (def->arguments.at(i - 1).type.isVolatile)
            fprintf(out, ", const_cast<void*>(static_cast<const volatile void*>(&_t%d))", i);
        else
            fprintf(out, ", const_cast<void*>(static_cast<const void*>(&_t%d))", i);
    fprintf(out, " };\n");
    fprintf(out, "    QMetaObject::activate(%s, &staticMetaObject, %d, _a);\n", thisPtr.constData(), index);
    if (def->normalizedType.size())
        fprintf(out, "    return _t0;\n");
    fprintf(out, "}\n");
}

QT_END_NAMESPACE
