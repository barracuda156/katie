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

#ifndef QGRAPHICSITEM_P_H
#define QGRAPHICSITEM_P_H

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

#include "qgraphicsitem.h"
#include "qset.h"
#include "qpixmapcache.h"
#include "qgraphicsview_p.h"
#include "qgraphicstransform.h"
#include "qgraphicstransform_p.h"
#include "qpoint.h"

#ifndef QT_NO_GRAPHICSVIEW

QT_BEGIN_NAMESPACE

class QGraphicsItemCache
{
public:
    QGraphicsItemCache() : allExposed(false) { }

    // ItemCoordinateCache only
    QRect boundingRect;
    QSize fixedSize;
    QPixmapCache::Key key;

    // DeviceCoordinateCache only
    struct DeviceData {
        DeviceData() {}
        QTransform lastTransform;
        QPoint cacheIndent;
        QPixmapCache::Key key;
    };
    QMap<QPaintDevice *, DeviceData> deviceData;

    // List of logical exposed rects
    QVector<QRectF> exposed;
    bool allExposed;

    // Empty cache
    void purge();
};

class Q_GUI_EXPORT QGraphicsItemPrivate
{
    Q_DECLARE_PUBLIC(QGraphicsItem)
public:
    enum Extra {
        ExtraToolTip,
        ExtraCursor,
        ExtraCacheData,
        ExtraBoundingRegionGranularity
    };

    enum AncestorFlag {
        NoFlag = 0,
        AncestorHandlesChildEvents = 0x1,
        AncestorClipsChildren = 0x2,
        AncestorIgnoresTransformations = 0x4,
        AncestorFiltersChildEvents = 0x8
    };

    inline QGraphicsItemPrivate()
        : z(0),
        opacity(1.),
        scene(0),
        parent(0),
        transformData(0),
        index(-1),
        siblingIndex(-1),
        itemDepth(-1),
        focusProxy(0),
        subFocusItem(0),
        focusScopeItem(0),
        panelModality(QGraphicsItem::NonModal),
        visible(true),
        explicitlyHidden(false),
        enabled(true),
        explicitlyDisabled(false),
        selected(false),
        acceptsHover(false),
        acceptDrops(false),
        isMemberOfGroup(false),
        handlesChildEvents(false),
        itemDiscovered(false),
        hasCursor(false),
        hasBoundingRegionGranularity(false),
        isWidget(false),
        dirty(false),
        dirtyChildren(false),
        localCollisionHack(false),
        inSetPosHelper(false),
        needSortChildren(false),
        allChildrenDirty(false),
        fullUpdatePending(false),
        dirtyChildrenBoundingRect(true),
        paintedViewBoundingRectsNeedRepaint(false),
        dirtySceneTransform(true),
        geometryChanged(true),
        inDestructor(false),
        isObject(false),
        ignoreVisible(false),
        ignoreOpacity(false),
        filtersDescendantEvents(false),
        sceneTransformTranslateOnly(false),
        explicitActivate(false),
        wantsActive(false),
        holesInSiblingIndex(false),
        sequentialOrdering(true),
        scenePosDescendants(false),
        pendingPolish(false),
        cacheMode(QGraphicsItem::NoCache),
        acceptedMouseButtons(Qt::LeftButton | Qt::RightButton | Qt::MiddleButton),
        flags(0),
        ancestorFlags(0),
        q_ptr(0)
    {
    }

    inline virtual ~QGraphicsItemPrivate()
    { }

    static const QGraphicsItemPrivate *get(const QGraphicsItem *item)
    {
        return item->d_ptr.data();
    }
    static QGraphicsItemPrivate *get(QGraphicsItem *item)
    {
        return item->d_ptr.data();
    }

    void updateAncestorFlag(QGraphicsItem::GraphicsItemFlag childFlag,
                            AncestorFlag flag = NoFlag, bool enabled = false, bool root = true);
    void updateAncestorFlags();
    void setIsMemberOfGroup(bool enabled);
    void remapItemPos(QEvent *event, QGraphicsItem *item);
    QPointF genericMapFromScene(const QPointF &pos, const QWidget *viewport) const;
    inline bool itemIsUntransformable() const
    {
        return (flags & QGraphicsItem::ItemIgnoresTransformations)
            || (ancestorFlags & AncestorIgnoresTransformations);
    }

    void combineTransformToParent(QTransform *x) const;
    void combineTransformFromParent(QTransform *x) const;
    void updateSceneTransformFromParent();

    static bool movableAncestorIsSelected(const QGraphicsItem *item);

