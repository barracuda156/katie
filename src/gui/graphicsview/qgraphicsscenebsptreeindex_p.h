/****************************************************************************
**
** Copyright (C) 2015 The Qt Company Ltd.
** Copyright (C) 2016 Ivailo Monev
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
** $QT_END_LICENSE$
**
****************************************************************************/

//
//  W A R N I N G
//  -------------
//
// This file is not part of the Katie API.  It exists for the convenience
// of other Qt classes.  This header file may change from version to
// version without notice, or even be removed.
//
// We mean it.
//

#ifndef QGRAPHICSBSPTREEINDEX_H
#define QGRAPHICSBSPTREEINDEX_H

#include <QtCore/qglobal.h>

#ifndef QT_NO_GRAPHICSVIEW

#include "qgraphicssceneindex_p.h"
#include "qgraphicsitem_p.h"
#include "qgraphicsscene_bsp_p.h"

#include <QtCore/qrect.h>
#include <QtCore/qlist.h>

QT_BEGIN_NAMESPACE

static const int QGRAPHICSSCENE_INDEXTIMER_TIMEOUT = 2000;

class QGraphicsScene;
class QGraphicsSceneBspTreeIndexPrivate;

class Q_AUTOTEST_EXPORT QGraphicsSceneBspTreeIndex : public QGraphicsSceneIndex
{
    Q_OBJECT
    Q_PROPERTY(int bspTreeDepth READ bspTreeDepth WRITE setBspTreeDepth)
public:
    QGraphicsSceneBspTreeIndex(QGraphicsScene *scene = 0);
    ~QGraphicsSceneBspTreeIndex();

    QList<QGraphicsItem *> estimateItems(const QRectF &rect, Qt::SortOrder order) const;
    QList<QGraphicsItem *> estimateTopLevelItems(const QRectF &rect, Qt::SortOrder order) const;
    QList<QGraphicsItem *> items(Qt::SortOrder order = Qt::DescendingOrder) const;

    int bspTreeDepth();
    void setBspTreeDepth(int depth);

protected Q_SLOTS:
    void updateSceneRect(const QRectF &rect);

protected:
    bool event(QEvent *event);
    void clear();

    void addItem(QGraphicsItem *item);
    void removeItem(QGraphicsItem *item);
    void prepareBoundingRectChange(const QGraphicsItem *item);

    void itemChange(const QGraphicsItem *item, QGraphicsItem::GraphicsItemChange change, const void *const value);

private :
    Q_DECLARE_PRIVATE(QGraphicsSceneBspTreeIndex)
    Q_DISABLE_COPY(QGraphicsSceneBspTreeIndex)

    friend class QGraphicsScene;
    friend class QGraphicsScenePrivate;
};

class QGraphicsSceneBspTreeIndexPrivate : public QGraphicsSceneIndexPrivate
{
    Q_DECLARE_PUBLIC(QGraphicsSceneBspTreeIndex)
public:
    QGraphicsSceneBspTreeIndexPrivate(QGraphicsScene *scene);

    QGraphicsSceneBspTree bsp;
    QRectF sceneRect;
    int bspTreeDepth;
    int indexTimerId;
    bool restartIndexTimer;
    bool regenerateIndex;
    int lastItemCount;

    QList<QGraphicsItem *> indexedItems;
    QList<QGraphicsItem *> unindexedItems;
    QList<QGraphicsItem *> untransformableItems;
    QList<int> freeItemIndexes;

    bool purgePending;
    QSet<QGraphicsItem *> removedItems;
    void purgeRemovedItems();

    void updateIndex();
    void startIndexTimer(int interval = QGRAPHICSSCENE_INDEXTIMER_TIMEOUT);
    void resetIndex();

    void addItem(QGraphicsItem *item, bool recursive = false);
    void removeItem(QGraphicsItem *item, bool recursive = false, bool moveToUnindexedItems = false);
    QList<QGraphicsItem *> estimateItems(const QRectF &, Qt::SortOrder, bool b = false);

    static void sortItems(QList<QGraphicsItem *> *itemList, Qt::SortOrder order,
                          bool onlyTopLevelItems = false);
};

QT_END_NAMESPACE

#endif // QT_NO_GRAPHICSVIEW

#endif // QGRAPHICSBSPTREEINDEX_H
