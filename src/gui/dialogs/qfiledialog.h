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

#ifndef QFILEDIALOG_H
#define QFILEDIALOG_H

#include <QtCore/qdir.h>
#include <QtCore/qstring.h>
#include <QtGui/qdialog.h>


QT_BEGIN_NAMESPACE


#ifndef QT_NO_FILEDIALOG

class QModelIndex;
class QItemSelection;
struct QFileDialogArgs;
class QFileIconProvider;
class QFileDialogPrivate;
class QAbstractItemDelegate;
class QAbstractProxyModel;
class QUrl;

class Q_GUI_EXPORT QFileDialog : public QDialog
{
    Q_OBJECT
    Q_ENUMS(ViewMode FileMode AcceptMode Option)
    Q_FLAGS(Options)
    Q_PROPERTY(ViewMode viewMode READ viewMode WRITE setViewMode)
    Q_PROPERTY(FileMode fileMode READ fileMode WRITE setFileMode)
    Q_PROPERTY(AcceptMode acceptMode READ acceptMode WRITE setAcceptMode)
    Q_PROPERTY(bool readOnly READ isReadOnly WRITE setReadOnly)
    Q_PROPERTY(bool confirmOverwrite READ confirmOverwrite WRITE setConfirmOverwrite)
    Q_PROPERTY(QString defaultSuffix READ defaultSuffix WRITE setDefaultSuffix)
    Q_PROPERTY(bool nameFilterDetailsVisible READ isNameFilterDetailsVisible WRITE setNameFilterDetailsVisible)
    Q_PROPERTY(Options options READ options WRITE setOptions)

public:
    enum ViewMode { Detail, List };
    enum FileMode { AnyFile, ExistingFile, Directory, ExistingFiles, DirectoryOnly };
    enum AcceptMode { AcceptOpen, AcceptSave };
    enum DialogLabel { LookIn, FileName, FileType, Accept, Reject };

    // ### Rename to FileDialogOption and FileDialogOptions for Qt 5.0
    enum Option
    {
        ShowDirsOnly                = 0x00000001,
        DontConfirmOverwrite        = 0x00000002,
        DontUseSheet                = 0x00000004,
        ReadOnly                    = 0x00000008,
        HideNameFilterDetails       = 0x00000010
    };
    Q_DECLARE_FLAGS(Options, Option)

    QFileDialog(QWidget *parent, Qt::WindowFlags f);
    explicit QFileDialog(QWidget *parent = nullptr,
                         const QString &caption = QString(),
                         const QString &directory = QString(),
                         const QString &filter = QString());
    ~QFileDialog();

    void setDirectory(const QString &directory);
    inline void setDirectory(const QDir &directory);
    QDir directory() const;

    void selectFile(const QString &filename);
    QStringList selectedFiles() const;

    void setNameFilterDetailsVisible(bool enabled);
    bool isNameFilterDetailsVisible() const;

    void setNameFilter(const QString &filter);
    void setNameFilters(const QStringList &filters);
    QStringList nameFilters() const;
    void selectNameFilter(const QString &filter);
    QString selectedNameFilter() const;

    QDir::Filters filter() const;
    void setFilter(QDir::Filters filters);

    void setViewMode(ViewMode mode);
    ViewMode viewMode() const;

    void setFileMode(FileMode mode);
    FileMode fileMode() const;

    void setAcceptMode(AcceptMode mode);
    AcceptMode acceptMode() const;

    void setReadOnly(bool enabled);
    bool isReadOnly() const;

    void setSidebarUrls(const QList<QUrl> &urls);
    QList<QUrl> sidebarUrls() const;

    QByteArray saveState() const;
    bool restoreState(const QByteArray &state);

    void setConfirmOverwrite(bool enabled);
    bool confirmOverwrite() const;

    void setDefaultSuffix(const QString &suffix);
    QString defaultSuffix() const;

    void setHistory(const QStringList &paths);
    QStringList history() const;

    void setItemDelegate(QAbstractItemDelegate *delegate);
    QAbstractItemDelegate *itemDelegate() const;

    void setIconProvider(QFileIconProvider *provider);
    QFileIconProvider *iconProvider() const;

    void setLabelText(DialogLabel label, const QString &text);
    QString labelText(DialogLabel label) const;

#ifndef QT_NO_PROXYMODEL
    void setProxyModel(QAbstractProxyModel *model);
    QAbstractProxyModel *proxyModel() const;
#endif

