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

#include "qicon.h"
#include "qicon_p.h"
#include "qiconengine.h"
#include "qiconloader_p.h"
#include "qapplication.h"
#include "qstyleoption.h"
#include "qpainter.h"
#include "qfileinfo.h"
#include "qstyle.h"
#include "qpixmapcache.h"
#include "qvariant.h"
#include "qcache.h"
#include "qdebug.h"
#include "qguiplatformplugin.h"
#include "qguicommon_p.h"
#include "qstylehelper_p.h"

#ifndef QT_NO_ICON
QT_BEGIN_NAMESPACE

/*!
    \enum QIcon::Mode

    This enum type describes the mode for which a pixmap is intended
    to be used. The currently defined modes are:

    \value Normal
         Display the pixmap when the user is
        not interacting with the icon, but the
        functionality represented by the icon is available.
    \value Disabled
         Display the pixmap when the
        functionality represented by the icon is not available.
    \value Active
         Display the pixmap when the
        functionality represented by the icon is available and
        the user is interacting with the icon, for example, moving the
        mouse over it or clicking it.
   \value Selected
        Display the pixmap when the item represented by the icon is
        selected.
*/

/*!
    \enum QIcon::State

    This enum describes the state for which a pixmap is intended to be
    used. The \e state can be:

    \value Off  Display the pixmap when the widget is in an "off" state
    \value On  Display the pixmap when the widget is in an "on" state
*/

static QAtomicInt serialNumCounter(1);

typedef QCache<QString, QIcon> IconCache;
Q_GLOBAL_STATIC(IconCache, qtIconCache)

QIconPrivate::QIconPrivate()
    : engine(nullptr), ref(1),
    serialNum(serialNumCounter.fetchAndAddRelaxed(1)),
    detach_no(0)
{
}

QIconPrivate::~QIconPrivate() {
    delete engine;
}

QPixmapIconEngine::QPixmapIconEngine()
{
}

QPixmapIconEngine::QPixmapIconEngine(const QPixmapIconEngine &other)
    : QIconEngine(other), pixmaps(other.pixmaps)
{
}

QPixmapIconEngine::~QPixmapIconEngine()
{
}

void QPixmapIconEngine::paint(QPainter *painter, const QRect &rect, QIcon::Mode mode, QIcon::State state)
{
    painter->drawPixmap(rect, pixmap(rect.size(), mode, state));
}

static inline int area(const QSize &s) { return s.width() * s.height(); }

// returns the smallest of the two that is still larger than or equal to size.
static QPixmapIconEngineEntry *bestSizeMatch( const QSize &size, QPixmapIconEngineEntry *pa, QPixmapIconEngineEntry *pb)
{
    int s = area(size);
    if (pa->size.isNull() && pa->pixmap.isNull()) {
        pa->pixmap = QPixmap(pa->fileName);
        pa->size = pa->pixmap.size();
    }
    int a = area(pa->size);
    if (pb->size.isNull() && pb->pixmap.isNull()) {
        pb->pixmap = QPixmap(pb->fileName);
        pb->size = pb->pixmap.size();
    }
    int b = area(pb->size);
    int res = a;
    if (qMin(a,b) >= s)
        res = qMin(a,b);
    else
        res = qMax(a,b);
    if (res == a)
        return pa;
    return pb;
}

QPixmapIconEngineEntry *QPixmapIconEngine::tryMatch(const QSize &size, QIcon::Mode mode, QIcon::State state)
{
    QPixmapIconEngineEntry *pe = 0;
    for (int i = 0; i < pixmaps.count(); ++i)
        if (pixmaps.at(i).mode == mode && pixmaps.at(i).state == state) {
            if (pe)
                pe = bestSizeMatch(size, &pixmaps[i], pe);
            else
                pe = &pixmaps[i];
        }
    return pe;
}


