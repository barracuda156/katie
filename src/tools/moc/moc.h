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

#ifndef MOC_H
#define MOC_H

#include "parser.h"
#include <QMap>
#include <QPair>
#include <stdio.h>
#include <ctype.h>

QT_BEGIN_NAMESPACE

struct QMetaObject;

struct Type
{
    enum ReferenceType { NoReference, Reference, RValueReference, Pointer };

    inline Type() : isVolatile(false), isScoped(false), firstToken(NOTOKEN), referenceType(NoReference) {}
    inline explicit Type(const QByteArray &_name) : name(_name), isVolatile(false), isScoped(false), firstToken(NOTOKEN), referenceType(NoReference) {}
    QByteArray name;
    bool isVolatile;
    bool isScoped;
    Token firstToken;
    ReferenceType referenceType;
};

struct EnumDef
{
    QByteArray name;
    QList<QByteArray> values;
};

struct ArgumentDef
{
    ArgumentDef() : isDefault(false) {}
    Type type;
    QByteArray rightType, normalizedType, name;
    QByteArray typeNameForCast; // type name to be used in cast from void * in metacall
    bool isDefault;
};

struct FunctionDef
{
    FunctionDef() :
        returnTypeIsVolatile(false), access(Private), isConst(false), isVirtual(false), isStatic(false),
        inlineCode(false), wasCloned(false), isInvokable(false),
        isScriptable(false), isSlot(false), isSignal(false),
        isConstructor(false), isDestructor(false), isAbstract(false)
    {
    }

    Type type;
    QByteArray normalizedType;
    QByteArray tag;
    QByteArray name;
    bool returnTypeIsVolatile;

    QList<ArgumentDef> arguments;

    enum Access { Private, Protected, Public };
    Access access;
    bool isConst;
    bool isVirtual;
    bool isStatic;
    bool inlineCode;
    bool wasCloned;

    QByteArray inPrivateClass;
    bool isInvokable;
    bool isScriptable;
    bool isSlot;
    bool isSignal;
    bool isConstructor;
    bool isDestructor;
    bool isAbstract;
};

struct PropertyDef
{
    PropertyDef() :
        notifyId(-1), gspec(ValueSpec)
    {
    }

    QByteArray name, type, read, write, reset, scriptable, notify, inPrivateClass;
    int notifyId;
    enum Specification  { ValueSpec, ReferenceSpec, PointerSpec };
    Specification gspec;
    bool stdCppSet() const {
        QByteArray s("set");
        s += toupper(name[0]);
        s += name.mid(1);
        return (s == write);
    }
};


struct ClassInfoDef
{
    QByteArray name;
    QByteArray value;
};

struct ClassDef {
    ClassDef():
        hasQObject(false), hasQGadget(false), notifyableProperties(0), begin(0), end(0)
    {
    }

    QByteArray classname;
    QByteArray qualified;
    QList<QPair<QByteArray, FunctionDef::Access> > superclassList;

    struct Interface
    {
        inline explicit Interface(const QByteArray &_className)
            : className(_className) {}
        QByteArray className;
        QByteArray interfaceId;
    };
    QList<QList<Interface> >interfaceList;

    bool hasQObject;
    bool hasQGadget;

    QList<FunctionDef> constructorList;
    QList<FunctionDef> signalList, slotList, methodList, publicList;
    int notifyableProperties;
    QList<PropertyDef> propertyList;
    QList<ClassInfoDef> classInfoList;
    QMap<QByteArray, bool> enumDeclarations;
    QList<EnumDef> enumList;
    QMap<QByteArray, QByteArray> flagAliases;

    int begin;
    int end;
};

struct NamespaceDef {
    QByteArray name;
    int begin;
    int end;
};

class Moc : public Parser
{
public:
    Moc()
        : noInclude(false), mustIncludeQMetaTypeH(false)
        {}

    QByteArray filename;

    bool noInclude;
    bool mustIncludeQMetaTypeH;
    QByteArray includePath;
    QList<QByteArray> includeFiles;
    QList<ClassDef> classList;
    QMap<QByteArray, QByteArray> interface2IdMap;
    QList<QByteArray> metaTypes;
    QSet<QByteArray> knownQObjectClasses;

    void parse();
    void generate(FILE *out);
    QList<QMetaObject*> generate(bool ignoreProperties);

    bool parseClassHead(ClassDef *def);
    inline bool inClass(const ClassDef *def) const {
        return index > def->begin && index < def->end - 1;
    }

    inline bool inNamespace(const NamespaceDef *def) const {
        return index > def->begin && index < def->end - 1;
    }

    Type parseType();

    bool parseEnum(EnumDef *def);

    bool parseFunction(FunctionDef *def, bool inMacro = false);
    bool parseMaybeFunction(const ClassDef *cdef, FunctionDef *def);

    void parseSlots(ClassDef *def, FunctionDef::Access access);
    void parseSignals(ClassDef *def);
    void parseProperty(ClassDef *def);
    void createPropertyDef(PropertyDef &def);
    void parseEnumOrFlag(ClassDef *def, bool isFlag);
    void parseFlag(ClassDef *def);
    void parseClassInfo(ClassDef *def);
    void parseInterfaces(ClassDef *def);
    void parseDeclareInterface();
    void parseDeclareMetatype();
    void parseSlotInPrivate(ClassDef *def, FunctionDef::Access access);
    void parsePrivateProperty(ClassDef *def);

    void parseFunctionArguments(FunctionDef *def);

    QByteArray lexemUntil(Token);
    bool until(Token);

    // test for Q_INVOCABLE, Q_SCRIPTABLE, etc. and set the flags
    // in FunctionDef accordingly
    bool testFunctionAttribute(FunctionDef *def);
    bool testFunctionAttribute(Token tok, FunctionDef *def);

    void checkSuperClasses(ClassDef *def);
    void checkProperties(ClassDef* cdef);
};

inline QByteArray noRef(const QByteArray &type)
{
    if (type.endsWith('&')) {
        if (type.endsWith("&&"))
            return type.left(type.length()-2);
        return type.left(type.length()-1);
    }
    return type;
}

QT_END_NAMESPACE

#endif // MOC_H