    void setPosHelper(const QPointF &pos);
    void setTransformHelper(const QTransform &transform);
    void prependGraphicsTransform(QGraphicsTransform *t);
    void appendGraphicsTransform(QGraphicsTransform *t);
    void setVisibleHelper(bool newVisible, bool explicitly, bool update = true);
    void setEnabledHelper(bool newEnabled, bool explicitly, bool update = true);
    bool discardUpdateRequest(bool ignoreVisibleBit = false,
                              bool ignoreDirtyBit = false, bool ignoreOpacity = false) const;
    int depth() const;
    void invalidateDepthRecursively();
    void resolveDepth();
    void addChild(QGraphicsItem *child);
    void removeChild(QGraphicsItem *child);
    void setParentItemHelper(QGraphicsItem *parent, const QVariant *newParentVariant,
                             const QVariant *thisPointerVariant);
    void childrenBoundingRectHelper(QTransform *x, QRectF *rect, QGraphicsItem *topMostEffectItem);
    void initStyleOption(QStyleOptionGraphicsItem *option, const QTransform &worldTransform,
                         const QRegion &exposedRegion, bool allItems = false) const;
    QRectF sceneEffectiveBoundingRect() const;

    virtual void resolveFont(uint inheritedMask)
    {
        for (int i = 0; i < children.size(); ++i)
            children.at(i)->d_ptr->resolveFont(inheritedMask);
    }

    virtual void resolvePalette(uint inheritedMask)
    {
        for (int i = 0; i < children.size(); ++i)
            children.at(i)->d_ptr->resolveFont(inheritedMask);
    }

    virtual bool isProxyWidget() const;

    inline QVariant extra(Extra type) const
    {
        for (int i = 0; i < extras.size(); ++i) {
            const ExtraStruct &extra = extras.at(i);
            if (extra.type == type)
                return extra.value;
        }
        return QVariant();
    }

    inline void setExtra(Extra type, const QVariant &value)
    {
        for (int i = 0; i < extras.size(); ++i) {
            if (extras.at(i).type == type) {
                extras[i].value = value;
                return;
            }
        }

        extras << ExtraStruct(type, value);
    }

    inline void unsetExtra(Extra type)
    {
        for (int i = 0; i < extras.size(); ++i) {
            if (extras.at(i).type == type) {
                extras.removeAt(i);
                return;
            }
        }
    }

    struct ExtraStruct {
        ExtraStruct(Extra type, QVariant value)
            : type(type), value(value)
        { }

        Extra type;
        QVariant value;

        bool operator<(Extra extra) const
        { return type < extra; }
    };

    QList<ExtraStruct> extras;

    QGraphicsItemCache *extraItemCache();
    void removeExtraItemCache();

    void updatePaintedViewBoundingRects(bool updateChildren);
    void ensureSceneTransformRecursive(QGraphicsItem **topMostDirtyItem);
    inline void ensureSceneTransform()
    {
        QGraphicsItem *that = q_func();
        ensureSceneTransformRecursive(&that);
    }

    inline bool hasTranslateOnlySceneTransform()
    {
        ensureSceneTransform();
        return sceneTransformTranslateOnly;
    }

    inline void invalidateChildrenSceneTransform()
    {
        for (int i = 0; i < children.size(); ++i)
            children.at(i)->d_ptr->dirtySceneTransform = true;
    }

    inline qreal calcEffectiveOpacity() const
    {
        qreal o = opacity;
        QGraphicsItem *p = parent;
        int myFlags = flags;
        while (p) {
            int parentFlags = p->d_ptr->flags;

            // If I have a parent, and I don't ignore my parent's opacity, and my
            // parent propagates to me, then combine my local opacity with my parent's
            // effective opacity into my effective opacity.
            if ((myFlags & QGraphicsItem::ItemIgnoresParentOpacity)
                || (parentFlags & QGraphicsItem::ItemDoesntPropagateOpacityToChildren)) {
                break;
            }

            o *= p->d_ptr->opacity;
            p = p->d_ptr->parent;
            myFlags = parentFlags;
        }
        return o;
    }

    inline bool isOpacityNull() const
    { return (opacity < qreal(0.001)); }

    static inline bool isOpacityNull(qreal opacity)
    { return (opacity < qreal(0.001)); }

    inline bool isFullyTransparent() const
    {
        if (isOpacityNull())
            return true;
        if (!parent)
            return false;

        return isOpacityNull(calcEffectiveOpacity());
    }

    inline qreal effectiveOpacity() const {
        if (!parent || !opacity)
            return opacity;

        return calcEffectiveOpacity();
    }

    inline qreal combineOpacityFromParent(qreal parentOpacity) const
    {
        if (parent && !(flags & QGraphicsItem::ItemIgnoresParentOpacity)
            && !(parent->d_ptr->flags & QGraphicsItem::ItemDoesntPropagateOpacityToChildren)) {
            return parentOpacity * opacity;
        }
        return opacity;
    }