QPixmapIconEngineEntry *QPixmapIconEngine::bestMatch(const QSize &size, QIcon::Mode mode, QIcon::State state, bool sizeOnly)
{
    QPixmapIconEngineEntry *pe = tryMatch(size, mode, state);
    while (!pe){
        QIcon::State oppositeState = (state == QIcon::On) ? QIcon::Off : QIcon::On;
        if (mode == QIcon::Disabled || mode == QIcon::Selected) {
            QIcon::Mode oppositeMode = (mode == QIcon::Disabled) ? QIcon::Selected : QIcon::Disabled;
            if ((pe = tryMatch(size, QIcon::Normal, state)))
                break;
            if ((pe = tryMatch(size, QIcon::Active, state)))
                break;
            if ((pe = tryMatch(size, mode, oppositeState)))
                break;
            if ((pe = tryMatch(size, QIcon::Normal, oppositeState)))
                break;
            if ((pe = tryMatch(size, QIcon::Active, oppositeState)))
                break;
            if ((pe = tryMatch(size, oppositeMode, state)))
                break;
            if ((pe = tryMatch(size, oppositeMode, oppositeState)))
                break;
        } else {
            QIcon::Mode oppositeMode = (mode == QIcon::Normal) ? QIcon::Active : QIcon::Normal;
            if ((pe = tryMatch(size, oppositeMode, state)))
                break;
            if ((pe = tryMatch(size, mode, oppositeState)))
                break;
            if ((pe = tryMatch(size, oppositeMode, oppositeState)))
                break;
            if ((pe = tryMatch(size, QIcon::Disabled, state)))
                break;
            if ((pe = tryMatch(size, QIcon::Selected, state)))
                break;
            if ((pe = tryMatch(size, QIcon::Disabled, oppositeState)))
                break;
            if ((pe = tryMatch(size, QIcon::Selected, oppositeState)))
                break;
        }

        if (!pe)
            return pe;
    }

    if (sizeOnly ? (pe->size.isNull() || !pe->size.isValid()) : pe->pixmap.isNull()) {
        pe->pixmap = QPixmap(pe->fileName);
        if (!pe->pixmap.isNull())
            pe->size = pe->pixmap.size();
    }

    return pe;
}

QPixmap QPixmapIconEngine::pixmap(const QSize &size, QIcon::Mode mode, QIcon::State state)
{
    QPixmap pm;
    QPixmapIconEngineEntry *pe = bestMatch(size, mode, state, false);
    if (pe)
        pm = pe->pixmap;

    if (pm.isNull()) {
        int idx = pixmaps.count();
        while (--idx >= 0) {
            if (pe == &pixmaps[idx]) {
                pixmaps.remove(idx);
                break;
            }
        }
        if (pixmaps.isEmpty())
            return pm;
        else
            return pixmap(size, mode, state);
    }

    QSize actualSize = pm.size();
    if (!actualSize.isNull() && (actualSize.width() > size.width() || actualSize.height() > size.height()))
        actualSize.scale(size, Qt::KeepAspectRatio);

    const QByteArray key = qHexString(
        "qt_icon_%lld_%d_%lld_%d_%d",
        pm.cacheKey(),
        static_cast<int>(pe ? pe->mode : QIcon::Normal),
        QApplication::palette().cacheKey(),
        actualSize.width(),
        actualSize.height()
    );
    const QByteArray key2 = key + '_' + static_cast<char>(mode);

    if (mode == QIcon::Active) {
        if (QPixmapCache::find(key2, pm))
            return pm; // horray
        const QByteArray key3 = key + '_' + static_cast<char>(QIcon::Normal);
        if (QPixmapCache::find(key + key3, pm)) {
            QStyleOption opt(0);
            opt.palette = QApplication::palette();
            QPixmap active = QApplication::style()->generatedIconPixmap(QIcon::Active, pm, &opt);
            if (pm.cacheKey() == active.cacheKey())
                return pm;
        }
    }

    if (!QPixmapCache::find(key2, pm)) {
        if (pm.size() != actualSize)
            pm = pm.scaled(actualSize, Qt::IgnoreAspectRatio, Qt::SmoothTransformation);
        if (pe && pe->mode != mode && mode != QIcon::Normal) {
            QStyleOption opt(0);
            opt.palette = QApplication::palette();
            QPixmap generated = QApplication::style()->generatedIconPixmap(mode, pm, &opt);
            if (!generated.isNull())
                pm = generated;
        }
        QPixmapCache::insert(key2, pm);
    }
    return pm;
}

QSize QPixmapIconEngine::actualSize(const QSize &size, QIcon::Mode mode, QIcon::State state)
{
    QSize actualSize;
    if (QPixmapIconEngineEntry *pe = bestMatch(size, mode, state, true))
        actualSize = pe->size;

    if (actualSize.isNull())
        return actualSize;

    if (!actualSize.isNull() && (actualSize.width() > size.width() || actualSize.height() > size.height()))
        actualSize.scale(size, Qt::KeepAspectRatio);
    return actualSize;
}

