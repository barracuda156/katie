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

#include "qfont.h"
#include "qdebug.h"
#include "qpaintdevice.h"
#include "qfontdatabase_p.h"
#include "qfontmetrics.h"
#include "qhash.h"
#include "qdatastream.h"
#include "qapplication.h"
#include "qstringlist.h"
#include "qunicodetables_p.h"
#include "qfont_p.h"
#include "qfontengine_p.h"
#include "qcorecommon_p.h"

#include <limits.h>

#ifdef Q_WS_X11
#include "qx11info_x11.h"
#include "qt_x11_p.h"
#endif

// #define QFONTCACHE_DEBUG
#ifdef QFONTCACHE_DEBUG
#  define FC_DEBUG qDebug
#else
#  define FC_DEBUG if (false) qDebug
#endif

QT_BEGIN_NAMESPACE

#ifdef Q_WS_X11
extern const QX11Info *qt_x11Info(const QPaintDevice *pd);
#endif

bool QFontDef::exactMatch(const QFontDef &other) const
{
    /*
      QFontDef comparison is more complicated than just simple
      per-member comparisons.

      When comparing point/pixel sizes, either point or pixelsize
      could be -1.  in This case we have to compare the non negative
      size value.

      This test will fail if the point-sizes differ by 1/2 point or
      more or they do not round to the same value.  We have to do this
      since our API still uses 'int' point-sizes in the API, but store
      deci-point-sizes internally.

      To compare the family members, we need to parse the font names
      and compare the family/foundry strings separately.  This allows
      us to compare e.g. "FreeSans" and "FreeSans [GNU]" with
      positive results.
    */
    if (pixelSize != -1 && other.pixelSize != -1) {
        if (pixelSize != other.pixelSize)
            return false;
    } else if (pointSize != -1 && other.pointSize != -1) {
        if (pointSize != other.pointSize)
            return false;
    } else {
        return false;
    }

    if (!ignorePitch && !other.ignorePitch && fixedPitch != other.fixedPitch)
        return false;

    if (stretch != 0 && other.stretch != 0 && stretch != other.stretch)
        return false;

    QString this_family, this_foundry, other_family, other_foundry;
    QFontDatabasePrivate::parseFontName(family, this_foundry, this_family);
    QFontDatabasePrivate::parseFontName(other.family, other_foundry, other_family);

    return (hintingPreference == other.hintingPreference
            && weight        == other.weight
            && style        == other.style
            && this_family   == other_family
            && (styleName.isEmpty() || other.styleName.isEmpty() || styleName == other.styleName)
            && (this_foundry.isEmpty()
                || other_foundry.isEmpty()
                || this_foundry == other_foundry)
       );
}

QFontPrivate::QFontPrivate()
    : dpi(QX11Info::appDpiY()), screen(QX11Info::appScreen()),
      underline(false), overline(false), strikeOut(false), kerning(true)
{
}

QFontPrivate::QFontPrivate(const QFontPrivate &other)
    : request(other.request), dpi(other.dpi), screen(other.screen),
      underline(other.underline), overline(other.overline),
      strikeOut(other.strikeOut), kerning(other.kerning)
{
}

QFontPrivate::~QFontPrivate()
{
}

QFontEngine *QFontPrivate::engineForScript(QUnicodeTables::Script script) const
{
    if (script > QUnicodeTables::ScriptCount)
        script = QUnicodeTables::Common;
    return QFontDatabasePrivate::load(this, script);
}

void QFontPrivate::resolve(uint mask, const QFontPrivate *other)
{
    Q_ASSERT(other != 0);

    dpi = other->dpi;

    if ((mask & QFont::AllPropertiesResolved) == QFont::AllPropertiesResolved) return;

    // assign the unset-bits with the set-bits of the other font def
    if (! (mask & QFont::FamilyResolved))
        request.family = other->request.family;

    if (! (mask & QFont::StyleNameResolved))
        request.styleName = other->request.styleName;

    if (! (mask & QFont::SizeResolved)) {
        request.pointSize = other->request.pointSize;
        request.pixelSize = other->request.pixelSize;
    }

    if (! (mask & QFont::WeightResolved))
        request.weight = other->request.weight;

    if (! (mask & QFont::StyleResolved))
        request.style = other->request.style;

    if (! (mask & QFont::FixedPitchResolved))
        request.fixedPitch = other->request.fixedPitch;

    if (! (mask & QFont::StretchResolved))
        request.stretch = other->request.stretch;

    if (! (mask & QFont::HintingPreferenceResolved))
        request.hintingPreference = other->request.hintingPreference;

    if (! (mask & QFont::UnderlineResolved))
        underline = other->underline;

    if (! (mask & QFont::OverlineResolved))
        overline = other->overline;

    if (! (mask & QFont::StrikeOutResolved))
        strikeOut = other->strikeOut;

    if (! (mask & QFont::KerningResolved))
        kerning = other->kerning;
}

