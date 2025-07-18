/****************************************************************************
**
** Copyright (C) 2015 The Qt Company Ltd.
** Copyright (C) 2016 Ivailo Monev
**
** This file is part of the QtCore module of the Katie Toolkit.
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

#include "qplatformdefs.h"
#include "qdir.h"
#include "qdir_p.h"
#ifndef QT_NO_DEBUG_STREAM
#include "qdebug.h"
#endif
#include "qdiriterator.h"
#include "qdatetime.h"
#include "qstring.h"
#include "qregexp.h"
#include "qvector.h"
#include "qalgorithms.h"
#include "qfilesystementry_p.h"
#include "qfilesystemmetadata_p.h"
#include "qfilesystemengine_p.h"
#include "qscopedpointer.h"
#include "qstdcontainers_p.h"
#include "qcorecommon_p.h"

#include <stdlib.h>

QT_BEGIN_NAMESPACE

//************* QDirPrivate
QDirPrivate::QDirPrivate(const QString &path, const QStringList &nameFilters_, QDir::SortFlags sort_, QDir::Filters filters_)
    : QSharedData()
    , nameFilters(nameFilters_)
    , sort(sort_)
    , filters(filters_)
    , fileListsInitialized(false)
{
    setPath(path.isEmpty() ? QString::fromLatin1(".") : path);

    bool empty = nameFilters.isEmpty();
    if (!empty) {
        empty = true;
        for (int i = 0; i < nameFilters.size(); ++i) {
            if (!nameFilters.at(i).isEmpty()) {
                empty = false;
                break;
            }
        }
    }
    if (empty)
        nameFilters = QStringList(QString::fromLatin1("*"));
}

QDirPrivate::QDirPrivate(const QDirPrivate &copy)
    : QSharedData(copy)
    , nameFilters(copy.nameFilters)
    , sort(copy.sort)
    , filters(copy.filters)
    , fileListsInitialized(false)
    , dirEntry(copy.dirEntry)
    , metaData(copy.metaData)
{
}

QDirPrivate::~QDirPrivate()
{
}

bool QDirPrivate::exists() const
{
    QFileSystemEngine::fillMetaData(dirEntry, metaData,
            QFileSystemMetaData::ExistsAttribute | QFileSystemMetaData::DirectoryType); // always stat
    return metaData.exists() && metaData.isDirectory();
}

inline void QDirPrivate::setPath(const QString &path)
{
    QString p = path;
    if (p.endsWith(QLatin1Char('/')) && p.length() > 1) {
        p.truncate(p.length() - 1);
    }

    dirEntry = QFileSystemEntry(path);
    metaData.clear();
    clearFileLists();
    absoluteDirEntry = QFileSystemEntry();
}

inline void QDirPrivate::clearFileLists()
{
    fileListsInitialized = false;
    files.clear();
    fileInfos.clear();
}

inline void QDirPrivate::resolveAbsoluteEntry() const
{
    if (!absoluteDirEntry.isEmpty() || dirEntry.isEmpty())
        return;

    QString absoluteName = QFileSystemEngine::absoluteName(dirEntry).filePath();
    absoluteDirEntry = QFileSystemEntry(QDir::cleanPath(absoluteName));
}

/* For sorting */
struct QDirSortItem
{
    mutable QString filename_cache;
    mutable QString suffix_cache;
    QFileInfo item;
};


class QDirSortItemComparator
{
    QDir::SortFlags qt_cmp_si_sort_flags;
public:
    QDirSortItemComparator(QDir::SortFlags flags) : qt_cmp_si_sort_flags(flags) {}
    bool operator()(const QDirSortItem &, const QDirSortItem &) const;
};

bool QDirSortItemComparator::operator()(const QDirSortItem &n1, const QDirSortItem &n2) const
{
    const QDirSortItem* f1 = &n1;
    const QDirSortItem* f2 = &n2;

    if ((qt_cmp_si_sort_flags & QDir::DirsFirst) && (f1->item.isDir() != f2->item.isDir()))
        return f1->item.isDir();
    if ((qt_cmp_si_sort_flags & QDir::DirsLast) && (f1->item.isDir() != f2->item.isDir()))
        return !f1->item.isDir();

    int r = 0;
    int sortBy = (qt_cmp_si_sort_flags & QDir::SortByMask)
                 | (qt_cmp_si_sort_flags & QDir::Type);

    switch (sortBy) {
        case QDir::Time: {
            r = f1->item.lastModified().secsTo(f2->item.lastModified());
            break;
        }
        case QDir::Size: {
            r = int(qBound<qint64>(-1, f2->item.size() - f1->item.size(), 1));
            break;
        }
        case QDir::Type: {
            bool ic = qt_cmp_si_sort_flags & QDir::IgnoreCase;

            if (f1->suffix_cache.isNull())
                f1->suffix_cache = ic ? f1->item.suffix().toLower()
                                : f1->item.suffix();
            if (f2->suffix_cache.isNull())
                f2->suffix_cache = ic ? f2->item.suffix().toLower()
                                : f2->item.suffix();

            r = qt_cmp_si_sort_flags & QDir::LocaleAware
                ? f1->suffix_cache.localeAwareCompare(f2->suffix_cache)
                : f1->suffix_cache.compare(f2->suffix_cache);
            break;
        }
        default:
            ;
    }

    if (r == 0 && sortBy != QDir::Unsorted) {
        // Still not sorted - sort by name
        bool ic = qt_cmp_si_sort_flags & QDir::IgnoreCase;

        if (f1->filename_cache.isNull())
            f1->filename_cache = ic ? f1->item.fileName().toLower()
                                    : f1->item.fileName();
        if (f2->filename_cache.isNull())
            f2->filename_cache = ic ? f2->item.fileName().toLower()
                                    : f2->item.fileName();

        r = qt_cmp_si_sort_flags & QDir::LocaleAware
            ? f1->filename_cache.localeAwareCompare(f2->filename_cache)
            : f1->filename_cache.compare(f2->filename_cache);
    }
    if (r == 0) // Enforce an order - the order the items appear in the array
        r = (&n1) - (&n2);
    if (qt_cmp_si_sort_flags & QDir::Reversed)
        return r > 0;
    return r < 0;
}