    inline bool childrenCombineOpacity() const
    {
        if (!children.size())
            return true;
        if (flags & QGraphicsItem::ItemDoesntPropagateOpacityToChildren)
            return false;

        for (int i = 0; i < children.size(); ++i) {
            if (children.at(i)->d_ptr->flags & QGraphicsItem::ItemIgnoresParentOpacity)
                return false;
        }
        return true;
    }

    inline bool childrenClippedToShape() const
    { return (flags & QGraphicsItem::ItemClipsChildrenToShape) || children.isEmpty(); }

    inline bool isInvisible() const
    {
        return !visible || (childrenCombineOpacity() && isFullyTransparent());
    }

    inline void markParentDirty(bool updateBoundingRect = false);

    void setFocusHelper(Qt::FocusReason focusReason, bool climb, bool focusFromHide);
    void clearFocusHelper(bool giveFocusToParent);
    void setSubFocus(QGraphicsItem *rootItem = 0, QGraphicsItem *stopItem = 0);
    void clearSubFocus(QGraphicsItem *rootItem = 0, QGraphicsItem *stopItem = 0);
    void resetFocusProxy();

    inline QTransform transformToParent() const;
    inline void ensureSortedChildren();
    static inline bool insertionOrder(QGraphicsItem *a, QGraphicsItem *b);
    void ensureSequentialSiblingIndex();
    inline void sendScenePosChange();

    QRectF childrenBoundingRect;
    QRectF needsRepaint;
    QMap<QWidget *, QRect> paintedViewBoundingRects;
    QPointF pos;
    qreal z;
    qreal opacity;
    QGraphicsScene *scene;
    QGraphicsItem *parent;
    QList<QGraphicsItem *> children;
    struct TransformData;
    TransformData *transformData;
    QTransform sceneTransform;
    int index;
    int siblingIndex;
    int itemDepth;  // Lazily calculated when calling depth().
    QGraphicsItem *focusProxy;
    QList<QGraphicsItem **> focusProxyRefs;
    QGraphicsItem *subFocusItem;
    QGraphicsItem *focusScopeItem;
    QGraphicsItem::PanelModality panelModality;

    bool visible;
    bool explicitlyHidden;
    bool enabled;
    bool explicitlyDisabled;
    bool selected;
    bool acceptsHover;
    bool acceptDrops;
    bool isMemberOfGroup;
    bool handlesChildEvents;
    bool itemDiscovered;
    bool hasCursor;
    bool hasBoundingRegionGranularity;
    bool isWidget;
    bool dirty;
    bool dirtyChildren;
    bool localCollisionHack;
    bool inSetPosHelper;
    bool needSortChildren;
    bool allChildrenDirty;
    bool fullUpdatePending;
    bool dirtyChildrenBoundingRect;
    bool paintedViewBoundingRectsNeedRepaint;
    bool dirtySceneTransform;
    bool geometryChanged;
    bool inDestructor;
    bool isObject;
    bool ignoreVisible;
    bool ignoreOpacity;
    bool filtersDescendantEvents;
    bool sceneTransformTranslateOnly;
    bool explicitActivate;
    bool wantsActive;
    bool holesInSiblingIndex;
    bool sequentialOrdering;
    bool scenePosDescendants;
    bool pendingPolish;

    QGraphicsItem::CacheMode cacheMode;
    Qt::MouseButtons acceptedMouseButtons;
    QGraphicsItem::GraphicsItemFlags flags;
    uint ancestorFlags;

    QMap<int, QVariant> datastore;

    QGraphicsItem *q_ptr;
};

struct QGraphicsItemPrivate::TransformData
{
    QTransform transform;
    qreal scale;
    qreal rotation;
    qreal xOrigin;
    qreal yOrigin;
    QList<QGraphicsTransform *> graphicsTransforms;
    bool onlyTransform;

    TransformData() :
        scale(1.0), rotation(0.0),
        xOrigin(0.0), yOrigin(0.0),
        onlyTransform(true)
    { }

    QTransform computedFullTransform(QTransform *postmultiplyTransform = 0) const
    {
        if (onlyTransform) {
            if (!postmultiplyTransform || postmultiplyTransform->isIdentity())
                return transform;
            if (transform.isIdentity())
                return *postmultiplyTransform;
            return transform * *postmultiplyTransform;
        }

        QTransform x(transform);
        if (!graphicsTransforms.isEmpty()) {
            QMatrix4x4 m;
            for (int i = 0; i < graphicsTransforms.size(); ++i)
                graphicsTransforms.at(i)->applyTo(&m);
            x *= m.toTransform();
        }
        x.translate(xOrigin, yOrigin);
        x.rotate(rotation);
        x.scale(scale, scale);
        x.translate(-xOrigin, -yOrigin);
        if (postmultiplyTransform)
            x *= *postmultiplyTransform;
        return x;
    }
};

