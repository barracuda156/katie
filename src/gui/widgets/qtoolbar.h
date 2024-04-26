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

#ifndef QDYNAMICTOOLBAR_H
#define QDYNAMICTOOLBAR_H

#include <QtGui/qwidget.h>


QT_BEGIN_NAMESPACE


#ifndef QT_NO_TOOLBAR

class QToolBarPrivate;

class QAction;
class QIcon;
class QMainWindow;
class QStyleOptionToolBar;

class Q_GUI_EXPORT QToolBar : public QWidget
{
    Q_OBJECT

    Q_PROPERTY(bool movable READ isMovable WRITE setMovable NOTIFY movableChanged)
    Q_PROPERTY(Qt::ToolBarAreas allowedAreas READ allowedAreas WRITE setAllowedAreas NOTIFY allowedAreasChanged)
    Q_PROPERTY(Qt::Orientation orientation READ orientation WRITE setOrientation NOTIFY orientationChanged)
    Q_PROPERTY(QSize iconSize READ iconSize WRITE setIconSize NOTIFY iconSizeChanged)
    Q_PROPERTY(Qt::ToolButtonStyle toolButtonStyle READ toolButtonStyle WRITE setToolButtonStyle NOTIFY toolButtonStyleChanged)
    Q_PROPERTY(bool floating READ isFloating)
    Q_PROPERTY(bool floatable READ isFloatable WRITE setFloatable)

public:
    explicit QToolBar(const QString &title, QWidget *parent = nullptr);
    explicit QToolBar(QWidget *parent = nullptr);
    ~QToolBar();

    void setMovable(bool movable);
    bool isMovable() const;

    void setAllowedAreas(Qt::ToolBarAreas areas);
    Qt::ToolBarAreas allowedAreas() const;

    inline bool isAreaAllowed(Qt::ToolBarArea area) const
    { return (allowedAreas() & area) == area; }

    void setOrientation(Qt::Orientation orientation);
    Qt::Orientation orientation() const;

    void clear();

#ifdef Q_NO_USING_KEYWORD
    inline void addAction(QAction *action)
    { QWidget::addAction(action); }
#else
    using QWidget::addAction;
#endif

    QAction *addAction(const QString &text);
    QAction *addAction(const QIcon &icon, const QString &text);
    QAction *addAction(const QString &text, const QObject *receiver, const char* member);
    QAction *addAction(const QIcon &icon, const QString &text,
		       const QObject *receiver, const char* member);

    QAction *addSeparator();
    QAction *insertSeparator(QAction *before);

    QAction *addWidget(QWidget *widget);
    QAction *insertWidget(QAction *before, QWidget *widget);

    QRect actionGeometry(QAction *action) const;
    QAction *actionAt(const QPoint &p) const;
    inline QAction *actionAt(int x, int y) const;

    QAction *toggleViewAction() const;

    QSize iconSize() const;
    Qt::ToolButtonStyle toolButtonStyle() const;

    QWidget *widgetForAction(QAction *action) const;

    bool isFloatable() const;
    void setFloatable(bool floatable);
    bool isFloating() const;

public Q_SLOTS:
    void setIconSize(const QSize &iconSize);
    void setToolButtonStyle(Qt::ToolButtonStyle toolButtonStyle);

Q_SIGNALS:
    void actionTriggered(QAction *action);
    void movableChanged(bool movable);
    void allowedAreasChanged(Qt::ToolBarAreas allowedAreas);
    void orientationChanged(Qt::Orientation orientation);
    void iconSizeChanged(const QSize &iconSize);
    void toolButtonStyleChanged(Qt::ToolButtonStyle toolButtonStyle);
    void topLevelChanged(bool topLevel);
    void visibilityChanged(bool visible);

protected:
    void actionEvent(QActionEvent *event);
    void changeEvent(QEvent *event);
    void paintEvent(QPaintEvent *event);
    bool event(QEvent *event);
    void initStyleOption(QStyleOptionToolBar *option) const;


private:
    Q_DECLARE_PRIVATE(QToolBar)
    Q_DISABLE_COPY(QToolBar)
    Q_PRIVATE_SLOT(d_func(), void _q_toggleView(bool))
    Q_PRIVATE_SLOT(d_func(), void _q_updateIconSize(const QSize &))
    Q_PRIVATE_SLOT(d_func(), void _q_updateToolButtonStyle(Qt::ToolButtonStyle))

    friend class QMainWindow;
    friend class QMainWindowLayout;
    friend class QToolBarLayout;
    friend class QToolBarAreaLayout;
};

inline QAction *QToolBar::actionAt(int ax, int ay) const
{ return actionAt(QPoint(ax, ay)); }

#endif // QT_NO_TOOLBAR

QT_END_NAMESPACE


#endif // QDYNAMICTOOLBAR_H
