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

#ifndef QGRAPHICSITEM_H
#define QGRAPHICSITEM_H

#include <QtCore/qglobal.h>
#include <QtCore/qobject.h>
#include <QtCore/qvariant.h>
#include <QtCore/qrect.h>
#include <QtCore/qscopedpointer.h>
#include <QtGui/qpainterpath.h>
#include <QtGui/qpixmap.h>

class tst_QGraphicsItem;


QT_BEGIN_NAMESPACE

#ifndef QT_NO_GRAPHICSVIEW

class QBrush;
class QCursor;
class QFocusEvent;
class QGraphicsItemGroup;
class QGraphicsObject;
class QGraphicsSceneContextMenuEvent;
class QGraphicsSceneDragDropEvent;
class QGraphicsSceneHoverEvent;
class QGraphicsSceneMouseEvent;
class QGraphicsSceneWheelEvent;
class QGraphicsScene;
class QGraphicsWidget;
class QKeyEvent;
class QMatrix;
class QPainter;
class QPen;
class QPointF;
class QRectF;
class QStyleOptionGraphicsItem;

class QGraphicsItemPrivate;
class Q_GUI_EXPORT QGraphicsItem
{
public:
    enum GraphicsItemFlag {
        ItemIsMovable = 0x1,
        ItemIsSelectable = 0x2,
        ItemIsFocusable = 0x4,
        ItemClipsToShape = 0x8,
        ItemClipsChildrenToShape = 0x10,
        ItemIgnoresTransformations = 0x20,
        ItemIgnoresParentOpacity = 0x40,
        ItemDoesntPropagateOpacityToChildren = 0x80,
        ItemStacksBehindParent = 0x100,
        ItemUsesExtendedStyleOption = 0x200,
        ItemHasNoContents = 0x400,
        ItemSendsGeometryChanges = 0x800,
        ItemNegativeZStacksBehindParent = 0x2000,
        ItemIsPanel = 0x4000,
        ItemIsFocusScope = 0x8000, // internal
        ItemSendsScenePositionChanges = 0x10000,
        ItemStopsClickFocusPropagation = 0x20000,
        ItemStopsFocusHandling = 0x40000
        // NB! Don't forget to increase the d_ptr->flags bit field by 1 when adding a new flag.
    };
    Q_DECLARE_FLAGS(GraphicsItemFlags, GraphicsItemFlag)

    enum GraphicsItemChange {
        ItemPositionChange,
        ItemMatrixChange,
        ItemVisibleChange,
        ItemEnabledChange,
        ItemSelectedChange,
        ItemParentChange,
        ItemChildAddedChange,
        ItemChildRemovedChange,
        ItemTransformChange,
        ItemPositionHasChanged,
        ItemTransformHasChanged,
        ItemSceneChange,
        ItemVisibleHasChanged,
        ItemEnabledHasChanged,
        ItemSelectedHasChanged,
        ItemParentHasChanged,
        ItemSceneHasChanged,
        ItemCursorChange,
        ItemCursorHasChanged,
        ItemToolTipChange,
        ItemToolTipHasChanged,
        ItemFlagsChange,
        ItemFlagsHaveChanged,
        ItemZValueChange,
        ItemZValueHasChanged,
        ItemOpacityChange,
        ItemOpacityHasChanged,
        ItemScenePositionHasChanged,
        ItemRotationChange,
        ItemRotationHasChanged,
        ItemScaleChange,
        ItemScaleHasChanged,
        ItemTransformOriginPointChange,
        ItemTransformOriginPointHasChanged
    };

    enum CacheMode {
        NoCache,
        ItemCoordinateCache,
        DeviceCoordinateCache
    };

    enum PanelModality
    {
        NonModal,
        PanelModal,
        SceneModal
    };

    QGraphicsItem(QGraphicsItem *parent = 0
                  // ### obsolete argument
                  , QGraphicsScene *scene = 0
        );
    virtual ~QGraphicsItem();

    QGraphicsScene *scene() const;

    QGraphicsItem *parentItem() const;
    QGraphicsItem *topLevelItem() const;
    QGraphicsObject *parentObject() const;
    QGraphicsWidget *parentWidget() const;
    QGraphicsWidget *topLevelWidget() const;
    QGraphicsWidget *window() const;
    QGraphicsItem *panel() const;
    void setParentItem(QGraphicsItem *parent);
    QList<QGraphicsItem *> childItems() const;
    bool isWidget() const;
    bool isWindow() const;
    bool isPanel() const;

    QGraphicsObject *toGraphicsObject();
    const QGraphicsObject *toGraphicsObject() const;

    QGraphicsItemGroup *group() const;
    void setGroup(QGraphicsItemGroup *group);

    GraphicsItemFlags flags() const;
    void setFlag(GraphicsItemFlag flag, bool enabled = true);
    void setFlags(GraphicsItemFlags flags);

    CacheMode cacheMode() const;
    void setCacheMode(CacheMode mode, const QSize &cacheSize = QSize());