QList<QSize> QPixmapIconEngine::availableSizes(QIcon::Mode mode, QIcon::State state) const
{
    QList<QSize> result;
    for (int i = 0; i < pixmaps.size(); i++) {
        const QPixmapIconEngineEntry &pe = pixmaps.at(i);
        if (pe.mode == mode && pe.state == state && !pe.size.isEmpty()) {
            result.append(pe.size);
        }
    }
    return result;
}

void QPixmapIconEngine::addPixmap(const QPixmap &pixmap, QIcon::Mode mode, QIcon::State state)
{
    if (!pixmap.isNull()) {
        QPixmapIconEngineEntry *pe = tryMatch(pixmap.size(), mode, state);
        if(pe && pe->size == pixmap.size()) {
            pe->pixmap = pixmap;
            pe->fileName.clear();
        } else {
            pixmaps += QPixmapIconEngineEntry(pixmap, mode, state);
        }
    }
}

void QPixmapIconEngine::addFile(const QString &fileName, const QSize &_size, QIcon::Mode mode, QIcon::State state)
{
    if (!fileName.isEmpty()) {
        QSize size = _size;
        QPixmap pixmap;

        QString abs = QFileInfo(fileName).absoluteFilePath();

        for (int i = 0; i < pixmaps.count(); ++i) {
            if (pixmaps.at(i).mode == mode && pixmaps.at(i).state == state) {
                QPixmapIconEngineEntry *pe = &pixmaps[i];
                if(size.isNull()) {
                    pixmap = QPixmap(abs);
                    size = pixmap.size();
                }
                if (pe->size.isNull() && pe->pixmap.isNull()) {
                    pe->pixmap = QPixmap(pe->fileName);
                    pe->size = pe->pixmap.size();
                }
                if(pe->size == size) {
                    pe->pixmap = pixmap;
                    pe->fileName = abs;
                    return;
                }
            }
        }
        QPixmapIconEngineEntry e(abs, size, mode, state);
        e.pixmap = pixmap;
        pixmaps += e;
    }
}

QString QPixmapIconEngine::key() const
{
    return QLatin1String("QPixmapIconEngine");
}

QIconEngine *QPixmapIconEngine::clone() const
{
    return new QPixmapIconEngine(*this);
}

bool QPixmapIconEngine::read(QDataStream &in)
{
    int num_entries;
    QPixmap pm;
    QString fileName;
    QSize sz;
    uint mode;
    uint state;

    in >> num_entries;
    for (int i=0; i < num_entries; ++i) {
        if (in.atEnd()) {
            pixmaps.clear();
            return false;
        }
        in >> pm;
        in >> fileName;
        in >> sz;
        in >> mode;
        in >> state;
        if (pm.isNull()) {
            addFile(fileName, sz, QIcon::Mode(mode), QIcon::State(state));
        } else {
            QPixmapIconEngineEntry pe(fileName, sz, QIcon::Mode(mode), QIcon::State(state));
            pe.pixmap = pm;
            pixmaps += pe;
        }
    }
    return true;
}

bool QPixmapIconEngine::write(QDataStream &out) const
{
    int num_entries = pixmaps.size();
    out << num_entries;
    for (int i=0; i < num_entries; ++i) {
        if (pixmaps.at(i).pixmap.isNull())
            out << QPixmap(pixmaps.at(i).fileName);
        else
            out << pixmaps.at(i).pixmap;
        out << pixmaps.at(i).fileName;
        out << pixmaps.at(i).size;
        out << (uint) pixmaps.at(i).mode;
        out << (uint) pixmaps.at(i).state;
    }
    return true;
}

