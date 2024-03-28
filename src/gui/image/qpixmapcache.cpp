/****************************************************************************
**
** Copyright (C) 2015 The Qt Company Ltd.
** Copyright (C) 2016 Ivailo Monev
**
** This file is part of the QtGui module of the Katie Toolkit.
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

#include "qpixmapcache.h"
#include "qcache.h"

QT_BEGIN_NAMESPACE

/*!
    \class QPixmapCache

    \brief The QPixmapCache class provides an application-wide cache for pixmaps.

    \ingroup painting

    This class is a tool for optimized drawing with QPixmap. You can
    use it to store temporary pixmaps that are expensive to generate
    without using more storage space than cacheLimit(). Use insert()
    to insert pixmaps, find() to find them, and clear() to empty the
    cache.

    QPixmapCache contains no member data, only static functions to
    access the global pixmap cache. It creates an internal QCache
    object for caching the pixmaps.

    The cache associates a pixmap with a user-provided string as a key,
    or generates one if not provided.

    If two pixmaps are inserted into the cache using equal keys then the
    last pixmap will replace the first pixmap in the cache. This follows the
    behavior of the QHash and QCache classes.

    The cache becomes full when the total size of all pixmaps in the
    cache exceeds cacheLimit(). The initial cache limit is 10240 KB
    (10 MB) on desktop, the value can be changed by calling
    setCacheLimit() with the required value.
    A pixmap takes roughly (\e{width} * \e{height} * \e{depth})/8 bytes of
    memory.

    The \e{Qt Quarterly} article
    \l{http://doc.qt.digia.com/qq/qq12-qpixmapcache.html}{Optimizing
    with QPixmapCache} explains how to use QPixmapCache to speed up
    applications by caching the results of painting.

    \sa QCache, QPixmap
*/

typedef QCache<QByteArray, QPixmap> PixmapCacheType;
Q_GLOBAL_STATIC_WITH_ARGS(PixmapCacheType, pm_cache, (10240))

/*!
    \obsolete
    \overload

    Returns the pixmap associated with the \a key in the cache, or
    null if there is no such pixmap.

    \warning If valid, you should copy the pixmap immediately (this is
    fast). Subsequent insertions into the cache could cause the
    pointer to become invalid. For this reason, we recommend you use
    bool find(const QByteArray&, QPixmap*) instead.

    Example:
    \snippet doc/src/snippets/code/src_gui_image_qpixmapcache.cpp 0
*/
QPixmap *QPixmapCache::find(const QByteArray &key)
{
    return pm_cache()->object(key);
}

/*!
    \obsolete
    \fn QPixmapCache::find(const QByteArray &key, QPixmap& pixmap)

    Use bool find(const QByteArray&, QPixmap*) instead.
*/

/*!
    Looks for a cached pixmap associated with the given \a key in the cache.
    If the pixmap is found, the function sets \a pixmap to that pixmap and
    returns true; otherwise it leaves \a pixmap alone and returns false.

    \since 4.6

    Example:
    \snippet doc/src/snippets/code/src_gui_image_qpixmapcache.cpp 1
*/
bool QPixmapCache::find(const QByteArray &key, QPixmap* pixmap)
{
    QPixmap *ptr = pm_cache()->object(key);
    if (ptr && pixmap)
        *pixmap = *ptr;
    return ptr != nullptr;
}

/*!
    Inserts a copy of the pixmap \a pixmap associated with the \a key into
    the cache.

    All pixmaps inserted by the Katie library have a key starting with
    "qt_", so your own pixmap keys should never begin "qt_".

    When a pixmap is inserted and the cache is about to exceed its
    limit, it removes pixmaps until there is enough room for the
    pixmap to be inserted.

    The oldest pixmaps (least recently accessed in the cache) are
    deleted when more space is needed.

    The function returns true if the object was inserted into the
    cache; otherwise it returns false.

    \sa setCacheLimit()
*/
bool QPixmapCache::insert(const QByteArray &key, const QPixmap &pixmap)
{
    return pm_cache()->insert(key, new QPixmap(pixmap));
}

/*!
    Inserts a copy of the given \a pixmap into the cache and returns a key
    that can be used to retrieve it.

    When a pixmap is inserted and the cache is about to exceed its
    limit, it removes pixmaps until there is enough room for the
    pixmap to be inserted.

    The oldest pixmaps (least recently accessed in the cache) are
    deleted when more space is needed.

    \sa setCacheLimit(), replace()

    \since 4.6
*/
QByteArray QPixmapCache::insert(const QPixmap &pixmap)
{
    QPixmap *cpixmap = new QPixmap(pixmap);
    const QByteArray key = QByteArray::number(cpixmap->cacheKey());
    pm_cache()->insert(key, cpixmap);
    return key;
}

/*!
    Replaces the pixmap associated with the given \a key with the \a pixmap
    specified. Returns true if the \a pixmap has been correctly inserted into
    the cache; otherwise returns false.

    \sa setCacheLimit(), insert()

    \since 4.6
*/
bool QPixmapCache::replace(const QByteArray &key, const QPixmap &pixmap)
{
    return pm_cache()->insert(key, new QPixmap(pixmap));
}

/*!
    Returns the cache limit (in kilobytes).

    The default cache limit is 2048 KB on embedded platforms, 10240 KB on
    desktop platforms.

    \sa setCacheLimit()
*/
int QPixmapCache::cacheLimit()
{
    return pm_cache()->maxCost();
}

/*!
    Sets the cache limit to \a n kilobytes.

    The default setting is 2048 KB on embedded platforms, 10240 KB on
    desktop platforms.

    \sa cacheLimit()
*/
void QPixmapCache::setCacheLimit(int n)
{
    pm_cache()->setMaxCost(n);
}

/*!
  Removes the pixmap associated with \a key from the cache.
*/
void QPixmapCache::remove(const QByteArray &key)
{
    pm_cache()->remove(key);
}

/*!
    Removes all pixmaps from the cache.
*/
void QPixmapCache::clear()
{
    pm_cache()->clear();
}

QT_END_NAMESPACE
