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

#ifndef BUTTON_TASKMENU_H
#define BUTTON_TASKMENU_H

#include <QtGui/QAbstractButton>
#include <QtGui/QCommandLinkButton>
#include <QtGui/QButtonGroup>

#include "qdesigner_taskmenu_p.h"
#include <extensionfactory_p.h>

QT_BEGIN_NAMESPACE

class QMenu;
class QActionGroup;
class QDesignerFormWindowCursorInterface;

namespace qdesigner_internal {

// ButtonGroupMenu: Mixin menu for the 'select members'/'break group' options of
// the task menu of buttons and button group
class ButtonGroupMenu : public QObject
{
    Q_OBJECT
    Q_DISABLE_COPY(ButtonGroupMenu)
public:
    ButtonGroupMenu(QObject *parent = nullptr);

    void initialize(QDesignerFormWindowInterface *formWindow,
                    QButtonGroup *buttonGroup = 0,
                    /* Current button for selection in ButtonMode */
                    QAbstractButton *currentButton = 0);

    QAction *selectGroupAction() const { return m_selectGroupAction; }
    QAction *breakGroupAction() const  { return m_breakGroupAction; }

private slots:
    void selectGroup();
    void breakGroup();

private:
    QAction *m_selectGroupAction;
    QAction *m_breakGroupAction;

    QDesignerFormWindowInterface *m_formWindow;
    QButtonGroup *m_buttonGroup;
    QAbstractButton *m_currentButton;
};

// Task menu extension of a QButtonGroup
class ButtonGroupTaskMenu : public QObject, public QDesignerTaskMenuExtension
{
    Q_OBJECT
    Q_DISABLE_COPY(ButtonGroupTaskMenu)
    Q_INTERFACES(QDesignerTaskMenuExtension)
public:
    explicit ButtonGroupTaskMenu(QButtonGroup *buttonGroup, QObject *parent = nullptr);

    virtual QAction *preferredEditAction() const;
    virtual QList<QAction*> taskActions() const;

private:
    QButtonGroup *m_buttonGroup;
    QList<QAction*> m_taskActions;
    mutable ButtonGroupMenu m_menu;
};

// Task menu extension of a QAbstractButton
class ButtonTaskMenu: public QDesignerTaskMenu
{
    Q_OBJECT
    Q_DISABLE_COPY(ButtonTaskMenu)
public:
    explicit ButtonTaskMenu(QAbstractButton *button, QObject *parent = nullptr);
    virtual ~ButtonTaskMenu();

    virtual QAction *preferredEditAction() const;
    virtual QList<QAction*> taskActions() const;

    QAbstractButton *button() const;

protected:
    void insertAction(int index, QAction *a);

private slots:
    void createGroup();
    void addToGroup(QAction *a);
    void removeFromGroup();

private:
    enum SelectionType {
        OtherSelection,
        UngroupedButtonSelection,
        GroupedButtonSelection
    };

    SelectionType selectionType(const QDesignerFormWindowCursorInterface *cursor, QButtonGroup ** ptrToGroup = 0) const;
    bool refreshAssignMenu(const QDesignerFormWindowInterface *fw, int buttonCount, SelectionType st, QButtonGroup *currentGroup);
    QMenu *createGroupSelectionMenu(const QDesignerFormWindowInterface *fw);

    QList<QAction*> m_taskActions;
    mutable ButtonGroupMenu m_groupMenu;
    QMenu *m_assignGroupSubMenu;
    QActionGroup *m_assignActionGroup;
    QAction *m_assignToGroupSubMenuAction;
    QMenu *m_currentGroupSubMenu;
    QAction *m_currentGroupSubMenuAction;

    QAction *m_createGroupAction;
    QAction *m_preferredEditAction;
    QAction *m_removeFromGroupAction;
};

// Task menu extension of a QCommandLinkButton
class CommandLinkButtonTaskMenu: public ButtonTaskMenu
{
    Q_OBJECT
    Q_DISABLE_COPY(CommandLinkButtonTaskMenu)
public:
    explicit CommandLinkButtonTaskMenu(QCommandLinkButton *button, QObject *parent = nullptr);
};

typedef ExtensionFactory<QDesignerTaskMenuExtension, QButtonGroup, ButtonGroupTaskMenu> ButtonGroupTaskMenuFactory;
typedef ExtensionFactory<QDesignerTaskMenuExtension, QCommandLinkButton, CommandLinkButtonTaskMenu>  CommandLinkButtonTaskMenuFactory;
typedef ExtensionFactory<QDesignerTaskMenuExtension, QAbstractButton, ButtonTaskMenu>  ButtonTaskMenuFactory;
}  // namespace qdesigner_internal

QT_END_NAMESPACE

#endif // BUTTON_TASKMENU_H
