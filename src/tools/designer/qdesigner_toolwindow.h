/****************************************************************************
**
** Copyright (C) 2015 The Qt Company Ltd.
** Copyright (C) 2016 Ivailo Monev
**
** This file is part of the Katie Designer of the Katie Toolkit.
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

#ifndef QDESIGNER_TOOLWINDOW_H
#define QDESIGNER_TOOLWINDOW_H

#include "mainwindow.h"

#include <QtCore/QPointer>
#include <QtGui/QFontDatabase>
#include <QtGui/QMainWindow>

QT_BEGIN_NAMESPACE

struct ToolWindowFontSettings {
    ToolWindowFontSettings();
    bool equals(const ToolWindowFontSettings &) const;

    QFont m_font;
    bool m_useFont;
};

inline bool operator==(const ToolWindowFontSettings &tw1, const ToolWindowFontSettings &tw2)
{
    return tw1.equals(tw2);
}

inline bool operator!=(const ToolWindowFontSettings &tw1, const ToolWindowFontSettings &tw2)
{
    return !tw1.equals(tw2);
}

class QDesignerWorkbench;

/* A tool window with an action that activates it. Note that in toplevel mode,
 * the Widget box is a tool window as well as the applications' main window,
 * So, we need to inherit from MainWindowBase. */

class QDesignerToolWindow : public MainWindowBase
{
    Q_OBJECT
protected:
    explicit QDesignerToolWindow(QDesignerWorkbench *workbench,
                                 QWidget *w,
                                 const QString &objectName,
                                 const QString &title,
                                 const QString &actionObjectName,
                                 Qt::DockWidgetArea dockAreaHint,
                                 QWidget *parent = nullptr,
                                 Qt::WindowFlags flags = Qt::Window);

public:
    // Note: The order influences the dock widget position.
    enum StandardToolWindow { WidgetBox,  ObjectInspector, PropertyEditor,
                              ResourceEditor, ActionEditor, SignalSlotEditor,
                              StandardToolWindowCount };

    static QDesignerToolWindow *createStandardToolWindow(StandardToolWindow which, QDesignerWorkbench *workbench);

    QDesignerWorkbench *workbench() const;
    QAction *action() const;

    Qt::DockWidgetArea dockWidgetAreaHint() const { return m_dockAreaHint; }
    virtual QRect geometryHint() const;

private slots:
    void showMe(bool);

protected:
    virtual void showEvent(QShowEvent *e);
    virtual void hideEvent(QHideEvent *e);
    virtual void changeEvent(QEvent *e);

    QRect availableToolWindowGeometry() const;

private:
    const Qt::DockWidgetArea m_dockAreaHint;
    QDesignerWorkbench *m_workbench;
    QAction *m_action;
};

QT_END_NAMESPACE

#endif // QDESIGNER_TOOLWINDOW_H