    PanelModality panelModality() const;
    void setPanelModality(PanelModality panelModality);
    bool isBlockedByModalPanel(QGraphicsItem **blockingPanel = 0) const;

#ifndef QT_NO_TOOLTIP
    QString toolTip() const;
    void setToolTip(const QString &toolTip);
#endif

#ifndef QT_NO_CURSOR
    QCursor cursor() const;
    void setCursor(const QCursor &cursor);
    bool hasCursor() const;
    void unsetCursor();
#endif

    bool isVisible() const;
    bool isVisibleTo(const QGraphicsItem *parent) const;
    void setVisible(bool visible);
    inline void hide() { setVisible(false); }
    inline void show() { setVisible(true); }

    bool isEnabled() const;
    void setEnabled(bool enabled);

    bool isSelected() const;
    void setSelected(bool selected);

    bool acceptDrops() const;
    void setAcceptDrops(bool on);

    qreal opacity() const;
    qreal effectiveOpacity() const;
    void setOpacity(qreal opacity);

    Qt::MouseButtons acceptedMouseButtons() const;
    void setAcceptedMouseButtons(Qt::MouseButtons buttons);

    bool acceptHoverEvents() const;
    void setAcceptHoverEvents(bool enabled);

    bool filtersChildEvents() const;
    void setFiltersChildEvents(bool enabled);

    bool handlesChildEvents() const;
    void setHandlesChildEvents(bool enabled);

    bool isActive() const;
    void setActive(bool active);

    bool hasFocus() const;
    void setFocus(Qt::FocusReason focusReason = Qt::OtherFocusReason);
    void clearFocus();

    QGraphicsItem *focusProxy() const;
    void setFocusProxy(QGraphicsItem *item);

    QGraphicsItem *focusItem() const;
    QGraphicsItem *focusScopeItem() const;

    void grabMouse();
    void ungrabMouse();
    void grabKeyboard();
    void ungrabKeyboard();

    // Positioning in scene coordinates
    QPointF pos() const;
    inline qreal x() const { return pos().x(); }
    void setX(qreal x);
    inline qreal y() const { return pos().y(); }
    void setY(qreal y);
    QPointF scenePos() const;
    void setPos(const QPointF &pos);
    inline void setPos(qreal x, qreal y);
    inline void moveBy(qreal dx, qreal dy) { setPos(pos().x() + dx, pos().y() + dy); }

    void ensureVisible(const QRectF &rect = QRectF(), int xmargin = 50, int ymargin = 50);
    inline void ensureVisible(qreal x, qreal y, qreal w, qreal h, int xmargin = 50, int ymargin = 50);

    // Local transformation
    QMatrix matrix() const;
    QMatrix sceneMatrix() const;
    void setMatrix(const QMatrix &matrix, bool combine = false);
    void resetMatrix();
    QTransform transform() const;
    QTransform sceneTransform() const;
    QTransform deviceTransform(const QTransform &viewportTransform) const;
    QTransform itemTransform(const QGraphicsItem *other, bool *ok = 0) const;
    void setTransform(const QTransform &matrix, bool combine = false);
    void resetTransform();

    void rotate(qreal angle);           // ### obsolete
    void scale(qreal sx, qreal sy);     // ### obsolete
    void shear(qreal sh, qreal sv);     // ### obsolete
    void translate(qreal dx, qreal dy); // ### obsolete

    void setRotation(qreal angle);
    qreal rotation() const;

    void setScale(qreal scale);
    qreal scale() const;

    QPointF transformOriginPoint() const;
    void setTransformOriginPoint(const QPointF &origin);
    inline void setTransformOriginPoint(qreal ax, qreal ay)
    { setTransformOriginPoint(QPointF(ax,ay)); }

    virtual void advance(int phase);

    // Stacking order
    qreal zValue() const;
    void setZValue(qreal z);
    void stackBefore(const QGraphicsItem *sibling);

    // Hit test
    virtual QRectF boundingRect() const = 0;
    QRectF childrenBoundingRect() const;
    QRectF sceneBoundingRect() const;
    virtual QPainterPath shape() const;
    bool isClipped() const;
    QPainterPath clipPath() const;
    virtual bool contains(const QPointF &point) const;
    virtual bool collidesWithItem(const QGraphicsItem *other, Qt::ItemSelectionMode mode = Qt::IntersectsItemShape) const;
    virtual bool collidesWithPath(const QPainterPath &path, Qt::ItemSelectionMode mode = Qt::IntersectsItemShape) const;
    QList<QGraphicsItem *> collidingItems(Qt::ItemSelectionMode mode = Qt::IntersectsItemShape) const;
    bool isObscured(const QRectF &rect = QRectF()) const;
    inline bool isObscured(qreal x, qreal y, qreal w, qreal h) const;
    virtual bool isObscuredBy(const QGraphicsItem *item) const;
    virtual QPainterPath opaqueArea() const;