/*!
    \class QIcon

    \brief The QIcon class provides scalable icons in different modes
    and states.

    \ingroup painting
    \ingroup shared


    A QIcon can generate smaller, larger, active, and disabled pixmaps
    from the set of pixmaps it is given. Such pixmaps are used by Qt
    widgets to show an icon representing a particular action.

    The simplest use of QIcon is to create one from a QPixmap file or
    resource, and then use it, allowing Qt to work out all the required
    icon styles and sizes. For example:

    \snippet doc/src/snippets/code/src_gui_image_qicon.cpp 0

    To undo a QIcon, simply set a null icon in its place:

    \snippet doc/src/snippets/code/src_gui_image_qicon.cpp 1

    Use the QImageReader::supportedImageFormats() and
    QImageWriter::supportedImageFormats() functions to retrieve a
    complete list of the supported file formats.

    When you retrieve a pixmap using pixmap(QSize, Mode, State), and no
    pixmap for this given size, mode and state has been added with
    addFile() or addPixmap(), then QIcon will generate one on the
    fly. This pixmap generation happens in a QIconEngine. The default
    engine scales pixmaps down if required, but never up, and it uses
    the current style to calculate a disabled appearance. By using
    custom icon engines, you can customize every aspect of generated
    icons.

    \note Since Qt 4.2, an icon engine that supports SVG is included.

    \section1 Making Classes that Use QIcon

    If you write your own widgets that have an option to set a small
    pixmap, consider allowing a QIcon to be set for that pixmap.  The
    Qt class QToolButton is an example of such a widget.

    Provide a method to set a QIcon, and when you draw the icon, choose
    whichever pixmap is appropriate for the current state of your widget.
    For example:
    \snippet doc/src/snippets/code/src_gui_image_qicon.cpp 2

    You might also make use of the \c Active mode, perhaps making your
    widget \c Active when the mouse is over the widget (see \l
    QWidget::enterEvent()), while the mouse is pressed pending the
    release that will activate the function, or when it is the currently
    selected item. If the widget can be toggled, the "On" mode might be
    used to draw a different icon.

    \img icon.png QIcon

    \sa {fowler}{GUI Design Handbook: Iconic Label}, {Icons Example}
*/


/*!
    Constructs a null icon.
*/
QIcon::QIcon()
    : d(nullptr)
{
}

/*!
    Constructs an icon from a \a pixmap.
*/
QIcon::QIcon(const QPixmap &pixmap)
    : d(nullptr)
{
    addPixmap(pixmap);
}

/*!
    Constructs a copy of \a other. This is very fast.
*/
QIcon::QIcon(const QIcon &other)
    : d(other.d)
{
    if (d)
        d->ref.ref();
}

/*!
    Constructs an icon from the file with the given \a fileName. The
    file will be loaded on demand.

    If \a fileName contains a relative path (e.g. the filename only)
    the relevant file must be found relative to the runtime working
    directory.

    The file name refers to an actual file on disk.

    Use the QImageReader::supportedImageFormats() and
    QImageWriter::supportedImageFormats() functions to retrieve a
    complete list of the supported file formats.
*/
QIcon::QIcon(const QString &fileName)
    : d(nullptr)
{
    addFile(fileName);
}


/*!
    Creates an icon with a specific icon \a engine. The icon takes
    ownership of the engine.
*/
QIcon::QIcon(QIconEngine *engine)
    : d(new QIconPrivate())
{
    d->engine = engine;
}

/*!
    Destroys the icon.
*/
QIcon::~QIcon()
{
    if (d && !d->ref.deref())
        delete d;
}

/*!
    Assigns the \a other icon to this icon and returns a reference to
    this icon.
*/
QIcon &QIcon::operator=(const QIcon &other)
{
    qAtomicAssign(d, other.d);
    return *this;
}

/*!
    \fn void QIcon::swap(QIcon &other)
    \since 4.8

    Swaps icon \a other with this icon. This operation is very
    fast and never fails.
*/

/*!
    Returns the icon as a QVariant.
*/
QIcon::operator QVariant() const
{
    return QVariant(QVariant::Icon, this);
}

/*!
    Returns a number that identifies the contents of this QIcon
    object. Distinct QIcon objects can have the same key if
    they refer to the same contents.
    \since 4.3

    The cacheKey() will change when the icon is altered via
    addPixmap() or addFile().

    Cache keys are mostly useful in conjunction with caching.

    \sa QPixmap::cacheKey()
*/
qint64 QIcon::cacheKey() const
{
    if (!d)
        return 0;
    return (((qint64) d->serialNum) << 32) | ((qint64) (d->detach_no));
}

/*!
    Returns a pixmap with the requested \a size, \a mode, and \a
    state, generating one if necessary. The pixmap might be smaller than
    requested, but never larger.

    \sa actualSize(), paint()
*/
QPixmap QIcon::pixmap(const QSize &size, Mode mode, State state) const
{
    if (!d)
        return QPixmap();
    return d->engine->pixmap(size, mode, state);
}

/*!
    \fn QPixmap QIcon::pixmap(int w, int h, Mode mode = Normal, State state = Off) const

    \overload

    Returns a pixmap of size QSize(\a w, \a h). The pixmap might be smaller than
    requested, but never larger.
*/

/*!
    \fn QPixmap QIcon::pixmap(int extent, Mode mode = Normal, State state = Off) const

    \overload

    Returns a pixmap of size QSize(\a extent, \a extent). The pixmap might be smaller
    than requested, but never larger.
*/