/*!
    \class QFont
    \reentrant

    \brief The QFont class specifies a font used for drawing text.

    \ingroup painting
    \ingroup appearance
    \ingroup shared
    \ingroup richtext-processing


    When you create a QFont object you specify various attributes that
    you want the font to have. Qt will use the font with the specified
    attributes, or if no matching font exists, Qt will use the closest
    matching installed font. The attributes of the font that is
    actually used are retrievable from a QFontDatabase object. If the
    window system provides an exact match exactMatch() returns true.
    Use QFontMetrics to get measurements, e.g. the pixel length of a
    string using QFontMetrics::width().

    Note that a QApplication instance must exist before a QFont can be
    used. You can set the application's default font with
    QApplication::setFont().

    If a chosen font does not include all the characters that
    need to be displayed, QFont will try to find the characters in the
    nearest equivalent fonts. When a QPainter draws a character from a
    font the QFont will report whether or not it has the character; if
    it does not, QPainter will draw an unfilled square.

    Create QFonts like this:

    \snippet doc/src/snippets/code/src_gui_text_qfont.cpp 0

    The attributes set in the constructor can also be set later, e.g.
    setFamily(), setPointSize(), setPointSizeF(), setWeight() and
    setItalic(). The remaining attributes must be set after
    contstruction, e.g. setBold(), setUnderline(), setOverline(),
    setStrikeOut() and setFixedPitch().

    The font-matching algorithm has a lastResortFamily() in cases
    where a suitable match cannot be found.

    Every QFont has a key() which you can use, for example, as the key
    in a cache or dictionary. If you want to store a user's font
    preferences you could use QSettings, writing the font information
    with toString() and reading it back with fromString(). The
    operator<<() and operator>>() functions are also available, but
    they work on a data stream.

    It is possible to set the height of characters shown on the screen
    to a specified number of pixels with setPixelSize(); however using
    setPointSize() has a similar effect and provides device
    independence.

    Loading fonts can be expensive, especially on X11. QFont contains
    extensive optimizations to make the copying of QFont objects fast,
    and to cache the results of the slow window system functions it
    depends upon.

    \target fontmatching
    The font matching algorithm works as follows:
    \list 1
    \o The specified font family is searched for.
    \o Each replacement font family is searched for.
    \o If none of these are found application font will be searched for.
    \o If none of these are found lastResortFamily() will be searched for.
    \o If the lastResortFamily() isn't found Katie will not abort but no
       text will be visible unless a valid font filepath has been set in
       the global settings file or via QApplication::setFont().
    \endlist

    Note that the actual font matching algorithm varies from platform to
    platform.

    Once a font is found, the remaining attributes are matched in order
    of priority:
    \list 1
    \o fixedPitch()
    \o pointSize() (see below)
    \o weight()
    \o style()
    \endlist

    If you have a font which matches on family, even if none of the
    other attributes match, this font will be chosen in preference to
    a font which doesn't match on family but which does match on the
    other attributes. This is because font family is the dominant
    search criteria.

    The point size is defined to match if it is within 20% of the
    requested point size. When several fonts match and are only
    distinguished by point size, the font with the closest point size
    to the one requested will be chosen.

    The actual family, font size, weight and other font attributes
    used for drawing text will depend on what's available for the
    chosen family under the window system. A QFontDatabase object can
    be used to determine the actual values used for drawing the text.
    If you have both an Adobe and a Cronyx Helvetica for example, you
    might get either. To find out font metrics use QFontMetrics.

    For more general information on fonts, see the
    \link http://nwalsh.com/comp.fonts/FAQ/ comp.fonts FAQ.\endlink
    Information on encodings can be found from
    \link http://czyborra.com/ Roman Czyborra's\endlink page.

    \sa QFontComboBox, QFontMetrics, QFontDatabase, {Character Map Example}
*/

/*!
    \internal
    \enum QFont::ResolveProperties

    This enum describes the properties of a QFont that can be set on a font
    individually and then considered resolved.

    \value FamilyResolved
    \value SizeResolved
    \value WeightResolved
    \value StyleResolved
    \value UnderlineResolved
    \value OverlineResolved
    \value StrikeOutResolved
    \value FixedPitchResolved
    \value StretchResolved
    \value KerningResolved
    \value AllPropertiesResolved
*/

