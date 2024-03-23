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

#ifndef QFILESYSTEMMODEL_P_H
#define QFILESYSTEMMODEL_P_H

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

#include "qfilesystemmodel.h"

#ifndef QT_NO_FILESYSTEMMODEL

#include "qabstractitemmodel_p.h"
#include "qabstractitemmodel.h"
#include "qfileinfogatherer_p.h"
#include "qpair.h"
#include "qdir.h"
#include "qicon.h"
#include "qfileinfo.h"
#include "qtimer.h"
#include "qhash.h"
#include "qbasictimer.h"

QT_BEGIN_NAMESPACE

class QExtendedInformation;
class QFileSystemModelPrivate;
class QFileIconProvider;

class Q_AUTOTEST_EXPORT QFileSystemModelPrivate : public QAbstractItemModelPrivate
{
    Q_DECLARE_PUBLIC(QFileSystemModel)

public:
    class QFileSystemNode
    {
    public:
        QFileSystemNode(const QString &filename = QString(), QFileSystemNode *p = 0)
            : fileName(filename), populatedChildren(false), isVisible(false), dirtyChildrenIndex(-1), parent(p), info(0) {}
        ~QFileSystemNode() {
            QHash<QString, QFileSystemNode*>::const_iterator i = children.constBegin();
            while (i != children.constEnd()) {
                    delete i.value();
                    ++i;
            }
            delete info;
            info = 0;
            parent = 0;
        }

        QString fileName;

        inline qint64 size() const { if (info && !info->isDir()) return info->size(); return 0; }
        inline QString type() const { if (info) return info->displayType; return QLatin1String(""); }
        inline QDateTime lastModified() const { if (info) return info->lastModified(); return QDateTime(); }
        inline QFile::Permissions permissions() const { if (info) return info->permissions(); return 0; }
        inline bool isReadable() const { return ((permissions() & QFile::ReadUser) != 0); }
        inline bool isWritable() const { return ((permissions() & QFile::WriteUser) != 0); }
        inline bool isExecutable() const { return ((permissions() & QFile::ExeUser) != 0); }
        inline bool isDir() const {
            if (info)
                return info->isDir();
            if (children.count() > 0)
                return true;
            return false;
        }
        inline bool isFile() const { if (info) return info->isFile(); return true; }
        inline bool isSystem() const { if (info) return info->isSystem(); return true; }
        inline bool isHidden() const { if (info) return info->isHidden(); return false; }
        inline bool isSymLink() const { return info && info->isSymLink(); }
        inline QIcon icon() const { if (info) return info->icon; return QIcon(); }

        inline bool operator <(const QFileSystemNode &node) const {
            return fileName < node.fileName;
        }
        inline bool operator >(const QString &name) const {
            return fileName > name;
        }
        inline bool operator <(const QString &name) const {
            return fileName < name;
        }
        inline bool operator !=(const QExtendedInformation &fileInfo) const {
            return !operator==(fileInfo);
        }
        bool operator ==(const QString &name) const {
            return fileName == name;
        }
        bool operator ==(const QExtendedInformation &fileInfo) const {
            return info && (*info == fileInfo);
        }

        inline bool hasInformation() const { return info != 0; }

        void populate(const QExtendedInformation &fileInfo) {
            if (!info)
                info = new QExtendedInformation(fileInfo.fileInfo());
            (*info) = fileInfo;
        }

        // children shouldn't normally be accessed directly, use node()
        inline int visibleLocation(const QString &childName) {
            return visibleChildren.indexOf(childName);
        }
        void updateIcon(QFileIconProvider *iconProvider, const QString &path) {
            if (info)
                info->icon = iconProvider->icon(QFileInfo(path));
            QHash<QString, QFileSystemNode *>::const_iterator iterator;
            for(iterator = children.constBegin() ; iterator != children.constEnd() ; ++iterator) {
                //On windows the root (My computer) has no path so we don't want to add a / for nothing (e.g. /C:/)
                if (!path.isEmpty()) {
                    if (path.endsWith(QLatin1Char('/')))
                        iterator.value()->updateIcon(iconProvider, path + iterator.value()->fileName);
                    else
                        iterator.value()->updateIcon(iconProvider, path + QLatin1Char('/') + iterator.value()->fileName);
                } else
                    iterator.value()->updateIcon(iconProvider, iterator.value()->fileName);
            }
        }