    QRegion boundingRegion(const QTransform &itemToDeviceTransform) const;
    qreal boundingRegionGranularity() const;
    void setBoundingRegionGranularity(qreal granularity);

    // Drawing
    virtual void paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget = 0) = 0;
    void update(const QRectF &rect = QRectF());
    inline void update(qreal x, qreal y, qreal width, qreal height);
    void scroll(qreal dx, qreal dy, const QRectF &rect = QRectF());

    // Coordinate mapping
    QPointF mapToItem(const QGraphicsItem *item, const QPointF &point) const;
    QPointF mapToParent(const QPointF &point) const;
    QPointF mapToScene(const QPointF &point) const;
    QPolygonF mapToItem(const QGraphicsItem *item, const QRectF &rect) const;
    QPolygonF mapToParent(const QRectF &rect) const;
    QPolygonF mapToScene(const QRectF &rect) const;
    QRectF mapRectToItem(const QGraphicsItem *item, const QRectF &rect) const;
    QRectF mapRectToParent(const QRectF &rect) const;
    QRectF mapRectToScene(const QRectF &rect) const;
    QPolygonF mapToItem(const QGraphicsItem *item, const QPolygonF &polygon) const;
    QPolygonF mapToParent(const QPolygonF &polygon) const;
    QPolygonF mapToScene(const QPolygonF &polygon) const;
    QPainterPath mapToItem(const QGraphicsItem *item, const QPainterPath &path) const;
    QPainterPath mapToParent(const QPainterPath &path) const;
    QPainterPath mapToScene(const QPainterPath &path) const;
    QPointF mapFromItem(const QGraphicsItem *item, const QPointF &point) const;
    QPointF mapFromParent(const QPointF &point) const;
    QPointF mapFromScene(const QPointF &point) const;
    QPolygonF mapFromItem(const QGraphicsItem *item, const QRectF &rect) const;
    QPolygonF mapFromParent(const QRectF &rect) const;
    QPolygonF mapFromScene(const QRectF &rect) const;
    QRectF mapRectFromItem(const QGraphicsItem *item, const QRectF &rect) const;
    QRectF mapRectFromParent(const QRectF &rect) const;
    QRectF mapRectFromScene(const QRectF &rect) const;
    QPolygonF mapFromItem(const QGraphicsItem *item, const QPolygonF &polygon) const;
    QPolygonF mapFromParent(const QPolygonF &polygon) const;
    QPolygonF mapFromScene(const QPolygonF &polygon) const;
    QPainterPath mapFromItem(const QGraphicsItem *item, const QPainterPath &path) const;
    QPainterPath mapFromParent(const QPainterPath &path) const;
    QPainterPath mapFromScene(const QPainterPath &path) const;

    inline QPointF mapToItem(const QGraphicsItem *item, qreal x, qreal y) const;
    inline QPointF mapToParent(qreal x, qreal y) const;
    inline QPointF mapToScene(qreal x, qreal y) const;
    inline QPolygonF mapToItem(const QGraphicsItem *item, qreal x, qreal y, qreal w, qreal h) const;
    inline QPolygonF mapToParent(qreal x, qreal y, qreal w, qreal h) const;
    inline QPolygonF mapToScene(qreal x, qreal y, qreal w, qreal h) const;
    inline QRectF mapRectToItem(const QGraphicsItem *item, qreal x, qreal y, qreal w, qreal h) const;
    inline QRectF mapRectToParent(qreal x, qreal y, qreal w, qreal h) const;
    inline QRectF mapRectToScene(qreal x, qreal y, qreal w, qreal h) const;
    inline QPointF mapFromItem(const QGraphicsItem *item, qreal x, qreal y) const;
    inline QPointF mapFromParent(qreal x, qreal y) const;
    inline QPointF mapFromScene(qreal x, qreal y) const;
    inline QPolygonF mapFromItem(const QGraphicsItem *item, qreal x, qreal y, qreal w, qreal h) const;
    inline QPolygonF mapFromParent(qreal x, qreal y, qreal w, qreal h) const;
    inline QPolygonF mapFromScene(qreal x, qreal y, qreal w, qreal h) const;
    inline QRectF mapRectFromItem(const QGraphicsItem *item, qreal x, qreal y, qreal w, qreal h) const;
    inline QRectF mapRectFromParent(qreal x, qreal y, qreal w, qreal h) const;
    inline QRectF mapRectFromScene(qreal x, qreal y, qreal w, qreal h) const;

    bool isAncestorOf(const QGraphicsItem *child) const;
    QGraphicsItem *commonAncestorItem(const QGraphicsItem *other) const;
    bool isUnderMouse() const;

    enum {
        Type = 1,
        UserType = 65536
    };
    virtual int type() const;

    void installSceneEventFilter(QGraphicsItem *filterItem);
    void removeSceneEventFilter(QGraphicsItem *filterItem);

