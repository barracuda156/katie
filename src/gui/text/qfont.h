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

#ifndef QFONT_H
#define QFONT_H

#include <QtGui/qwindowdefs.h>
#include <QtCore/qstring.h>
#include <QtCore/qsharedpointer.h>

#if defined(Q_WS_X11)
typedef struct FT_FaceRec_* FT_Face;
#endif

QT_BEGIN_NAMESPACE

class QPaintDevice;
class QStringList;
class QVariant;
class QFontPrivate;

class Q_GUI_EXPORT QFont
{
public:
    enum HintingPreference {
        PreferDefaultHinting        = 0,
        PreferNoHinting             = 1,
        PreferVerticalHinting       = 2,
        PreferFullHinting           = 3
    };

    enum Weight {
        Light    = 25,
        Normal   = 50,
        DemiBold = 63,
        Bold     = 75,
        Black    = 87
    };

    enum Style {
        StyleNormal,
        StyleItalic,
        StyleOblique
    };

    enum Stretch {
        UltraCondensed =  50,
        ExtraCondensed =  62,
        Condensed      =  75,
        SemiCondensed  =  87,
        Unstretched    = 100,
        SemiExpanded   = 112,
        Expanded       = 125,
        ExtraExpanded  = 150,
        UltraExpanded  = 200
    };

    enum ResolveProperties {
        FamilyResolved              = 0x0001,
        SizeResolved                = 0x0002,
        WeightResolved              = 0x0004,
        StyleResolved               = 0x0008,
        UnderlineResolved           = 0x0010,
        OverlineResolved            = 0x0020,
        StrikeOutResolved           = 0x0040,
        FixedPitchResolved          = 0x0080,
        StretchResolved             = 0x0100,
        KerningResolved             = 0x0200,
        HintingPreferenceResolved   = 0x0400,
        StyleNameResolved           = 0x0800,
        AllPropertiesResolved       = 0x1ffff
    };

    QFont();
    QFont(const QString &family, int pointSize = -1, int weight = -1, bool italic = false);
    QFont(const QFont &, QPaintDevice *pd);
    QFont(const QFont &);
    ~QFont();

    QString family() const;
    void setFamily(const QString &);

    QString styleName() const;
    void setStyleName(const QString &);

    int pointSize() const;
    void setPointSize(int);
    qreal pointSizeF() const;
    void setPointSizeF(qreal);

    int pixelSize() const;
    void setPixelSize(int);

    int weight() const;
    void setWeight(int);

    inline bool bold() const;
    inline void setBold(bool);

    void setStyle(Style style);
    Style style() const;

    inline bool italic() const;
    inline void setItalic(bool b);

    bool underline() const;
    void setUnderline(bool);

    bool overline() const;
    void setOverline(bool);

    bool strikeOut() const;
    void setStrikeOut(bool);

    bool fixedPitch() const;
    void setFixedPitch(bool);

    bool kerning() const;
    void setKerning(bool);

    int stretch() const;
    void setStretch(int);

    void setHintingPreference(HintingPreference hintingPreference);
    HintingPreference hintingPreference() const;

    bool exactMatch() const;

    QFont &operator=(const QFont &);
    bool operator==(const QFont &) const;
    bool operator!=(const QFont &) const;
    bool operator<(const QFont &) const;
    operator QVariant() const;
    bool isCopyOf(const QFont &) const;
    inline QFont &operator=(QFont &&other)
    { qSwap(d, other.d); qSwap(resolve_mask, other.resolve_mask);  return *this; }

#if defined(Q_WS_X11)
    FT_Face freetypeFace() const;
#endif

    QString toString() const;
    bool fromString(const QString &);

    static QString lastResortFamily();

    QFont resolve(const QFont &) const;
    inline uint resolve() const { return resolve_mask; }
    inline void resolve(uint mask) { resolve_mask = mask; }


private:
    QFont(QFontPrivate *);

    void detach();

#if defined(Q_WS_X11)
    void x11SetScreen(int screen = -1);
    int x11Screen() const;
#endif

    friend class QFontPrivate;
    friend class QFontDialogPrivate;
    friend class QFontMetrics;
    friend class QFontMetricsF;
    friend class QPainter;
    friend class QPainterPrivate;
    friend class QApplication;
    friend class QWidget;
    friend class QWidgetPrivate;
    friend class QTextLayout;
    friend class QTextEngine;
    friend class QTextLine;
    friend struct QScriptLine;
    friend class QAlphaPaintEngine;
    friend class QPainterPath;
    friend class QTextItemInt;
    friend class QCommandLinkButtonPrivate;
    friend class QFontEngine;
    friend class QFontDatabasePrivate;

#ifndef QT_NO_DATASTREAM
    friend Q_GUI_EXPORT QDataStream &operator<<(QDataStream &, const QFont &);
    friend Q_GUI_EXPORT QDataStream &operator>>(QDataStream &, QFont &);
#endif

    QExplicitlySharedDataPointer<QFontPrivate> d;
    uint resolve_mask;
};


inline bool QFont::bold() const
{ return weight() > Normal; }


inline void QFont::setBold(bool enable)
{ setWeight(enable ? Bold : Normal); }

inline bool QFont::italic() const
{
    return (style() != StyleNormal);
}

inline void QFont::setItalic(bool b) {
    setStyle(b ? StyleItalic : StyleNormal);
}


/*****************************************************************************
  QFont stream functions
 *****************************************************************************/

#ifndef QT_NO_DATASTREAM
Q_GUI_EXPORT QDataStream &operator<<(QDataStream &, const QFont &);
Q_GUI_EXPORT QDataStream &operator>>(QDataStream &, QFont &);
#endif

#ifndef QT_NO_DEBUG_STREAM
Q_GUI_EXPORT QDebug operator<<(QDebug, const QFont &);
#endif

QT_END_NAMESPACE


#endif // QFONT_H