/*!
    \enum QFont::Style

    This enum describes the different styles of glyphs that are used to
    display text.

    \value StyleNormal  Normal glyphs used in unstyled text.
    \value StyleItalic  Italic glyphs that are specifically designed for
                        the purpose of representing italicized text.
    \value StyleOblique Glyphs with an italic appearance that are typically
                        based on the unstyled glyphs, but are not fine-tuned
                        for the purpose of representing italicized text.

    \sa Weight
*/

/*!
    \fn FT_Face QFont::freetypeFace() const

    Returns the handle to the primary FreeType face of the font. If font merging is not disabled a
    QFont can contain several physical fonts.

    Returns 0 if the font does not contain a FreeType face.

    \note This function is only available on platforms that provide the FreeType library;
    i.e., X11 and some Embedded Linux platforms.
*/

/*!
    Constructs a font from \a font for use on the paint device \a pd.
*/
QFont::QFont(const QFont &font, QPaintDevice *pd)
    : resolve_mask(font.resolve_mask)
{
    Q_ASSERT(pd != 0);
    int dpi = pd->logicalDpiY();
#ifdef Q_WS_X11
    const QX11Info *info = qt_x11Info(pd);
    int screen = info ? info->screen() : 0;
#else
    const int screen = 0;
#endif
    if (font.d->dpi != dpi || font.d->screen != screen ) {
        d = new QFontPrivate(*font.d);
        d->dpi = dpi;
        d->screen = screen;
    } else {
        d = font.d.data();
    }
}

/*!
    \internal
*/
QFont::QFont(QFontPrivate *data)
    : d(data), resolve_mask(QFont::AllPropertiesResolved)
{
}

/*!
    \internal

    Detaches the font object from common font data.
*/
void QFont::detach()
{
    if (d->ref == 1) {
        return;
    }

    d.detach();
}

/*!
    Constructs a font object that uses the application's default font.

    \sa QApplication::setFont(), QApplication::font()
*/
QFont::QFont()
    : d(QApplication::font().d.data()), resolve_mask(0)
{
}

/*!
    Constructs a font object with the specified \a family, \a
    pointSize, \a weight and \a italic settings.

    If \a pointSize is zero or negative, the point size of the font
    is set to a system-dependent default value. Generally, this is
    12 points, except on Symbian where it is 7 points.

    The \a family name may optionally also include a foundry name,
    e.g. "FreeSans [GNU]". If the \a family is available from more
    than one foundry and the foundry isn't specified, an arbitrary
    foundry is chosen. If the family isn't available a family will
    be set using the \l{QFont}{font matching} algorithm.

    \sa Weight, setFamily(), setPointSize(), setWeight(), setItalic(),
    QApplication::font()
*/
QFont::QFont(const QString &family, int pointSize, int weight, bool italic)
    : d(new QFontPrivate()), resolve_mask(QFont::FamilyResolved)
{
    if (pointSize <= 0) {
        pointSize = 12;
    } else {
        resolve_mask |= QFont::SizeResolved;
    }

    if (weight < 0) {
        weight = QFont::Normal;
    } else {
        resolve_mask |= QFont::WeightResolved | QFont::StyleResolved;
    }

    if (italic) {
        resolve_mask |= QFont::StyleResolved;
    }

    d->request.family = family;
    d->request.pointSize = qreal(pointSize);
    d->request.pixelSize = -1;
    d->request.weight = weight;
    d->request.style = italic ? QFont::StyleItalic : QFont::StyleNormal;
}

/*!
    Constructs a font that is a copy of \a font.
*/
QFont::QFont(const QFont &font)
    : d(font.d.data()), resolve_mask(font.resolve_mask)
{
}

/*!
    Destroys the font object and frees all allocated resources.
*/
QFont::~QFont()
{
}

/*!
    Assigns \a font to this font and returns a reference to it.
*/
QFont &QFont::operator=(const QFont &font)
{
    d = font.d.data();
    resolve_mask = font.resolve_mask;
    return *this;
}

/*!
    Returns the requested font family name, i.e. the name set in the
    constructor or the last setFont() call.

    \sa setFamily()
*/
QString QFont::family() const
{
    return d->request.family;
}

/*!
    Sets the family name of the font. The name is case insensitive and
    may include a foundry name.

    The \a family name may optionally also include a foundry name,
    e.g. "FreeSans [GNU]". If the \a family is available from more
    than one foundry and the foundry isn't specified, an arbitrary
    foundry is chosen. If the family isn't available a family will be
    set using the \l{QFont}{font matching} algorithm.

    \sa family(), QFontDatabase
*/
void QFont::setFamily(const QString &family)
{
    detach();

    d->request.family = family;

    resolve_mask |= QFont::FamilyResolved;
}

