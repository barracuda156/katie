/****************************************************************************
**
** Copyright (C) 2015 The Qt Company Ltd.
** Copyright (C) 2016 Ivailo Monev
**
** This file is part of the test suite of the Katie Toolkit.
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


#include <QtTest/QtTest>

#include <qcoreapplication.h>
#include <qdebug.h>
#include <qdir.h>
#include <qfileinfo.h>
#include <qregexp.h>
#include <qstringlist.h>

#include "../../shared/filesystem.h"

#include <unistd.h>
#include <sys/stat.h>

// #define Q_NO_SYMLINKS
// Q_NO_SYMLINKS_TO_DIRS

//TESTED_CLASS=
//TESTED_FILES=

class tst_QDir : public QObject
{
Q_OBJECT

public:
    tst_QDir();
    virtual ~tst_QDir();

private slots:
    void getSetCheck();
    void construction();

    void setPath_data();
    void setPath();

    void entryList_data();
    void entryList();

    void entryListSimple_data();
    void entryListSimple();

    void entryListWithSymLinks();

    void mkdir_data();
    void mkdir();

    void makedirReturnCode();

    void rmdir_data();
    void rmdir();

    void exists_data();
    void exists();

    void isRelativePath_data();
    void isRelativePath();

    void canonicalPath_data();
    void canonicalPath();

    void current_data();
    void current();

    void cd_data();
    void cd();

    void setNameFilters_data();
    void setNameFilters();

    void cleanPath_data();
    void cleanPath();

    void compare();
    void QDir_default();

    void filePath_data();
    void filePath();

    void absoluteFilePath_data();
    void absoluteFilePath();

    void absolutePath_data();
    void absolutePath();

    void relativeFilePath_data();
    void relativeFilePath();

    void remove();
    void rename();

    void exists2_data();
    void exists2();

    void dirName_data();
    void dirName();

    void operator_eq();

    void dotAndDotDot();
    void homePath();
    void tempPath();
    void rootPath();

    void longFileName_data();
    void longFileName();

    void updateFileLists();

    void detachingOperations();

    void testCaching();

    void isRoot_data();
    void isRoot();

#ifndef QT_NO_REGEXP
    void match_data();
    void match();
#endif

    void arrayOperator();

    void equalityOperator_data();
    void equalityOperator();

    void isRelative_data();
    void isRelative();

    void isReadable();
};

// Testing get/set functions
void tst_QDir::getSetCheck()
{
    QDir obj1;
    // Filters QDir::filter()
    // void QDir::setFilter(Filters)
    obj1.setFilter(QDir::Filters(QDir::Dirs));
    QCOMPARE(QDir::Filters(QDir::Dirs), obj1.filter());
    obj1.setFilter(QDir::Filters(QDir::Dirs | QDir::Files));
    QCOMPARE(QDir::Filters(QDir::Dirs | QDir::Files), obj1.filter());
    obj1.setFilter(QDir::Filters(QDir::NoFilter));
    QCOMPARE(QDir::Filters(QDir::NoFilter), obj1.filter());

    // SortFlags QDir::sorting()
    // void QDir::setSorting(SortFlags)
    obj1.setSorting(QDir::SortFlags(QDir::Name));
    QCOMPARE(QDir::SortFlags(QDir::Name), obj1.sorting());
    obj1.setSorting(QDir::SortFlags(QDir::Name | QDir::IgnoreCase));
    QCOMPARE(QDir::SortFlags(QDir::Name | QDir::IgnoreCase), obj1.sorting());
    obj1.setSorting(QDir::SortFlags(QDir::NoSort));
    QCOMPARE(QDir::SortFlags(QDir::NoSort), obj1.sorting());
}

tst_QDir::tst_QDir()
{
}

tst_QDir::~tst_QDir()
{
}

void tst_QDir::construction()
{
    QFileInfo myFileInfo("/machine/share/dir1/file1");
    QDir myDir(myFileInfo.absoluteDir()); // this asserted
    QCOMPARE(myFileInfo.absoluteDir().absolutePath(), myDir.absolutePath());
}

void tst_QDir::setPath_data()
{
    QTest::addColumn<QString>("dir1");
    QTest::addColumn<QString>("dir2");

    QTest::newRow("data0") << QString(".") << QString("..");
}

void tst_QDir::setPath()
{
    QFETCH(QString, dir1);
    QFETCH(QString, dir2);

    QDir shared;
    QDir qDir1(dir1);
    QStringList entries1 = qDir1.entryList();
    shared.setPath(dir1);
    QCOMPARE(shared.entryList(), entries1);

    QDir qDir2(dir2);
    QStringList entries2 = qDir2.entryList();
    shared.setPath(dir2);
    QCOMPARE(shared.entryList(), entries2);
}

void tst_QDir::mkdir_data()
{
    QTest::addColumn<QString>("path");
    QTest::addColumn<bool>("recurse");

    QStringList dirs;
    dirs << QDir::currentPath() + "/testdir/one/two/three"
         << QDir::currentPath() + "/testdir/two"
         << QDir::currentPath() + "/testdir/two/three";
    QTest::newRow("data0") << dirs.at(0) << true;
    QTest::newRow("data1") << dirs.at(1) << false;
    QTest::newRow("data2") << dirs.at(2) << false;

    // Ensure that none of these directories already exist
    QDir dir;
    for (int i = 0; i < dirs.count(); ++i)
        dir.rmpath(dirs.at(i));
}

void tst_QDir::mkdir()
{
    QFETCH(QString, path);
    QFETCH(bool, recurse);

    QDir dir;
    dir.rmdir(path);
    if (recurse)
        QVERIFY(dir.mkpath(path));
    else
        QVERIFY(dir.mkdir(path));

    //make sure it really exists (ie that mkdir returns the right value)
    QFileInfo fi(path);
    QVERIFY(fi.exists() && fi.isDir());
}

void tst_QDir::makedirReturnCode()
{
    QString dirName = QString::fromLatin1("makedirReturnCode");
    QDir::current().rmdir(dirName); // cleanup a previous run.
    QDir dir(dirName);
    QVERIFY(!dir.exists());
    QVERIFY(QDir::current().mkdir(dirName));
    QVERIFY(!QDir::current().mkdir(dirName)); // calling mkdir on an existing dir will fail.
    QVERIFY(QDir::current().mkpath(dirName)); // calling mkpath on an existing dir will pass
}

void tst_QDir::rmdir_data()
{
    QTest::addColumn<QString>("path");
    QTest::addColumn<bool>("recurse");

    QTest::newRow("data0") << QDir::currentPath() + "/testdir/one/two/three" << true;
    QTest::newRow("data1") << QDir::currentPath() + "/testdir/two/three" << false;
    QTest::newRow("data2") << QDir::currentPath() + "/testdir/two" << false;
}

void tst_QDir::rmdir()
{
    QFETCH(QString, path);
    QFETCH(bool, recurse);

    QDir dir;
    if (recurse)
        QVERIFY(dir.rmpath(path));
    else
        QVERIFY(dir.rmdir(path));

    //make sure it really doesn't exist (ie that rmdir returns the right value)
    QFileInfo fi(path);
    QVERIFY(!fi.exists());
}

void tst_QDir::exists_data()
{
    QTest::addColumn<QString>("path");
    QTest::addColumn<bool>("expected");

    QTest::newRow("data0") << QDir::currentPath() << true;
    QTest::newRow("data0.1") << QDir::currentPath() + "/" << true;
    QTest::newRow("data1") << QString("/I/Do_not_expect_this_path_to_exist/") << false;

    QTest::newRow("simple dir") << SRCDIR "testdir" << true;
    QTest::newRow("simple dir with slash") << SRCDIR "testdir/" << true;
}

void tst_QDir::exists()
{
    QFETCH(QString, path);
    QFETCH(bool, expected);

    QDir dir(path);
    QCOMPARE(dir.exists(), expected);
}

void tst_QDir::isRelativePath_data()
{
    QTest::addColumn<QString>("path");
    QTest::addColumn<bool>("relative");

    QTest::newRow("data0") << "../somedir" << true;
    QTest::newRow("data2") << "somedir" << true;
    QTest::newRow("data3") << "/somedir" << false;
}

void tst_QDir::isRelativePath()
{
    QFETCH(QString, path);
    QFETCH(bool, relative);

    QCOMPARE(QDir::isRelativePath(path),relative);
}


void tst_QDir::QDir_default()
{
    //default constructor QDir();
    QDir dir; // according to documentation should be currentDirPath
    QCOMPARE(dir.absolutePath(), QDir::currentPath());
}

void tst_QDir::compare()
{
    // operator==

    // Not using QCOMPARE to test result of QDir::operator==

    QDir dir;
    dir.makeAbsolute();
    QVERIFY(dir == QDir::currentPath());

    QVERIFY(QDir() == QDir(QDir::currentPath()));
    QVERIFY(QDir("../") == QDir(QDir::currentPath() + "/.."));
}

static QStringList filterLinks(const QStringList &list)
{
#ifndef Q_NO_SYMLINKS
    return list;
#else
    QStringList result;
    foreach (QString str, list) {
        if (!str.endsWith(QLatin1String(".lnk")))
            result.append(str);
    }
    return result;
#endif
}

void tst_QDir::entryList_data()
{
    QTest::addColumn<QString>("dirName"); // relative from current path or abs
    QTest::addColumn<QStringList>("nameFilters");
    QTest::addColumn<int>("filterspec");
    QTest::addColumn<int>("sortspec");
    QTest::addColumn<QStringList>("expected");
    QTest::newRow("spaces1") << SRCDIR "testdir/spaces" << QStringList("*. bar")
              << (int)(QDir::NoFilter) << (int)(QDir::NoSort)
              << QStringList("foo. bar"); // notice how spaces5 works
    QTest::newRow("spaces2") << SRCDIR "testdir/spaces" << QStringList("*.bar")
              << (int)(QDir::NoFilter) << (int)(QDir::NoSort)
              << QStringList("foo.bar");
    QTest::newRow("spaces3") << SRCDIR "testdir/spaces" << QStringList("foo.*")
              << (int)(QDir::NoFilter) << (int)(QDir::NoSort)
              << QString("foo. bar,foo.bar").split(',');
    QTest::newRow("files1")  << SRCDIR "testdir/dir" << QString("*r.cpp *.pro").split(" ")
              << (int)(QDir::NoFilter) << (int)(QDir::NoSort)
              << QString("qdir.pro,qrc_qdir.cpp,tst_qdir.cpp").split(',');
    QTest::newRow("testdir1")  << SRCDIR "testdir" << QStringList()
              << (int)(QDir::AllDirs) << (int)(QDir::NoSort)
              << QString(".,..,dir,spaces").split(',');
// #### this test uses filenames that cannot be represented on all filesystems we test, in
// particular HFS+ on the Mac. When checking out the files with perforce it silently ignores the
// error that it cannot represent the file names stored in the repository and the test fails. That
// is why the test is marked as 'skip' for the mac. When checking out the files with git on the mac
// the error of not being able to represent the files stored in the repository is not silently
// ignored but git reports an error. The test only tried to prevent QDir from _hanging_ when listing
// the directory.
//    QTest::newRow("unprintablenames")  << SRCDIR "unprintablenames" << QStringList("*")
//              << (int)(QDir::NoFilter) << (int)(QDir::NoSort)
//              << QString(".,..").split(",");
    QTest::newRow("nofilter") << SRCDIR "entrylist/" << QStringList("*")
                              << int(QDir::NoFilter) << int(QDir::Name)
                              << filterLinks(QString(".,..,directory,file,linktodirectory.lnk,linktofile.lnk,writable").split(','));
    QTest::newRow("QDir::AllEntries") << SRCDIR "entrylist/" << QStringList("*")
                              << int(QDir::AllEntries) << int(QDir::Name)
                              << filterLinks(QString(".,..,directory,file,linktodirectory.lnk,linktofile.lnk,writable").split(','));
    QTest::newRow("QDir::Files") << SRCDIR "entrylist/" << QStringList("*")
                                 << int(QDir::Files) << int(QDir::Name)
                                 << filterLinks(QString("file,linktofile.lnk,writable").split(','));
    QTest::newRow("QDir::Dirs") << SRCDIR "entrylist/" << QStringList("*")
                                << int(QDir::Dirs) << int(QDir::Name)
                                << filterLinks(QString(".,..,directory,linktodirectory.lnk").split(','));
    QTest::newRow("QDir::Dirs | QDir::NoDotAndDotDot") << SRCDIR "entrylist/" << QStringList("*")
                                                       << int(QDir::Dirs | QDir::NoDotAndDotDot) << int(QDir::Name)
                                << filterLinks(QString("directory,linktodirectory.lnk").split(','));
    QTest::newRow("QDir::AllDirs") << SRCDIR "entrylist/" << QStringList("*")
                                   << int(QDir::AllDirs) << int(QDir::Name)
                                   << filterLinks(QString(".,..,directory,linktodirectory.lnk").split(','));
    QTest::newRow("QDir::AllDirs | QDir::Dirs") << SRCDIR "entrylist/" << QStringList("*")
                                                << int(QDir::AllDirs | QDir::Dirs) << int(QDir::Name)
                                                << filterLinks(QString(".,..,directory,linktodirectory.lnk").split(','));
    QTest::newRow("QDir::AllDirs | QDir::Files") << SRCDIR "entrylist/" << QStringList("*")
                                                 << int(QDir::AllDirs | QDir::Files) << int(QDir::Name)
                                                 << filterLinks(QString(".,..,directory,file,linktodirectory.lnk,linktofile.lnk,writable").split(','));
    QTest::newRow("QDir::AllEntries | QDir::NoSymLinks") << SRCDIR "entrylist/" << QStringList("*")
                                      << int(QDir::AllEntries | QDir::NoSymLinks) << int(QDir::Name)
                                      << filterLinks(QString(".,..,directory,file,writable").split(','));
    QTest::newRow("QDir::AllEntries | QDir::NoSymLinks | QDir::NoDotAndDotDot") << SRCDIR "entrylist/" << QStringList("*")
                                      << int(QDir::AllEntries | QDir::NoSymLinks | QDir::NoDotAndDotDot) << int(QDir::Name)
                                      << filterLinks(QString("directory,file,writable").split(','));
    QTest::newRow("QDir::Files | QDir::NoSymLinks") << SRCDIR "entrylist/" << QStringList("*")
                                                    << int(QDir::Files | QDir::NoSymLinks) << int(QDir::Name)
                                                    << filterLinks(QString("file,writable").split(','));
    QTest::newRow("QDir::Dirs | QDir::NoSymLinks") << SRCDIR "entrylist/" << QStringList("*")
                                                   << int(QDir::Dirs | QDir::NoSymLinks) << int(QDir::Name)
                                                   << filterLinks(QString(".,..,directory").split(','));
    QTest::newRow("QDir::Drives | QDir::Files | QDir::NoDotAndDotDot") << SRCDIR "entrylist/" << QStringList("*")
                                                   << int(QDir::Drives | QDir::Files | QDir::NoDotAndDotDot) << int(QDir::Name)
                                                   << filterLinks(QString("file,linktofile.lnk,writable").split(','));
    QTest::newRow("QDir::System") << SRCDIR "entrylist/" << QStringList("*")
                                  << int(QDir::System) << int(QDir::Name)
                                  << filterLinks(QStringList("brokenlink.lnk"));
    QTest::newRow("QDir::Hidden") << SRCDIR "entrylist/" << QStringList("*")
                                  << int(QDir::Hidden) << int(QDir::Name)
                                  << QStringList();
    QTest::newRow("QDir::System | QDir::Hidden") << SRCDIR "entrylist/" << QStringList("*")
                                  << int(QDir::System | QDir::Hidden) << int(QDir::Name)
                                  << filterLinks(QStringList("brokenlink.lnk"));
    QTest::newRow("QDir::AllDirs | QDir::NoSymLinks") << SRCDIR "entrylist/" << QStringList("*")
                                                      << int(QDir::AllDirs | QDir::NoSymLinks) << int(QDir::Name)
                                                      << filterLinks(QString(".,..,directory").split(','));
    QTest::newRow("QDir::AllEntries | QDir::Hidden | QDir::System") << SRCDIR "entrylist/" << QStringList("*")
                                  << int(QDir::AllEntries | QDir::Hidden | QDir::System) << int(QDir::Name)
                                  << filterLinks(QString(".,..,brokenlink.lnk,directory,file,linktodirectory.lnk,linktofile.lnk,writable").split(','));
    QTest::newRow("QDir::AllEntries | QDir::Readable") << SRCDIR "entrylist/" << QStringList("*")
                                  << int(QDir::AllEntries | QDir::Readable) << int(QDir::Name)
                                                << filterLinks(QString(".,..,directory,file,linktodirectory.lnk,linktofile.lnk,writable").split(','));
    QTest::newRow("QDir::AllEntries | QDir::Writable") << SRCDIR "entrylist/" << QStringList("*")
                                  << int(QDir::AllEntries | QDir::Writable) << int(QDir::Name)
                                  << filterLinks(QString(".,..,directory,linktodirectory.lnk,writable").split(','));
    QTest::newRow("QDir::Files | QDir::Readable") << SRCDIR "entrylist/" << QStringList("*")
                                  << int(QDir::Files | QDir::Readable) << int(QDir::Name)
                                  << filterLinks(QString("file,linktofile.lnk,writable").split(','));
    QTest::newRow("QDir::Dirs | QDir::Readable") << SRCDIR "entrylist/" << QStringList("*")
                                  << int(QDir::Dirs | QDir::Readable) << int(QDir::Name)
                                  << filterLinks(QString(".,..,directory,linktodirectory.lnk").split(','));
    QTest::newRow("Namefilters b*") << SRCDIR "entrylist/" << QStringList("d*")
                                  << int(QDir::NoFilter) << int(QDir::Name)
                                  << filterLinks(QString("directory").split(','));
    QTest::newRow("Namefilters f*") << SRCDIR "entrylist/" << QStringList("f*")
                                  << int(QDir::NoFilter) << int(QDir::Name)
                                  << filterLinks(QString("file").split(','));
    QTest::newRow("Namefilters link*") << SRCDIR "entrylist/" << QStringList("link*")
                                  << int(QDir::NoFilter) << int(QDir::Name)
                                  << filterLinks(QString("linktodirectory.lnk,linktofile.lnk").split(','));
    QTest::newRow("Namefilters *to*") << SRCDIR "entrylist/" << QStringList("*to*")
                                  << int(QDir::NoFilter) << int(QDir::Name)
                                  << filterLinks(QString("directory,linktodirectory.lnk,linktofile.lnk").split(','));
    QTest::newRow("Sorting QDir::Name") << SRCDIR "entrylist/" << QStringList("*")
                                  << int(QDir::NoFilter) << int(QDir::Name)
                                  << filterLinks(QString(".,..,directory,file,linktodirectory.lnk,linktofile.lnk,writable").split(','));
    QTest::newRow("Sorting QDir::Name | QDir::Reversed") << SRCDIR "entrylist/" << QStringList("*")
                                  << int(QDir::NoFilter) << int(QDir::Name | QDir::Reversed)
                                  << filterLinks(QString("writable,linktofile.lnk,linktodirectory.lnk,file,directory,..,.").split(','));

    QTest::newRow("Sorting QDir::Type") << SRCDIR "types/" << QStringList("*")
                                  << int(QDir::NoFilter) << int(QDir::Type)
                                  << QString(".,..,a,b,c,d,e,f,a.a,b.a,c.a,d.a,e.a,f.a,a.b,b.b,c.b,d.b,e.b,f.b,a.c,b.c,c.c,d.c,e.c,f.c").split(',');
    QTest::newRow("Sorting QDir::Type | QDir::Reversed") << SRCDIR "types/" << QStringList("*")
                                  << int(QDir::NoFilter) << int(QDir::Type | QDir::Reversed)
                                  << QString("f.c,e.c,d.c,c.c,b.c,a.c,f.b,e.b,d.b,c.b,b.b,a.b,f.a,e.a,d.a,c.a,b.a,a.a,f,e,d,c,b,a,..,.").split(',');
    QTest::newRow("Sorting QDir::Type | QDir::DirsLast") << SRCDIR "types/" << QStringList("*")
                                  << int(QDir::NoFilter) << int(QDir::Type | QDir::DirsLast)
                                  << QString("a,b,c,a.a,b.a,c.a,a.b,b.b,c.b,a.c,b.c,c.c,.,..,d,e,f,d.a,e.a,f.a,d.b,e.b,f.b,d.c,e.c,f.c").split(',');
    QTest::newRow("Sorting QDir::Type | QDir::DirsFirst") << SRCDIR "types/" << QStringList("*")
                                  << int(QDir::NoFilter) << int(QDir::Type | QDir::DirsFirst)
                                  << QString(".,..,d,e,f,d.a,e.a,f.a,d.b,e.b,f.b,d.c,e.c,f.c,a,b,c,a.a,b.a,c.a,a.b,b.b,c.b,a.c,b.c,c.c").split(',');
    QTest::newRow("Sorting QDir::Size") << SRCDIR "types/" << QStringList("*")
                                  << int(QDir::AllEntries|QDir::NoDotAndDotDot) << int(QDir::Size | QDir::DirsFirst)
                                  << QString("d,d.a,d.b,d.c,e,e.a,e.b,e.c,f,f.a,f.b,f.c,c.a,c.b,c.c,b.a,b.c,b.b,a.c,a.b,a.a,a,b,c").split(',');
    QTest::newRow("Sorting QDir::Size | QDir::Reversed") << SRCDIR "types/" << QStringList("*")
                                  << int(QDir::AllEntries|QDir::NoDotAndDotDot) << int(QDir::Size | QDir::Reversed | QDir::DirsLast)
                                  << QString("c,b,a,a.a,a.b,a.c,b.b,b.c,b.a,c.c,c.b,c.a,f.c,f.b,f.a,f,e.c,e.b,e.a,e,d.c,d.b,d.a,d").split(',');
}

void tst_QDir::entryList()
{
    QFETCH(QString, dirName);
    QFETCH(QStringList, nameFilters);
    QFETCH(int, filterspec);
    QFETCH(int, sortspec);
    QFETCH(QStringList, expected);

    QFile(SRCDIR "entrylist/writable").open(QIODevice::ReadWrite);
    QFile(SRCDIR "entrylist/file").setPermissions(QFile::ReadOwner | QFile::ReadUser);
    QFile::remove(SRCDIR "entrylist/linktofile");
    QFile::remove(SRCDIR "entrylist/linktodirectory");
    QFile::remove(SRCDIR "entrylist/linktofile.lnk");
    QFile::remove(SRCDIR "entrylist/linktodirectory.lnk");
    QFile::remove(SRCDIR "entrylist/brokenlink.lnk");
    QFile::remove(SRCDIR "entrylist/brokenlink");

#ifndef Q_NO_SYMLINKS
    QFile::link("file", SRCDIR "entrylist/linktofile.lnk");
    QFile::link("directory", SRCDIR "entrylist/linktodirectory.lnk");
    QFile::link("nothing", SRCDIR "entrylist/brokenlink.lnk");
#endif //Q_NO_SYMLINKS

    QDir dir(dirName);
    QVERIFY(dir.exists());

    QStringList actual = dir.entryList(nameFilters, (QDir::Filters)filterspec,
                                       (QDir::SortFlags)sortspec);

    int max = qMin(actual.count(), expected.count());

    if (qstrcmp(QTest::currentDataTag(), "unprintablenames") == 0) {
        // The purpose of this entry is to check that QDir doesn't
        // lock up. The actual result depends on the file system.
        return;
    }
    bool doContentCheck = true;
    if (qstrcmp(QTest::currentDataTag(), "QDir::AllEntries | QDir::Writable") == 0) {
        // for root, everything is writeable
        if (::getuid() == 0)
            doContentCheck = false;
    }

    if (doContentCheck) {
        for (int i=0; i<max; ++i)
            QCOMPARE(actual[i], expected[i]);

        QCOMPARE(actual.count(), expected.count());
    }

    QFile::remove(SRCDIR "entrylist/writable");
    QFile::remove(SRCDIR "entrylist/linktofile");
    QFile::remove(SRCDIR "entrylist/linktodirectory");
    QFile::remove(SRCDIR "entrylist/linktofile.lnk");
    QFile::remove(SRCDIR "entrylist/linktodirectory.lnk");
    QFile::remove(SRCDIR "entrylist/brokenlink.lnk");
    QFile::remove(SRCDIR "entrylist/brokenlink");
}

void tst_QDir::entryListSimple_data()
{
    QTest::addColumn<QString>("dirName");
    QTest::addColumn<int>("countMin");

    QTest::newRow("data2") << "do_not_expect_this_path_to_exist/" << 0;
    QTest::newRow("simple dir") << SRCDIR "testdir" << 2;
    QTest::newRow("simple dir with slash") << SRCDIR "testdir/" << 2;
}

void tst_QDir::entryListSimple()
{
    QFETCH(QString, dirName);
    QFETCH(int, countMin);

    QDir dir(dirName);
    QStringList actual = dir.entryList();
    QVERIFY(actual.count() >= countMin);
}

void tst_QDir::entryListWithSymLinks()
{
#ifndef Q_NO_SYMLINKS
#  ifndef Q_NO_SYMLINKS_TO_DIRS
    QFile::remove("myLinkToDir.lnk");
#  endif
    QFile::remove("myLinkToFile.lnk");
    QFile::remove("testfile.cpp");
    QDir dir;
    dir.mkdir("myDir");
    QFile("testfile.cpp").open(QIODevice::WriteOnly);
#  ifndef Q_NO_SYMLINKS_TO_DIRS
    QVERIFY(QFile::link("myDir", "myLinkToDir.lnk"));
#  endif
    QVERIFY(QFile::link("testfile.cpp", "myLinkToFile.lnk"));

    {
        QStringList entryList = QDir().entryList();
        QVERIFY(entryList.contains("myDir"));
#  ifndef Q_NO_SYMLINKS_TO_DIRS
        QVERIFY(entryList.contains("myLinkToDir.lnk"));
#endif
        QVERIFY(entryList.contains("myLinkToFile.lnk"));
    }
    {
        QStringList entryList = QDir().entryList(QDir::Dirs);
        QVERIFY(entryList.contains("myDir"));
#  ifndef Q_NO_SYMLINKS_TO_DIRS
        QVERIFY(entryList.contains("myLinkToDir.lnk"));
#endif
        QVERIFY(!entryList.contains("myLinkToFile.lnk"));
    }
    {
        QStringList entryList = QDir().entryList(QDir::Dirs | QDir::NoSymLinks);
        QVERIFY(entryList.contains("myDir"));
        QVERIFY(!entryList.contains("myLinkToDir.lnk"));
        QVERIFY(!entryList.contains("myLinkToFile.lnk"));
    }

    QFile::remove("myLinkToDir.lnk");
    QFile::remove("myLinkToFile.lnk");
    QFile::remove("testfile.cpp");
    dir.rmdir("myDir");
#endif
}

void tst_QDir::canonicalPath_data()
{
    QTest::addColumn<QString>("path");
    QTest::addColumn<QString>("canonicalPath");
    QString appPath = SRCDIR;
    if (appPath.isEmpty())
        appPath = QCoreApplication::instance()->applicationDirPath();
    else
        appPath.chop(1);        // remove the ending slash

    QTest::newRow("relative") << "." << appPath;
    QTest::newRow("relativeSubDir") << "./testData/../testData" << appPath + "/testData";

    QTest::newRow("absPath") << appPath + "/testData/../testData" << appPath + "/testData";
    QTest::newRow("nonexistant") << "testd" << QString();

    QTest::newRow("rootPath") << QDir::rootPath() << QDir::rootPath();
    QTest::newRow("rootPath + ./") << QDir::rootPath().append("./") << QDir::rootPath();

    QTest::newRow("rootPath + ../.. ") << QDir::rootPath().append("../..") << QDir::rootPath();
}

void tst_QDir::canonicalPath()
{
    QDir srcPath;
    if (strlen(SRCDIR) > 0)
        srcPath = QDir(SRCDIR);
    else
        srcPath = QDir(".");
    if (srcPath.absolutePath() != srcPath.canonicalPath())
        QSKIP("This test does not work if this directory path consists of symlinks.", SkipAll);

    QString oldpwd = QDir::currentPath();
    QDir::setCurrent(srcPath.absolutePath());

    QFETCH(QString, path);
    QFETCH(QString, canonicalPath);

    QDir dir(path);
    QCOMPARE(dir.canonicalPath(), canonicalPath);

    QDir::setCurrent(oldpwd);
}

void tst_QDir::current_data()
{
    QTest::addColumn<QString>("path");
    QTest::addColumn<QString>("currentDir");
    QString appPath = SRCDIR;
    if (appPath.isEmpty())
        appPath = QCoreApplication::instance()->applicationDirPath();
    else
        appPath.chop(1);        // remove the ending slash

    QTest::newRow("startup") << QString() << appPath;
    QTest::newRow("relPath") << "testData" << appPath + "/testData";
    QTest::newRow("absPath") << appPath + "/testData" << appPath + "/testData";
    QTest::newRow("nonexistant") << "testd" << QString();

    QTest::newRow("parent") << ".." << appPath.left(appPath.lastIndexOf('/'));
}

void tst_QDir::current()
{
    QString oldDir = QDir::currentPath();
    QString appPath = SRCDIR;
    if (appPath.isEmpty())
        appPath = QCoreApplication::instance()->applicationDirPath();
    QDir::setCurrent(appPath);
    QFETCH(QString, path);
    QFETCH(QString, currentDir);

    if (!path.isEmpty()) {
        bool b = QDir::setCurrent(path);
        // If path is non existent, then setCurrent should be false (currentDir is empty in testData)
        QVERIFY(b == !currentDir.isEmpty());
    }
    if (!currentDir.isEmpty()) {
        QDir newCurrent = QDir::current();
        QDir::setCurrent(oldDir);
        QCOMPARE(newCurrent.absolutePath(), currentDir);
    }

    QDir::setCurrent(oldDir);
}

void tst_QDir::cd_data()
{
    QTest::addColumn<QString>("startDir");
    QTest::addColumn<QString>("cdDir");
    QTest::addColumn<bool>("successExpected");
    QTest::addColumn<QString>("newDir");

    QString appPath = QDir::currentPath();
    int index = appPath.lastIndexOf("/");
    QTest::newRow("cdUp") << QDir::currentPath() << ".." << true << appPath.left(index==0?1:index);
    QTest::newRow("noChange") << QDir::currentPath() << "." << true << appPath;
    QTest::newRow("absolute") << QDir::currentPath() << "/" << true << QDir::root().absolutePath();
    QTest::newRow("non existant") << "." << "../anonexistingdir" << false << QDir::currentPath();
    QTest::newRow("self") << "." << (QString("../") + QFileInfo(QDir::currentPath()).fileName()) << true << QDir::currentPath();
    QTest::newRow("file") << "." << "qdir.pro" << false << "";
}

void tst_QDir::cd()
{
    QFETCH(QString, startDir);
    QFETCH(QString, cdDir);
    QFETCH(bool, successExpected);
    QFETCH(QString, newDir);

    QDir d = startDir;
    bool notUsed = d.exists(); // make sure we cache this before so we can see if 'cd' fails to flush this
    Q_UNUSED(notUsed);
    QCOMPARE(d.cd(cdDir), successExpected);
    if (successExpected)
        QCOMPARE(d.absolutePath(), newDir);
}

void tst_QDir::setNameFilters_data()
{
    // Effectively copied from entryList2() test

    QTest::addColumn<QString>("dirName"); // relative from current path or abs
    QTest::addColumn<QStringList>("nameFilters");
    QTest::addColumn<QStringList>("expected");

    QString appPath = SRCDIR;
    if (appPath.isEmpty())
        appPath = QCoreApplication::instance()->applicationDirPath();
    if (!appPath.endsWith("/"))
        appPath.append("/");

    QTest::newRow("spaces1") << appPath + "testdir/spaces" << QStringList("*. bar")
                          << QStringList("foo. bar");
    QTest::newRow("spaces2") << appPath + "testdir/spaces" << QStringList("*.bar")
                          << QStringList("foo.bar");
    QTest::newRow("spaces3") << appPath + "testdir/spaces" << QStringList("foo.*")
                            << QString("foo. bar,foo.bar").split(",");
    QTest::newRow("files1")  << appPath + "testdir/dir" << QString("*r.cpp *.pro").split(" ")
                          << QString("qdir.pro,qrc_qdir.cpp,tst_qdir.cpp").split(",");
}

void tst_QDir::setNameFilters()
{
    QFETCH(QString, dirName);
    QFETCH(QStringList, nameFilters);
    QFETCH(QStringList, expected);

    QDir dir(dirName);
    QVERIFY(dir.exists());

    dir.setNameFilters(nameFilters);
    QStringList actual = dir.entryList();
    int max = qMin(actual.count(), expected.count());

    for (int i=0; i<max; ++i)
        QCOMPARE(actual[i], expected[i]);
    QCOMPARE(actual.count(), expected.count());
}

void
tst_QDir::cleanPath_data()
{
    QTest::addColumn<QString>("path");
    QTest::addColumn<QString>("expected");

    QTest::newRow("data0") << "/Users/sam/troll/qt4.0//.." << "/Users/sam/troll";
    QTest::newRow("data1") << "/Users/sam////troll/qt4.0//.." << "/Users/sam/troll";
    QTest::newRow("data2") << "/" << "/";
    QTest::newRow("data3") << QDir::cleanPath("../.") << "..";
    QTest::newRow("data4") << QDir::cleanPath("../..") << "../..";
    QTest::newRow("data5") << "d:\\a\\bc\\def\\.." << "d:\\a\\bc\\def\\..";
    QTest::newRow("data6") << "d:\\a\\bc\\def\\../../.." << "d:\\a\\bc\\def\\../../..";
    QTest::newRow("data7") << ".//file1.txt" << "file1.txt";
    QTest::newRow("data8") << "/foo/bar/..//file1.txt" << "/foo/file1.txt";
    QTest::newRow("data9") << "//" << "/";
    QTest::newRow("data10") << "/:/" << "/:";
}


void tst_QDir::cleanPath()
{
    QFETCH(QString, path);
    QFETCH(QString, expected);
    QString cleaned = QDir::cleanPath(path);
    QCOMPARE(cleaned, expected);
}

void tst_QDir::absoluteFilePath_data()
{
    QTest::addColumn<QString>("path");
    QTest::addColumn<QString>("fileName");
    QTest::addColumn<QString>("expectedFilePath");

    QTest::newRow("0") << "/etc" << "/passwd" << "/passwd";
    QTest::newRow("1") << "/etc" << "passwd" << "/etc/passwd";
    QTest::newRow("2") << "/" << "passwd" << "/passwd";
    QTest::newRow("3") << "relative" << "path" << QDir::currentPath() + "/relative/path";
    QTest::newRow("4") << "" << "" << QDir::currentPath();
}

void tst_QDir::absoluteFilePath()
{
    QFETCH(QString, path);
    QFETCH(QString, fileName);
    QFETCH(QString, expectedFilePath);

    QDir dir(path);
    QString absFilePath = dir.absoluteFilePath(fileName);
    QCOMPARE(absFilePath, expectedFilePath);
}

void tst_QDir::absolutePath_data()
{
    QTest::addColumn<QString>("path");
    QTest::addColumn<QString>("expectedPath");

    QTest::newRow("0") << "/machine/share/dir1" << "/machine/share/dir1";
    //test dirty paths are cleaned (QTBUG-19995)
    QTest::newRow("/home/qt/.") << QDir::rootPath() + "home/qt/." << QDir::rootPath() + "home/qt";
    QTest::newRow("/system/data/../config") << QDir::rootPath() + "system/data/../config" << QDir::rootPath() + "system/config";
    QTest::newRow("//home//qt/") << QDir::rootPath() + "/home//qt/" << QDir::rootPath() + "home/qt";
    QTest::newRow("foo/../bar") << "foo/../bar" << QDir::currentPath() + "/bar";
}

void tst_QDir::absolutePath()
{
    QFETCH(QString, path);
    QFETCH(QString, expectedPath);

    QDir dir(path);
    QCOMPARE(dir.absolutePath(), expectedPath);
}

void tst_QDir::relativeFilePath_data()
{
    QTest::addColumn<QString>("dir");
    QTest::addColumn<QString>("path");
    QTest::addColumn<QString>("expected");

    QTest::newRow("0") << "/foo/bar" << "ding.txt" << "ding.txt";
    QTest::newRow("1") << "/foo/bar" << "ding/dong.txt"    << "ding/dong.txt";
    QTest::newRow("2") << "/foo/bar" << "../ding/dong.txt" << "../ding/dong.txt";

    QTest::newRow("3") << "/foo/bar" << "/foo/bar/ding.txt" << "ding.txt";
    QTest::newRow("4") << "/foo/bar/" << "/foo/bar/ding/dong.txt" << "ding/dong.txt";
    QTest::newRow("5") << "/foo/bar/" << "/ding/dong.txt" << "../../ding/dong.txt";

    QTest::newRow("6") << "/" << "/ding/dong.txt" << "ding/dong.txt";
    QTest::newRow("7") << "/" << "/ding/" << "ding";
    QTest::newRow("8") << "/" << "/ding//" << "ding";
    QTest::newRow("9") << "/" << "/ding/../dong" << "dong";
    QTest::newRow("10") << "/" << "/ding/../../../../dong" << "../../../dong";

    QTest::newRow("11") << "" << "" << "";
}

void tst_QDir::relativeFilePath()
{
    QFETCH(QString, dir);
    QFETCH(QString, path);
    QFETCH(QString, expected);

    QCOMPARE(QDir(dir).relativeFilePath(path), expected);
}

void tst_QDir::filePath_data()
{
    QTest::addColumn<QString>("path");
    QTest::addColumn<QString>("fileName");
    QTest::addColumn<QString>("expectedFilePath");

    QTest::newRow("0") << "/etc" << "/passwd" << "/passwd";
    QTest::newRow("1") << "/etc" << "passwd" << "/etc/passwd";
    QTest::newRow("2") << "/" << "passwd" << "/passwd";
    QTest::newRow("3") << "relative" << "path" << "relative/path";
    QTest::newRow("4") << "" << "" << ".";
}

void tst_QDir::filePath()
{
    QFETCH(QString, path);
    QFETCH(QString, fileName);
    QFETCH(QString, expectedFilePath);

    QDir dir(path);
    QString absFilePath = dir.filePath(fileName);
    QCOMPARE(absFilePath, expectedFilePath);
}

void tst_QDir::remove()
{
    QFile f("remove-test");
    f.open(QIODevice::WriteOnly);
    f.close();
    QDir dir;
    QVERIFY(dir.remove("remove-test"));
    QVERIFY(!dir.remove("/remove-test"));
    QTest::ignoreMessage(QtWarningMsg, "QDir::remove: Empty or null file name");
    QVERIFY(!dir.remove(""));
}

void tst_QDir::rename()
{
    QFile f("rename-test");
    f.open(QIODevice::WriteOnly);
    f.close();
    QDir dir;
    QVERIFY(dir.rename("rename-test", "rename-test-renamed"));
    QVERIFY(dir.rename("rename-test-renamed", "rename-test"));
    QTest::ignoreMessage(QtWarningMsg, "QDir::rename: Empty or null file name(s)");
    QVERIFY(!dir.rename("rename-test", ""));
    QTest::ignoreMessage(QtWarningMsg, "QDir::rename: Empty or null file name(s)");
    QVERIFY(!dir.rename("", "rename-test-renamed"));
    QVERIFY(!dir.rename("some-file-that-does-not-exist", "rename-test-renamed"));

    QVERIFY(dir.remove("rename-test"));

    if (::getuid() == 0)
        // root and Chuck Norris don't care for file permissions. Skip.
        QSKIP("Running this test as root doesn't make sense", SkipAll);
    QVERIFY(!dir.rename("rename-test", "/rename-test-renamed"));
}

void tst_QDir::exists2_data()
{
    QTest::addColumn<QString>("path");
    QTest::addColumn<bool>("exists");

    QTest::newRow("0") << "." << true;
    QTest::newRow("1") << "/" << true;
    QTest::newRow("2") << "" << false;
    QTest::newRow("3") << "testData" << true;
    QTest::newRow("4") << "/testData" << false;
    QTest::newRow("5") << "tst_qdir.cpp" << true;
    QTest::newRow("6") << "/resources.cpp" << false;
}

void tst_QDir::exists2()
{
    QFETCH(QString, path);
    QFETCH(bool, exists);

    QString oldpwd = QDir::currentPath();
    if (strlen(SRCDIR) > 0)
        QDir::setCurrent(SRCDIR);

    if (path.isEmpty())
        QTest::ignoreMessage(QtWarningMsg, "QDir::exists: Empty or null file name");

    QDir dir;
    if (exists)
        QVERIFY(dir.exists(path));
    else
        QVERIFY(!dir.exists(path));

    QDir::setCurrent(oldpwd);
}

void tst_QDir::dirName_data()
{
    QTest::addColumn<QString>("path");
    QTest::addColumn<QString>("dirName");

    QTest::newRow("slash0") << "c:/winnt/system32" << "system32";
    QTest::newRow("slash1") << "/winnt/system32" << "system32";
    QTest::newRow("slash2") << "c:/winnt/system32/kernel32.dll" << "kernel32.dll";
}

void tst_QDir::dirName()
{
    QFETCH(QString, path);
    QFETCH(QString, dirName);

    QDir dir(path);
    QCOMPARE(dir.dirName(), dirName);
}

void tst_QDir::operator_eq()
{
    QDir dir1(".");
    dir1 = dir1;
    dir1.setPath("..");
}

void tst_QDir::dotAndDotDot()
{
    QDir dir(QString(SRCDIR "testdir/"));
    QStringList entryList = dir.entryList(QDir::Dirs);
    QCOMPARE(entryList, QStringList() << QString(".") << QString("..") << QString("dir") << QString("spaces"));
    entryList = dir.entryList(QDir::Dirs | QDir::NoDotAndDotDot);
    QCOMPARE(entryList, QStringList() << QString("dir") << QString("spaces"));
}

void tst_QDir::homePath()
{
    QDir homeDir = QDir::home();
    QString strHome = QDir::homePath();

    // docs say that homePath() is an absolute path
    QCOMPARE(strHome, homeDir.absolutePath());
    QVERIFY(QDir::isAbsolutePath(strHome));

    if (strHome.length() > 1)      // root dir = "/"
        QVERIFY(!strHome.endsWith('/'));

    QByteArray envHome = qgetenv("HOME");
    ::unsetenv("HOME");
    QCOMPARE(QDir::homePath(), QDir::rootPath());
    qputenv("HOME", envHome);

    QStringList entries = homeDir.entryList();
    for (int i = 0; i < entries.count(); ++i) {
        QFileInfo fi(QDir::homePath() + "/" + entries[i]);
        QCOMPARE(fi.exists(), true);
    }
}

void tst_QDir::tempPath()
{
    QDir dir = QDir::temp();
    QString path = QDir::tempPath();

    // docs say that tempPath() is an absolute path
    QCOMPARE(path, dir.absolutePath());
    QVERIFY(QDir::isAbsolutePath(path));

    if (path.length() > 1)      // root dir = "/"
        QVERIFY(!path.endsWith('/'));
}

void tst_QDir::rootPath()
{
    QDir dir = QDir::root();
    QString path = QDir::rootPath();

    // docs say that tempPath() is an absolute path
    QCOMPARE(path, dir.absolutePath());
    QVERIFY(QDir::isAbsolutePath(path));

    QCOMPARE(path, QString("/"));
}

void tst_QDir::longFileName_data()
{
    QTest::addColumn<int>("length");

    QTest::newRow("128") << 128;
    QTest::newRow("256") << 256;
    QTest::newRow("512") << 512;
    QTest::newRow("1024") << 1024;
    QTest::newRow("2048") << 2048;
    QTest::newRow("4096") << 4096;
}

void tst_QDir::longFileName()
{
    QFETCH(int, length);

    QString fileName(length, QLatin1Char('a'));
    fileName += QLatin1String(".txt");

    QFile file(fileName);
    if (!file.open(QFile::WriteOnly))
        QSKIP("Cannot create long file names", SkipAll);

    QFile file2(fileName);
    QVERIFY(file2.open(QFile::ReadOnly));

    QVERIFY(QDir().entryList().contains(fileName));

    file.close();
    file2.close();

    QFile::remove(fileName);
}

void tst_QDir::updateFileLists()
{
    //  Test setup

    FileSystem fs;

    QVERIFY( fs.createDirectory("update-file-lists") );
    QVERIFY( fs.createFile("update-file-lists/file1.txt") );
    QVERIFY( fs.createFile("update-file-lists/file2.doc") );

    QVERIFY( fs.createDirectory("update-file-lists/sub-dir1") );
    QVERIFY( fs.createFile("update-file-lists/sub-dir1/file3.txt") );
    QVERIFY( fs.createFile("update-file-lists/sub-dir1/file4.doc") );
    QVERIFY( fs.createFile("update-file-lists/sub-dir1/file5.txt") );

    QVERIFY( fs.createDirectory("update-file-lists/sub-dir2") );
    QVERIFY( fs.createFile("update-file-lists/sub-dir2/file6.txt") );
    QVERIFY( fs.createFile("update-file-lists/sub-dir2/file7.txt") );
    QVERIFY( fs.createFile("update-file-lists/sub-dir2/file8.doc") );
    QVERIFY( fs.createFile("update-file-lists/sub-dir2/file9.doc") );

    //  Actual test

    QDir dir("update-file-lists");

    QCOMPARE(dir.count(), uint(6));
    QCOMPARE(dir.entryList().size(), 6);
    QCOMPARE(dir.entryInfoList().size(), 6);

    dir.setFilter(QDir::AllEntries | QDir::NoDotAndDotDot);

    QCOMPARE(dir.entryList().size(), 4);
    QCOMPARE(dir.count(), uint(4));
    QCOMPARE(dir.entryInfoList().size(), 4);

    dir.setPath("update-file-lists/sub-dir1");

    QCOMPARE(dir.entryInfoList().size(), 3);
    QCOMPARE(dir.count(), uint(3));
    QCOMPARE(dir.entryList().size(), 3);

    dir.setNameFilters(QStringList("*.txt"));

    QCOMPARE(dir.entryInfoList().size(), 2);
    QCOMPARE(dir.entryList().size(), 2);
    QCOMPARE(dir.count(), uint(2));

    dir.setPath("update-file-lists");
    dir = QDir(dir.path(),
            "*.txt",
            QDir::Name | QDir::DirsLast,
            QDir::AllEntries | QDir::AllDirs | QDir::NoDotAndDotDot);

    QCOMPARE(dir.count(), uint(3));
    QCOMPARE(dir.entryList().size(), 3);
    QCOMPARE(dir.entryInfoList().size(), 3);
    QCOMPARE(dir.entryList(), QStringList() << "file1.txt" << "sub-dir1" << "sub-dir2");

    dir.setSorting(QDir::Name | QDir::DirsFirst);

    QCOMPARE(dir.count(), uint(3));
    QCOMPARE(dir.entryList().size(), 3);
    QCOMPARE(dir.entryInfoList().size(), 3);
    QCOMPARE(dir.entryList(), QStringList() << "sub-dir1" << "sub-dir2" << "file1.txt");

    {
        QVERIFY( fs.createFile("update-file-lists/extra-file.txt") );

        QDir dir2(dir);

        QCOMPARE(dir2.count(), uint(3));
        QCOMPARE(dir2.entryList().size(), 3);
        QCOMPARE(dir2.entryInfoList().size(), 3);
        QCOMPARE(dir2.entryList(), QStringList() << "sub-dir1" << "sub-dir2" << "file1.txt");

        dir2.refresh();

        QCOMPARE(dir2.count(), uint(4));
        QCOMPARE(dir2.entryList().size(), 4);
        QCOMPARE(dir2.entryInfoList().size(), 4);
        QCOMPARE(dir2.entryList(), QStringList() << "sub-dir1" << "sub-dir2" << "extra-file.txt" << "file1.txt");
    }

    QCOMPARE(dir.count(), uint(3));
    QCOMPARE(dir.entryList().size(), 3);
    QCOMPARE(dir.entryInfoList().size(), 3);
    QCOMPARE(dir.entryList(), QStringList() << "sub-dir1" << "sub-dir2" << "file1.txt");
}

void tst_QDir::detachingOperations()
{
    QString const defaultPath(".");
    QStringList const defaultNameFilters = QStringList("*");
    QDir::SortFlags const defaultSorting = QDir::Name | QDir::IgnoreCase;
    QDir::Filters const defaultFilter = QDir::AllEntries;

    QString const path1("..");
    QString const path2("./foo");
    QStringList const nameFilters = QStringList(QString("*.txt"));
    QDir::SortFlags const sorting = QDir::Name | QDir::DirsLast | QDir::Reversed;
    QDir::Filters const filter = QDir::Writable;

    QDir dir1;

    QCOMPARE(dir1.path(), defaultPath);
    QCOMPARE(dir1.filter(), defaultFilter);
    QCOMPARE(dir1.nameFilters(), defaultNameFilters);
    QCOMPARE(dir1.sorting(), defaultSorting);

    dir1.setPath(path1);
    QCOMPARE(dir1.path(), path1);
    QCOMPARE(dir1.filter(), defaultFilter);
    QCOMPARE(dir1.nameFilters(), defaultNameFilters);
    QCOMPARE(dir1.sorting(), defaultSorting);

    dir1.setFilter(filter);
    QCOMPARE(dir1.path(), path1);
    QCOMPARE(dir1.filter(), filter);
    QCOMPARE(dir1.nameFilters(), defaultNameFilters);
    QCOMPARE(dir1.sorting(), defaultSorting);

    dir1.setNameFilters(nameFilters);
    QCOMPARE(dir1.path(), path1);
    QCOMPARE(dir1.filter(), filter);
    QCOMPARE(dir1.nameFilters(), nameFilters);
    QCOMPARE(dir1.sorting(), defaultSorting);

    dir1.setSorting(sorting);
    QCOMPARE(dir1.path(), path1);
    QCOMPARE(dir1.filter(), filter);
    QCOMPARE(dir1.nameFilters(), nameFilters);
    QCOMPARE(dir1.sorting(), sorting);

    dir1.setPath(path2);
    QCOMPARE(dir1.path(), path2);
    QCOMPARE(dir1.filter(), filter);
    QCOMPARE(dir1.nameFilters(), nameFilters);
    QCOMPARE(dir1.sorting(), sorting);

    {
        QDir dir2(dir1);
        QCOMPARE(dir2.path(), path2);
        QCOMPARE(dir2.filter(), filter);
        QCOMPARE(dir2.nameFilters(), nameFilters);
        QCOMPARE(dir2.sorting(), sorting);
    }

    {
        QDir dir2;
        QCOMPARE(dir2.path(), defaultPath);
        QCOMPARE(dir2.filter(), defaultFilter);
        QCOMPARE(dir2.nameFilters(), defaultNameFilters);
        QCOMPARE(dir2.sorting(), defaultSorting);

        dir2 = dir1;
        QCOMPARE(dir2.path(), path2);
        QCOMPARE(dir2.filter(), filter);
        QCOMPARE(dir2.nameFilters(), nameFilters);
        QCOMPARE(dir2.sorting(), sorting);

        dir2 = path1;
        QCOMPARE(dir2.path(), path1);
        QCOMPARE(dir2.filter(), filter);
        QCOMPARE(dir2.nameFilters(), nameFilters);
        QCOMPARE(dir2.sorting(), sorting);
    }

    dir1.refresh();
    QCOMPARE(dir1.path(), path2);
    QCOMPARE(dir1.filter(), filter);
    QCOMPARE(dir1.nameFilters(), nameFilters);
    QCOMPARE(dir1.sorting(), sorting);

    QString const currentPath = QDir::currentPath();
    QVERIFY(dir1.cd(currentPath));
    QCOMPARE(dir1.path(), currentPath);
    QCOMPARE(dir1.filter(), filter);
    QCOMPARE(dir1.nameFilters(), nameFilters);
    QCOMPARE(dir1.sorting(), sorting);

    QVERIFY(dir1.cdUp());
    QCOMPARE(dir1.filter(), filter);
    QCOMPARE(dir1.nameFilters(), nameFilters);
    QCOMPARE(dir1.sorting(), sorting);
}

void tst_QDir::testCaching()
{
    QString dirName = QString::fromLatin1("testCaching");
    QDir::current().rmdir(dirName); // cleanup a previous run.
    QDir dir(dirName);
    QVERIFY(!dir.exists());
    QDir::current().mkdir(dirName);
    QVERIFY(QDir(dirName).exists()); // dir exists
    QVERIFY(dir.exists()); // QDir doesn't cache the 'exist' between calls.
}

void tst_QDir::isRoot_data()
{
    QTest::addColumn<QString>("path");
    QTest::addColumn<bool>("isRoot");

    QString test = QDir::rootPath();
    QTest::newRow(QString("rootPath " + test).toLatin1()) << test << true;
    test = QDir::rootPath().append("./");
    QTest::newRow(QString("./ appended " + test).toLatin1()) << test << false;

    test = QDir(QDir::rootPath().append("./")).canonicalPath();
    QTest::newRow(QString("canonicalPath " + test).toLatin1()) << test << true;
}

void tst_QDir::isRoot()
{
    QFETCH(QString, path);
    QFETCH(bool, isRoot);

    QDir dir(path);
    QCOMPARE(dir.isRoot(),isRoot);
}

#ifndef QT_NO_REGEXP
void tst_QDir::match_data()
{
    QTest::addColumn<QString>("filter");
    QTest::addColumn<QString>("filename");
    QTest::addColumn<bool>("match");

    QTest::newRow("single, matching") << "*.cpp" << "tst_qdir.cpp" << true;
    QTest::newRow("single, not matching") << "*.cpp" << "tst_qdir.h" << false;
    QTest::newRow("multi, matching") << "*.cpp;*.h" << "tst_qdir.cpp" << true;
    QTest::newRow("multi, matching2") << "*.cpp;*.h" << "tst_qdir.h" << true;
    QTest::newRow("multi, not matching") << "*.cpp;*.h" << "readme.txt" << false;
}

void tst_QDir::match()
{
    QFETCH(QString, filter);
    QFETCH(QString, filename);
    QFETCH(bool, match);

    QCOMPARE(QDir::match(filter, filename), match);
    QCOMPARE(QDir::match(filter.split(QLatin1Char(';')), filename), match);
}
#endif

void tst_QDir::arrayOperator()
{
    QDir dir1(SRCDIR "entrylist/");
    QDir dir2(SRCDIR "entrylist/");

    QStringList entries(dir1.entryList());
    int i = dir2.count();
    QCOMPARE(i, entries.count());
    --i;
    for (;i>=0;--i) {
        QCOMPARE(dir2[i], entries.at(i));
    }
}

void tst_QDir::equalityOperator_data()
{
    QTest::addColumn<QString>("leftPath");
    QTest::addColumn<QString>("leftNameFilters");
    QTest::addColumn<int>("leftSort");
    QTest::addColumn<int>("leftFilters");
    QTest::addColumn<QString>("rightPath");
    QTest::addColumn<QString>("rightNameFilters");
    QTest::addColumn<int>("rightSort");
    QTest::addColumn<int>("rightFilters");
    QTest::addColumn<bool>("expected");

    QTest::newRow("same") << SRCDIR << "*.cpp" << int(QDir::Name) << int(QDir::Files)
        << SRCDIR << "*.cpp" << int(QDir::Name) << int(QDir::Files)
        << true;

    QTest::newRow("relativepaths") << "entrylist/" << "*.cpp" << int(QDir::Name) << int(QDir::Files)
        << "./entrylist" << "*.cpp" << int(QDir::Name) << int(QDir::Files)
        << true;

    QTest::newRow("QTBUG-20495") << QDir::currentPath() + "/entrylist/.." << "*.cpp" << int(QDir::Name) << int(QDir::Files)
        << "." << "*.cpp" << int(QDir::Name) << int(QDir::Files)
        << true;

    // need a path in the root directory that is unlikely to be a symbolic link.
    QString pathinroot("/boot/..");
    QTest::newRow("QTBUG-20495-root") << pathinroot << "*.cpp" << int(QDir::Name) << int(QDir::Files)
        << QDir::rootPath() << "*.cpp" << int(QDir::Name) << int(QDir::Files)
        << true;

    QTest::newRow("slashdot") << QDir::rootPath() + "." << "*.cpp" << int(QDir::Name) << int(QDir::Files)
        << QDir::rootPath() << "*.cpp" << int(QDir::Name) << int(QDir::Files)
        << true;

    QTest::newRow("slashdotslash") << QDir::rootPath() + "./" << "*.cpp" << int(QDir::Name) << int(QDir::Files)
        << QDir::rootPath() << "*.cpp" << int(QDir::Name) << int(QDir::Files)
        << true;

    QTest::newRow("nonexistantpaths") << "dir-that-dont-exist" << "*.cpp" << int(QDir::Name) << int(QDir::Files)
        << "another-dir-that-dont-exist" << "*.cpp" << int(QDir::Name) << int(QDir::Files)
        << false;

    QTest::newRow("diff-filters") << SRCDIR << "*.cpp" << int(QDir::Name) << int(QDir::Files)
        << SRCDIR << "*.cpp" << int(QDir::Name) << int(QDir::Dirs)
        << false;

    QTest::newRow("diff-sort") << SRCDIR << "*.cpp" << int(QDir::Name) << int(QDir::Files)
        << SRCDIR << "*.cpp" << int(QDir::Time) << int(QDir::Files)
        << false;

    QTest::newRow("diff-namefilters") << SRCDIR << "*.cpp" << int(QDir::Name) << int(QDir::Files)
        << SRCDIR << "*.png" << int(QDir::Name) << int(QDir::Files)
        << false;
}

void tst_QDir::equalityOperator()
{
    QFETCH(QString, leftPath);
    QFETCH(QString, leftNameFilters);
    QFETCH(int, leftSort);
    QFETCH(int, leftFilters);
    QFETCH(QString, rightPath);
    QFETCH(QString, rightNameFilters);
    QFETCH(int, rightSort);
    QFETCH(int, rightFilters);
    QFETCH(bool, expected);

    QDir dir1(leftPath, leftNameFilters, QDir::SortFlags(leftSort), QDir::Filters(leftFilters));
    QDir dir2(rightPath, rightNameFilters, QDir::SortFlags(rightSort), QDir::Filters(rightFilters));

    QCOMPARE((dir1 == dir2), expected);
    QCOMPARE((dir2 == dir1), expected);
    QCOMPARE((dir1 != dir2), !expected);
    QCOMPARE((dir2 != dir1), !expected);
}

void tst_QDir::isRelative_data()
{
    QTest::addColumn<QString>("path");
    QTest::addColumn<bool>("relative");

    QTest::newRow(".") << "./" << true;
    QTest::newRow("..") << "../" << true;
    QTest::newRow("content") << "entrylist/" << true;
    QTest::newRow("current") << QDir::currentPath() << false;
    QTest::newRow("homepath") << QDir::homePath() << false;
    QTest::newRow("temppath") << QDir::tempPath() << false;
    QTest::newRow("rootpath") << QDir::rootPath() << false;
}

void tst_QDir::isRelative()
{
    QFETCH(QString, path);
    QFETCH(bool, relative);

    QCOMPARE(QDir(path).isRelative(), relative);
}

void tst_QDir::isReadable()
{
    if (::getuid() == 0)
        // root and Chuck Norris don't care for file permissions. Skip.
        QSKIP("Running this test as root doesn't make sense", SkipAll);
    QDir dir;

    QVERIFY(dir.isReadable());
    QVERIFY(dir.mkdir("nonreadabledir"));
    QVERIFY(0 == ::chmod("nonreadabledir", 0));
    QVERIFY(!QDir("nonreadabledir").isReadable());
    QVERIFY(0 == ::chmod("nonreadabledir", S_IRUSR | S_IWUSR | S_IXUSR));
    QVERIFY(dir.rmdir("nonreadabledir"));
}

QTEST_MAIN(tst_QDir)

#include "moc_tst_qdir.cpp"