        void retranslateStrings(QFileIconProvider *iconProvider, const QString &path) {
            if (info)
                info->displayType = iconProvider->type(QFileInfo(path));
            QHash<QString, QFileSystemNode *>::const_iterator iterator;
            for(iterator = children.constBegin() ; iterator != children.constEnd() ; ++iterator) {
                //On windows the root (My computer) has no path so we don't want to add a / for nothing (e.g. /C:/)
                if (!path.isEmpty()) {
                    if (path.endsWith(QLatin1Char('/')))
                        iterator.value()->retranslateStrings(iconProvider, path + iterator.value()->fileName);
                    else
                        iterator.value()->retranslateStrings(iconProvider, path + QLatin1Char('/') + iterator.value()->fileName);
                } else
                    iterator.value()->retranslateStrings(iconProvider, iterator.value()->fileName);
            }
        }

        bool populatedChildren;
        bool isVisible;
        QHash<QString,QFileSystemNode *> children;
        QList<QString> visibleChildren;
        int dirtyChildrenIndex;
        QFileSystemNode *parent;


        QExtendedInformation *info;

    };

    QFileSystemModelPrivate() :
            forceSort(true),
            sortColumn(0),
            sortOrder(Qt::AscendingOrder),
            readOnly(true),
            setRootPath(false),
            filters(QDir::AllEntries | QDir::NoDotAndDotDot | QDir::AllDirs),
            nameFilterDisables(true), // false on windows, true on mac and unix
            disableRecursiveSort(false)
    {
        delayedSortTimer.setSingleShot(true);
    }

    void init();
    /*
      \internal

      Return true if index which is owned by node is hidden by the filter.
    */
    inline bool isHiddenByFilter(QFileSystemNode *indexNode, const QModelIndex &index) const
    {
       return (indexNode != &root && !index.isValid());
    }
    QFileSystemNode *node(const QModelIndex &index) const;
    QFileSystemNode *node(const QString &path, bool fetch = true) const;
    inline QModelIndex index(const QString &path) { return index(node(path)); }
    QModelIndex index(const QFileSystemNode *node) const;
    bool filtersAcceptsNode(const QFileSystemNode *node) const;
    bool passNameFilters(const QFileSystemNode *node) const;
    void removeNode(QFileSystemNode *parentNode, const QString &name);
    QFileSystemNode* addNode(QFileSystemNode *parentNode, const QString &fileName, const QFileInfo &info);
    void addVisibleFiles(QFileSystemNode *parentNode, const QStringList &newFiles);
    void removeVisibleFile(QFileSystemNode *parentNode, int visibleLocation);
    void sortChildren(int column, const QModelIndex &parent);

    inline int translateVisibleLocation(QFileSystemNode *parent, int row) const {
        if (sortOrder != Qt::AscendingOrder) {
            if (parent->dirtyChildrenIndex == -1)
                return parent->visibleChildren.count() - row - 1;

            if (row < parent->dirtyChildrenIndex)
                return parent->dirtyChildrenIndex - row - 1;
        }

        return row;
    }

    inline static QString myComputer() {
        return QFileSystemModel::tr("Computer");
    }

    inline void delayedSort() {
        if (!delayedSortTimer.isActive())
            delayedSortTimer.start(0);
    }

    QIcon icon(const QModelIndex &index) const;
    QString name(const QModelIndex &index) const;
    QString filePath(const QModelIndex &index) const;
    QString size(const QModelIndex &index) const;
    static QString size(qint64 bytes);
    QString type(const QModelIndex &index) const;
    QString time(const QModelIndex &index) const;

    void _q_directoryChanged(const QString &directory, const QStringList &list);
    void _q_performDelayedSort();
    void _q_fileSystemChanged(const QString &path, const QList<QPair<QString, QFileInfo> > &);

    static int naturalCompare(const QString &s1, const QString &s2, Qt::CaseSensitivity cs);

    QDir rootDir;
    QFileInfoGatherer fileInfoGatherer;
    QTimer delayedSortTimer;
    bool forceSort;
    int sortColumn;
    Qt::SortOrder sortOrder;
    bool readOnly;
    bool setRootPath;
    QDir::Filters filters;
    QSet<const QFileSystemNode*> bypassFilters;
    bool nameFilterDisables;
    //This flag is an optimization for the QFileDialog
    //It enable a sort which is not recursive, it means
    //we sort only what we see.
    bool disableRecursiveSort;
    QList<QRegExp> nameFilters;

    QFileSystemNode root;

    QBasicTimer fetchingTimer;
    struct Fetching {
        QString dir;
        QString file;
        const QFileSystemNode *node;
    };
    QList<Fetching> toFetch;

};

QT_END_NAMESPACE

#endif // QT_NO_FILESYSTEMMODEL

#endif // QFILESYSTEMMODEL_P_H