inline void QDirPrivate::sortFileList(QDir::SortFlags sort, QFileInfoList &l,
                                      QStringList *names, QFileInfoList *infos)
{
    // names and infos are always empty lists or 0 here
    int n = l.size();
    if (n > 0) {
        if (n == 1 || (sort & QDir::SortByMask) == QDir::Unsorted) {
            if (infos)
                *infos = l;
            if (names) {
                for (int i = 0; i < n; ++i)
                    names->append(l.at(i).fileName());
            }
        } else {
            QStdVector<QDirSortItem> si(n);
            for (int i = 0; i < n; ++i)
                si[i].item = l.at(i);
            qSort(si.data(), si.data() + n, QDirSortItemComparator(sort));
            // put them back in the list(s)
            if (infos) {
                for (int i = 0; i < n; ++i)
                    infos->append(si[i].item);
            }
            if (names) {
                for (int i = 0; i < n; ++i)
                    names->append(si[i].item.fileName());
            }
        }
    }
}
inline void QDirPrivate::initFileLists(const QDir &dir) const
{
    if (!fileListsInitialized) {
        QFileInfoList l;
        QDirIterator it(dir);
        while (it.hasNext()) {
            it.next();
            l.append(it.fileInfo());
        }
        sortFileList(sort, l, &files, &fileInfos);
        fileListsInitialized = true;
    }
}

/*!
    \class QDir
    \brief The QDir class provides access to directory structures and their contents.

    \ingroup io
    \ingroup shared
    \reentrant


    A QDir is used to manipulate path names, access information
    regarding paths and files, and manipulate the underlying file
    system. It can also be used to access Qt's \l{resource system}.

    Qt uses "/" as a universal directory separator in the same way
    that "/" is used as a path separator in URLs. If you always use
    "/" as a directory separator, Qt will translate your paths to
    conform to the underlying operating system.

    A QDir can point to a file using either a relative or an absolute
    path. Absolute paths begin with the directory separator
    (optionally preceded by a drive specification under Windows).
    Relative file names begin with a directory name or a file name and
    specify a path relative to the current directory.

    Examples of absolute paths:

    \snippet doc/src/snippets/code/src_corelib_io_qdir.cpp 0

    On Windows, the second example above will be translated to
    \c{C:\Documents and Settings} when used to access files.

    Examples of relative paths:

    \snippet doc/src/snippets/code/src_corelib_io_qdir.cpp 1

    You can use the isRelative() or isAbsolute() functions to check if
    a QDir is using a relative or an absolute file path. Call
    makeAbsolute() to convert a relative QDir to an absolute one.

    \section1 Navigation and Directory Operations

    A directory's path can be obtained with the path() function, and
    a new path set with the setPath() function. The absolute path to
    a directory is found by calling absolutePath().

    The name of a directory is found using the dirName() function. This
    typically returns the last element in the absolute path that specifies
    the location of the directory. However, it can also return "." if
    the QDir represents the current directory.

    \snippet doc/src/snippets/code/src_corelib_io_qdir.cpp 2

    The path for a directory can also be changed with the cd() and cdUp()
    functions, both of which operate like familiar shell commands.
    When cd() is called with the name of an existing directory, the QDir
    object changes directory so that it represents that directory instead.
    The cdUp() function changes the directory of the QDir object so that
    it refers to its parent directory; i.e. cd("..") is equivalent to
    cdUp().

    Directories can be created with mkdir(), renamed with rename(), and
    removed with rmdir().

    You can test for the presence of a directory with a given name by
    using exists(), and the properties of a directory can be tested with
    isReadable(), isAbsolute(), isRelative(), and isRoot().

    The refresh() function re-reads the directory's data from disk.

    \section1 Files and Directory Contents

    Directories contain a number of entries, representing files,
    directories, and symbolic links. The number of entries in a
    directory is returned by count().
    A string list of the names of all the entries in a directory can be
    obtained with entryList(). If you need information about each
    entry, use entryInfoList() to obtain a list of QFileInfo objects.

    Paths to files and directories within a directory can be
    constructed using filePath() and absoluteFilePath().
    The filePath() function returns a path to the specified file
    or directory relative to the path of the QDir object;
    absoluteFilePath() returns an absolute path to the specified
    file or directory. Neither of these functions checks for the
    existence of files or directory; they only construct paths.

    \snippet doc/src/snippets/code/src_corelib_io_qdir.cpp 3

    Files can be removed by using the remove() function. Directories
    cannot be removed in the same way as files; use rmdir() to remove
    them instead.

    It is possible to reduce the number of entries returned by
    entryList() and entryInfoList() by applying filters to a QDir object.
    You can apply a name filter to specify a pattern with wildcards that
    file names need to match, an attribute filter that selects properties
    of entries and can distinguish between files and directories, and a
    sort order.

    Name filters are lists of strings that are passed to setNameFilters().
    Attribute filters consist of a bitwise OR combination of Filters, and
    these are specified when calling setFilter().
    The sort order is specified using setSorting() with a bitwise OR
    combination of SortFlags.

    You can test to see if a filename matches a filter using the match()
    function.

    Filter and sort order flags may also be specified when calling
    entryList() and entryInfoList() in order to override previously defined
    behavior.

    \section1 The Current Directory and Other Special Paths

    Access to some common directories is provided with a number of static
    functions that return QDir objects. There are also corresponding functions
    for these that return strings:

    \table
    \header \o QDir      \o QString         \o Return Value
    \row    \o current() \o currentPath()   \o The application's working directory
    \row    \o home()    \o homePath()      \o The user's home directory
    \row    \o root()    \o rootPath()      \o The root directory
    \row    \o temp()    \o tempPath()      \o The system's temporary directory
    \endtable

    The setCurrent() static function can also be used to set the application's
    working directory.

    If you want to find the directory containing the application's executable,
    see \l{QCoreApplication::applicationDirPath()}.

    \section1 Path Manipulation and Strings

    Paths containing "." elements that reference the current directory at that
    point in the path, ".." elements that reference the parent directory, and
    symbolic links can be reduced to a canonical form using the canonicalPath()
    function.

    Paths can also be simplified by using cleanPath() to remove redundant "/"
    and ".." elements.

    \section1 Examples

    Check if a directory exists:

    \snippet doc/src/snippets/code/src_corelib_io_qdir.cpp 4

    (We could also use the static convenience function
    QFile::exists().)

    Traversing directories and reading a file:

    \snippet doc/src/snippets/code/src_corelib_io_qdir.cpp 5

    A program that lists all the files in the current directory
    (excluding symbolic links), sorted by size, smallest first:

    \snippet doc/src/snippets/qdir-listfiles/main.cpp 0

    \sa QFileInfo, QFile, QFileDialog, QApplication::applicationDirPath(), {Find Files Example}
*/

/*!
    Constructs a QDir pointing to the given directory \a path. If path
    is empty the program's working directory, ("."), is used.

    \sa currentPath()
*/
QDir::QDir(const QString &path) : d_ptr(new QDirPrivate(path))
{
}

