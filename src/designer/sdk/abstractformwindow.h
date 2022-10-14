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

#ifndef ABSTRACTFORMWINDOW_H
#define ABSTRACTFORMWINDOW_H

#include <QtGui/QWidget>


QT_BEGIN_NAMESPACE

class QDesignerFormEditorInterface;
class QDesignerFormWindowCursorInterface;
class QDesignerFormWindowToolInterface;
class DomUI;
class QUndoStack;
class QDir;

class Q_DESIGNER_EXPORT QDesignerFormWindowInterface: public QWidget
{
    Q_OBJECT
public:
    enum FeatureFlag
    {
        EditFeature = 0x01,
        GridFeature = 0x02,
        TabOrderFeature = 0x04,
        DefaultFeature = EditFeature | GridFeature
    };
    Q_DECLARE_FLAGS(Feature, FeatureFlag)

public:
    QDesignerFormWindowInterface(QWidget *parent = nullptr, Qt::WindowFlags flags = 0);
    virtual ~QDesignerFormWindowInterface();

    virtual QString fileName() const = 0;
    virtual QDir absoluteDir() const = 0;

    virtual QString contents() const = 0;
    virtual void setContents(QIODevice *dev) = 0;

    virtual Feature features() const = 0;
    virtual bool hasFeature(Feature f) const = 0;

    virtual QString author() const = 0;
    virtual void setAuthor(const QString &author) = 0;

    virtual QString comment() const = 0;
    virtual void setComment(const QString &comment) = 0;

    virtual void layoutDefault(int *margin, int *spacing) = 0;
    virtual void setLayoutDefault(int margin, int spacing) = 0;

    virtual void layoutFunction(QString *margin, QString *spacing) = 0;
    virtual void setLayoutFunction(const QString &margin, const QString &spacing) = 0;

    virtual QString exportMacro() const = 0;
    virtual void setExportMacro(const QString &exportMacro) = 0;

    virtual QStringList includeHints() const = 0;
    virtual void setIncludeHints(const QStringList &includeHints) = 0;

    virtual QDesignerFormEditorInterface *core() const;
    virtual QDesignerFormWindowCursorInterface *cursor() const = 0;

    virtual int toolCount() const = 0;

    virtual int currentTool() const = 0;
    virtual void setCurrentTool(int index) = 0;

    virtual QDesignerFormWindowToolInterface *tool(int index) const = 0;
    virtual void registerTool(QDesignerFormWindowToolInterface *tool) = 0;

    virtual QPoint grid() const = 0;

    virtual QWidget *mainContainer() const = 0;
    virtual void setMainContainer(QWidget *mainContainer) = 0;

    virtual bool isManaged(QWidget *widget) const = 0;

    virtual bool isDirty() const = 0;

    static QDesignerFormWindowInterface *findFormWindow(QWidget *w);
    static QDesignerFormWindowInterface *findFormWindow(QObject *obj);

    virtual QUndoStack *commandHistory() const = 0;
    virtual void beginCommand(const QString &description) = 0;
    virtual void endCommand() = 0;

    virtual void simplifySelection(QList<QWidget*> *widgets) const = 0;

    // notifications
    virtual void emitSelectionChanged() = 0;

    virtual QStringList resourceFiles() const = 0;
    virtual void addResourceFile(const QString &path) = 0;
    virtual void removeResourceFile(const QString &path) = 0;

    virtual void ensureUniqueObjectName(QObject *object) = 0;

public Q_SLOTS:
    virtual void manageWidget(QWidget *widget) = 0;
    virtual void unmanageWidget(QWidget *widget) = 0;

    virtual void setFeatures(Feature f) = 0;
    virtual void setDirty(bool dirty) = 0;
    virtual void clearSelection(bool changePropertyDisplay = true) = 0;
    virtual void selectWidget(QWidget *w, bool select = true) = 0;
    virtual void setGrid(const QPoint &grid) = 0;
    virtual void setFileName(const QString &fileName) = 0;
    virtual void setContents(const QString &contents) = 0;

    virtual void editWidgets() = 0;

Q_SIGNALS:
    void mainContainerChanged(QWidget *mainContainer);
    void toolChanged(int toolIndex);
    void fileNameChanged(const QString &fileName);
    void featureChanged(Feature f);
    void selectionChanged();
    void geometryChanged();

    void resourceFilesChanged();

    void widgetManaged(QWidget *widget);
    void widgetUnmanaged(QWidget *widget);
    void aboutToUnmanageWidget(QWidget *widget);
    void activated(QWidget *widget);

    void changed();
    void widgetRemoved(QWidget *w);
    void objectRemoved(QObject *o);
};

QT_END_NAMESPACE


#endif // ABSTRACTFORMWINDOW_H