protected:
    virtual bool sceneEventFilter(QGraphicsItem *watched, QEvent *event);
    virtual bool sceneEvent(QEvent *event);
    virtual void contextMenuEvent(QGraphicsSceneContextMenuEvent *event);
    virtual void dragEnterEvent(QGraphicsSceneDragDropEvent *event);
    virtual void dragLeaveEvent(QGraphicsSceneDragDropEvent *event);
    virtual void dragMoveEvent(QGraphicsSceneDragDropEvent *event);
    virtual void dropEvent(QGraphicsSceneDragDropEvent *event);
    virtual void focusInEvent(QFocusEvent *event);
    virtual void focusOutEvent(QFocusEvent *event);
    virtual void hoverEnterEvent(QGraphicsSceneHoverEvent *event);
    virtual void hoverMoveEvent(QGraphicsSceneHoverEvent *event);
    virtual void hoverLeaveEvent(QGraphicsSceneHoverEvent *event);
    virtual void keyPressEvent(QKeyEvent *event);
    virtual void keyReleaseEvent(QKeyEvent *event);
    virtual void mousePressEvent(QGraphicsSceneMouseEvent *event);
    virtual void mouseMoveEvent(QGraphicsSceneMouseEvent *event);
    virtual void mouseReleaseEvent(QGraphicsSceneMouseEvent *event);
    virtual void mouseDoubleClickEvent(QGraphicsSceneMouseEvent *event);
    virtual void wheelEvent(QGraphicsSceneWheelEvent *event);

    virtual QVariant itemChange(GraphicsItemChange change, const QVariant &value);

protected:
    QGraphicsItem(QGraphicsItemPrivate &dd,
                  QGraphicsItem *parent, QGraphicsScene *scene);
    QScopedPointer<QGraphicsItemPrivate> d_ptr;

    void addToIndex();
    void removeFromIndex();
    void prepareGeometryChange();

private:
    Q_DISABLE_COPY(QGraphicsItem)
    Q_DECLARE_PRIVATE(QGraphicsItem)
    friend class QGraphicsItemGroup;
    friend class QGraphicsScene;
    friend class QGraphicsScenePrivate;
    friend class QGraphicsSceneFindItemBspTreeVisitor;
    friend class QGraphicsSceneBspTree;
    friend class QGraphicsView;
    friend class QGraphicsViewPrivate;
    friend class QGraphicsObject;
    friend class QGraphicsWidget;
    friend class QGraphicsWidgetPrivate;
    friend class QGraphicsProxyWidgetPrivate;
    friend class QGraphicsSceneIndex;
    friend class QGraphicsSceneIndexPrivate;
    friend class QGraphicsSceneBspTreeIndex;
    friend class QGraphicsSceneBspTreeIndexPrivate;
    friend class ::tst_QGraphicsItem;
    friend bool qt_closestLeaf(const QGraphicsItem *, const QGraphicsItem *);
    friend bool qt_closestItemFirst(const QGraphicsItem *, const QGraphicsItem *);
};

Q_DECLARE_OPERATORS_FOR_FLAGS(QGraphicsItem::GraphicsItemFlags)

QT_END_NAMESPACE
Q_DECLARE_INTERFACE(QGraphicsItem, "Katie.QGraphicsItem")
QT_BEGIN_NAMESPACE