/*!
    Constructs a QDir with path \a path, that filters its entries by
    name using \a nameFilter and by attributes using \a filters. It
    also sorts the names using \a sort.

    The default \a nameFilter is an empty string, which excludes
    nothing; the default \a filters is \l AllEntries, which also means
    exclude nothing. The default \a sort is \l Name | \l IgnoreCase,
    i.e. sort by name case-insensitively.

    If \a path is an empty string, QDir uses "." (the current
    directory). If \a nameFilter is an empty string, QDir uses the
    name filter "*" (all files).

    Note that \a path need not exist.

    \sa exists(), setPath(), setNameFilter(), setFilter(), setSorting()
*/
QDir::QDir(const QString &path, const QString &nameFilter,
           SortFlags sort, Filters filters)
    : d_ptr(new QDirPrivate(path, QDir::nameFiltersFromString(nameFilter), sort, filters))
{
}

/*!
    Constructs a QDir object that is a copy of the QDir object for
    directory \a dir.

    \sa operator=()
*/
QDir::QDir(const QDir &dir)
    : d_ptr(dir.d_ptr)
{
}

/*!
    Destroys the QDir object frees up its resources. This has no
    effect on the underlying directory in the file system.
*/
QDir::~QDir()
{
}

/*!
    Sets the path of the directory to \a path. The path is cleaned of
    redundant ".", ".." and of multiple separators. No check is made
    to see whether a directory with this path actually exists; but you
    can check for yourself using exists().

    The path can be either absolute or relative. Absolute paths begin
    with the directory separator "/" (optionally preceded by a drive
    specification under Windows). Relative file names begin with a
    directory name or a file name and specify a path relative to the
    current directory. An example of an absolute path is the string
    "/tmp/quartz", a relative path might look like "src/fatlib".

    \sa path(), absolutePath(), exists(), cleanPath(), dirName(),
      absoluteFilePath(), isRelative(), makeAbsolute()
*/
void QDir::setPath(const QString &path)
{
    d_ptr->setPath(path);
}

/*!
    Returns the path. This may contain symbolic links, but never
    contains redundant ".", ".." or multiple separators.

    The returned path can be either absolute or relative (see
    setPath()).

    \sa setPath(), absolutePath(), exists(), cleanPath(), dirName(),
    absoluteFilePath(), makeAbsolute()
*/
QString QDir::path() const
{
    const QDirPrivate* d = d_ptr.constData();
    return d->dirEntry.filePath();
}

/*!
    Returns the absolute path (a path that starts with "/" or with a
    drive specification), which may contain symbolic links, but never
    contains redundant ".", ".." or multiple separators.

    \sa setPath(), canonicalPath(), exists(), cleanPath(),
    dirName(), absoluteFilePath()
*/
QString QDir::absolutePath() const
{
    const QDirPrivate* d = d_ptr.constData();
    d->resolveAbsoluteEntry();
    return d->absoluteDirEntry.filePath();
}

/*!
    Returns the canonical path, i.e. a path without symbolic links or
    redundant "." or ".." elements.

    On systems that do not have symbolic links this function will
    always return the same string that absolutePath() returns. If the
    canonical path does not exist (normally due to dangling symbolic
    links) canonicalPath() returns an empty string.

    Example:

    \snippet doc/src/snippets/code/src_corelib_io_qdir.cpp 6

    \sa path(), absolutePath(), exists(), cleanPath(), dirName(),
        absoluteFilePath()
*/
QString QDir::canonicalPath() const
{
    const QDirPrivate* d = d_ptr.constData();
    QFileSystemEntry answer = QFileSystemEngine::canonicalName(d->dirEntry, d->metaData);
    return answer.filePath();
}

/*!
    Returns the name of the directory; this is \e not the same as the
    path, e.g. a directory with the name "mail", might have the path
    "/var/spool/mail". If the directory has no name (e.g. it is the
    root directory) an empty string is returned.

    No check is made to ensure that a directory with this name
    actually exists; but see exists().

    \sa path(), filePath(), absolutePath(), absoluteFilePath()
*/
QString QDir::dirName() const
{
    const QDirPrivate* d = d_ptr.constData();
    return d->dirEntry.fileName();
}

/*!
    Returns the path name of a file in the directory. Does \e not
    check if the file actually exists in the directory; but see
    exists(). If the QDir is relative the returned path name will also
    be relative. Redundant multiple separators or "." and ".."
    directories in \a fileName are not removed (see cleanPath()).

    \sa dirName() absoluteFilePath(), isRelative(), canonicalPath()
*/
QString QDir::filePath(const QString &fileName) const
{
    const QDirPrivate* d = d_ptr.constData();
    if (isAbsolutePath(fileName))
        return fileName;

    QString ret = d->dirEntry.filePath();
    if (!fileName.isEmpty()) {
        if (!ret.isEmpty() && ret[ret.length()-1] != QLatin1Char('/') && fileName[0] != QLatin1Char('/'))
            ret += QLatin1Char('/');
        ret += fileName;
    }
    return ret;
}

/*!
    Returns the absolute path name of a file in the directory. Does \e
    not check if the file actually exists in the directory; but see
    exists(). Redundant multiple separators or "." and ".."
    directories in \a fileName are not removed (see cleanPath()).

    \sa relativeFilePath() filePath() canonicalPath()
*/
QString QDir::absoluteFilePath(const QString &fileName) const
{
    const QDirPrivate* d = d_ptr.constData();
    if (isAbsolutePath(fileName))
        return fileName;

    d->resolveAbsoluteEntry();
    if (fileName.isEmpty())
        return d->absoluteDirEntry.filePath();
    return QDir::cleanPath(d->absoluteDirEntry.filePath() + QLatin1Char('/') + fileName);
}

/*!
    Returns the path to \a fileName relative to the directory.

    \snippet doc/src/snippets/code/src_corelib_io_qdir.cpp 7

    \sa absoluteFilePath() filePath() canonicalPath()
*/
QString QDir::relativeFilePath(const QString &fileName) const
{
    QString dir = cleanPath(absolutePath());
    QString file = cleanPath(fileName);

    if (isRelativePath(file) || isRelativePath(dir))
        return file;

    if (file.startsWith(QLatin1String("//")) && !dir.startsWith(QLatin1String("//")))
        return file;

    QString result;
    QStringList dirElts = dir.split(QLatin1Char('/'), QString::SkipEmptyParts);
    QStringList fileElts = file.split(QLatin1Char('/'), QString::SkipEmptyParts);

    int i = 0;
    while (i < dirElts.size() && i < fileElts.size() && dirElts.at(i) == fileElts.at(i)) {
        ++i;
    }

    for (int j = 0; j < dirElts.size() - i; ++j)
        result += QLatin1String("../");

    for (int j = i; j < fileElts.size(); ++j) {
        result += fileElts.at(j);
        if (j < fileElts.size() - 1)
            result += QLatin1Char('/');
    }

    return result;
}