/*!
    \since 4.8

    Returns the requested font style name, it will be used to match the
    font with irregular styles (that can't be normalized in other style
    properties). It depends on system font support, thus only works for
    Mac OS X and X11 so far. On Windows irregular styles will be added
    as separate font families so there is no need for this.

    \sa setFamily() setStyle()
*/
QString QFont::styleName() const
{
    return d->request.styleName;
}

/*!
    \since 4.8

    Sets the style name of the font to the given \a styleName.
    When set, other style properties like style() and weight() will be ignored
    for font matching.

    \sa styleName()
*/
void QFont::setStyleName(const QString &styleName)
{
    detach();

    d->request.styleName = styleName;
    resolve_mask |= QFont::StyleNameResolved;
}

/*!
    Returns the point size of the font. Returns -1 if the font size
    was specified in pixels.

    \sa setPointSize() pointSizeF()
*/
int QFont::pointSize() const
{
    return qRound(d->request.pointSize);
}

/*!
    \since 4.8

    \enum QFont::HintingPreference

    This enum describes the different levels of hinting that can be applied
    to glyphs to improve legibility on displays where it might be warranted
    by the density of pixels.

    \value PreferDefaultHinting Use the default hinting level for the target platform.
    \value PreferNoHinting If possible, render text without hinting the outlines
           of the glyphs. The text layout will be typographically accurate and
           scalable, using the same metrics as are used e.g. when printing.
    \value PreferVerticalHinting If possible, render text with no horizontal hinting,
           but align glyphs to the pixel grid in the vertical direction. The text will appear
           crisper on displays where the density is too low to give an accurate rendering
           of the glyphs. But since the horizontal metrics of the glyphs are unhinted, the text's
           layout will be scalable to higher density devices (such as printers) without impacting
           details such as line breaks.
    \value PreferFullHinting If possible, render text with hinting in both horizontal and
           vertical directions. The text will be altered to optimize legibility on the target
           device, but since the metrics will depend on the target size of the text, the positions
           of glyphs, line breaks, and other typographical detail will not scale, meaning that a
           text layout may look different on devices with different pixel densities.

    Please note that this enum only describes a preference, as the full range of hinting levels
    are not supported on all of Qt's supported platforms. The following table details the effect
    of a given hinting preference on a selected set of target platforms.

    \table
    \header
    \o
    \o PreferDefaultHinting
    \o PreferNoHinting
    \o PreferVerticalHinting
    \o PreferFullHinting
    \row
    \o Operating System setting
    \o No hinting
    \o Vertical hinting (light)
    \o Full hinting
    \endtable

*/

/*!
    \since 4.8

    Set the preference for the hinting level of the glyphs to \a hintingPreference. This is a hint
    to the underlying font rendering system to use a certain level of hinting, and has varying
    support across platforms. See the table in the documentation for QFont::HintingPreference for
    more details.

    The default hinting preference is QFont::PreferDefaultHinting.
*/
void QFont::setHintingPreference(HintingPreference hintingPreference)
{
    detach();

    d->request.hintingPreference = hintingPreference;

    resolve_mask |= QFont::HintingPreferenceResolved;
}

/*!
    \since 4.8

    Returns the currently preferred hinting level for glyphs rendered with this font.
*/
QFont::HintingPreference QFont::hintingPreference() const
{
    return d->request.hintingPreference;
}

/*!
    Sets the point size to \a pointSize. The point size must be
    greater than zero.

    \sa pointSize() setPointSizeF()
*/
void QFont::setPointSize(int pointSize)
{
    if (Q_UNLIKELY(pointSize <= 0)) {
        qWarning("QFont::setPointSize: Point size <= 0 (%d), must be greater than 0", pointSize);
        return;
    }

    detach();

    d->request.pointSize = qreal(pointSize);
    d->request.pixelSize = -1;

    resolve_mask |= QFont::SizeResolved;
}

/*!
    Sets the point size to \a pointSize. The point size must be
    greater than zero. The requested precision may not be achieved on
    all platforms.

    \sa pointSizeF() setPointSize() setPixelSize()
*/
void QFont::setPointSizeF(qreal pointSize)
{
    if (Q_UNLIKELY(pointSize <= 0)) {
        qWarning("QFont::setPointSizeF: Point size <= 0 (%f), must be greater than 0", pointSize);
        return;
    }

    detach();

    d->request.pointSize = pointSize;
    d->request.pixelSize = -1;

    resolve_mask |= QFont::SizeResolved;
}

