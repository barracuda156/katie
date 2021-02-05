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

#ifndef LISTWIDGETEDITOR_H
#define LISTWIDGETEDITOR_H

#include "itemlisteditor.h"
#include "qdesigner_command_p.h"

#include <QtGui/QWidget>

QT_BEGIN_NAMESPACE

class QListWidget;
class QComboBox;
class QDesignerFormWindowInterface;

namespace qdesigner_internal {

class ListWidgetEditor: public QDialog
{
    Q_OBJECT

public:
    ListWidgetEditor(QDesignerFormWindowInterface *form,
                     QWidget *parent);

    ListContents fillContentsFromListWidget(QListWidget *listWidget);
    ListContents fillContentsFromComboBox(QComboBox *comboBox);
    ListContents contents() const;

private:
    ItemListEditor *m_itemsEditor;
};

}  // namespace qdesigner_internal

QT_END_NAMESPACE

#endif // LISTWIDGETEDITOR_H