/*!
    Changes the QDir's directory to \a dirName.

    Returns true if the new directory exists and is readable;
    otherwise returns false. Note that the logical cd() operation is
    not performed if the new directory does not exist.

    Calling cd("..") is equivalent to calling cdUp().

    \sa cdUp(), isReadable(), exists(), path()
*/
bool QDir::cd(const QString &dirName)
{
    // Don't detach just yet.
    const QDirPrivate * const d = d_ptr.constData();

    if (dirName.isEmpty() || dirName == QLatin1String("."))
        return true;
    QString newPath;
    if (isAbsolutePath(dirName)) {
        newPath = cleanPath(dirName);
    } else {
        if (isRoot()) {
            if (dirName == QLatin1String(".."))
                return false;
            newPath = d->dirEntry.filePath();
        } else {
            newPath = d->dirEntry.filePath() + QLatin1Char('/');
        }

        newPath += dirName;
        if (dirName.indexOf(QLatin1Char('/')) >= 0
            || dirName == QLatin1String("..")
            || d->dirEntry.filePath() == QLatin1String(".")) {
            newPath = cleanPath(newPath);
            /*
              If newPath starts with .., we convert it to absolute to
              avoid infinite looping on

                  QDir dir(".");
                  while (dir.cdUp())
                      ;
            */
            if (newPath.startsWith(QLatin1String(".."))) {
                newPath = QFileInfo(newPath).absoluteFilePath();
            }
        }
    }

    QScopedPointer<QDirPrivate> dir(new QDirPrivate(*d_ptr.constData()));
    dir->setPath(newPath);
    if (!dir->exists())
        return false;

    d_ptr = dir.take();
    return true;
}

/*!
    Changes directory by moving one directory up from the QDir's
    current directory.

    Returns true if the new directory exists and is readable;
    otherwise returns false. Note that the logical cdUp() operation is
    not performed if the new directory does not exist.

    \sa cd(), isReadable(), exists(), path()
*/
bool QDir::cdUp()
{
    return cd(QString::fromLatin1(".."));
}

/*!
    Returns the string list set by setNameFilters()
*/
QStringList QDir::nameFilters() const
{
    const QDirPrivate* d = d_ptr.constData();
    return d->nameFilters;
}

/*!
    Sets the name filters used by entryList() and entryInfoList() to the
    list of filters specified by \a nameFilters.

    Each name filter is a wildcard (globbing) filter that understands
    \c{*} and \c{?} wildcards. (See \l{QRegExp wildcard matching}.)

    For example, the following code sets three name filters on a QDir
    to ensure that only files with extensions typically used for C++
    source files are listed:

    \snippet doc/src/snippets/qdir-namefilters/main.cpp 0

    \sa nameFilters(), setFilter()
*/
void QDir::setNameFilters(const QStringList &nameFilters)
{
    QDirPrivate* d = d_ptr.data();
    d->clearFileLists();

    d->nameFilters = nameFilters;
}

/*!
    Returns the value set by setFilter()
*/
QDir::Filters QDir::filter() const
{
    const QDirPrivate* d = d_ptr.constData();
    return d->filters;
}

/*!
    \enum QDir::Filter

    This enum describes the filtering options available to QDir; e.g.
    for entryList() and entryInfoList(). The filter value is specified
    by combining values from the following list using the bitwise OR
    operator:

    \value Dirs    List directories that match the filters.
    \value AllDirs  List all directories; i.e. don't apply the filters
                    to directory names.
    \value Files   List files.
    \value Drives  List disk drives (ignored under Unix).
    \value NoSymLinks  Do not list symbolic links (ignored by operating
                       systems that don't support symbolic links).
    \value NoDotAndDotDot Do not list the special entries "." and "..".
    \value NoDot       Do not list the special entry ".".
    \value NoDotDot    Do not list the special entry "..".
    \value AllEntries  List directories, files, drives and symlinks (this does not list
                broken symlinks unless you specify System).
    \value Readable    List files for which the application has read
                       access. The Readable value needs to be combined
                       with Dirs or Files.
    \value Writable    List files for which the application has write
                       access. The Writable value needs to be combined
                       with Dirs or Files.
    \value Executable  List files for which the application has
                       execute access. The Executable value needs to be
                       combined with Dirs or Files.
    \value Modified  Only list files that have been modified (ignored
                     on Unix).
    \value Hidden  List hidden files (on Unix, files starting with a ".").
    \value System  List system files (on Unix, FIFOs, sockets and
                   device files are included; on Windows, \c {.lnk}
                   files are included)
    \value CaseSensitive  The filter should be case sensitive.

    \omitvalue DefaultFilter
    \omitvalue TypeMask
    \omitvalue All
    \omitvalue RWEMask
    \omitvalue AccessMask
    \omitvalue PermissionMask
    \omitvalue NoFilter

    Functions that use Filter enum values to filter lists of files
    and directories will include symbolic links to files and directories
    unless you set the NoSymLinks value.

    A default constructed QDir will not filter out files based on
    their permissions, so entryList() and entryInfoList() will return
    all files that are readable, writable, executable, or any
    combination of the three.  This makes the default easy to write,
    and at the same time useful.

    For example, setting the \c Readable, \c Writable, and \c Files
    flags allows all files to be listed for which the application has read
    access, write access or both. If the \c Dirs and \c Drives flags are
    also included in this combination then all drives, directories, all
    files that the application can read, write, or execute, and symlinks
    to such files/directories can be listed.

    To retrieve the permissons for a directory, use the
    entryInfoList() function to get the associated QFileInfo objects
    and then use the QFileInfo::permissons() to obtain the permissions
    and ownership for each file.
*/

/*!
    Sets the filter used by entryList() and entryInfoList() to \a
    filters. The filter is used to specify the kind of files that
    should be returned by entryList() and entryInfoList(). See
    \l{QDir::Filter}.

    \sa filter(), setNameFilters()
*/
void QDir::setFilter(Filters filters)
{
    QDirPrivate* d = d_ptr.data();
    d->clearFileLists();

    d->filters = filters;
}

/*!
    Returns the value set by setSorting()

    \sa setSorting() SortFlag
*/
QDir::SortFlags QDir::sorting() const
{
    const QDirPrivate* d = d_ptr.constData();
    return d->sort;
}

