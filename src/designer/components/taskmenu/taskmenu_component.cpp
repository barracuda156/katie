/****************************************************************************
**
** Copyright (C) 2015 The Qt Company Ltd.
**
** This file is part of the Qt Designer of the Katie Toolkit.
**
** $QT_BEGIN_LICENSE:LGPL$
** Commercial License Usage
** Licensees holding valid commercial Qt licenses may use this file in
** accordance with the commercial license agreement provided with the
** Software or, alternatively, in accordance with the terms contained in
** a written agreement between you and The Qt Company. For licensing terms
** and conditions see http://www.qt.io/terms-conditions. For further
** information use the contact form at http://www.qt.io/contact-us.
**
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

#include "taskmenu_component.h"
#include "button_taskmenu.h"
#include "groupbox_taskmenu.h"
#include "label_taskmenu.h"
#include "lineedit_taskmenu.h"
#include "listwidget_taskmenu.h"
#include "treewidget_taskmenu.h"
#include "tablewidget_taskmenu.h"
#include "containerwidget_taskmenu.h"
#include "combobox_taskmenu.h"
#include "textedit_taskmenu.h"
#include "menutaskmenu.h"
#include "toolbar_taskmenu.h"
#include "layouttaskmenu.h"

#include <QtDesigner/abstractformeditor.h>
#include <QtDesigner/QExtensionManager>

QT_BEGIN_NAMESPACE

using namespace qdesigner_internal;

TaskMenuComponent::TaskMenuComponent(QDesignerFormEditorInterface *core, QObject *parent)
    : QObject(parent),
      m_core(core)
{
    Q_ASSERT(m_core != 0);

    QExtensionManager *mgr = core->extensionManager();
    const QString taskMenuId =  QLatin1String("QDesignerInternalTaskMenuExtension");

    ButtonTaskMenuFactory::registerExtension(mgr, taskMenuId);
    CommandLinkButtonTaskMenuFactory::registerExtension(mgr, taskMenuId); // Order!
    ButtonGroupTaskMenuFactory::registerExtension(mgr, taskMenuId);

    GroupBoxTaskMenuFactory::registerExtension(mgr, taskMenuId);
    LabelTaskMenuFactory::registerExtension(mgr, taskMenuId);
    LineEditTaskMenuFactory::registerExtension(mgr, taskMenuId);
    ListWidgetTaskMenuFactory::registerExtension(mgr, taskMenuId);
    TreeWidgetTaskMenuFactory::registerExtension(mgr, taskMenuId);
    TableWidgetTaskMenuFactory::registerExtension(mgr, taskMenuId);
    TextEditTaskMenuFactory::registerExtension(mgr, taskMenuId);
    PlainTextEditTaskMenuFactory::registerExtension(mgr, taskMenuId);
    MenuTaskMenuFactory::registerExtension(mgr, taskMenuId);
    MenuBarTaskMenuFactory::registerExtension(mgr, taskMenuId);
    ToolBarTaskMenuFactory::registerExtension(mgr, taskMenuId);
    StatusBarTaskMenuFactory::registerExtension(mgr, taskMenuId);
    LayoutWidgetTaskMenuFactory::registerExtension(mgr, taskMenuId);
    SpacerTaskMenuFactory::registerExtension(mgr, taskMenuId);

    mgr->registerExtensions(new ContainerWidgetTaskMenuFactory(core, mgr), taskMenuId);
    mgr->registerExtensions(new ComboBoxTaskMenuFactory(taskMenuId, mgr), taskMenuId);
}

TaskMenuComponent::~TaskMenuComponent()
{
}

QDesignerFormEditorInterface *TaskMenuComponent::core() const
{
    return m_core;

}
QT_END_NAMESPACE

#include <moc_taskmenu_component.h>