/*!
    Returns the point size of the font. Returns -1 if the font size was
    specified in pixels.

    \sa pointSize(), setPointSizeF(), pixelSize()
*/
qreal QFont::pointSizeF() const
{
    return d->request.pointSize;
}

/*!
    Sets the font size to \a pixelSize pixels.

    Using this function makes the font device dependent. Use
    setPointSize() or setPointSizeF() to set the size of the font
    in a device independent manner.

    \sa pixelSize()
*/
void QFont::setPixelSize(int pixelSize)
{
    if (Q_UNLIKELY(pixelSize <= 0)) {
        qWarning("QFont::setPixelSize: Pixel size <= 0 (%d)", pixelSize);
        return;
    }

    detach();

    d->request.pixelSize = pixelSize;
    d->request.pointSize = -1;

    resolve_mask |= QFont::SizeResolved;
}

/*!
    Returns the pixel size of the font if it was set with
    setPixelSize(). Returns -1 if the size was set with setPointSize()
    or setPointSizeF().

    \sa setPixelSize(), pointSize()
*/
int QFont::pixelSize() const
{
    return d->request.pixelSize;
}

/*!
    \fn bool QFont::italic() const

    Returns true if the style() of the font is not QFont::StyleNormal

    \sa setItalic(), style()
*/

/*!
  \fn void QFont::setItalic(bool enable)

  Sets the style() of the font to QFont::StyleItalic if \a enable is true;
  otherwise the style is set to QFont::StyleNormal.

  \sa italic()
*/

/*!
    Returns the style of the font.

    \sa setStyle()
*/
QFont::Style QFont::style() const
{
    return d->request.style;
}

/*!
    Sets the style of the font to \a style.

    \sa italic()
*/
void QFont::setStyle(Style style)
{
    detach();

    d->request.style = style;
    resolve_mask |= QFont::StyleResolved;
}

/*!
    Returns the weight of the font which is one of the enumerated
    values from \l{QFont::Weight}.

    \sa setWeight(), Weight
*/
int QFont::weight() const
{
    return d->request.weight;
}

/*!
    \enum QFont::Weight

    Qt uses a weighting scale from 0 to 99 similar to, but not the
    same as, the scales used in Windows or CSS. A weight of 0 is
    ultralight, whilst 99 will be an extremely black.

    This enum contains the predefined font weights:

    \value Light 25
    \value Normal 50
    \value DemiBold 63
    \value Bold 75
    \value Black 87
*/

/*!
    Sets the weight the font to \a weight, which should be a value
    from the \l QFont::Weight enumeration.

    \sa weight()
*/
void QFont::setWeight(int weight)
{
    Q_ASSERT_X(weight >= 0 && weight <= 99, "QFont::setWeight", "Weight must be between 0 and 99");

    detach();

    d->request.weight = weight;
    resolve_mask |= QFont::WeightResolved;
}

/*!
    \fn bool QFont::bold() const

    Returns true if weight() is a value greater than \link Weight
    QFont::Normal \endlink; otherwise returns false.

    \sa weight(), setBold()
*/

/*!
    \fn void QFont::setBold(bool enable)

    If \a enable is true sets the font's weight to \link Weight
    QFont::Bold \endlink; otherwise sets the weight to \link Weight
    QFont::Normal\endlink.

    For finer boldness control use setWeight().

    \sa bold(), setWeight()
*/

/*!
    Returns true if underline has been set; otherwise returns false.

    \sa setUnderline()
*/
bool QFont::underline() const
{
    return d->underline;
}

/*!
    If \a enable is true, sets underline on; otherwise sets underline
    off.

    \sa underline()
*/
void QFont::setUnderline(bool enable)
{
    detach();

    d->underline = enable;
    resolve_mask |= QFont::UnderlineResolved;
}

/*!
    Returns true if overline has been set; otherwise returns false.

    \sa setOverline()
*/
bool QFont::overline() const
{
    return d->overline;
}

/*!
    If \a enable is true, sets overline on; otherwise sets overline off.

    \sa overline()
*/
void QFont::setOverline(bool enable)
{
    detach();

    d->overline = enable;
    resolve_mask |= QFont::OverlineResolved;
}

/*!
    Returns true if strikeout has been set; otherwise returns false.

    \sa setStrikeOut()
*/
bool QFont::strikeOut() const
{
    return d->strikeOut;
}

/*!
    If \a enable is true, sets strikeout on; otherwise sets strikeout
    off.

    \sa strikeOut()
*/
void QFont::setStrikeOut(bool enable)
{
    detach();

    d->strikeOut = enable;
    resolve_mask |= QFont::StrikeOutResolved;
}