/*!
    \enum QDir::SortFlag

    This enum describes the sort options available to QDir, e.g. for
    entryList() and entryInfoList(). The sort value is specified by
    OR-ing together values from the following list:

    \value Name  Sort by name.
    \value Time  Sort by time (modification time).
    \value Size  Sort by file size.
    \value Type  Sort by file type (extension).
    \value Unsorted  Do not sort.
    \value NoSort Not sorted by default.

    \value DirsFirst  Put the directories first, then the files.
    \value DirsLast Put the files first, then the directories.
    \value Reversed  Reverse the sort order.
    \value IgnoreCase  Sort case-insensitively.
    \value LocaleAware Sort items appropriately using the current locale settings.

    \omitvalue SortByMask
    \omitvalue DefaultSort

    You can only specify one of the first four.

    If you specify both DirsFirst and Reversed, directories are
    still put first, but in reverse order; the files will be listed
    after the directories, again in reverse order.
*/

/*!
    Sets the sort order used by entryList() and entryInfoList().

    The \a sort is specified by OR-ing values from the enum
    \l{QDir::SortFlag}.

    \sa sorting() SortFlag
*/
void QDir::setSorting(SortFlags sort)
{
    QDirPrivate* d = d_ptr.data();
    d->clearFileLists();

    d->sort = sort;
}

/*!
    Returns the total number of directories and files in the directory.

    Equivalent to entryList().count().

    \sa operator[](), entryList()
*/
uint QDir::count() const
{
    const QDirPrivate* d = d_ptr.constData();
    d->initFileLists(*this);
    return d->files.count();
}

/*!
    Returns the file name at position \a pos in the list of file
    names. Equivalent to entryList().at(index).
    \a pos must be a valid index position in the list (i.e., 0 <= pos < count()).

    \sa count(), entryList()
*/
QString QDir::operator[](int pos) const
{
    const QDirPrivate* d = d_ptr.constData();
    d->initFileLists(*this);
    return d->files[pos];
}

/*!
    \overload

    Returns a list of the names of all the files and directories in
    the directory, ordered according to the name and attribute filters
    previously set with setNameFilters() and setFilter(), and sorted according
    to the flags set with setSorting().

    The attribute filter and sorting specifications can be overridden using the
    \a filters and \a sort arguments.

    Returns an empty list if the directory is unreadable, does not
    exist, or if nothing matches the specification.

    \note To list symlinks that point to non existing files, \l System must be
     passed to the filter.

    \sa entryInfoList(), setNameFilters(), setSorting(), setFilter()
*/
QStringList QDir::entryList(Filters filters, SortFlags sort) const
{
    const QDirPrivate* d = d_ptr.constData();
    return entryList(d->nameFilters, filters, sort);
}


/*!
    \overload

    Returns a list of QFileInfo objects for all the files and directories in
    the directory, ordered according to the name and attribute filters
    previously set with setNameFilters() and setFilter(), and sorted according
    to the flags set with setSorting().

    The attribute filter and sorting specifications can be overridden using the
    \a filters and \a sort arguments.

    Returns an empty list if the directory is unreadable, does not
    exist, or if nothing matches the specification.

    \sa entryList(), setNameFilters(), setSorting(), setFilter(), isReadable(), exists()
*/
QFileInfoList QDir::entryInfoList(Filters filters, SortFlags sort) const
{
    const QDirPrivate* d = d_ptr.constData();
    return entryInfoList(d->nameFilters, filters, sort);
}

/*!
    Returns a list of the names of all the files and
    directories in the directory, ordered according to the name
    and attribute filters previously set with setNameFilters()
    and setFilter(), and sorted according to the flags set with
    setSorting().

    The name filter, file attribute filter, and sorting specification
    can be overridden using the \a nameFilters, \a filters, and \a sort
    arguments.

    Returns an empty list if the directory is unreadable, does not
    exist, or if nothing matches the specification.

    \sa entryInfoList(), setNameFilters(), setSorting(), setFilter()
*/
QStringList QDir::entryList(const QStringList &nameFilters, Filters filters,
                            SortFlags sort) const
{
    const QDirPrivate* d = d_ptr.constData();

    if (filters == NoFilter)
        filters = d->filters;
    if (sort == NoSort)
        sort = d->sort;

    if (filters == d->filters && sort == d->sort && nameFilters == d->nameFilters) {
        d->initFileLists(*this);
        return d->files;
    }

    QFileInfoList l;
    QDirIterator it(d->dirEntry.filePath(), nameFilters, filters);
    while (it.hasNext()) {
        it.next();
        l.append(it.fileInfo());
    }
    QStringList ret;
    d->sortFileList(sort, l, &ret, 0);
    return ret;
}

/*!
    Returns a list of QFileInfo objects for all the files and
    directories in the directory, ordered according to the name
    and attribute filters previously set with setNameFilters()
    and setFilter(), and sorted according to the flags set with
    setSorting().

    The name filter, file attribute filter, and sorting specification
    can be overridden using the \a nameFilters, \a filters, and \a sort
    arguments.

    Returns an empty list if the directory is unreadable, does not
    exist, or if nothing matches the specification.

    \sa entryList(), setNameFilters(), setSorting(), setFilter(), isReadable(), exists()
*/
QFileInfoList QDir::entryInfoList(const QStringList &nameFilters, Filters filters,
                                  SortFlags sort) const
{
    const QDirPrivate* d = d_ptr.constData();

    if (filters == NoFilter)
        filters = d->filters;
    if (sort == NoSort)
        sort = d->sort;

    if (filters == d->filters && sort == d->sort && nameFilters == d->nameFilters) {
        d->initFileLists(*this);
        return d->fileInfos;
    }

    QFileInfoList l;
    QDirIterator it(d->dirEntry.filePath(), nameFilters, filters);
    while (it.hasNext()) {
        it.next();
        l.append(it.fileInfo());
    }
    QFileInfoList ret;
    d->sortFileList(sort, l, 0, &ret);
    return ret;
}

/*!
    Creates a sub-directory called \a dirName.

    Returns true on success; otherwise returns false.

    If the directory already exists when this function is called, it will return false.

    \sa rmdir()
*/
// ### Qt5: behaviour when directory already exists should be made consistent for mkdir and mkpath
bool QDir::mkdir(const QString &dirName) const
{
    if (Q_UNLIKELY(dirName.isEmpty())) {
        qWarning("QDir::mkdir: Empty or null file name(s)");
        return false;
    }

    QString fn = filePath(dirName);
    return QFileSystemEngine::createDirectory(QFileSystemEntry(fn), false);
}

/*!
    Removes the directory specified by \a dirName.

    The directory must be empty for rmdir() to succeed.

    Returns true if successful; otherwise returns false.

    \sa mkdir()
*/
bool QDir::rmdir(const QString &dirName) const
{
    if (Q_UNLIKELY(dirName.isEmpty())) {
        qWarning("QDir::rmdir: Empty or null file name(s)");
        return false;
    }

    QString fn = filePath(dirName);
    return QFileSystemEngine::removeDirectory(QFileSystemEntry(fn), false);
}

