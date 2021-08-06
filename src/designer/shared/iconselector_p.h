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

//
//  W A R N I N G
//  -------------
//
// This file is not part of the Katie API.  It exists for the convenience
// of Katie Designer.  This header
// file may change from version to version without notice, or even be removed.
//
// We mean it.
//


#ifndef ICONSELECTOR_H
#define ICONSELECTOR_H

#include <QtGui/QWidget>
#include <QtGui/QDialog>

#include <QtCore/QScopedPointer>

QT_BEGIN_NAMESPACE

class QtResourceModel;
class QDesignerFormEditorInterface;
class QDesignerDialogGuiInterface;
class QDesignerResourceBrowserInterface;

namespace qdesigner_internal {

class DesignerIconCache;
class DesignerPixmapCache;
class PropertySheetIconValue;
struct IconThemeEditorPrivate;

// Resource Dialog that embeds the language-dependent resource widget as returned by the language extension
class Q_DESIGNER_EXPORT LanguageResourceDialog : public QDialog
{
    Q_OBJECT

    explicit LanguageResourceDialog(QDesignerResourceBrowserInterface *rb, QWidget *parent = nullptr);

public:
    virtual ~LanguageResourceDialog();
    // Factory: Returns 0 if the language extension does not provide a resource browser.
    static LanguageResourceDialog* create(QDesignerFormEditorInterface *core, QWidget *parent);

    void setCurrentPath(const QString &filePath);
    QString currentPath() const;

private:
    QScopedPointer<class LanguageResourceDialogPrivate> d_ptr;
    Q_DECLARE_PRIVATE(LanguageResourceDialog)
    Q_DISABLE_COPY(LanguageResourceDialog)
    Q_PRIVATE_SLOT(d_func(), void slotAccepted())
    Q_PRIVATE_SLOT(d_func(), void slotPathChanged(QString))

};

class Q_DESIGNER_EXPORT IconSelector: public QWidget
{
    Q_OBJECT
public:
    IconSelector(QWidget *parent = nullptr);
    virtual ~IconSelector();

    void setFormEditor(QDesignerFormEditorInterface *core); // required for dialog gui.
    void setIconCache(DesignerIconCache *iconCache);
    void setPixmapCache(DesignerPixmapCache *pixmapCache);

    void setIcon(const PropertySheetIconValue &icon);
    PropertySheetIconValue icon() const;

    // Check whether a pixmap may be read
    enum CheckMode { CheckFast, CheckFully };
    static bool checkPixmap(const QString &fileName, CheckMode cm = CheckFully, QString *errorMessage = 0);
    // Choose a pixmap from file
    static QString choosePixmapFile(const QString &directory, QDesignerDialogGuiInterface *dlgGui, QWidget *parent);
    // Choose a pixmap from resource; use language-dependent resource browser if present
    static QString choosePixmapResource(QDesignerFormEditorInterface *core, QtResourceModel *resourceModel, const QString &oldPath, QWidget *parent);

signals:
    void iconChanged(const PropertySheetIconValue &icon);
private:
    QScopedPointer<class IconSelectorPrivate> d_ptr;
    Q_DECLARE_PRIVATE(IconSelector)
    Q_DISABLE_COPY(IconSelector)

    Q_PRIVATE_SLOT(d_func(), void slotStateActivated())
    Q_PRIVATE_SLOT(d_func(), void slotSetActivated())
    Q_PRIVATE_SLOT(d_func(), void slotSetResourceActivated())
    Q_PRIVATE_SLOT(d_func(), void slotSetFileActivated())
    Q_PRIVATE_SLOT(d_func(), void slotResetActivated())
    Q_PRIVATE_SLOT(d_func(), void slotResetAllActivated())
    Q_PRIVATE_SLOT(d_func(), void slotUpdate())
};

// IconThemeEditor: Let's the user input theme icon names and shows a preview label.
class Q_DESIGNER_EXPORT IconThemeEditor : public QWidget
{
    Q_OBJECT
    Q_PROPERTY(QString theme READ theme WRITE setTheme DESIGNABLE true)
public:
    explicit IconThemeEditor(QWidget *parent = nullptr, bool wantResetButton = true);
    virtual ~IconThemeEditor();

    QString theme() const;
    void setTheme(const QString &theme);

signals:
    void edited(const QString &);

public slots:
    void reset();

private slots:
    void slotChanged(const QString &);

private:
    void updatePreview(const QString &);

    QScopedPointer<IconThemeEditorPrivate> d;
};

} // namespace qdesigner_internal

QT_END_NAMESPACE

#endif // ICONSELECTOR_H