/*!
    Returns the actual size of the icon for the requested \a size, \a
    mode, and \a state. The result might be smaller than requested, but
    never larger.

    \sa pixmap(), paint()
*/
QSize QIcon::actualSize(const QSize &size, Mode mode, State state) const
{
    if (!d)
        return QSize();
    return d->engine->actualSize(size, mode, state);
}


/*!
    Uses the \a painter to paint the icon with specified \a alignment,
    required \a mode, and \a state into the rectangle \a rect.

    \sa actualSize(), pixmap()
*/
void QIcon::paint(QPainter *painter, const QRect &rect, Qt::Alignment alignment, Mode mode, State state) const
{
    if (!d || !painter)
        return;
    QRect alignedRect = QStyle::alignedRect(painter->layoutDirection(), alignment, d->engine->actualSize(rect.size(), mode, state), rect);
    d->engine->paint(painter, alignedRect, mode, state);
}

/*!
    \fn void QIcon::paint(QPainter *painter, int x, int y, int w, int h, Qt::Alignment alignment,
                          Mode mode, State state) const

    \overload

    Paints the icon into the rectangle QRect(\a x, \a y, \a w, \a h).
*/

/*!
    Returns true if the icon is empty; otherwise returns false.

    An icon is empty if it has neither a pixmap nor a filename.

    Note: Even a non-null icon might not be able to create valid
    pixmaps, eg. if the file does not exist or cannot be read.
*/
bool QIcon::isNull() const
{
    return !d;
}

/*!
    \internal
 */
void QIcon::detach()
{
    if (d) {
        if (d->ref != 1) {
            QIconPrivate *x = new QIconPrivate();
            x->engine = d->engine->clone();
            if (!d->ref.deref())
                delete d;
            d = x;
        }
        ++d->detach_no;
    }
}

/*!
    Adds \a pixmap to the icon, as a specialization for \a mode and
    \a state.

    Custom icon engines are free to ignore additionally added
    pixmaps.

    \sa addFile()
*/
void QIcon::addPixmap(const QPixmap &pixmap, Mode mode, State state)
{
    if (pixmap.isNull())
        return;
    if (!d) {
        d = new QIconPrivate();
        d->engine = new QPixmapIconEngine();
    } else {
        detach();
    }
    d->engine->addPixmap(pixmap, mode, state);
}


/*!
    Adds an image from the file with the given \a fileName to the
    icon, as a specialization for \a size, \a mode and \a state. The
    file will be loaded on demand. Note: custom icon engines are free
    to ignore additionally added pixmaps.

    If \a fileName contains a relative path (e.g. the filename only)
    the relevant file must be found relative to the runtime working
    directory.

    The file name can be either refer to an actual file on disk or to
    one of the application's embedded resources. See the
    \l{resources.html}{Resource System} overview for details on how to
    embed images and other resource files in the application's
    executable.

    Use the QImageReader::supportedImageFormats() and
    QImageWriter::supportedImageFormats() functions to retrieve a
    complete list of the supported file formats.

    Note: When you add a non-empty filename to a QIcon, the icon becomes
    non-null, even if the file doesn't exist or points to a corrupt file.

    \sa addPixmap()
 */
void QIcon::addFile(const QString &fileName, const QSize &size, Mode mode, State state)
{
    if (fileName.isEmpty())
        return;
    if (!d) {
        d = new QIconPrivate();
        d->engine = new QPixmapIconEngine();
    } else {
        detach();
    }
    d->engine->addFile(fileName, size, mode, state);
}

/*!
    \since 4.5

    Returns a list of available icon sizes for the specified \a mode and
    \a state.
*/
QList<QSize> QIcon::availableSizes(Mode mode, State state) const
{
    if (!d || !d->engine)
        return QList<QSize>();
    return d->engine->availableSizes(mode, state);
}

/*!
    \since 4.7

    Returns the name used to create the icon, if available.

    Depending on the way the icon was created, it may have an associated
    name. This is the case for icons created with fromTheme().

    \sa fromTheme(), QIconEngine
*/
QString QIcon::name() const
{
    if (!d || !d->engine)
        return QString();
    return d->engine->iconName();
}

/*!
    \since 4.6

    Sets the search paths for icon themes to \a paths.
    \sa themeSearchPaths(), fromTheme(), setThemeName()
*/
void QIcon::setThemeSearchPaths(const QStringList &paths)
{
    QIconLoader::instance()->setThemeSearchPath(paths);
}