/*!
    Returns true if \a item1 is on top of \a item2.
    The items don't need to be siblings.

    \internal
*/
inline bool qt_closestItemFirst(const QGraphicsItem *item1, const QGraphicsItem *item2)
{
    // Siblings? Just check their z-values.
    const QGraphicsItemPrivate *d1 = item1->d_ptr.data();
    const QGraphicsItemPrivate *d2 = item2->d_ptr.data();
    if (d1->parent == d2->parent)
        return qt_closestLeaf(item1, item2);

    // Find common ancestor, and each item's ancestor closest to the common
    // ancestor.
    int item1Depth = d1->depth();
    int item2Depth = d2->depth();
    const QGraphicsItem *p = item1;
    const QGraphicsItem *t1 = item1;
    while (item1Depth > item2Depth && (p = p->d_ptr->parent)) {
        if (p == item2) {
            // item2 is one of item1's ancestors; item1 is on top
            return !(t1->d_ptr->flags & QGraphicsItem::ItemStacksBehindParent);
        }
        t1 = p;
        --item1Depth;
    }
    p = item2;
    const QGraphicsItem *t2 = item2;
    while (item2Depth > item1Depth && (p = p->d_ptr->parent)) {
        if (p == item1) {
            // item1 is one of item2's ancestors; item1 is not on top
            return (t2->d_ptr->flags & QGraphicsItem::ItemStacksBehindParent);
        }
        t2 = p;
        --item2Depth;
    }

    // item1Ancestor is now at the same level as item2Ancestor, but not the same.
    const QGraphicsItem *p1 = t1;
    const QGraphicsItem *p2 = t2;
    while (t1 && t2 && t1 != t2) {
        p1 = t1;
        p2 = t2;
        t1 = t1->d_ptr->parent;
        t2 = t2->d_ptr->parent;
    }

    // in case we have a common ancestor, we compare the immediate children in the ancestor's path.
    // otherwise we compare the respective items' topLevelItems directly.
    return qt_closestLeaf(p1, p2);
}

/*!
    Returns true if \a item2 is on top of \a item1.
    The items don't need to be siblings.

    \internal
*/
inline bool qt_closestItemLast(const QGraphicsItem *item1, const QGraphicsItem *item2)
{
    return qt_closestItemFirst(item2, item1);
}

/*!
    \internal
*/
inline bool qt_closestLeaf(const QGraphicsItem *item1, const QGraphicsItem *item2)
{
    // Return true if sibling item1 is on top of item2.
    const QGraphicsItemPrivate *d1 = item1->d_ptr.data();
    const QGraphicsItemPrivate *d2 = item2->d_ptr.data();
    bool f1 = d1->flags & QGraphicsItem::ItemStacksBehindParent;
    bool f2 = d2->flags & QGraphicsItem::ItemStacksBehindParent;
    if (f1 != f2)
        return f2;
    if (d1->z != d2->z)
        return d1->z > d2->z;
    return d1->siblingIndex > d2->siblingIndex;
}

/*!
    \internal
*/
inline bool qt_notclosestLeaf(const QGraphicsItem *item1, const QGraphicsItem *item2)
{ return qt_closestLeaf(item2, item1); }

/*
   return the full transform of the item to the parent.  This include the position and all the transform data
*/
inline QTransform QGraphicsItemPrivate::transformToParent() const
{
    QTransform matrix;
    combineTransformToParent(&matrix);
    return matrix;
}

/*!
    \internal
*/
inline void QGraphicsItemPrivate::ensureSortedChildren()
{
    if (needSortChildren) {
        needSortChildren = false;
        sequentialOrdering = true;
        if (children.isEmpty())
            return;
        qSort(children.begin(), children.end(), qt_notclosestLeaf);
        for (int i = 0; i < children.size(); ++i) {
            if (children.at(i)->d_ptr->siblingIndex != i) {
                sequentialOrdering = false;
                break;
            }
        }
    }
}

/*!
    \internal
*/
inline bool QGraphicsItemPrivate::insertionOrder(QGraphicsItem *a, QGraphicsItem *b)
{
    return a->d_ptr->siblingIndex < b->d_ptr->siblingIndex;
}

/*!
    \internal
*/
inline void QGraphicsItemPrivate::markParentDirty(bool updateBoundingRect)
{
    QGraphicsItemPrivate *parentp = this;
    while (parentp->parent) {
        parentp = parentp->parent->d_ptr.data();
        parentp->dirtyChildren = 1;

        if (updateBoundingRect) {
            parentp->dirtyChildrenBoundingRect = true;
        }
    }
}

QT_END_NAMESPACE

#endif // QT_NO_GRAPHICSVIEW

#endif
