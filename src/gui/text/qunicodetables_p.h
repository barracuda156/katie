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

/* This file is autogenerated from the Unicode 5.0 database. Do not edit */

//
//  W A R N I N G
//  -------------
//
// This file is not part of the Katie API.  It exists for the convenience
// of internal files.  This header file may change from version to version
// without notice, or even be removed.
//
// We mean it.
//

#ifndef QUNICODETABLES_P_H
#define QUNICODETABLES_P_H

#include "qglobal.h"

QT_BEGIN_NAMESPACE

namespace QUnicodeTables {

    // required by Harfbuzz
    enum GraphemeBreak {
        GraphemeBreak_Other,
        GraphemeBreak_CR,
        GraphemeBreak_LF,
        GraphemeBreak_Control,
        GraphemeBreak_Extend,
        GraphemeBreak_L,
        GraphemeBreak_V,
        GraphemeBreak_T,
        GraphemeBreak_LV,
        GraphemeBreak_LVT
    };

    enum LineBreak {
        LineBreak_OP,
        LineBreak_CL,
        LineBreak_QU,
        LineBreak_GL,
        LineBreak_NS,
        LineBreak_EX,
        LineBreak_SY,
        LineBreak_IS,
        LineBreak_PR,
        LineBreak_PO,
        LineBreak_NU,
        LineBreak_AL,
        LineBreak_ID,
        LineBreak_IN,
        LineBreak_HY,
        LineBreak_BA,
        LineBreak_BB,
        LineBreak_B2,
        LineBreak_ZW,
        LineBreak_CM,
        LineBreak_WJ,
        LineBreak_H2,
        LineBreak_H3,
        LineBreak_JL,
        LineBreak_JV,
        LineBreak_JT,
        LineBreak_SA,
        LineBreak_SG,
        LineBreak_SP,
        LineBreak_CR,
        LineBreak_LF,
        LineBreak_BK
    };

    Q_GUI_EXPORT GraphemeBreak QT_FASTCALL graphemeBreak(uint ucs4);
    Q_GUI_EXPORT LineBreak QT_FASTCALL lineBreakClass(uint ucs4);

    // required by text engine and font database
    // See https://www.unicode.org/reports/tr24/
    enum Script {
        Common,
        Adlam,
        Ahom,
        AnatolianHieroglyphs,
        Arabic,
        Armenian,
        Avestan,
        Balinese,
        Bamum,
        BassaVah,
        Batak,
        Bengali,
        Bhaiksuki,
        Bopomofo,
        Brahmi,
        Braille,
        Buginese,
        Buhid,
        CanadianAboriginal,
        Carian,
        CaucasianAlbanian,
        Chakma,
        Cham,
        Cherokee,
        Chorasmian,
        Coptic,
        Cuneiform,
        Cypriot,
        CyproMinoan,
        Cyrillic,
        Deseret,
        Devanagari,
        DivesAkuru,
        Dogra,
        Duployan,
        EgyptianHieroglyphs,
        Elbasan,
        Elymaic,
        Ethiopic,
        Georgian,
        Glagolitic,
        Gothic,
        Grantha,
        Greek,
        Gujarati,
        GunjalaGondi,
        Gurmukhi,
        Han,
        Hangul,
        HanifiRohingya,
        Hanunoo,
        Hatran,
        Hebrew,
        Hiragana,
        ImperialAramaic,
        Inherited,
        InscriptionalPahlavi,
        InscriptionalParthian,
        Javanese,
        Kaithi,
        Kannada,
        Katakana,
        Kawi,
        KayahLi,
        Kharoshthi,
        KhitanSmallScript,
        Khmer,
        Khojki,
        Khudawadi,
        Lao,
        Latin,
        Lepcha,
        Limbu,
        LinearA,
        LinearB,
        Lisu,
        Lycian,
        Lydian,
        Mahajani,
        Makasar,
        Malayalam,
        Mandaic,
        Manichaean,
        Marchen,
        MasaramGondi,
        Medefaidrin,
        MeeteiMayek,
        MendeKikakui,
        MeroiticCursive,
        MeroiticHieroglyphs,
        Miao,
        Modi,
        Mongolian,
        Mro,
        Multani,
        Myanmar,
        Nabataean,
        NagMundari,
        Nandinagari,
        Newa,
        NewTaiLue,
        Nko,
        Nushu,
        NyiakengPuachueHmong,
        Ogham,
        OlChiki,
        OldHungarian,
        OldItalic,
        OldNorthArabian,
        OldPermic,
        OldPersian,
        OldSogdian,
        OldSouthArabian,
        OldTurkic,
        OldUyghur,
        Oriya,
        Osage,
        Osmanya,
        PahawhHmong,
        Palmyrene,
        PauCinHau,
        PhagsPa,
        Phoenician,
        PsalterPahlavi,
        Rejang,
        Runic,
        Samaritan,
        Saurashtra,
        Sharada,
        Shavian,
        Siddham,
        SignWriting,
        Sinhala,
        Sogdian,
        SoraSompeng,
        Soyombo,
        Sundanese,
        SylotiNagri,
        Syriac,
        Tagalog,
        Tagbanwa,
        TaiLe,
        TaiTham,
        TaiViet,
        Takri,
        Tamil,
        Tangsa,
        Tangut,
        Telugu,
        Thaana,
        Thai,
        Tibetan,
        Tifinagh,
        Tirhuta,
        Toto,
        Ugaritic,
        Vai,
        Vithkuqi,
        Wancho,
        WarangCiti,
        Yezidi,
        Yi,
        ZanabazarSquare,

        ScriptCount
    };

    Q_GUI_EXPORT Script QT_FASTCALL script(uint ucs4);

} // namespace QUnicodeTables

QT_END_NAMESPACE

#endif // QUNICODETABLES_P_H