inline void QGraphicsItem::setPos(qreal ax, qreal ay)
{ setPos(QPointF(ax, ay)); }
inline void QGraphicsItem::ensureVisible(qreal ax, qreal ay, qreal w, qreal h, int xmargin, int ymargin)
{ ensureVisible(QRectF(ax, ay, w, h), xmargin, ymargin); }
inline void QGraphicsItem::update(qreal ax, qreal ay, qreal width, qreal height)
{ update(QRectF(ax, ay, width, height)); }
inline bool QGraphicsItem::isObscured(qreal ax, qreal ay, qreal w, qreal h) const
{ return isObscured(QRectF(ax, ay, w, h)); }
inline QPointF QGraphicsItem::mapToItem(const QGraphicsItem *item, qreal ax, qreal ay) const
{ return mapToItem(item, QPointF(ax, ay)); }
inline QPointF QGraphicsItem::mapToParent(qreal ax, qreal ay) const
{ return mapToParent(QPointF(ax, ay)); }
inline QPointF QGraphicsItem::mapToScene(qreal ax, qreal ay) const
{ return mapToScene(QPointF(ax, ay));  }
inline QPointF QGraphicsItem::mapFromItem(const QGraphicsItem *item, qreal ax, qreal ay) const
{ return mapFromItem(item, QPointF(ax, ay)); }
inline QPointF QGraphicsItem::mapFromParent(qreal ax, qreal ay) const
{ return mapFromParent(QPointF(ax, ay));  }
inline QPointF QGraphicsItem::mapFromScene(qreal ax, qreal ay) const
{ return mapFromScene(QPointF(ax, ay));  }
inline QPolygonF QGraphicsItem::mapToItem(const QGraphicsItem *item, qreal ax, qreal ay, qreal w, qreal h) const
{ return mapToItem(item, QRectF(ax, ay, w, h)); }
inline QPolygonF QGraphicsItem::mapToParent(qreal ax, qreal ay, qreal w, qreal h) const
{ return mapToParent(QRectF(ax, ay, w, h)); }
inline QPolygonF QGraphicsItem::mapToScene(qreal ax, qreal ay, qreal w, qreal h) const
{ return mapToScene(QRectF(ax, ay, w, h)); }
inline QRectF QGraphicsItem::mapRectToItem(const QGraphicsItem *item, qreal ax, qreal ay, qreal w, qreal h) const
{ return mapRectToItem(item, QRectF(ax, ay, w, h)); }
inline QRectF QGraphicsItem::mapRectToParent(qreal ax, qreal ay, qreal w, qreal h) const
{ return mapRectToParent(QRectF(ax, ay, w, h)); }
inline QRectF QGraphicsItem::mapRectToScene(qreal ax, qreal ay, qreal w, qreal h) const
{ return mapRectToScene(QRectF(ax, ay, w, h)); }
inline QPolygonF QGraphicsItem::mapFromItem(const QGraphicsItem *item, qreal ax, qreal ay, qreal w, qreal h) const
{ return mapFromItem(item, QRectF(ax, ay, w, h)); }
inline QPolygonF QGraphicsItem::mapFromParent(qreal ax, qreal ay, qreal w, qreal h) const
{ return mapFromParent(QRectF(ax, ay, w, h)); }
inline QPolygonF QGraphicsItem::mapFromScene(qreal ax, qreal ay, qreal w, qreal h) const
{ return mapFromScene(QRectF(ax, ay, w, h)); }
inline QRectF QGraphicsItem::mapRectFromItem(const QGraphicsItem *item, qreal ax, qreal ay, qreal w, qreal h) const
{ return mapRectFromItem(item, QRectF(ax, ay, w, h)); }
inline QRectF QGraphicsItem::mapRectFromParent(qreal ax, qreal ay, qreal w, qreal h) const
{ return mapRectFromParent(QRectF(ax, ay, w, h)); }
inline QRectF QGraphicsItem::mapRectFromScene(qreal ax, qreal ay, qreal w, qreal h) const
{ return mapRectFromScene(QRectF(ax, ay, w, h)); }


class Q_GUI_EXPORT QGraphicsObject : public QObject, public QGraphicsItem
{
    Q_OBJECT
    Q_PROPERTY(QGraphicsObject * parent READ parentObject WRITE setParentItem NOTIFY parentChanged)
    Q_PROPERTY(qreal opacity READ opacity WRITE setOpacity NOTIFY opacityChanged)
    Q_PROPERTY(bool enabled READ isEnabled WRITE setEnabled NOTIFY enabledChanged)
    Q_PROPERTY(bool visible READ isVisible WRITE setVisible NOTIFY visibleChanged)
    Q_PROPERTY(QPointF pos READ pos WRITE setPos)
    Q_PROPERTY(qreal x READ x WRITE setX NOTIFY xChanged)
    Q_PROPERTY(qreal y READ y WRITE setY NOTIFY yChanged)
    Q_PROPERTY(qreal z READ zValue WRITE setZValue NOTIFY zChanged)
    Q_PROPERTY(qreal rotation READ rotation WRITE setRotation NOTIFY rotationChanged)
    Q_PROPERTY(qreal scale READ scale WRITE setScale NOTIFY scaleChanged)
    Q_PROPERTY(QPointF transformOriginPoint READ transformOriginPoint WRITE setTransformOriginPoint)
    Q_INTERFACES(QGraphicsItem)
public:
    QGraphicsObject(QGraphicsItem *parent = 0);

Q_SIGNALS:
    void parentChanged();
    void opacityChanged();
    void visibleChanged();
    void enabledChanged();
    void xChanged();
    void yChanged();
    void zChanged();
    void rotationChanged();
    void scaleChanged();
    void childrenChanged();
    void widthChanged();
    void heightChanged();

protected:
    QGraphicsObject(QGraphicsItemPrivate &dd, QGraphicsItem *parent, QGraphicsScene *scene);
private:
    friend class QGraphicsItem;
    friend class QGraphicsItemPrivate;
};


class QAbstractGraphicsShapeItemPrivate;
class Q_GUI_EXPORT QAbstractGraphicsShapeItem : public QGraphicsItem
{
public:
    QAbstractGraphicsShapeItem(QGraphicsItem *parent = 0
                               // ### obsolete argument
                               , QGraphicsScene *scene = 0
        );
    ~QAbstractGraphicsShapeItem();

    QPen pen() const;
    void setPen(const QPen &pen);

    QBrush brush() const;
    void setBrush(const QBrush &brush);