/*!
    Creates the directory path \a dirPath.

    The function will create all parent directories necessary to
    create the directory.

    Returns true if successful; otherwise returns false.

    If the path already exists when this function is called, it will return true.

    \sa rmpath()
*/
// ### Qt5: behaviour when directory already exists should be made consistent for mkdir and mkpath
bool QDir::mkpath(const QString &dirPath) const
{
    if (Q_UNLIKELY(dirPath.isEmpty())) {
        qWarning("QDir::mkpath: Empty or null file name(s)");
        return false;
    }

    QString fn = filePath(dirPath);
    return QFileSystemEngine::createDirectory(QFileSystemEntry(fn), true);
}

/*!
    Removes the directory path \a dirPath.

    The function will remove all parent directories in \a dirPath,
    provided that they are empty. This is the opposite of
    mkpath(dirPath).

    Returns true if successful; otherwise returns false.

    \sa mkpath()
*/
bool QDir::rmpath(const QString &dirPath) const
{
    if (Q_UNLIKELY(dirPath.isEmpty())) {
        qWarning("QDir::rmpath: Empty or null file name(s)");
        return false;
    }

    QString fn = filePath(dirPath);
    return QFileSystemEngine::removeDirectory(QFileSystemEntry(fn), true);
}

/*!
    Returns true if the directory is readable \e and we can open files
    by name; otherwise returns false.

    \warning A false value from this function is not a guarantee that
    files in the directory are not accessible.

    \sa QFileInfo::isReadable()
*/
bool QDir::isReadable() const
{
    const QDirPrivate* d = d_ptr.constData();

    if (!d->metaData.hasFlags(QFileSystemMetaData::UserReadPermission))
        QFileSystemEngine::fillMetaData(d->dirEntry, d->metaData, QFileSystemMetaData::UserReadPermission);

    return (d->metaData.permissions() & QFile::ReadUser) != 0;
}

/*!
    \overload

    Returns true if the directory exists; otherwise returns false.
    (If a file with the same name is found this function will return false).

    The overload of this function that accepts an argument is used to test
    for the presence of files and directories within a directory.

    \sa QFileInfo::exists(), QFile::exists()
*/
bool QDir::exists() const
{
    return d_ptr->exists();
}

/*!
    Returns true if the directory is the root directory; otherwise
    returns false.

    Note: If the directory is a symbolic link to the root directory
    this function returns false. If you want to test for this use
    canonicalPath(), e.g.

    \snippet doc/src/snippets/code/src_corelib_io_qdir.cpp 9

    \sa root(), rootPath()
*/
bool QDir::isRoot() const
{
    return d_ptr->dirEntry.isRoot();
}

/*!
    \fn bool QDir::isAbsolute() const

    Returns true if the directory's path is absolute; otherwise
    returns false. See isAbsolutePath().

    \sa isRelative() makeAbsolute() cleanPath()
*/

/*!
   \fn bool QDir::isAbsolutePath(const QString &)

    Returns true if \a path is absolute; returns false if it is
    relative.

    \sa isAbsolute() isRelativePath() makeAbsolute() cleanPath()
*/

/*!
    Returns true if the directory path is relative; otherwise returns
    false. (Under Unix a path is relative if it does not start with a
    "/").

    \sa makeAbsolute() isAbsolute() isAbsolutePath() cleanPath()
*/
bool QDir::isRelative() const
{
    return d_ptr->dirEntry.isRelative();
}

/*!
    Converts the directory path to an absolute path. If it is already
    absolute nothing happens. Returns true if the conversion
    succeeded; otherwise returns false.

    \sa isAbsolute() isAbsolutePath() isRelative() cleanPath()
*/
bool QDir::makeAbsolute()
{
    const QDirPrivate *d = d_ptr.constData();
    QScopedPointer<QDirPrivate> dir;
    d->resolveAbsoluteEntry();
    dir.reset(new QDirPrivate(*d_ptr.constData()));
    dir->setPath(d->absoluteDirEntry.filePath());
    d_ptr = dir.take(); // actually detach
    return true;
}

/*!
    Returns true if directory \a dir and this directory have the same
    path and their sort and filter settings are the same; otherwise
    returns false.

    Example:

    \snippet doc/src/snippets/code/src_corelib_io_qdir.cpp 10
*/
bool QDir::operator==(const QDir &dir) const
{
    const QDirPrivate *d = d_ptr.constData();
    const QDirPrivate *other = dir.d_ptr.constData();

    if (d == other)
        return true;

    if (d->filters == other->filters
       && d->sort == other->sort
       && d->nameFilters == other->nameFilters) {

        // Assume directories are the same if path is the same
        if (d->dirEntry.filePath() == other->dirEntry.filePath())
            return true;

        if (exists() && dir.exists()) {
            // Both exist, fallback to expensive canonical path computation
            return canonicalPath().compare(dir.canonicalPath(), Qt::CaseSensitive) == 0;
        } else {
            // Neither exists, compare absolute paths rather than canonical (which would be empty strings)
            d->resolveAbsoluteEntry();
            other->resolveAbsoluteEntry();
            return d->absoluteDirEntry.filePath().compare(other->absoluteDirEntry.filePath(), Qt::CaseSensitive) == 0;
        }
    }
    return false;
}

/*!
    Makes a copy of the \a dir object and assigns it to this QDir
    object.
*/
QDir &QDir::operator=(const QDir &dir)
{
    d_ptr = dir.d_ptr;
    return *this;
}

/*!
    \overload
    \obsolete

    Sets the directory path to the given \a path.

    Use setPath() instead.
*/
QDir &QDir::operator=(const QString &path)
{
    d_ptr->setPath(path);
    return *this;
}

/*!
    \fn bool QDir::operator!=(const QDir &dir) const

    Returns true if directory \a dir and this directory have different
    paths or different sort or filter settings; otherwise returns
    false.

    Example:

    \snippet doc/src/snippets/code/src_corelib_io_qdir.cpp 11
*/

/*!
    Removes the file, \a fileName.

    Returns true if the file is removed successfully; otherwise
    returns false.
*/
bool QDir::remove(const QString &fileName)
{
    if (Q_UNLIKELY(fileName.isEmpty())) {
        qWarning("QDir::remove: Empty or null file name");
        return false;
    }
    return QFile::remove(filePath(fileName));
}

