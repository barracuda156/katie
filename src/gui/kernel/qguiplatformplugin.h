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

#ifndef QGUIPLATFORM_P_H
#define QGUIPLATFORM_P_H

#include <QtCore/qplugin.h>
#include <QtGui/qdialog.h>

QT_BEGIN_NAMESPACE

class QPalette;
class QIcon;
class QFileDialog;
class QColorDialog;
class QFileInfo;

class Q_GUI_EXPORT QGuiPlatformPlugin : public QObject
{
    Q_OBJECT
public:
    enum PlatformHint {
        PH_ToolButtonStyle,
        PH_ToolBarIconSize,
        PH_ItemView_ActivateItemOnSingleClick
    };

    explicit QGuiPlatformPlugin(QObject *parent = nullptr);
    ~QGuiPlatformPlugin();

    virtual QString styleName();
    virtual QPalette palette();
    virtual QString systemIconThemeName();
    virtual QStringList iconThemeSearchPaths();
    virtual QIcon systemIcon(const QString &name);
    virtual QIcon fileSystemIcon(const QFileInfo &name);
    virtual int platformHint(PlatformHint hint);
};

// internal
Q_GUI_EXPORT QGuiPlatformPlugin *qt_guiPlatformPlugin();

QT_END_NAMESPACE


#endif // QGUIPLATFORMPLUGIN_H