    QPainterPath opaqueArea() const;

protected:
    QAbstractGraphicsShapeItem(QAbstractGraphicsShapeItemPrivate &dd,
                               QGraphicsItem *parent, QGraphicsScene *scene);

private:
    Q_DISABLE_COPY(QAbstractGraphicsShapeItem)
    Q_DECLARE_PRIVATE(QAbstractGraphicsShapeItem)
};

class QGraphicsPathItemPrivate;
class Q_GUI_EXPORT QGraphicsPathItem : public QAbstractGraphicsShapeItem
{
public:
    QGraphicsPathItem(QGraphicsItem *parent = 0
                      // ### obsolete argument
                      , QGraphicsScene *scene = 0
        );
    QGraphicsPathItem(const QPainterPath &path, QGraphicsItem *parent = 0
                      // ### obsolete argument
                      , QGraphicsScene *scene = 0
        );
    ~QGraphicsPathItem();

    QPainterPath path() const;
    void setPath(const QPainterPath &path);

    QRectF boundingRect() const;
    QPainterPath shape() const;
    bool contains(const QPointF &point) const;

    void paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget = 0);

    enum { Type = 2 };
    int type() const;

private:
    Q_DISABLE_COPY(QGraphicsPathItem)
    Q_DECLARE_PRIVATE(QGraphicsPathItem)
};

class QGraphicsRectItemPrivate;
class Q_GUI_EXPORT QGraphicsRectItem : public QAbstractGraphicsShapeItem
{
public:
    QGraphicsRectItem(QGraphicsItem *parent = 0
                      // ### obsolete argument
                      , QGraphicsScene *scene = 0
        );
    QGraphicsRectItem(const QRectF &rect, QGraphicsItem *parent = 0
                      // ### obsolete argument
                      , QGraphicsScene *scene = 0
        );
    QGraphicsRectItem(qreal x, qreal y, qreal w, qreal h, QGraphicsItem *parent = 0
                      // ### obsolete argument
                      , QGraphicsScene *scene = 0
        );
    ~QGraphicsRectItem();

    QRectF rect() const;
    void setRect(const QRectF &rect);
    inline void setRect(qreal x, qreal y, qreal w, qreal h);

    QRectF boundingRect() const;
    QPainterPath shape() const;
    bool contains(const QPointF &point) const;

    void paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget = 0);

    enum { Type = 3 };
    int type() const;

private:
    Q_DISABLE_COPY(QGraphicsRectItem)
    Q_DECLARE_PRIVATE(QGraphicsRectItem)
};

inline void QGraphicsRectItem::setRect(qreal ax, qreal ay, qreal w, qreal h)
{ setRect(QRectF(ax, ay, w, h)); }

class QGraphicsEllipseItemPrivate;
class Q_GUI_EXPORT QGraphicsEllipseItem : public QAbstractGraphicsShapeItem
{
public:
    QGraphicsEllipseItem(QGraphicsItem *parent = 0
                         // ### obsolete argument
                         , QGraphicsScene *scene = 0
        );
    QGraphicsEllipseItem(const QRectF &rect, QGraphicsItem *parent = 0
                         // ### obsolete argument
                         , QGraphicsScene *scene = 0
        );
    QGraphicsEllipseItem(qreal x, qreal y, qreal w, qreal h, QGraphicsItem *parent = 0
                         // ### obsolete argument
                         , QGraphicsScene *scene = 0
        );
    ~QGraphicsEllipseItem();

    QRectF rect() const;
    void setRect(const QRectF &rect);
    inline void setRect(qreal x, qreal y, qreal w, qreal h);

    int startAngle() const;
    void setStartAngle(int angle);

    int spanAngle() const;
    void setSpanAngle(int angle);

    QRectF boundingRect() const;
    QPainterPath shape() const;
    bool contains(const QPointF &point) const;

    void paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget = 0);

    enum { Type = 4 };
    int type() const;

private:
    Q_DISABLE_COPY(QGraphicsEllipseItem)
    Q_DECLARE_PRIVATE(QGraphicsEllipseItem)
};

inline void QGraphicsEllipseItem::setRect(qreal ax, qreal ay, qreal w, qreal h)
{ setRect(QRectF(ax, ay, w, h)); }

class QGraphicsPolygonItemPrivate;
class Q_GUI_EXPORT QGraphicsPolygonItem : public QAbstractGraphicsShapeItem
{
public:
    QGraphicsPolygonItem(QGraphicsItem *parent = 0
                         // ### obsolete argument
                         , QGraphicsScene *scene = 0
        );
    QGraphicsPolygonItem(const QPolygonF &polygon,
                         QGraphicsItem *parent = 0
                         // ### obsolete argument
                         , QGraphicsScene *scene = 0
        );
    ~QGraphicsPolygonItem();

    QPolygonF polygon() const;
    void setPolygon(const QPolygonF &polygon);

    Qt::FillRule fillRule() const;
    void setFillRule(Qt::FillRule rule);

    QRectF boundingRect() const;
    QPainterPath shape() const;
    bool contains(const QPointF &point) const;

