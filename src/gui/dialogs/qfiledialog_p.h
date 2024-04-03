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

#ifndef QFILEDIALOG_P_H
#define QFILEDIALOG_P_H

//
//  W A R N I N G
//  -------------
//
// This file is not part of the Katie API.  It exists purely as an
// implementation detail.  This header file may change from version to
// version without notice, or even be removed.
//
// We mean it.
//

#ifndef QT_NO_FILEDIALOG

#include "qfiledialog.h"
#include "qdialog_p.h"
#include "qplatformdefs.h"

#include "qfilesystemmodel_p.h"
#include "qlistview.h"
#include "qtreeview.h"
#include "qcombobox.h"
#include "qtoolbutton.h"
#include "qlabel.h"
#include "qevent.h"
#include "qlineedit.h"
#include "qurl.h"
#include "qstackedwidget.h"
#include "qdialogbuttonbox.h"
#include "qabstractproxymodel.h"
#include "qcompleter.h"
#include "qpointer.h"
#include "qdebug.h"
#include "qsidebar_p.h"
#include "qfscompleter_p.h"
#include "qguiplatformplugin.h"

#include <unistd.h>

QT_BEGIN_NAMESPACE

class QFileDialogListView;
class QFileDialogTreeView;
class QFileDialogLineEdit;
class QGridLayout;
class QCompleter;
class QHBoxLayout;
class Ui_QFileDialog;

class Q_AUTOTEST_EXPORT QFileDialogPrivate : public QDialogPrivate
{
    Q_DECLARE_PUBLIC(QFileDialog)

public:
    QFileDialogPrivate();

    void createToolButtons();
    void createMenuActions();
    void createWidgets();

    void init(const QString &directory = QString(), const QString &nameFilter = QString(),
              const QString &caption = QString());
    bool itemViewKeyboardEvent(QKeyEvent *event);
    QString getEnvironmentVariable(const QString &string);
    static QString workingDirectory(const QString &path);
    static QString initialSelection(const QString &path);
    QStringList typedFiles() const;
    QStringList addDefaultSuffixToFiles(const QStringList &filesToFix) const;
    bool removeDirectory(const QString &path);

    inline QModelIndex mapToSource(const QModelIndex &index) const;
    inline QModelIndex mapFromSource(const QModelIndex &index) const;
    inline QModelIndex rootIndex() const;
    inline void setRootIndex(const QModelIndex &index) const;
    inline QModelIndex select(const QModelIndex &index) const;
    inline QString rootPath() const;

    QLineEdit *lineEdit() const;

    int maxNameLength(const QString &path) {
        return ::pathconf(QFile::encodeName(path).data(), _PC_NAME_MAX);
    }

    QString basename(const QString &path) const
    {
        int separator = path.lastIndexOf(QDir::separator());
        if (separator != -1)
            return path.mid(separator + 1);
        return path;
    }

    QDir::Filters filterForMode(QDir::Filters filters) const
    {
        if (fileMode == QFileDialog::DirectoryOnly) {
            filters |= QDir::Drives | QDir::AllDirs | QDir::Dirs;
            filters &= ~QDir::Files;
        } else {
            filters |= QDir::Drives | QDir::AllDirs | QDir::Files | QDir::Dirs;
        }
        return filters;
    }

    QAbstractItemView *currentView() const;

    void setLastVisitedDirectory(const QString &dir);
    void retranslateWindowTitle();
    void retranslateStrings();
    void emitFilesSelected(const QStringList &files);

    void _q_goHome();
    void _q_pathChanged(const QString &);
    void _q_navigateBackward();
    void _q_navigateForward();
    void _q_navigateToParent();
    void _q_createDirectory();
    void _q_showListView();
    void _q_showDetailsView();
    void _q_showContextMenu(const QPoint &position);
    void _q_renameCurrent();
    void _q_deleteCurrent();
    void _q_showHidden();
    void _q_showHeader(QAction *);
    void _q_updateOkButton();
    void _q_currentChanged(const QModelIndex &index);
    void _q_enterDirectory(const QModelIndex &index);
    void _q_goToDirectory(const QString &);
    void _q_useNameFilter(int index);
    void _q_selectionChanged();
    void _q_goToUrl(const QUrl &url);
    void _q_autoCompleteFileName(const QString &);
    void _q_rowsInserted(const QModelIndex &parent);
    void _q_fileRenamed(const QString &path, const QString &oldName, const QString &newName);