/*!
    \since 4.6

    Returns the search paths for icon themes.

    The default value will depend on the platform:

    On X11, the search path will use the XDG_DATA_DIRS environment
    variable if available.

    \sa setThemeSearchPaths(), fromTheme(), setThemeName()
*/
QStringList QIcon::themeSearchPaths()
{
    return QIconLoader::instance()->themeSearchPaths();
}

/*!
    \since 4.6

    Sets the current icon theme to \a name.

    The \a name should correspond to a directory name in the
    themeSearchPath() containing an index.theme
    file describing it's contents.

    \sa themeSearchPaths(), themeName()
*/
void QIcon::setThemeName(const QString &name)
{
    QIconLoader::instance()->setThemeName(name);
}

/*!
    \since 4.6

    Returns the name of the current icon theme.

    On X11, the current icon theme depends on your desktop
    settings. On other platforms it is not set by default.

    \sa setThemeName(), themeSearchPaths(), fromTheme(),
    hasThemeIcon()
*/
QString QIcon::themeName()
{
    return QIconLoader::instance()->themeName();
}

/*!
    \since 4.6

    Returns the QIcon corresponding to \a name in the current
    icon theme. If no such icon is found in the current theme
    \a fallback is returned instead.

    The latest version of the freedesktop icon specification and naming
    specification can be obtained here:

    \list
    \o \l{http://standards.freedesktop.org/icon-theme-spec/icon-theme-spec-latest.html}
    \o \l{http://standards.freedesktop.org/icon-naming-spec/icon-naming-spec-latest.html}
    \endlist

    To fetch an icon from the current icon theme:

    \snippet doc/src/snippets/code/src_gui_image_qicon.cpp 3

    Or if you want to provide a guaranteed fallback for platforms that
    do not support theme icons, you can use the second argument:

    \snippet doc/src/snippets/code/src_gui_image_qicon.cpp 4

    \sa themeName(), setThemeName(), themeSearchPaths()
*/
QIcon QIcon::fromTheme(const QString &name, const QIcon &fallback)
{
    QIcon icon = qt_guiPlatformPlugin()->systemIcon(name);
    if (!icon.isNull()) {
        return icon;
    }

    QIcon *cachedIcon = qtIconCache()->object(name);
    if (!cachedIcon) {
        cachedIcon = new QIcon(new QIconLoaderEngine(name));
        qtIconCache()->insert(name, cachedIcon);
    }
    icon = *cachedIcon;

    // Note the qapp check is to allow lazy loading of static icons
    // Supporting fallbacks will not work for this case.
    if (qApp && icon.availableSizes().isEmpty())
        return fallback;

    return icon;
}

/*!
    \since 4.6

    Returns true if there is an icon available for \a name in the
    current icon theme, otherwise returns false.

    \sa themeSearchPaths(), fromTheme(), setThemeName()
*/
bool QIcon::hasThemeIcon(const QString &name)
{
    return !fromTheme(name).isNull();
}


/*****************************************************************************
  QIcon stream functions
 *****************************************************************************/
#if !defined(QT_NO_DATASTREAM)
/*!
    \fn QDataStream &operator<<(QDataStream &stream, const QIcon &icon)
    \relates QIcon
    \since 4.2

    Writes the given \a icon to the given \a stream as a PNG
    image. If the icon contains more than one image, all images will
    be written to the stream. Note that writing the stream to a file
    will not produce a valid image file.
*/

QDataStream &operator<<(QDataStream &s, const QIcon &icon)
{
    if (icon.isNull()) {
        s << QString();
    } else {
        QIconEngine *engine = icon.d->engine;
        s << engine->key();
        engine->write(s);
    }
    return s;
}

/*!
    \fn QDataStream &operator>>(QDataStream &stream, QIcon &icon)
    \relates QIcon
    \since 4.2

    Reads an image, or a set of images, from the given \a stream into
    the given \a icon.
*/

QDataStream &operator>>(QDataStream &s, QIcon &icon)
{
    icon = QIcon();
    QString key;
    s >> key;
    if (key == QLatin1String("QPixmapIconEngine")) {
        icon.d = new QIconPrivate();
        QIconEngine *engine = new QPixmapIconEngine();
        icon.d->engine = engine;
        engine->read(s);
    } else if (key == QLatin1String("QIconLoaderEngine")) {
        icon.d = new QIconPrivate();
        QIconEngine *engine = new QIconLoaderEngine();
        icon.d->engine = engine;
        engine->read(s);
    }
    return s;
}

#endif //QT_NO_DATASTREAM

QT_END_NAMESPACE

#endif //QT_NO_ICON