    void paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget = 0);

    enum { Type = 5 };
    int type() const;

private:
    Q_DISABLE_COPY(QGraphicsPolygonItem)
    Q_DECLARE_PRIVATE(QGraphicsPolygonItem)
};

class QGraphicsLineItemPrivate;
class Q_GUI_EXPORT QGraphicsLineItem : public QGraphicsItem
{
public:
    QGraphicsLineItem(QGraphicsItem *parent = 0
                      // ### obsolete argument
                      , QGraphicsScene *scene = 0
        );
    QGraphicsLineItem(const QLineF &line, QGraphicsItem *parent = 0
                      // ### obsolete argument
                      , QGraphicsScene *scene = 0
        );
    QGraphicsLineItem(qreal x1, qreal y1, qreal x2, qreal y2, QGraphicsItem *parent = 0
                      // ### obsolete argument
                      , QGraphicsScene *scene = 0
        );
    ~QGraphicsLineItem();

    QPen pen() const;
    void setPen(const QPen &pen);

    QLineF line() const;
    void setLine(const QLineF &line);
    inline void setLine(qreal x1, qreal y1, qreal x2, qreal y2)
    { setLine(QLineF(x1, y1, x2, y2)); }

    QRectF boundingRect() const;
    QPainterPath shape() const;
    bool contains(const QPointF &point) const;

    void paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget = 0);

    enum { Type = 6 };
    int type() const;

private:
    Q_DISABLE_COPY(QGraphicsLineItem)
    Q_DECLARE_PRIVATE(QGraphicsLineItem)
};

class QGraphicsPixmapItemPrivate;
class Q_GUI_EXPORT QGraphicsPixmapItem : public QGraphicsItem
{
public:
    enum ShapeMode {
        MaskShape,
        BoundingRectShape,
        HeuristicMaskShape
    };

    QGraphicsPixmapItem(QGraphicsItem *parent = 0
                        // ### obsolete argument
                        , QGraphicsScene *scene = 0
        );
    QGraphicsPixmapItem(const QPixmap &pixmap, QGraphicsItem *parent = 0
                        // ### obsolete argument
                        , QGraphicsScene *scene = 0
        );
    ~QGraphicsPixmapItem();

    QPixmap pixmap() const;
    void setPixmap(const QPixmap &pixmap);

    QPointF offset() const;
    void setOffset(const QPointF &offset);
    inline void setOffset(qreal x, qreal y);

    QRectF boundingRect() const;
    QPainterPath shape() const;
    bool contains(const QPointF &point) const;

    void paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget);

    QPainterPath opaqueArea() const;

    enum { Type = 7 };
    int type() const;

    ShapeMode shapeMode() const;
    void setShapeMode(ShapeMode mode);

private:
    Q_DISABLE_COPY(QGraphicsPixmapItem)
    Q_DECLARE_PRIVATE(QGraphicsPixmapItem)
};

inline void QGraphicsPixmapItem::setOffset(qreal ax, qreal ay)
{ setOffset(QPointF(ax, ay)); }

class QGraphicsTextItemPrivate;
class QTextDocument;
class QTextCursor;
class Q_GUI_EXPORT QGraphicsTextItem : public QGraphicsObject
{
    Q_OBJECT
    Q_PROPERTY(bool openExternalLinks READ openExternalLinks WRITE setOpenExternalLinks)
    Q_PROPERTY(QTextCursor textCursor READ textCursor WRITE setTextCursor)

public:
    QGraphicsTextItem(QGraphicsItem *parent = 0
                      // ### obsolete argument
                      , QGraphicsScene *scene = 0
        );
    QGraphicsTextItem(const QString &text, QGraphicsItem *parent = 0
                      // ### obsolete argument
                      , QGraphicsScene *scene = 0
        );
    ~QGraphicsTextItem();

    QString toHtml() const;
    void setHtml(const QString &html);

    QString toPlainText() const;
    void setPlainText(const QString &text);

    QFont font() const;
    void setFont(const QFont &font);

    void setDefaultTextColor(const QColor &c);
    QColor defaultTextColor() const;

    QRectF boundingRect() const;
    QPainterPath shape() const;
    bool contains(const QPointF &point) const;

    void paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget);

    enum { Type = 8 };
    int type() const;

    void setTextWidth(qreal width);
    qreal textWidth() const;

    void adjustSize();

    void setDocument(QTextDocument *document);
    QTextDocument *document() const;

    void setTextInteractionFlags(Qt::TextInteractionFlags flags);
    Qt::TextInteractionFlags textInteractionFlags() const;

    void setTabChangesFocus(bool b);
    bool tabChangesFocus() const;

    void setOpenExternalLinks(bool open);
    bool openExternalLinks() const;

    void setTextCursor(const QTextCursor &cursor);
    QTextCursor textCursor() const;

Q_SIGNALS:
    void linkActivated(const QString &);
    void linkHovered(const QString &);