/*!
    Returns true if fixed pitch has been set; otherwise returns false.

    \sa setFixedPitch()
*/
bool QFont::fixedPitch() const
{
    return d->request.fixedPitch;
}

/*!
    If \a enable is true, sets fixed pitch on; otherwise sets fixed
    pitch off.

    \sa fixedPitch()
*/
void QFont::setFixedPitch(bool enable)
{
    detach();

    d->request.fixedPitch = enable;
    d->request.ignorePitch = false;
    resolve_mask |= QFont::FixedPitchResolved;
}

/*!
  Returns true if kerning should be used when drawing text with this font.

  \sa setKerning()
*/
bool QFont::kerning() const
{
    return d->kerning;
}

/*!
    Enables kerning for this font if \a enable is true; otherwise
    disables it. By default, kerning is enabled.

    When kerning is enabled, glyph metrics do not add up anymore,
    even for Latin text. In other words, the assumption that
    width('a') + width('b') is equal to width("ab") is not
    neccesairly true.

    \sa kerning(), QFontMetrics
*/
void QFont::setKerning(bool enable)
{
    detach();
    d->kerning = enable;
    resolve_mask |= QFont::KerningResolved;
}

/*!
    \enum QFont::Stretch

    Predefined stretch values that follow the CSS naming convention. The higher
    the value, the more stretched the text is.

    \value UltraCondensed 50
    \value ExtraCondensed 62
    \value Condensed 75
    \value SemiCondensed 87
    \value Unstretched 100
    \value SemiExpanded 112
    \value Expanded 125
    \value ExtraExpanded 150
    \value UltraExpanded 200

    \sa setStretch() stretch()
*/

/*!
    Returns the stretch factor for the font.

    \sa setStretch()
 */
int QFont::stretch() const
{
    return d->request.stretch;
}

/*!
    Sets the stretch factor for the font.

    The stretch factor changes the width of all characters in the font
    by \a factor percent.  For example, setting \a factor to 150
    results in all characters in the font being 1.5 times (ie. 150%)
    wider.  The default stretch factor is 100.  The minimum stretch
    factor is 1, and the maximum stretch factor is 4000.

    The stretch factor is only applied to outline fonts.  The stretch
    factor is ignored for bitmap fonts.

    \sa stretch() QFont::Stretch
*/
void QFont::setStretch(int factor)
{
    if (Q_UNLIKELY(factor < 1 || factor > 4000)) {
        qWarning("QFont::setStretch: Parameter '%d' out of range", factor);
        return;
    }

    if ((resolve_mask & QFont::StretchResolved) && d->request.stretch == factor)
        return;

    detach();

    d->request.stretch = factor;
    resolve_mask |= QFont::StretchResolved;
}

/*!
    Returns true if a window system font exactly matching the settings
    of this font is available.

    \sa QFontDatabase
*/
bool QFont::exactMatch() const
{
    QFontEngine *engine = d->engineForScript(QUnicodeTables::Common);
    Q_ASSERT(engine != 0);
    return d->request.exactMatch(engine->fontDef);
}

/*!
    Returns true if this font is equal to \a f; otherwise returns
    false.

    Two QFonts are considered equal if their font attributes are
    equal.

    \sa operator!=() isCopyOf()
*/
bool QFont::operator==(const QFont &f) const
{
    return (f.d == d
            || (f.d->request   == d->request
                && f.d->request.pointSize == d->request.pointSize
                && f.d->underline == d->underline
                && f.d->overline  == d->overline
                && f.d->strikeOut == d->strikeOut
                && f.d->kerning == d->kerning
            ));
}


/*!
    Provides an arbitrary comparison of this font and font \a f.
    All that is guaranteed is that the operator returns false if both
    fonts are equal and that (f1 \< f2) == !(f2 \< f1) if the fonts
    are not equal.

    This function is useful in some circumstances, for example if you
    want to use QFont objects as keys in a QMap.

    \sa operator==() operator!=() isCopyOf()
*/
bool QFont::operator<(const QFont &f) const
{
    if (f.d == d) return false;
    // the < operator for fontdefs ignores point sizes.
    QFontDef &r1 = f.d->request;
    QFontDef &r2 = d->request;
    if (r1.pointSize != r2.pointSize) return r1.pointSize < r2.pointSize;
    if (r1.pixelSize != r2.pixelSize) return r1.pixelSize < r2.pixelSize;
    if (r1.weight != r2.weight) return r1.weight < r2.weight;
    if (r1.style != r2.style) return r1.style < r2.style;
    if (r1.stretch != r2.stretch) return r1.stretch < r2.stretch;
    if (r1.hintingPreference != r2.hintingPreference) return r1.hintingPreference < r2.hintingPreference;
    if (r1.family != r2.family) return r1.family < r2.family;

    int f1attrs = (f.d->underline << 3) + (f.d->overline << 2) + (f.d->strikeOut<<1) + f.d->kerning;
    int f2attrs = (d->underline << 3) + (d->overline << 2) + (d->strikeOut<<1) + d->kerning;
    return f1attrs < f2attrs;
}


