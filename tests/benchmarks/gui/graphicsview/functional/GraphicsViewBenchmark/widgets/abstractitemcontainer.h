/****************************************************************************
**
** Copyright (C) 2015 The Qt Company Ltd.
** Copyright (C) 2016-2019 Ivailo Monev
**
** This file is part of the examples of the Katie Toolkit.
**
** $QT_BEGIN_LICENSE:LGPL$
** GNU Lesser General Public License Usage
** Alternatively, this file may be used under the terms of the GNU Lesser
** General Public License version 2.1 or version 3 as published by the Free
** Software Foundation and appearing in the file LICENSE.LGPLv21 and
** LICENSE.LGPLv3 included in the packaging of this file. Please review the
** following information to ensure the GNU Lesser General Public License
** requirements will be met: https://www.gnu.org/licenses/lgpl.html and
** http://www.gnu.org/licenses/old-licenses/lgpl-2.1.html.
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

#ifndef ABSTRACTITEMCONTAINER_H
#define ABSTRACTITEMCONTAINER_H

#include <QModelIndex>

#include "gvbwidget.h"

class QGraphicsWidget;
class AbstractItemView;
class AbstractViewItem;

class AbstractItemContainer : public GvbWidget
{
  Q_OBJECT
public:
    AbstractItemContainer(int bufferSize, QGraphicsWidget *parent=0);
    virtual ~AbstractItemContainer();

    virtual void addItem(const QModelIndex &index);
    virtual void removeItem(const QModelIndex &index);

    virtual void setItemView(AbstractItemView *view);
    virtual void setItemPrototype(AbstractViewItem *ptype);
    virtual void reset();
    virtual int itemCount() const;
    virtual AbstractViewItem* itemAt(const int row) const;
    AbstractViewItem* findItemByIndex(const QModelIndex &index) const;
    AbstractViewItem *prototype();
    AbstractViewItem *firstItem();
    void updateContent();
    void themeChange();
    int bufferSize() const;
    virtual void setTwoColumns(const bool enabled);
    bool twoColumns();

#if (QT_VERSION >= 0x040600)
    void setSubtreeCacheEnabled(const bool enabled);
    virtual void setListItemCaching(const bool enabled, const int index) = 0;
#endif

protected:
    virtual void adjustVisibleContainerSize(const QSizeF &size) = 0;
    virtual void addItemToVisibleLayout(int index, AbstractViewItem *item) = 0;
    virtual void removeItemFromVisibleLayout(AbstractViewItem *item) = 0;

    virtual bool event(QEvent *e);
    virtual bool eventFilter(QObject *obj, QEvent *event);
    virtual QVariant itemChange(GraphicsItemChange change, const QVariant &value);
    virtual int maxItemCountInItemBuffer() const;
    bool itemVisibleInView(AbstractViewItem* item, const QRectF &viewRect, bool fullyVisible = true) const;

protected:
    void updateItemBuffer();
    void findFirstAndLastVisibleBufferIndex(int &firstVisibleBufferIndex,
                                            int &lastVisibleBufferIndex,
                                            const QRectF &viewRect,
                                            bool fullyVisible) const;
    QList<AbstractViewItem*>  m_items;
    AbstractItemView *m_itemView;
    AbstractViewItem *m_prototype;
    int m_bufferSize;

private:
    void insertItem(int pos, const QModelIndex &index);
    bool m_twoColumns;

    Q_DISABLE_COPY(AbstractItemContainer)
};

#endif // ABSTRACTITEMCONTAINER_H