protected:
    bool sceneEvent(QEvent *event);
    void mousePressEvent(QGraphicsSceneMouseEvent *event);
    void mouseMoveEvent(QGraphicsSceneMouseEvent *event);
    void mouseReleaseEvent(QGraphicsSceneMouseEvent *event);
    void mouseDoubleClickEvent(QGraphicsSceneMouseEvent *event);
    void contextMenuEvent(QGraphicsSceneContextMenuEvent *event);
    void keyPressEvent(QKeyEvent *event);
    void keyReleaseEvent(QKeyEvent *event);
    void focusInEvent(QFocusEvent *event);
    void focusOutEvent(QFocusEvent *event);
    void dragEnterEvent(QGraphicsSceneDragDropEvent *event);
    void dragLeaveEvent(QGraphicsSceneDragDropEvent *event);
    void dragMoveEvent(QGraphicsSceneDragDropEvent *event);
    void dropEvent(QGraphicsSceneDragDropEvent *event);
    void hoverEnterEvent(QGraphicsSceneHoverEvent *event);
    void hoverMoveEvent(QGraphicsSceneHoverEvent *event);
    void hoverLeaveEvent(QGraphicsSceneHoverEvent *event);

private:
    Q_DISABLE_COPY(QGraphicsTextItem)
    Q_PRIVATE_SLOT(dd, void _q_updateBoundingRect(const QSizeF &))
    Q_PRIVATE_SLOT(dd, void _q_update(QRectF))
    Q_PRIVATE_SLOT(dd, void _q_ensureVisible(QRectF))
    QGraphicsTextItemPrivate *dd;
    friend class QGraphicsTextItemPrivate;
};

class QGraphicsSimpleTextItemPrivate;
class Q_GUI_EXPORT QGraphicsSimpleTextItem : public QAbstractGraphicsShapeItem
{
public:
    QGraphicsSimpleTextItem(QGraphicsItem *parent = 0
                            // ### obsolete argument
                            , QGraphicsScene *scene = 0
        );
    QGraphicsSimpleTextItem(const QString &text, QGraphicsItem *parent = 0
                            // ### obsolete argument
                            , QGraphicsScene *scene = 0
        );
    ~QGraphicsSimpleTextItem();

    void setText(const QString &text);
    QString text() const;

    void setFont(const QFont &font);
    QFont font() const;

    QRectF boundingRect() const;
    QPainterPath shape() const;
    bool contains(const QPointF &point) const;

    void paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget);

    enum { Type = 9 };
    int type() const;

private:
    Q_DISABLE_COPY(QGraphicsSimpleTextItem)
    Q_DECLARE_PRIVATE(QGraphicsSimpleTextItem)
};

class QGraphicsItemGroupPrivate;
class Q_GUI_EXPORT QGraphicsItemGroup : public QGraphicsItem
{
public:
    QGraphicsItemGroup(QGraphicsItem *parent = 0
                       // ### obsolete argument
                       , QGraphicsScene *scene = 0
        );
    ~QGraphicsItemGroup();

    void addToGroup(QGraphicsItem *item);
    void removeFromGroup(QGraphicsItem *item);

    QRectF boundingRect() const;
    void paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget = 0);

    enum { Type = 10 };
    int type() const;

private:
    Q_DISABLE_COPY(QGraphicsItemGroup)
    Q_DECLARE_PRIVATE(QGraphicsItemGroup)
};

template <class T> inline T qgraphicsitem_cast(QGraphicsItem *item)
{
    return int(static_cast<T>(0)->Type) == int(QGraphicsItem::Type)
        || (item && int(static_cast<T>(0)->Type) == item->type()) ? static_cast<T>(item) : 0;
}

template <class T> inline T qgraphicsitem_cast(const QGraphicsItem *item)
{
    return int(static_cast<T>(0)->Type) == int(QGraphicsItem::Type)
        || (item && int(static_cast<T>(0)->Type) == item->type()) ? static_cast<T>(item) : 0;
}

#ifndef QT_NO_DEBUG_STREAM
Q_GUI_EXPORT QDebug operator<<(QDebug debug, QGraphicsItem *item);
Q_GUI_EXPORT QDebug operator<<(QDebug debug, QGraphicsObject *item);
Q_GUI_EXPORT QDebug operator<<(QDebug debug, QGraphicsItem::GraphicsItemChange change);
Q_GUI_EXPORT QDebug operator<<(QDebug debug, QGraphicsItem::GraphicsItemFlag flag);
Q_GUI_EXPORT QDebug operator<<(QDebug debug, QGraphicsItem::GraphicsItemFlags flags);
#endif

QT_END_NAMESPACE

Q_DECLARE_METATYPE(QGraphicsItem *)
Q_DECLARE_METATYPE(QGraphicsScene *)

QT_BEGIN_NAMESPACE

#endif // QT_NO_GRAPHICSVIEW

QT_END_NAMESPACE


#endif // QGRAPHICSITEM_H