/*!
    Renames a file or directory from \a oldName to \a newName, and returns
    true if successful; otherwise returns false.

    On most file systems, rename() fails only if \a oldName does not
    exist, if \a newName and \a oldName are not on the same
    partition or if a file with the new name already exists.
    However, there are also other reasons why rename() can
    fail. For example, on at least one file system rename() fails if
    \a newName points to an open file.
*/
bool QDir::rename(const QString &oldName, const QString &newName)
{
    if (Q_UNLIKELY(oldName.isEmpty() || newName.isEmpty())) {
        qWarning("QDir::rename: Empty or null file name(s)");
        return false;
    }

    return QFile::rename(filePath(oldName), filePath(newName));
}

/*!
    Returns true if the file called \a name exists; otherwise returns
    false.

    Unless \a name contains an absolute file path, the file name is assumed
    to be relative to the directory itself, so this function is typically used
    to check for the presence of files within a directory.

    \sa QFileInfo::exists(), QFile::exists()
*/
bool QDir::exists(const QString &name) const
{
    if (Q_UNLIKELY(name.isEmpty())) {
        qWarning("QDir::exists: Empty or null file name");
        return false;
    }
    QFileInfo info(filePath(name));
    return info.exists();
}

/*!
    \fn QChar QDir::separator()
    Returns the native directory separator: "/" under Unix.
*/

/*!
    Sets the application's current working directory to \a path.
    Returns true if the directory was successfully changed; otherwise
    returns false.

    \sa current(), currentPath(), home(), root(), temp()
*/
bool QDir::setCurrent(const QString &path)
{
    return QFileSystemEngine::setCurrentPath(QFileSystemEntry(path));
}

/*!
    \fn QDir QDir::current()

    Returns the application's current directory.

    The directory is constructed using the absolute path of the current directory,
    ensuring that its path() will be the same as its absolutePath().

    \sa currentPath(), setCurrent(), home(), root(), temp()
*/

/*!
    Returns the absolute path of the application's current directory.

    \sa current(), setCurrent(), homePath(), rootPath(), tempPath()
*/
QString QDir::currentPath()
{
    return QFileSystemEngine::currentPath().filePath();
}

/*!
  \fn QString QDir::currentDirPath()
    Returns the absolute path of the application's current directory.

    Use currentPath() instead.

    \sa currentPath(), setCurrent()
*/

/*!
    \fn QDir QDir::home()

    Returns the user's home directory.

    The directory is constructed using the absolute path of the home directory,
    ensuring that its path() will be the same as its absolutePath().

    See homePath() for details.

    \sa current(), root(), temp()
*/

/*!
    Returns the absolute path of the user's home directory.

    For Unix operating systems the \c HOME environment variable is used
    if it exists, otherwise the path returned by the rootPath().

    \sa home(), currentPath(), rootPath(), tempPath()
*/
QString QDir::homePath()
{
    return QFileSystemEngine::homePath();
}

/*!
    \fn QString QDir::homeDirPath()

    Returns the absolute path of the user's home directory.

    Use homePath() instead.

    \sa homePath()
*/

/*!
    \fn QDir QDir::temp()

    Returns the system's temporary directory.

    The directory is constructed using the absolute path of the temporary directory,
    ensuring that its path() will be the same as its absolutePath().

    See tempPath() for details.

    \sa current(), home(), root()
*/

/*!
    Returns the absolute path of the system's temporary directory.

    On Unix/Linux systems this is the path in the \c TMPDIR environment
    variable or \c{/tmp} if \c TMPDIR is not defined. On Windows this is
    usually the path in the \c TEMP or \c TMP environment
    variable. Whether a directory separator is added to the end or
    not, depends on the operating system.

    \sa temp(), currentPath(), homePath(), rootPath()
*/
QString QDir::tempPath()
{
    return QFileSystemEngine::tempPath();
}

/*!
    \fn QDir QDir::root()

    Returns the root directory.

    The directory is constructed using the absolute path of the root directory,
    ensuring that its path() will be the same as its absolutePath().

    See rootPath() for details.

    \sa current(), home(), temp()
*/

/*!
    Returns the absolute path of the root directory.

    For Unix operating systems this returns "/".

    \sa root(), currentPath(), homePath(), tempPath()
*/
QString QDir::rootPath()
{
    return QFileSystemEngine::rootPath();
}

/*!
    \fn QString QDir::rootDirPath()

    Returns the absolute path of the root directory.

    Use rootPath() instead.

    \sa rootPath()
*/

#ifndef QT_NO_REGEXP
/*!
    \overload

    Returns true if the \a fileName matches any of the wildcard (glob)
    patterns in the list of \a filters; otherwise returns false. The
    matching is case insensitive.

    \sa {QRegExp wildcard matching}, QRegExp::exactMatch() entryList() entryInfoList()
*/
bool QDir::match(const QStringList &filters, const QString &fileName)
{
    for (QStringList::ConstIterator sit = filters.constBegin(); sit != filters.constEnd(); ++sit) {
        QRegExp rx(*sit, Qt::CaseInsensitive, QRegExp::Wildcard);
        if (rx.exactMatch(fileName))
            return true;
    }
    return false;
}

/*!
    Returns true if the \a fileName matches the wildcard (glob)
    pattern \a filter; otherwise returns false. The \a filter may
    contain multiple patterns separated by spaces or semicolons.
    The matching is case insensitive.

    \sa {QRegExp wildcard matching}, QRegExp::exactMatch() entryList() entryInfoList()
*/
bool QDir::match(const QString &filter, const QString &fileName)
{
    return match(nameFiltersFromString(filter), fileName);
}
#endif // QT_NO_REGEXP