/*!
    Returns true if this font is different from \a f; otherwise
    returns false.

    Two QFonts are considered to be different if their font attributes
    are different.

    \sa operator==()
*/
bool QFont::operator!=(const QFont &f) const
{
    return !(operator==(f));
}

/*!
   Returns the font as a QVariant
*/
QFont::operator QVariant() const
{
    return QVariant(QVariant::Font, this);
}

/*!
    Returns true if this font and \a f are copies of each other, i.e.
    one of them was created as a copy of the other and neither has
    been modified since. This is much stricter than equality.

    \sa operator=() operator==()
*/
bool QFont::isCopyOf(const QFont & f) const
{
    return d == f.d;
}

/*!
    Returns a new QFont that has attributes copied from \a other that
    have not been previously set on this font.
*/
QFont QFont::resolve(const QFont &other) const
{
    if (*this == other
        && (resolve_mask == other.resolve_mask || resolve_mask == 0)
        && d->dpi == other.d->dpi) {
        QFont o = other;
        o.resolve_mask = resolve_mask;
        return o;
    }

    QFont font(*this);
    font.detach();
    font.d->resolve(resolve_mask, other.d.data());

    return font;
}

/*!
    \fn uint QFont::resolve() const
    \internal
*/

/*!
    \fn void QFont::resolve(uint mask)
    \internal
*/

#ifndef QT_NO_DATASTREAM
/*  \internal
    Internal function. Converts boolean font settings to an unsigned
    8-bit number. Used for serialization etc.
*/
static inline quint8 get_font_bits(const QFontPrivate *f)
{
    Q_ASSERT(f != 0);
    quint8 bits = 0;
    if (f->request.style)
        bits |= 0x01;
    if (f->underline)
        bits |= 0x02;
    if (f->overline)
        bits |= 0x40;
    if (f->strikeOut)
        bits |= 0x04;
    if (f->request.fixedPitch)
        bits |= 0x08;
    if (f->request.ignorePitch)
        bits |= 0x20;
    if (f->kerning)
        bits |= 0x10;
    if (f->request.style == QFont::StyleOblique)
        bits |= 0x80;
    return bits;
}

/*  \internal
    Internal function. Sets boolean font settings from an unsigned
    8-bit number. Used for serialization etc.
*/
static inline void set_font_bits(quint8 bits, QFontPrivate *f)
{
    Q_ASSERT(f != 0);
    f->request.style         = (bits & 0x01) != 0 ? QFont::StyleItalic : QFont::StyleNormal;
    f->underline             = (bits & 0x02) != 0;
    f->overline              = (bits & 0x40) != 0;
    f->strikeOut             = (bits & 0x04) != 0;
    f->request.fixedPitch    = (bits & 0x08) != 0;
    f->request.ignorePitch   = (bits & 0x20) != 0;
    f->kerning               = (bits & 0x10) != 0;
    if ((bits & 0x80) != 0)
        f->request.style         = QFont::StyleOblique;
}
#endif


/*!
    Returns the font's key, a textual representation of a font. It is
    typically used as the key for a cache or dictionary of fonts.

    \sa QMap
*/
QString QFont::key() const
{
    return toString();
}

/*!
    Returns a description of the font. The description is a
    comma-separated list of the attributes, perfectly suited for use
    in QSettings.

    \sa fromString()
 */
QString QFont::toString() const
{
    const QLatin1Char comma(',');
    return family() + comma +
        styleName() + comma +
        QString::number(     pointSizeF()) + comma +
        QString::number(      pixelSize()) + comma +
        QString::number(         weight()) + comma +
        QString::number((int)     style()) + comma +
        QString::number((int) underline()) + comma +
        QString::number((int) strikeOut()) + comma +
        QString::number((int)fixedPitch());
}


/*!
    Sets this font to match the description \a descrip. The description
    is a comma-separated list of the font attributes, as returned by
    toString().

    \sa toString()
 */
