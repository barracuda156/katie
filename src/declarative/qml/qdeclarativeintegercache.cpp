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

#include "qdeclarativeintegercache_p.h"

#include "qdeclarativeengine_p.h"
#include "qdeclarativemetatype_p.h"

QT_BEGIN_NAMESPACE

QDeclarativeIntegerCache::QDeclarativeIntegerCache(QDeclarativeEngine *e)
: QDeclarativeCleanup(e), engine(e)
{
}

QDeclarativeIntegerCache::~QDeclarativeIntegerCache()
{
    clear();
}

void QDeclarativeIntegerCache::clear()
{
    qDeleteAll(stringCache);
    stringCache.clear();
    identifierCache.clear();
    engine = 0;
}

QString QDeclarativeIntegerCache::findId(int value) const
{
    for (StringCache::ConstIterator iter = stringCache.begin();
            iter != stringCache.end(); ++iter) {
        if (iter.value() && iter.value()->value == value)
            return iter.key();
    }
    return QString();
}

void QDeclarativeIntegerCache::add(const QString &id, int value)
{
    Q_ASSERT(!stringCache.contains(id));

    QDeclarativeEnginePrivate *enginePriv = QDeclarativeEnginePrivate::get(engine);

    // ### use contextClass
    Data *d = new Data(enginePriv->objectClass->createPersistentIdentifier(id), value);

    stringCache.insert(id, d);
    identifierCache.insert(d->identifier, d);
}

int QDeclarativeIntegerCache::value(const QString &id)
{
    Data *d = stringCache.value(id);
    return d?d->value:-1;
}

QT_END_NAMESPACE