    void setOption(Option option, bool on = true);
    bool testOption(Option option) const;
    void setOptions(Options options);
    Options options() const;

#ifdef Q_NO_USING_KEYWORD
    void open() { QDialog::open(); }
#else
    using QDialog::open;
#endif
    void open(QObject *receiver, const char *member);
    void setVisible(bool visible);

Q_SIGNALS:
    void fileSelected(const QString &file);
    void filesSelected(const QStringList &files);
    void currentChanged(const QString &path);
    void directoryEntered(const QString &directory);
    void filterSelected(const QString &filter);

public:

    static QString getOpenFileName(QWidget *parent = nullptr,
                                   const QString &caption = QString(),
                                   const QString &dir = QString(),
                                   const QString &filter = QString(),
                                   QString *selectedFilter = 0,
                                   Options options = 0);

    static QString getSaveFileName(QWidget *parent = nullptr,
                                   const QString &caption = QString(),
                                   const QString &dir = QString(),
                                   const QString &filter = QString(),
                                   QString *selectedFilter = 0,
                                   Options options = 0);

    static QString getExistingDirectory(QWidget *parent = nullptr,
                                        const QString &caption = QString(),
                                        const QString &dir = QString(),
                                        Options options = ShowDirsOnly);

    static QStringList getOpenFileNames(QWidget *parent = nullptr,
                                        const QString &caption = QString(),
                                        const QString &dir = QString(),
                                        const QString &filter = QString(),
                                        QString *selectedFilter = 0,
                                        Options options = 0);


protected:
    void done(int result);
    void accept();
    void changeEvent(QEvent *e);

private:
    Q_DECLARE_PRIVATE(QFileDialog)
    Q_DISABLE_COPY(QFileDialog)

    Q_PRIVATE_SLOT(d_func(), void _q_pathChanged(const QString &))

    Q_PRIVATE_SLOT(d_func(), void _q_navigateBackward())
    Q_PRIVATE_SLOT(d_func(), void _q_navigateForward())
    Q_PRIVATE_SLOT(d_func(), void _q_navigateToParent())
    Q_PRIVATE_SLOT(d_func(), void _q_createDirectory())
    Q_PRIVATE_SLOT(d_func(), void _q_showListView())
    Q_PRIVATE_SLOT(d_func(), void _q_showDetailsView())
    Q_PRIVATE_SLOT(d_func(), void _q_showContextMenu(const QPoint &))
    Q_PRIVATE_SLOT(d_func(), void _q_renameCurrent())
    Q_PRIVATE_SLOT(d_func(), void _q_deleteCurrent())
    Q_PRIVATE_SLOT(d_func(), void _q_showHidden())
    Q_PRIVATE_SLOT(d_func(), void _q_updateOkButton())
    Q_PRIVATE_SLOT(d_func(), void _q_currentChanged(const QModelIndex &index))
    Q_PRIVATE_SLOT(d_func(), void _q_enterDirectory(const QModelIndex &index))
    Q_PRIVATE_SLOT(d_func(), void _q_goToDirectory(const QString &path))
    Q_PRIVATE_SLOT(d_func(), void _q_useNameFilter(int index))
    Q_PRIVATE_SLOT(d_func(), void _q_selectionChanged())
    Q_PRIVATE_SLOT(d_func(), void _q_goToUrl(const QUrl &url))
    Q_PRIVATE_SLOT(d_func(), void _q_goHome())
    Q_PRIVATE_SLOT(d_func(), void _q_showHeader(QAction *))
    Q_PRIVATE_SLOT(d_func(), void _q_autoCompleteFileName(const QString &text))
    Q_PRIVATE_SLOT(d_func(), void _q_rowsInserted(const QModelIndex & parent))
    Q_PRIVATE_SLOT(d_func(), void _q_fileRenamed(const QString &path,
                const QString oldName, const QString newName))
};

inline void QFileDialog::setDirectory(const QDir &adirectory)
{ setDirectory(adirectory.absolutePath()); }

Q_DECLARE_OPERATORS_FOR_FLAGS(QFileDialog::Options)

#endif // QT_NO_FILEDIALOG

QT_END_NAMESPACE


#endif // QFILEDIALOG_H