/*!
    Removes all multiple directory separators "/" and resolves any
    "."s or ".."s found in the path, \a path.

    Symbolic links are kept. This function does not return the
    canonical path, but rather the simplest version of the input.
    For example, "./local" becomes "local", "local/../bin" becomes
    "bin" and "/local/usr/../bin" becomes "/local/bin".

    \sa absolutePath() canonicalPath()
*/
QString QDir::cleanPath(const QString &path)
{
    if (path.isEmpty())
        return path;

    int used = 0, levels = 0;
    const int len = path.length();
    QStdVector<QChar> out(len);

    const QChar *p = path.unicode();
    for (int i = 0, last = -1, iwrite = 0; i < len; ++i) {
        if (p[i] == QLatin1Char('/')) {
            while (i+1 < len && p[i+1] == QLatin1Char('/')) {
                i++;
            }
            bool eaten = false;
            if (i+1 < len && p[i+1] == QLatin1Char('.')) {
                int dotcount = 1;
                if (i+2 < len && p[i+2] == QLatin1Char('.'))
                    dotcount++;
                if (i == len - dotcount - 1) {
                    if (dotcount == 1) {
                        break;
                    } else if (levels) {
                        if (last == -1) {
                            for (int i2 = iwrite-1; i2 >= 0; i2--) {
                                if (out[i2] == QLatin1Char('/')) {
                                    last = i2;
                                    break;
                                }
                            }
                        }
                        used -= iwrite - last - 1;
                        break;
                    }
                } else if (p[i+dotcount+1] == QLatin1Char('/')) {
                    if (dotcount == 2 && levels) {
                        if (last == -1 || iwrite - last == 1) {
                            for (int i2 = (last == -1) ? (iwrite-1) : (last-1); i2 >= 0; i2--) {
                                if (out[i2] == QLatin1Char('/')) {
                                    eaten = true;
                                    last = i2;
                                    break;
                                }
                            }
                        } else {
                            eaten = true;
                        }
                        if (eaten) {
                            levels--;
                            used -= iwrite - last;
                            iwrite = last;
                            last = -1;
                        }
                    } else if (dotcount == 2 && i > 0 && p[i - 1] != QLatin1Char('.')) {
                        eaten = true;
                        used -= iwrite - qMax(0, last);
                        iwrite = qMax(0, last);
                        last = -1;
                        ++i;
                    } else if (dotcount == 1) {
                        eaten = true;
                    }
                    if (eaten)
                        i += dotcount;
                } else {
                    levels++;
                }
            } else if (last != -1 && iwrite - last == 1) {
                eaten = true;
                last = -1;
            } else if (last != -1 && i == len-1) {
                eaten = true;
            } else {
                levels++;
            }
            if (!eaten)
                last = i - (i - iwrite);
            else
                continue;
        } else if (!i && p[i] == QLatin1Char('.')) {
            int dotcount = 1;
            if (len >= 1 && p[1] == QLatin1Char('.'))
                dotcount++;
            if (len >= dotcount && p[dotcount] == QLatin1Char('/')) {
                if (dotcount == 1) {
                    i++;
                    while (i+1 < len-1 && p[i+1] == QLatin1Char('/'))
                        i++;
                    continue;
                }
            }
        }
        out[iwrite++] = p[i];
        used++;
    }

    QString ret = (used == len ? path : QString(out.constData(), used));
    // Strip away last slash except for root directories
    if (ret.length() > 1 && ret.endsWith(QLatin1Char('/'))) {
        ret.chop(1);
    }

    return ret;
}

/*!
    Returns true if \a path is relative; returns false if it is
    absolute.

    \sa isRelative() isAbsolutePath() makeAbsolute()
*/
bool QDir::isRelativePath(const QString &path)
{
    return QFileInfo(path).isRelative();
}

/*!
    Refreshes the directory information.
*/
void QDir::refresh()
{
    QDirPrivate *d = d_ptr.data();
    d->metaData.clear();
    d->clearFileLists();
}

/*!
    \internal

    Returns a list of name filters from the given \a nameFilter. (If
    there is more than one filter, each pair of filters is separated
    by a space or by a semicolon.)
*/
QStringList QDir::nameFiltersFromString(const QString &nameFilter)
{
    QChar sep(QLatin1Char(';'));
    int i = nameFilter.indexOf(sep, 0);
    if (i == -1 && nameFilter.indexOf(QLatin1Char(' '), 0) != -1)
        sep = QChar(QLatin1Char(' '));
    QStringList ret = nameFilter.split(sep);
    for (int i = 0; i < ret.count(); ++i)
        ret[i] = ret[i].trimmed();
    return ret;
}

#ifndef QT_NO_DEBUG_STREAM
QDebug operator<<(QDebug debug, QDir::Filters filters)
{
    QStringList flags;
    if (filters == QDir::NoFilter) {
        flags << QLatin1String("NoFilter");
    } else {
        // QDir::NoDotAndDotDot not handled because it is NoDotAndDotDot=NoDot|NoDotDot
        if (filters & QDir::Dirs) flags << QLatin1String("Dirs");
        if (filters & QDir::AllDirs) flags << QLatin1String("AllDirs");
        if (filters & QDir::Files) flags << QLatin1String("Files");
        if (filters & QDir::Drives) flags << QLatin1String("Drives");
        if (filters & QDir::NoSymLinks) flags << QLatin1String("NoSymLinks");
        if (filters & QDir::NoDot) flags << QLatin1String("NoDot");
        if (filters & QDir::NoDotDot) flags << QLatin1String("NoDotDot");
        if ((filters & QDir::AllEntries) == QDir::AllEntries) flags << QLatin1String("AllEntries");
        if (filters & QDir::Readable) flags << QLatin1String("Readable");
        if (filters & QDir::Writable) flags << QLatin1String("Writable");
        if (filters & QDir::Executable) flags << QLatin1String("Executable");
        if (filters & QDir::Modified) flags << QLatin1String("Modified");
        if (filters & QDir::Hidden) flags << QLatin1String("Hidden");
        if (filters & QDir::System) flags << QLatin1String("System");
        if (filters & QDir::CaseSensitive) flags << QLatin1String("CaseSensitive");
    }
    debug << "QDir::Filters(" << qPrintable(flags.join(QLatin1String("|"))) << ')';
    return debug;
}

QDebug operator<<(QDebug debug, QDir::SortFlags sorting)
{
    if (sorting == QDir::NoSort) {
        debug << "QDir::SortFlags(NoSort)";
    } else {
        QString type;
        if ((sorting & QDir::SortByMask) == QDir::Name) type = QLatin1String("Name");
        if ((sorting & QDir::SortByMask) == QDir::Time) type = QLatin1String("Time");
        if ((sorting & QDir::SortByMask) == QDir::Size) type = QLatin1String("Size");
        if ((sorting & QDir::SortByMask) == QDir::Unsorted) type = QLatin1String("Unsorted");

        QStringList flags;
        if (sorting & QDir::DirsFirst) flags << QLatin1String("DirsFirst");
        if (sorting & QDir::DirsLast) flags << QLatin1String("DirsLast");
        if (sorting & QDir::IgnoreCase) flags << QLatin1String("IgnoreCase");
        if (sorting & QDir::LocaleAware) flags << QLatin1String("LocaleAware");
        if (sorting & QDir::Type) flags << QLatin1String("Type");
        debug << "QDir::SortFlags(" << qPrintable(type);
        if (!flags.isEmpty())
            debug << '|' << qPrintable(flags.join(QLatin1String("|")));
        debug << ')';
    }
    return debug;
}

QDebug operator<<(QDebug debug, const QDir &dir)
{
    debug.maybeSpace() << "QDir(" << dir.path()
                       << ", nameFilters = {"
                       << qPrintable(dir.nameFilters().join(QLatin1String(",")))
                       << "}, "
                       << dir.sorting()
                       << ','
                       << dir.filter()
                       << ')';
    return debug.space();
}
#endif // QT_NO_DEBUG_STREAM

QT_END_NAMESPACE
