/****************************************************************************
**
** Copyright (C) 2015 The Qt Company Ltd.
** Copyright (C) 2016 Ivailo Monev
**
** This file is part of the QtDeclarative module of the Katie Toolkit.
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

#ifndef QDECLARATIVETYPENAMECACHE_P_H
#define QDECLARATIVETYPENAMECACHE_P_H

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

#include "qdeclarativerefcount_p.h"
#include "qdeclarativecleanup_p.h"

#include "qscriptdeclarativeclass_p.h"

QT_BEGIN_NAMESPACE

class QDeclarativeType;
class QDeclarativeEngine;
class QDeclarativeTypeNameCache : public QDeclarativeRefCount, public QDeclarativeCleanup
{
public:
    QDeclarativeTypeNameCache(QDeclarativeEngine *);
    virtual ~QDeclarativeTypeNameCache();

    struct Data {
        inline Data();
        inline ~Data();
        QDeclarativeType *type;
        QDeclarativeTypeNameCache *typeNamespace;
        int importedScriptIndex;
    };

    void add(const QString &, int);
    void add(const QString &, QDeclarativeType *);
    void add(const QString &, QDeclarativeTypeNameCache *);

    Data *data(const QString &) const;
    inline Data *data(const QScriptDeclarativeClass::Identifier &id) const;

protected:
    virtual void clear();

private:
    struct RData : public Data { 
        QScriptDeclarativeClass::PersistentIdentifier identifier;
    };
    typedef QHash<QString, RData *> StringCache;
    typedef QHash<QScriptDeclarativeClass::Identifier, RData *> IdentifierCache;

    StringCache stringCache;
    IdentifierCache identifierCache;
    QDeclarativeEngine *engine;
};

QDeclarativeTypeNameCache::Data::Data()
: type(0), typeNamespace(0), importedScriptIndex(-1)
{
}

QDeclarativeTypeNameCache::Data::~Data()
{
    if (typeNamespace) typeNamespace->release();
}

QDeclarativeTypeNameCache::Data *QDeclarativeTypeNameCache::data(const QScriptDeclarativeClass::Identifier &id) const
{
    return identifierCache.value(id);
}

QT_END_NAMESPACE

#endif // QDECLARATIVETYPENAMECACHE_P_H