    // layout
#ifndef QT_NO_PROXYMODEL
    QAbstractProxyModel *proxyModel;
#endif

    // data
    QStringList watching;
    QFileSystemModel *model;

#ifndef QT_NO_FSCOMPLETER
    QFSCompleter *completer;
#endif //QT_NO_FSCOMPLETER

    QFileDialog::FileMode fileMode;
    QFileDialog::AcceptMode acceptMode;
    bool confirmOverwrite;
    QString defaultSuffix;
    QString setWindowTitle;

    QStringList currentHistory;
    int currentHistoryLocation;

    QAction *renameAction;
    QAction *deleteAction;
    QAction *showHiddenAction;
    QAction *newFolderAction;

    bool useDefaultCaption;
    bool defaultFileTypes;
    bool fileNameLabelExplicitlySat;
    QStringList nameFilters;

    //////////////////////////////////////////////
    QScopedPointer<Ui_QFileDialog> qFileDialogUi;

    QString acceptLabel;

    QPointer<QObject> receiverToDisconnectOnClose;
    QByteArray memberToDisconnectOnClose;
    QByteArray signalToDisconnectOnClose;

    QFileDialog::Options opts;

    ~QFileDialogPrivate();

private:
    Q_DISABLE_COPY(QFileDialogPrivate)
};

class QFileDialogLineEdit : public QLineEdit
{
public:
    QFileDialogLineEdit(QWidget *parent = nullptr) : QLineEdit(parent), hideOnEsc(false), d_ptr(0){}
    void init(QFileDialogPrivate *d_pointer) {d_ptr = d_pointer; }
    void keyPressEvent(QKeyEvent *e);
    bool hideOnEsc;
private:
    QFileDialogPrivate *d_ptr;
};

class QFileDialogComboBox : public QComboBox
{
public:
    QFileDialogComboBox(QWidget *parent = nullptr) : QComboBox(parent), urlModel(0) {}
    void init(QFileDialogPrivate *d_pointer);
    void showPopup();
    void setHistory(const QStringList &paths);
    QStringList history() const { return m_history; }
    void paintEvent(QPaintEvent *);

private:
    QUrlModel *urlModel;
    QFileDialogPrivate *d_ptr;
    QStringList m_history;
};

class QFileDialogListView : public QListView
{
public:
    QFileDialogListView(QWidget *parent = nullptr);
    void init(QFileDialogPrivate *d_pointer);
    QSize sizeHint() const;
protected:
    void keyPressEvent(QKeyEvent *e);
private:
    QFileDialogPrivate *d_ptr;
};

class QFileDialogTreeView : public QTreeView
{
public:
    QFileDialogTreeView(QWidget *parent);
    void init(QFileDialogPrivate *d_pointer);
    QSize sizeHint() const;

protected:
    void keyPressEvent(QKeyEvent *e);
private:
    QFileDialogPrivate *d_ptr;
};

inline QModelIndex QFileDialogPrivate::mapToSource(const QModelIndex &index) const
{
#ifdef QT_NO_PROXYMODEL
    return index;
#else
    return proxyModel ? proxyModel->mapToSource(index) : index;
#endif
}

inline QModelIndex QFileDialogPrivate::mapFromSource(const QModelIndex &index) const
{
#ifdef QT_NO_PROXYMODEL
    return index;
#else
    return proxyModel ? proxyModel->mapFromSource(index) : index;
#endif
}

inline QString QFileDialogPrivate::rootPath() const
{
    return model->rootPath();
}

QT_END_NAMESPACE

#endif // QT_NO_FILEDIALOG

#endif // QFILEDIALOG_P_H