bool QFont::fromString(const QString &descrip)
{
    QStringList l(descrip.split(QLatin1Char(',')));

    int count = l.count();
    if (Q_UNLIKELY(count <= 0 || count > 9)) {
        qWarning("QFont::fromString: Invalid description '%s'",
                 descrip.isEmpty() ? "(empty)" : descrip.toLatin1().data());
        return false;
    }

    setFamily(l[0]);
    if (count > 1)
        setStyleName(l[1]);
    if (count > 2) {
        const double l2 = l[2].toDouble();
        if (l2 > 0.0) {
            setPointSizeF(l2);
        }
    if (count == 9) {
        const int l3 = l[3].toInt();
        if (l3 > 0)
            setPixelSize(l3);
        }
        setWeight(qMax(qMin(99, l[4].toInt()), 0));
        setStyle(static_cast<QFont::Style>(l[5].toInt()));
        setUnderline(l[6].toInt());
        setStrikeOut(l[7].toInt());
        setFixedPitch(l[8].toInt());

        if (!d->request.fixedPitch) // assume 'false' fixedPitch equals default
            d->request.ignorePitch = true;
    }

    return true;
}


/*****************************************************************************
  QFont stream functions
 *****************************************************************************/
#ifndef QT_NO_DATASTREAM
/*!
    \relates QFont

    Writes the font \a font to the data stream \a s. (toString()
    writes to a text stream.)

    \sa \link datastreamformat.html Format of the QDataStream operators \endlink
*/
QDataStream &operator<<(QDataStream &s, const QFont &font)
{
    s << font.d->request.family;

    double pointSize = font.d->request.pointSize;
    double pixelSize = font.d->request.pixelSize;
    s << pointSize;
    s << pixelSize;

    s << (qint8) font.d->request.hintingPreference;
    s << (qint8) font.d->request.weight
      << get_font_bits(font.d.data());
    s << (qint16)font.d->request.stretch;
    return s;
}

/*!
    \relates QFont

    Reads the font \a font from the data stream \a s. (fromString()
    reads from a text stream.)

    \sa \link datastreamformat.html Format of the QDataStream operators \endlink
*/
QDataStream &operator>>(QDataStream &s, QFont &font)
{
    font.d = new QFontPrivate;
    font.resolve_mask = QFont::AllPropertiesResolved;

    qint8 hintingPreference = QFont::PreferDefaultHinting;
    qint8 bits = 0;
    qint8 weight = 50;

    s >> font.d->request.family;

    double pointSize = -1.0;
    double pixelSize = -1.0;
    s >> pointSize;
    s >> pixelSize;
    font.d->request.pointSize = qreal(pointSize);
    font.d->request.pixelSize = qreal(pixelSize);
    s >> hintingPreference;

    s >> weight;
    s >> bits;

    font.d->request.hintingPreference = QFont::HintingPreference(hintingPreference);
    font.d->request.weight = weight;

    set_font_bits(bits, font.d.data());

    qint16 stretch = QFont::Unstretched;
    s >> stretch;
    font.d->request.stretch = stretch;

    return s;
}
#endif // QT_NO_DATASTREAM


// **********************************************************************
// QFontCache
thread_local std::unique_ptr<QFontCache> theFontCache(nullptr);

QFontCache *QFontCache::instance()
{
    if (!theFontCache) {
        theFontCache = std::make_unique<QFontCache>();
    }
    return theFontCache.get();
}

QFontCache::QFontCache()
{
}

QFontCache::~QFontCache()
{
    clear();
}

void QFontCache::clear()
{
    EngineCache::ConstIterator it = engineCache.constBegin(),
                               end = engineCache.constEnd();
    while (it != end) {
        if (!it.value()->ref.deref())
            delete it.value();
        else
            FC_DEBUG("QFontCache::~QFontCache: engineData %p still has refcount %d",
                        it.value(), int(it.value()->ref));
        ++it;
    }

    engineCache.clear();
}

QFontEngine *QFontCache::findEngine(const Key &key)
{
    EngineCache::Iterator it = engineCache.find(key),
                         end = engineCache.end();
    if (it == end) {
        return nullptr;
    }

    FC_DEBUG("QFontCache: found font engine\n"
             "  %p: ref %2d, type '%s'",
             it.value(), int(it.value()->ref), it.value()->name());

    return it.value();
}

void QFontCache::insertEngine(const Key &key, QFontEngine *engine)
{
    FC_DEBUG("QFontCache: inserting new engine %p", engine);

    engineCache.insert(key, engine);
}

#ifndef QT_NO_DEBUG_STREAM
QDebug operator<<(QDebug stream, const QFont &font)
{
    return stream << "QFont(" << font.toString() << ')';
}
#endif

QT_END_NAMESPACE
