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
#include "qtextcodec.h"

#include "qlist.h"
#include "qfile.h"
#include "qstringlist.h"
#include "qlocale.h"
#include "qlocale_tools_p.h"
#include "qtextcodec_p.h"
#include "qmutex.h"
#include "qhash.h"
#include "qstdcontainers_p.h"
#include "qcorecommon_p.h"
#include "qdebug.h"

#include <stdlib.h>
#include <ctype.h>
#include <locale.h>
#include <langinfo.h>
#include <unicode/ucsdet.h>
#include <unicode/uclean.h>

QT_BEGIN_NAMESPACE

// generated via genmib.py
static const struct MIBTblData {
    const qint16 mib;
    const char* name;
} MIBTbl[] = {
    { 3, "US-ASCII\0" },
    { 3, "iso-ir-6\0" },
    { 3, "ANSI_X3.4-1968\0" },
    { 3, "ANSI_X3.4-1986\0" },
    { 3, "ISO_646.irv:1991\0" },
    { 3, "ISO646-US\0" },
    { 3, "us\0" },
    { 3, "IBM367\0" },
    { 3, "cp367\0" },
    { 3, "csASCII\0" },
    { 4, "ISO_8859-1:1987\0" },
    { 4, "iso-ir-100\0" },
    { 4, "ISO_8859-1\0" },
    { 4, "ISO-8859-1\0" },
    { 4, "latin1\0" },
    { 4, "l1\0" },
    { 4, "IBM819\0" },
    { 4, "CP819\0" },
    { 4, "csISOLatin1\0" },
    { 5, "ISO_8859-2:1987\0" },
    { 5, "iso-ir-101\0" },
    { 5, "ISO_8859-2\0" },
    { 5, "ISO-8859-2\0" },
    { 5, "latin2\0" },
    { 5, "l2\0" },
    { 5, "csISOLatin2\0" },
    { 6, "ISO_8859-3:1988\0" },
    { 6, "iso-ir-109\0" },
    { 6, "ISO_8859-3\0" },
    { 6, "ISO-8859-3\0" },
    { 6, "latin3\0" },
    { 6, "l3\0" },
    { 6, "csISOLatin3\0" },
    { 7, "ISO_8859-4:1988\0" },
    { 7, "iso-ir-110\0" },
    { 7, "ISO_8859-4\0" },
    { 7, "ISO-8859-4\0" },
    { 7, "latin4\0" },
    { 7, "l4\0" },
    { 7, "csISOLatin4\0" },
    { 8, "ISO_8859-5:1988\0" },
    { 8, "iso-ir-144\0" },
    { 8, "ISO_8859-5\0" },
    { 8, "ISO-8859-5\0" },
    { 8, "cyrillic\0" },
    { 8, "csISOLatinCyrillic\0" },
    { 9, "ISO_8859-6:1987\0" },
    { 9, "iso-ir-127\0" },
    { 9, "ISO_8859-6\0" },
    { 9, "ISO-8859-6\0" },
    { 9, "ECMA-114\0" },
    { 9, "ASMO-708\0" },
    { 9, "arabic\0" },
    { 9, "csISOLatinArabic\0" },
    { 10, "ISO_8859-7:1987\0" },
    { 10, "iso-ir-126\0" },
    { 10, "ISO_8859-7\0" },
    { 10, "ISO-8859-7\0" },
    { 10, "ELOT_928\0" },
    { 10, "ECMA-118\0" },
    { 10, "greek\0" },
    { 10, "greek8\0" },
    { 10, "csISOLatinGreek\0" },
    { 11, "ISO_8859-8:1988\0" },
    { 11, "iso-ir-138\0" },
    { 11, "ISO_8859-8\0" },
    { 11, "ISO-8859-8\0" },
    { 11, "hebrew\0" },
    { 11, "csISOLatinHebrew\0" },
    { 12, "ISO_8859-9:1989\0" },
    { 12, "iso-ir-148\0" },
    { 12, "ISO_8859-9\0" },
    { 12, "ISO-8859-9\0" },
    { 12, "latin5\0" },
    { 12, "l5\0" },
    { 12, "csISOLatin5\0" },
    { 13, "ISO-8859-10\0" },
    { 13, "iso-ir-157\0" },
    { 13, "l6\0" },
    { 13, "ISO_8859-10:1992\0" },
    { 13, "csISOLatin6\0" },
    { 13, "latin6\0" },
    { 14, "ISO_6937-2-add\0" },
    { 14, "iso-ir-142\0" },
    { 14, "csISOTextComm\0" },
    { 15, "JIS_X0201\0" },
    { 15, "X0201\0" },
    { 15, "csHalfWidthKatakana\0" },
    { 16, "JIS_Encoding\0" },
    { 16, "csJISEncoding\0" },
    { 17, "Shift_JIS\0" },
    { 17, "MS_Kanji\0" },
    { 17, "csShiftJIS\0" },
    { 18, "Extended_UNIX_Code_Packed_Format_for_Japanese\0" },
    { 18, "csEUCPkdFmtJapanese\0" },
    { 18, "EUC-JP\0" },
    { 19, "Extended_UNIX_Code_Fixed_Width_for_Japanese\0" },
    { 19, "csEUCFixWidJapanese\0" },
    { 20, "BS_4730\0" },
    { 20, "iso-ir-4\0" },
    { 20, "ISO646-GB\0" },
    { 20, "gb\0" },
    { 20, "uk\0" },
    { 20, "csISO4UnitedKingdom\0" },
    { 21, "SEN_850200_C\0" },
    { 21, "iso-ir-11\0" },
    { 21, "ISO646-SE2\0" },
    { 21, "se2\0" },
    { 21, "csISO11SwedishForNames\0" },
    { 22, "IT\0" },
    { 22, "iso-ir-15\0" },
    { 22, "ISO646-IT\0" },
    { 22, "csISO15Italian\0" },
    { 23, "ES\0" },
    { 23, "iso-ir-17\0" },
    { 23, "ISO646-ES\0" },
    { 23, "csISO17Spanish\0" },
    { 24, "DIN_66003\0" },
    { 24, "iso-ir-21\0" },
    { 24, "de\0" },
    { 24, "ISO646-DE\0" },
    { 24, "csISO21German\0" },
    { 25, "NS_4551-1\0" },
    { 25, "iso-ir-60\0" },
    { 25, "ISO646-NO\0" },
    { 25, "no\0" },
    { 25, "csISO60DanishNorwegian\0" },
    { 25, "csISO60Norwegian1\0" },
    { 26, "NF_Z_62-010\0" },
    { 26, "iso-ir-69\0" },
    { 26, "ISO646-FR\0" },
    { 26, "fr\0" },
    { 26, "csISO69French\0" },
    { 27, "ISO-10646-UTF-1\0" },
    { 27, "csISO10646UTF1\0" },
    { 28, "ISO_646.basic:1983\0" },
    { 28, "ref\0" },
    { 28, "csISO646basic1983\0" },
    { 29, "INVARIANT\0" },
    { 29, "csINVARIANT\0" },
    { 30, "ISO_646.irv:1983\0" },
    { 30, "iso-ir-2\0" },
    { 30, "irv\0" },
    { 30, "csISO2IntlRefVersion\0" },
    { 31, "NATS-SEFI\0" },
    { 31, "iso-ir-8-1\0" },
    { 31, "csNATSSEFI\0" },
    { 32, "NATS-SEFI-ADD\0" },
    { 32, "iso-ir-8-2\0" },
    { 32, "csNATSSEFIADD\0" },
    { 33, "NATS-DANO\0" },
    { 33, "iso-ir-9-1\0" },
    { 33, "csNATSDANO\0" },
    { 34, "NATS-DANO-ADD\0" },
    { 34, "iso-ir-9-2\0" },
    { 34, "csNATSDANOADD\0" },
    { 35, "SEN_850200_B\0" },
    { 35, "iso-ir-10\0" },
    { 35, "FI\0" },
    { 35, "ISO646-FI\0" },
    { 35, "ISO646-SE\0" },
    { 35, "se\0" },
    { 35, "csISO10Swedish\0" },
    { 36, "KS_C_5601-1987\0" },
    { 36, "iso-ir-149\0" },
    { 36, "KS_C_5601-1989\0" },
    { 36, "KSC_5601\0" },
    { 36, "korean\0" },
    { 36, "csKSC56011987\0" },
    { 37, "ISO-2022-KR\0" },
    { 37, "csISO2022KR\0" },
    { 38, "EUC-KR\0" },
    { 38, "csEUCKR\0" },
    { 39, "ISO-2022-JP\0" },
    { 39, "csISO2022JP\0" },
    { 40, "ISO-2022-JP-2\0" },
    { 40, "csISO2022JP2\0" },
    { 41, "JIS_C6220-1969-jp\0" },
    { 41, "JIS_C6220-1969\0" },
    { 41, "iso-ir-13\0" },
    { 41, "katakana\0" },
    { 41, "x0201-7\0" },
    { 41, "csISO13JISC6220jp\0" },
    { 42, "JIS_C6220-1969-ro\0" },
    { 42, "iso-ir-14\0" },
    { 42, "jp\0" },
    { 42, "ISO646-JP\0" },
    { 42, "csISO14JISC6220ro\0" },
    { 43, "PT\0" },
    { 43, "iso-ir-16\0" },
    { 43, "ISO646-PT\0" },
    { 43, "csISO16Portuguese\0" },
    { 44, "greek7-old\0" },
    { 44, "iso-ir-18\0" },
    { 44, "csISO18Greek7Old\0" },
    { 45, "latin-greek\0" },
    { 45, "iso-ir-19\0" },
    { 45, "csISO19LatinGreek\0" },
    { 46, "NF_Z_62-010_(1973)\0" },
    { 46, "iso-ir-25\0" },
    { 46, "ISO646-FR1\0" },
    { 46, "csISO25French\0" },
    { 47, "Latin-greek-1\0" },
    { 47, "iso-ir-27\0" },
    { 47, "csISO27LatinGreek1\0" },
    { 48, "ISO_5427\0" },
    { 48, "iso-ir-37\0" },
    { 48, "csISO5427Cyrillic\0" },
    { 49, "JIS_C6226-1978\0" },
    { 49, "iso-ir-42\0" },
    { 49, "csISO42JISC62261978\0" },
    { 50, "BS_viewdata\0" },
    { 50, "iso-ir-47\0" },
    { 50, "csISO47BSViewdata\0" },
    { 51, "INIS\0" },
    { 51, "iso-ir-49\0" },
    { 51, "csISO49INIS\0" },
    { 52, "INIS-8\0" },
    { 52, "iso-ir-50\0" },
    { 52, "csISO50INIS8\0" },
    { 53, "INIS-cyrillic\0" },
    { 53, "iso-ir-51\0" },
    { 53, "csISO51INISCyrillic\0" },
    { 54, "ISO_5427:1981\0" },
    { 54, "iso-ir-54\0" },
    { 54, "ISO5427Cyrillic1981\0" },
    { 54, "csISO54271981\0" },
    { 55, "ISO_5428:1980\0" },
    { 55, "iso-ir-55\0" },
    { 55, "csISO5428Greek\0" },
    { 56, "GB_1988-80\0" },
    { 56, "iso-ir-57\0" },
    { 56, "cn\0" },
    { 56, "ISO646-CN\0" },
    { 56, "csISO57GB1988\0" },
    { 57, "GB_2312-80\0" },
    { 57, "iso-ir-58\0" },
    { 57, "chinese\0" },
    { 57, "csISO58GB231280\0" },
    { 58, "NS_4551-2\0" },
    { 58, "ISO646-NO2\0" },
    { 58, "iso-ir-61\0" },
    { 58, "no2\0" },
    { 58, "csISO61Norwegian2\0" },
    { 59, "videotex-suppl\0" },
    { 59, "iso-ir-70\0" },
    { 59, "csISO70VideotexSupp1\0" },
    { 60, "PT2\0" },
    { 60, "iso-ir-84\0" },
    { 60, "ISO646-PT2\0" },
    { 60, "csISO84Portuguese2\0" },
    { 61, "ES2\0" },
    { 61, "iso-ir-85\0" },
    { 61, "ISO646-ES2\0" },
    { 61, "csISO85Spanish2\0" },
    { 62, "MSZ_7795.3\0" },
    { 62, "iso-ir-86\0" },
    { 62, "ISO646-HU\0" },
    { 62, "hu\0" },
    { 62, "csISO86Hungarian\0" },
    { 63, "JIS_C6226-1983\0" },
    { 63, "iso-ir-87\0" },
    { 63, "x0208\0" },
    { 63, "JIS_X0208-1983\0" },
    { 63, "csISO87JISX0208\0" },
    { 64, "greek7\0" },
    { 64, "iso-ir-88\0" },
    { 64, "csISO88Greek7\0" },
    { 65, "ASMO_449\0" },
    { 65, "ISO_9036\0" },
    { 65, "arabic7\0" },
    { 65, "iso-ir-89\0" },
    { 65, "csISO89ASMO449\0" },
    { 66, "iso-ir-90\0" },
    { 66, "csISO90\0" },
    { 67, "JIS_C6229-1984-a\0" },
    { 67, "iso-ir-91\0" },
    { 67, "jp-ocr-a\0" },
    { 67, "csISO91JISC62291984a\0" },
    { 68, "JIS_C6229-1984-b\0" },
    { 68, "iso-ir-92\0" },
    { 68, "ISO646-JP-OCR-B\0" },
    { 68, "jp-ocr-b\0" },
    { 68, "csISO92JISC62991984b\0" },
    { 69, "JIS_C6229-1984-b-add\0" },
    { 69, "iso-ir-93\0" },
    { 69, "jp-ocr-b-add\0" },
    { 69, "csISO93JIS62291984badd\0" },
    { 70, "JIS_C6229-1984-hand\0" },
    { 70, "iso-ir-94\0" },
    { 70, "jp-ocr-hand\0" },
    { 70, "csISO94JIS62291984hand\0" },
    { 71, "JIS_C6229-1984-hand-add\0" },
    { 71, "iso-ir-95\0" },
    { 71, "jp-ocr-hand-add\0" },
    { 71, "csISO95JIS62291984handadd\0" },
    { 72, "JIS_C6229-1984-kana\0" },
    { 72, "iso-ir-96\0" },
    { 72, "csISO96JISC62291984kana\0" },
    { 73, "ISO_2033-1983\0" },
    { 73, "iso-ir-98\0" },
    { 73, "e13b\0" },
    { 73, "csISO2033\0" },
    { 74, "ANSI_X3.110-1983\0" },
    { 74, "iso-ir-99\0" },
    { 74, "CSA_T500-1983\0" },
    { 74, "NAPLPS\0" },
    { 74, "csISO99NAPLPS\0" },
    { 75, "T.61-7bit\0" },
    { 75, "iso-ir-102\0" },
    { 75, "csISO102T617bit\0" },
    { 76, "T.61-8bit\0" },
    { 76, "T.61\0" },
    { 76, "iso-ir-103\0" },
    { 76, "csISO103T618bit\0" },
    { 77, "ECMA-cyrillic\0" },
    { 77, "iso-ir-111\0" },
    { 77, "KOI8-E\0" },
    { 77, "csISO111ECMACyrillic\0" },
    { 78, "CSA_Z243.4-1985-1\0" },
    { 78, "iso-ir-121\0" },
    { 78, "ISO646-CA\0" },
    { 78, "csa7-1\0" },
    { 78, "csa71\0" },
    { 78, "ca\0" },
    { 78, "csISO121Canadian1\0" },
    { 79, "CSA_Z243.4-1985-2\0" },
    { 79, "iso-ir-122\0" },
    { 79, "ISO646-CA2\0" },
    { 79, "csa7-2\0" },
    { 79, "csa72\0" },
    { 79, "csISO122Canadian2\0" },
    { 80, "CSA_Z243.4-1985-gr\0" },
    { 80, "iso-ir-123\0" },
    { 80, "csISO123CSAZ24341985gr\0" },
    { 81, "ISO_8859-6-E\0" },
    { 81, "csISO88596E\0" },
    { 81, "ISO-8859-6-E\0" },
    { 82, "ISO_8859-6-I\0" },
    { 82, "csISO88596I\0" },
    { 82, "ISO-8859-6-I\0" },
    { 83, "T.101-G2\0" },
    { 83, "iso-ir-128\0" },
    { 83, "csISO128T101G2\0" },
    { 84, "ISO_8859-8-E\0" },
    { 84, "csISO88598E\0" },
    { 84, "ISO-8859-8-E\0" },
    { 85, "ISO_8859-8-I\0" },
    { 85, "csISO88598I\0" },
    { 85, "ISO-8859-8-I\0" },
    { 86, "CSN_369103\0" },
    { 86, "iso-ir-139\0" },
    { 86, "csISO139CSN369103\0" },
    { 87, "JUS_I.B1.002\0" },
    { 87, "iso-ir-141\0" },
    { 87, "ISO646-YU\0" },
    { 87, "js\0" },
    { 87, "yu\0" },
    { 87, "csISO141JUSIB1002\0" },
    { 88, "IEC_P27-1\0" },
    { 88, "iso-ir-143\0" },
    { 88, "csISO143IECP271\0" },
    { 89, "JUS_I.B1.003-serb\0" },
    { 89, "iso-ir-146\0" },
    { 89, "serbian\0" },
    { 89, "csISO146Serbian\0" },
    { 90, "JUS_I.B1.003-mac\0" },
    { 90, "macedonian\0" },
    { 90, "iso-ir-147\0" },
    { 90, "csISO147Macedonian\0" },
    { 91, "greek-ccitt\0" },
    { 91, "iso-ir-150\0" },
    { 91, "csISO150\0" },
    { 91, "csISO150GreekCCITT\0" },
    { 92, "NC_NC00-10:81\0" },
    { 92, "cuba\0" },
    { 92, "iso-ir-151\0" },
    { 92, "ISO646-CU\0" },
    { 92, "csISO151Cuba\0" },
    { 93, "ISO_6937-2-25\0" },
    { 93, "iso-ir-152\0" },
    { 93, "csISO6937Add\0" },
    { 94, "GOST_19768-74\0" },
    { 94, "ST_SEV_358-88\0" },
    { 94, "iso-ir-153\0" },
    { 94, "csISO153GOST1976874\0" },
    { 95, "ISO_8859-supp\0" },
    { 95, "iso-ir-154\0" },
    { 95, "latin1-2-5\0" },
    { 95, "csISO8859Supp\0" },
    { 96, "ISO_10367-box\0" },
    { 96, "iso-ir-155\0" },
    { 96, "csISO10367Box\0" },
    { 97, "latin-lap\0" },
    { 97, "lap\0" },
    { 97, "iso-ir-158\0" },
    { 97, "csISO158Lap\0" },
    { 98, "JIS_X0212-1990\0" },
    { 98, "x0212\0" },
    { 98, "iso-ir-159\0" },
    { 98, "csISO159JISX02121990\0" },
    { 99, "DS_2089\0" },
    { 99, "DS2089\0" },
    { 99, "ISO646-DK\0" },
    { 99, "dk\0" },
    { 99, "csISO646Danish\0" },
    { 100, "us-dk\0" },
    { 100, "csUSDK\0" },
    { 101, "dk-us\0" },
    { 101, "csDKUS\0" },
    { 102, "KSC5636\0" },
    { 102, "ISO646-KR\0" },
    { 102, "csKSC5636\0" },
    { 103, "UNICODE-1-1-UTF-7\0" },
    { 103, "csUnicode11UTF7\0" },
    { 104, "ISO-2022-CN\0" },
    { 104, "csISO2022CN\0" },
    { 105, "ISO-2022-CN-EXT\0" },
    { 105, "csISO2022CNEXT\0" },
    { 106, "UTF-8\0" },
    { 106, "csUTF8\0" },
    { 109, "ISO-8859-13\0" },
    { 109, "csISO885913\0" },
    { 110, "ISO-8859-14\0" },
    { 110, "iso-ir-199\0" },
    { 110, "ISO_8859-14:1998\0" },
    { 110, "ISO_8859-14\0" },
    { 110, "latin8\0" },
    { 110, "iso-celtic\0" },
    { 110, "l8\0" },
    { 110, "csISO885914\0" },
    { 111, "ISO-8859-15\0" },
    { 111, "ISO_8859-15\0" },
    { 111, "Latin-9\0" },
    { 111, "csISO885915\0" },
    { 112, "ISO-8859-16\0" },
    { 112, "iso-ir-226\0" },
    { 112, "ISO_8859-16:2001\0" },
    { 112, "ISO_8859-16\0" },
    { 112, "latin10\0" },
    { 112, "l10\0" },
    { 112, "csISO885916\0" },
    { 113, "GBK\0" },
    { 113, "CP936\0" },
    { 113, "MS936\0" },
    { 113, "windows-936\0" },
    { 113, "csGBK\0" },
    { 114, "GB18030\0" },
    { 114, "csGB18030\0" },
    { 115, "OSD_EBCDIC_DF04_15\0" },
    { 115, "csOSDEBCDICDF0415\0" },
    { 116, "OSD_EBCDIC_DF03_IRV\0" },
    { 116, "csOSDEBCDICDF03IRV\0" },
    { 117, "OSD_EBCDIC_DF04_1\0" },
    { 117, "csOSDEBCDICDF041\0" },
    { 118, "ISO-11548-1\0" },
    { 118, "ISO_11548-1\0" },
    { 118, "ISO_TR_11548-1\0" },
    { 118, "csISO115481\0" },
    { 119, "KZ-1048\0" },
    { 119, "STRK1048-2002\0" },
    { 119, "RK1048\0" },
    { 119, "csKZ1048\0" },
    { 1000, "ISO-10646-UCS-2\0" },
    { 1000, "csUnicode\0" },
    { 1001, "ISO-10646-UCS-4\0" },
    { 1001, "csUCS4\0" },
    { 1002, "ISO-10646-UCS-Basic\0" },
    { 1002, "csUnicodeASCII\0" },
    { 1003, "ISO-10646-Unicode-Latin1\0" },
    { 1003, "csUnicodeLatin1\0" },
    { 1003, "ISO-10646\0" },
    { 1004, "ISO-10646-J-1\0" },
    { 1004, "csUnicodeJapanese\0" },
    { 1005, "ISO-Unicode-IBM-1261\0" },
    { 1005, "csUnicodeIBM1261\0" },
    { 1006, "ISO-Unicode-IBM-1268\0" },
    { 1006, "csUnicodeIBM1268\0" },
    { 1007, "ISO-Unicode-IBM-1276\0" },
    { 1007, "csUnicodeIBM1276\0" },
    { 1008, "ISO-Unicode-IBM-1264\0" },
    { 1008, "csUnicodeIBM1264\0" },
    { 1009, "ISO-Unicode-IBM-1265\0" },
    { 1009, "csUnicodeIBM1265\0" },
    { 1010, "UNICODE-1-1\0" },
    { 1010, "csUnicode11\0" },
    { 1011, "SCSU\0" },
    { 1011, "csSCSU\0" },
    { 1012, "UTF-7\0" },
    { 1012, "csUTF7\0" },
    { 1013, "UTF-16BE\0" },
    { 1013, "csUTF16BE\0" },
    { 1014, "UTF-16LE\0" },
    { 1014, "csUTF16LE\0" },
    { 1015, "UTF-16\0" },
    { 1015, "csUTF16\0" },
    { 1016, "CESU-8\0" },
    { 1016, "csCESU8\0" },
    { 1016, "csCESU-8\0" },
    { 1017, "UTF-32\0" },
    { 1017, "csUTF32\0" },
    { 1018, "UTF-32BE\0" },
    { 1018, "csUTF32BE\0" },
    { 1019, "UTF-32LE\0" },
    { 1019, "csUTF32LE\0" },
    { 1020, "BOCU-1\0" },
    { 1020, "csBOCU1\0" },
    { 1020, "csBOCU-1\0" },
    { 1021, "UTF-7-IMAP\0" },
    { 1021, "csUTF7IMAP\0" },
    { 2000, "ISO-8859-1-Windows-3.0-Latin-1\0" },
    { 2000, "csWindows30Latin1\0" },
    { 2001, "ISO-8859-1-Windows-3.1-Latin-1\0" },
    { 2001, "csWindows31Latin1\0" },
    { 2002, "ISO-8859-2-Windows-Latin-2\0" },
    { 2002, "csWindows31Latin2\0" },
    { 2003, "ISO-8859-9-Windows-Latin-5\0" },
    { 2003, "csWindows31Latin5\0" },
    { 2004, "hp-roman8\0" },
    { 2004, "roman8\0" },
    { 2004, "r8\0" },
    { 2004, "csHPRoman8\0" },
    { 2005, "Adobe-Standard-Encoding\0" },
    { 2005, "csAdobeStandardEncoding\0" },
    { 2006, "Ventura-US\0" },
    { 2006, "csVenturaUS\0" },
    { 2007, "Ventura-International\0" },
    { 2007, "csVenturaInternational\0" },
    { 2008, "DEC-MCS\0" },
    { 2008, "dec\0" },
    { 2008, "csDECMCS\0" },
    { 2009, "IBM850\0" },
    { 2009, "cp850\0" },
    { 2009, "850\0" },
    { 2009, "csPC850Multilingual\0" },
    { 2012, "PC8-Danish-Norwegian\0" },
    { 2012, "csPC8DanishNorwegian\0" },
    { 2013, "IBM862\0" },
    { 2013, "cp862\0" },
    { 2013, "862\0" },
    { 2013, "csPC862LatinHebrew\0" },
    { 2014, "PC8-Turkish\0" },
    { 2014, "csPC8Turkish\0" },
    { 2015, "IBM-Symbols\0" },
    { 2015, "csIBMSymbols\0" },
    { 2016, "IBM-Thai\0" },
    { 2016, "csIBMThai\0" },
    { 2017, "HP-Legal\0" },
    { 2017, "csHPLegal\0" },
    { 2018, "HP-Pi-font\0" },
    { 2018, "csHPPiFont\0" },
    { 2019, "HP-Math8\0" },
    { 2019, "csHPMath8\0" },
    { 2020, "Adobe-Symbol-Encoding\0" },
    { 2020, "csHPPSMath\0" },
    { 2021, "HP-DeskTop\0" },
    { 2021, "csHPDesktop\0" },
    { 2022, "Ventura-Math\0" },
    { 2022, "csVenturaMath\0" },
    { 2023, "Microsoft-Publishing\0" },
    { 2023, "csMicrosoftPublishing\0" },
    { 2024, "Windows-31J\0" },
    { 2024, "csWindows31J\0" },
    { 2025, "GB2312\0" },
    { 2025, "csGB2312\0" },
    { 2026, "Big5\0" },
    { 2026, "csBig5\0" },
    { 2027, "macintosh\0" },
    { 2027, "mac\0" },
    { 2027, "csMacintosh\0" },
    { 2028, "IBM037\0" },
    { 2028, "cp037\0" },
    { 2028, "ebcdic-cp-us\0" },
    { 2028, "ebcdic-cp-ca\0" },
    { 2028, "ebcdic-cp-wt\0" },
    { 2028, "ebcdic-cp-nl\0" },
    { 2028, "csIBM037\0" },
    { 2029, "IBM038\0" },
    { 2029, "EBCDIC-INT\0" },
    { 2029, "cp038\0" },
    { 2029, "csIBM038\0" },
    { 2030, "IBM273\0" },
    { 2030, "CP273\0" },
    { 2030, "csIBM273\0" },
    { 2031, "IBM274\0" },
    { 2031, "EBCDIC-BE\0" },
    { 2031, "CP274\0" },
    { 2031, "csIBM274\0" },
    { 2032, "IBM275\0" },
    { 2032, "EBCDIC-BR\0" },
    { 2032, "cp275\0" },
    { 2032, "csIBM275\0" },
    { 2033, "IBM277\0" },
    { 2033, "EBCDIC-CP-DK\0" },
    { 2033, "EBCDIC-CP-NO\0" },
    { 2033, "csIBM277\0" },
    { 2034, "IBM278\0" },
    { 2034, "CP278\0" },
    { 2034, "ebcdic-cp-fi\0" },
    { 2034, "ebcdic-cp-se\0" },
    { 2034, "csIBM278\0" },
    { 2035, "IBM280\0" },
    { 2035, "CP280\0" },
    { 2035, "ebcdic-cp-it\0" },
    { 2035, "csIBM280\0" },
    { 2036, "IBM281\0" },
    { 2036, "EBCDIC-JP-E\0" },
    { 2036, "cp281\0" },
    { 2036, "csIBM281\0" },
    { 2037, "IBM284\0" },
    { 2037, "CP284\0" },
    { 2037, "ebcdic-cp-es\0" },
    { 2037, "csIBM284\0" },
    { 2038, "IBM285\0" },
    { 2038, "CP285\0" },
    { 2038, "ebcdic-cp-gb\0" },
    { 2038, "csIBM285\0" },
    { 2039, "IBM290\0" },
    { 2039, "cp290\0" },
    { 2039, "EBCDIC-JP-kana\0" },
    { 2039, "csIBM290\0" },
    { 2040, "IBM297\0" },
    { 2040, "cp297\0" },
    { 2040, "ebcdic-cp-fr\0" },
    { 2040, "csIBM297\0" },
    { 2041, "IBM420\0" },
    { 2041, "cp420\0" },
    { 2041, "ebcdic-cp-ar1\0" },
    { 2041, "csIBM420\0" },
    { 2042, "IBM423\0" },
    { 2042, "cp423\0" },
    { 2042, "ebcdic-cp-gr\0" },
    { 2042, "csIBM423\0" },
    { 2043, "IBM424\0" },
    { 2043, "cp424\0" },
    { 2043, "ebcdic-cp-he\0" },
    { 2043, "csIBM424\0" },
    { 2011, "IBM437\0" },
    { 2011, "cp437\0" },
    { 2011, "437\0" },
    { 2011, "csPC8CodePage437\0" },
    { 2044, "IBM500\0" },
    { 2044, "CP500\0" },
    { 2044, "ebcdic-cp-be\0" },
    { 2044, "ebcdic-cp-ch\0" },
    { 2044, "csIBM500\0" },
    { 2045, "IBM851\0" },
    { 2045, "cp851\0" },
    { 2045, "851\0" },
    { 2045, "csIBM851\0" },
    { 2010, "IBM852\0" },
    { 2010, "cp852\0" },
    { 2010, "852\0" },
    { 2010, "csPCp852\0" },
    { 2046, "IBM855\0" },
    { 2046, "cp855\0" },
    { 2046, "855\0" },
    { 2046, "csIBM855\0" },
    { 2047, "IBM857\0" },
    { 2047, "cp857\0" },
    { 2047, "857\0" },
    { 2047, "csIBM857\0" },
    { 2048, "IBM860\0" },
    { 2048, "cp860\0" },
    { 2048, "860\0" },
    { 2048, "csIBM860\0" },
    { 2049, "IBM861\0" },
    { 2049, "cp861\0" },
    { 2049, "861\0" },
    { 2049, "cp-is\0" },
    { 2049, "csIBM861\0" },
    { 2050, "IBM863\0" },
    { 2050, "cp863\0" },
    { 2050, "863\0" },
    { 2050, "csIBM863\0" },
    { 2051, "IBM864\0" },
    { 2051, "cp864\0" },
    { 2051, "csIBM864\0" },
    { 2052, "IBM865\0" },
    { 2052, "cp865\0" },
    { 2052, "865\0" },
    { 2052, "csIBM865\0" },
    { 2053, "IBM868\0" },
    { 2053, "CP868\0" },
    { 2053, "cp-ar\0" },
    { 2053, "csIBM868\0" },
    { 2054, "IBM869\0" },
    { 2054, "cp869\0" },
    { 2054, "869\0" },
    { 2054, "cp-gr\0" },
    { 2054, "csIBM869\0" },
    { 2055, "IBM870\0" },
    { 2055, "CP870\0" },
    { 2055, "ebcdic-cp-roece\0" },
    { 2055, "ebcdic-cp-yu\0" },
    { 2055, "csIBM870\0" },
    { 2056, "IBM871\0" },
    { 2056, "CP871\0" },
    { 2056, "ebcdic-cp-is\0" },
    { 2056, "csIBM871\0" },
    { 2057, "IBM880\0" },
    { 2057, "cp880\0" },
    { 2057, "EBCDIC-Cyrillic\0" },
    { 2057, "csIBM880\0" },
    { 2058, "IBM891\0" },
    { 2058, "cp891\0" },
    { 2058, "csIBM891\0" },
    { 2059, "IBM903\0" },
    { 2059, "cp903\0" },
    { 2059, "csIBM903\0" },
    { 2060, "IBM904\0" },
    { 2060, "cp904\0" },
    { 2060, "904\0" },
    { 2060, "csIBBM904\0" },
    { 2061, "IBM905\0" },
    { 2061, "CP905\0" },
    { 2061, "ebcdic-cp-tr\0" },
    { 2061, "csIBM905\0" },
    { 2062, "IBM918\0" },
    { 2062, "CP918\0" },
    { 2062, "ebcdic-cp-ar2\0" },
    { 2062, "csIBM918\0" },
    { 2063, "IBM1026\0" },
    { 2063, "CP1026\0" },
    { 2063, "csIBM1026\0" },
    { 2064, "EBCDIC-AT-DE\0" },
    { 2064, "csIBMEBCDICATDE\0" },
    { 2065, "EBCDIC-AT-DE-A\0" },
    { 2065, "csEBCDICATDEA\0" },
    { 2066, "EBCDIC-CA-FR\0" },
    { 2066, "csEBCDICCAFR\0" },
    { 2067, "EBCDIC-DK-NO\0" },
    { 2067, "csEBCDICDKNO\0" },
    { 2068, "EBCDIC-DK-NO-A\0" },
    { 2068, "csEBCDICDKNOA\0" },
    { 2069, "EBCDIC-FI-SE\0" },
    { 2069, "csEBCDICFISE\0" },
    { 2070, "EBCDIC-FI-SE-A\0" },
    { 2070, "csEBCDICFISEA\0" },
    { 2071, "EBCDIC-FR\0" },
    { 2071, "csEBCDICFR\0" },
    { 2072, "EBCDIC-IT\0" },
    { 2072, "csEBCDICIT\0" },
    { 2073, "EBCDIC-PT\0" },
    { 2073, "csEBCDICPT\0" },
    { 2074, "EBCDIC-ES\0" },
    { 2074, "csEBCDICES\0" },
    { 2075, "EBCDIC-ES-A\0" },
    { 2075, "csEBCDICESA\0" },
    { 2076, "EBCDIC-ES-S\0" },
    { 2076, "csEBCDICESS\0" },
    { 2077, "EBCDIC-UK\0" },
    { 2077, "csEBCDICUK\0" },
    { 2078, "EBCDIC-US\0" },
    { 2078, "csEBCDICUS\0" },
    { 2079, "UNKNOWN-8BIT\0" },
    { 2079, "csUnknown8BiT\0" },
    { 2080, "MNEMONIC\0" },
    { 2080, "csMnemonic\0" },
    { 2081, "MNEM\0" },
    { 2081, "csMnem\0" },
    { 2082, "VISCII\0" },
    { 2082, "csVISCII\0" },
    { 2083, "VIQR\0" },
    { 2083, "csVIQR\0" },
    { 2084, "KOI8-R\0" },
    { 2084, "csKOI8R\0" },
    { 2085, "HZ-GB-2312\0" },
    { 2086, "IBM866\0" },
    { 2086, "cp866\0" },
    { 2086, "866\0" },
    { 2086, "csIBM866\0" },
    { 2087, "IBM775\0" },
    { 2087, "cp775\0" },
    { 2087, "csPC775Baltic\0" },
    { 2088, "KOI8-U\0" },
    { 2088, "csKOI8U\0" },
    { 2089, "IBM00858\0" },
    { 2089, "CCSID00858\0" },
    { 2089, "CP00858\0" },
    { 2089, "PC-Multilingual-850+euro\0" },
    { 2089, "csIBM00858\0" },
    { 2090, "IBM00924\0" },
    { 2090, "CCSID00924\0" },
    { 2090, "CP00924\0" },
    { 2090, "ebcdic-Latin9--euro\0" },
    { 2090, "csIBM00924\0" },
    { 2091, "IBM01140\0" },
    { 2091, "CCSID01140\0" },
    { 2091, "CP01140\0" },
    { 2091, "ebcdic-us-37+euro\0" },
    { 2091, "csIBM01140\0" },
    { 2092, "IBM01141\0" },
    { 2092, "CCSID01141\0" },
    { 2092, "CP01141\0" },
    { 2092, "ebcdic-de-273+euro\0" },
    { 2092, "csIBM01141\0" },
    { 2093, "IBM01142\0" },
    { 2093, "CCSID01142\0" },
    { 2093, "CP01142\0" },
    { 2093, "ebcdic-dk-277+euro\0" },
    { 2093, "ebcdic-no-277+euro\0" },
    { 2093, "csIBM01142\0" },
    { 2094, "IBM01143\0" },
    { 2094, "CCSID01143\0" },
    { 2094, "CP01143\0" },
    { 2094, "ebcdic-fi-278+euro\0" },
    { 2094, "ebcdic-se-278+euro\0" },
    { 2094, "csIBM01143\0" },
    { 2095, "IBM01144\0" },
    { 2095, "CCSID01144\0" },
    { 2095, "CP01144\0" },
    { 2095, "ebcdic-it-280+euro\0" },
    { 2095, "csIBM01144\0" },
    { 2096, "IBM01145\0" },
    { 2096, "CCSID01145\0" },
    { 2096, "CP01145\0" },
    { 2096, "ebcdic-es-284+euro\0" },
    { 2096, "csIBM01145\0" },
    { 2097, "IBM01146\0" },
    { 2097, "CCSID01146\0" },
    { 2097, "CP01146\0" },
    { 2097, "ebcdic-gb-285+euro\0" },
    { 2097, "csIBM01146\0" },
    { 2098, "IBM01147\0" },
    { 2098, "CCSID01147\0" },
    { 2098, "CP01147\0" },
    { 2098, "ebcdic-fr-297+euro\0" },
    { 2098, "csIBM01147\0" },
    { 2099, "IBM01148\0" },
    { 2099, "CCSID01148\0" },
    { 2099, "CP01148\0" },
    { 2099, "ebcdic-international-500+euro\0" },
    { 2099, "csIBM01148\0" },
    { 2100, "IBM01149\0" },
    { 2100, "CCSID01149\0" },
    { 2100, "CP01149\0" },
    { 2100, "ebcdic-is-871+euro\0" },
    { 2100, "csIBM01149\0" },
    { 2101, "Big5-HKSCS\0" },
    { 2101, "csBig5HKSCS\0" },
    { 2102, "IBM1047\0" },
    { 2102, "IBM-1047\0" },
    { 2102, "csIBM1047\0" },
    { 2103, "PTCP154\0" },
    { 2103, "csPTCP154\0" },
    { 2103, "PT154\0" },
    { 2103, "CP154\0" },
    { 2103, "Cyrillic-Asian\0" },
    { 2104, "Amiga-1251\0" },
    { 2104, "Ami1251\0" },
    { 2104, "Amiga1251\0" },
    { 2104, "Ami-1251\0" },
    { 2104, "csAmiga1251\0" },
    { 2105, "KOI7-switched\0" },
    { 2105, "csKOI7switched\0" },
    { 2106, "BRF\0" },
    { 2106, "csBRF\0" },
    { 2107, "TSCII\0" },
    { 2107, "csTSCII\0" },
    { 2108, "CP51932\0" },
    { 2108, "csCP51932\0" },
    { 2109, "windows-874\0" },
    { 2109, "cswindows874\0" },
    { 2250, "windows-1250\0" },
    { 2250, "cswindows1250\0" },
    { 2251, "windows-1251\0" },
    { 2251, "cswindows1251\0" },
    { 2252, "windows-1252\0" },
    { 2252, "cswindows1252\0" },
    { 2253, "windows-1253\0" },
    { 2253, "cswindows1253\0" },
    { 2254, "windows-1254\0" },
    { 2254, "cswindows1254\0" },
    { 2255, "windows-1255\0" },
    { 2255, "cswindows1255\0" },
    { 2256, "windows-1256\0" },
    { 2256, "cswindows1256\0" },
    { 2257, "windows-1257\0" },
    { 2257, "cswindows1257\0" },
    { 2258, "windows-1258\0" },
    { 2258, "cswindows1258\0" },
    { 2259, "TIS-620\0" },
    { 2259, "csTIS620\0" },
    { 2259, "ISO-8859-11\0" },
    { 2260, "CP50220\0" },
    { 2260, "csCP50220\0" },
};
static const qint16 MIBTblSize = sizeof(MIBTbl) / sizeof(MIBTblData);

static const UChar nullchar[2] = { 0x5c, 0x30 };
static const UChar questionmarkchar[1] = { 0x3f };

static QByteArray localecodec;

static inline bool nameMatch(const QByteArray &name, const QByteArray &name2)
{
    if (ucnv_compareNames(name.constData(), name2.constData()) == 0) {
        return true;
    }
    UErrorCode error = U_ZERO_ERROR;
    const char *iana = ucnv_getStandardName(name.constData(), "IANA", &error);
    if (Q_UNLIKELY(U_FAILURE(error) || !iana)) {
        return false;
    }
    return (ucnv_compareNames(iana, name2.constData()) == 0);
}

static QByteArray checkForCodec(const QByteArray &name)
{
    foreach(const QByteArray &codec, QTextCodecPrivate::allCodecs()) {
        if (nameMatch(name, codec)) {
            return name;
        }
    }
    const int index = name.indexOf('@');
    if (index != -1) {
        const QByteArray namepart = name.left(index);
        foreach(const QByteArray &codec, QTextCodecPrivate::allCodecs()) {
            if (nameMatch(namepart, codec)) {
                return namepart;
            }
        }
    }
    return QByteArray();
}

QByteArray qt_locale_codec()
{
    if (!localecodec.isEmpty()) {
        return localecodec;
    }

    const QByteArray lang = qGetLang();

    const int indexOfDot = lang.indexOf('.');
    if (indexOfDot != -1) {
        localecodec = checkForCodec(lang.mid(indexOfDot + 1));
    }

    if (localecodec.isEmpty() && !lang.isEmpty()) {
        localecodec = checkForCodec(lang);
    }

    // Fallback to implementation-defined default locale
    if (localecodec.isEmpty()) {
        const QByteArray charset = ::nl_langinfo(CODESET); // deep copy
        localecodec = checkForCodec(charset);
    }

    // nl_langinfo() is documented to return empty string only if its argument
    // is not valid
    if (Q_UNLIKELY(localecodec.isEmpty())) {
        qFatal("Could not detect codec for locale");
    }

    return localecodec;
}


QTextCodecPrivate::QTextCodecPrivate(const QByteArray &aname)
    : name(aname)
{
}

QTextCodecPrivate::QTextCodecPrivate(const int mib)
{
    for (qint16 i = 0; i < MIBTblSize; i++) {
        if (mib == MIBTbl[i].mib) {
            name = MIBTbl[i].name;
            return;
        }
    }

    name = "latin1";
}

QList<QByteArray> QTextCodecPrivate::allCodecs()
{
    QList<QByteArray> allcodecs;

    for (int i = 0; i < ucnv_countAvailable(); i++) {
        const char *name = ucnv_getAvailableName(i);
        UErrorCode error = U_ZERO_ERROR;
        const char *iana = ucnv_getStandardName(name, "IANA", &error);
        if (Q_UNLIKELY(U_FAILURE(error) || !iana)) {
            continue;
        }
        for (qint16 m = 0; m < MIBTblSize; m++) {
            if (ucnv_compareNames(iana, MIBTbl[m].name) == 0) {
                allcodecs += QByteArray::fromRawData(iana, qstrlen(iana));
                break;
            }
        }
    }

    return allcodecs;
}

QList<int> QTextCodecPrivate::allMibs()
{
    QList<int> allmibs;

    for (int i = 0; i < ucnv_countAvailable(); i++) {
        const char *name = ucnv_getAvailableName(i);
        UErrorCode error = U_ZERO_ERROR;
        const char *iana = ucnv_getStandardName(name, "IANA", &error);
        if (Q_UNLIKELY(U_FAILURE(error) || !iana)) {
            continue;
        }
        for (qint16 m = 0; m < MIBTblSize; m++) {
            if (ucnv_compareNames(iana, MIBTbl[m].name) == 0) {
                if (!allmibs.contains(MIBTbl[m].mib)) {
                    allmibs.append(MIBTbl[m].mib);
                }
                break;
            }
        }
    }

    return allmibs;
}

QString QTextCodecPrivate::convertTo(const char *data, int length, const char* const codec)
{
    UErrorCode error = U_ZERO_ERROR;
    UConverter *conv = ucnv_open(codec, &error);
    if (Q_UNLIKELY(U_FAILURE(error))) {
        return QString();
    }

    error = U_ZERO_ERROR;
    ucnv_setSubstString(conv, questionmarkchar, 1, &error);

    const int maxbytes = UCNV_GET_MAX_BYTES_FOR_STRING(length, ucnv_getMaxCharSize(conv));
    QStdVector<UChar> result(maxbytes);
    error = U_ZERO_ERROR;
    const int convresult = ucnv_toUChars(conv, result.data(), maxbytes, data, length, &error);
    ucnv_close(conv);

    if (Q_LIKELY(U_SUCCESS(error))) {
        return QString(reinterpret_cast<const QChar*>(result.constData()), convresult);
    }

    return QString();
}

QByteArray QTextCodecPrivate::convertFrom(const QChar *unicode, int length, const char* const codec)
{
    UErrorCode error = U_ZERO_ERROR;
    UConverter *conv = ucnv_open(codec, &error);
    if (Q_UNLIKELY(U_FAILURE(error))) {
        return QByteArray();
    }

    error = U_ZERO_ERROR;
    ucnv_setSubstString(conv, questionmarkchar, 1, &error);

    const int maxbytes = UCNV_GET_MAX_BYTES_FOR_STRING(length, ucnv_getMaxCharSize(conv));
    QStdVector<char> result(maxbytes);
    error = U_ZERO_ERROR;
    const int convresult = ucnv_fromUChars(conv, result.data(), maxbytes,
        reinterpret_cast<const UChar *>(unicode), length, &error);
    ucnv_close(conv);

    if (Q_LIKELY(U_SUCCESS(error))) {
        return QByteArray(result.constData(), convresult);
    }

    return QByteArray();
}

#ifndef QT_NO_TEXTCODEC
static void icu_from_callback(
    const void* context,
    UConverterFromUnicodeArgs *args,
    const UChar* codeUnits,
    int32_t length,
    UChar32 codePoint,
    UConverterCallbackReason reason,
    UErrorCode *pErrorCode)
{
    if (reason == UCNV_ILLEGAL || reason == UCNV_UNASSIGNED) {
        const QTextConverterPrivate* codec = static_cast<const QTextConverterPrivate*>(context);
        codec->invalidChars(length);
        // reset error code so that conversion continues
        *pErrorCode = U_ZERO_ERROR;
    }
    UCNV_FROM_U_CALLBACK_SUBSTITUTE(NULL, args, codeUnits, length, codePoint, reason, pErrorCode);
}

static void icu_to_callback (
    const void* context,
    UConverterToUnicodeArgs *args,
    const char *codeUnits,
    int32_t length,
    UConverterCallbackReason reason,
    UErrorCode *pErrorCode)
{
    if (reason == UCNV_ILLEGAL || reason == UCNV_UNASSIGNED) {
        const QTextConverterPrivate* codec = static_cast<const QTextConverterPrivate*>(context);
        codec->invalidChars(length);
        *pErrorCode = U_ZERO_ERROR;
    }
    UCNV_TO_U_CALLBACK_SUBSTITUTE(NULL, args, codeUnits, length, reason, pErrorCode);
}

QTextConverterPrivate::QTextConverterPrivate(const QByteArray &aname)
    : name(aname),
    flags(QTextConverter::DefaultConversion),
    conv(nullptr),
    invalidchars(0)
{
}

QTextConverterPrivate::QTextConverterPrivate(const int mib)
    : flags(QTextConverter::DefaultConversion),
    conv(nullptr),
    invalidchars(0)
{
    for (qint16 i = 0; i < MIBTblSize; i++) {
        if (mib == MIBTbl[i].mib) {
            name = MIBTbl[i].name;
            return;
        }
    }

    name = "latin1";
}

QTextConverterPrivate::~QTextConverterPrivate()
{
    if (conv) {
        ucnv_close(conv);
    }
}

UConverter* QTextConverterPrivate::getConverter()
{
    if (conv) {
        return conv;
    }

    UErrorCode error = U_ZERO_ERROR;
    conv = ucnv_open(name.constData(), &error);
    if (Q_UNLIKELY(U_FAILURE(error))) {
        return nullptr;
    }

    error = U_ZERO_ERROR;
    if (flags & QTextConverter::ConvertInvalidToNull) {
        ucnv_setSubstString(conv, nullchar, 2, &error);
    } else {
        ucnv_setSubstString(conv, questionmarkchar, 1, &error);
    }

    error = U_ZERO_ERROR;
    ucnv_setToUCallBack(conv, icu_to_callback, this, nullptr, nullptr, &error);
    error = U_ZERO_ERROR;
    ucnv_setFromUCallBack(conv, icu_from_callback, this, nullptr, nullptr, &error);

    return conv;
}

void QTextConverterPrivate::invalidChars(int length) const
{
    invalidchars += length;
}


#ifndef QT_NO_THREAD
Q_GLOBAL_STATIC(QMutex, textCodecsMutex)
#endif

class QTextCodecCleanup : public QList<QTextCodec*>
{
public:
    ~QTextCodecCleanup();
};

/*
    Deletes all the created codecs. This destructor is called just
    before exiting to delete any QTextCodec objects that may be lying
    around.
*/
QTextCodecCleanup::~QTextCodecCleanup()
{
    while (!isEmpty()) {
        delete takeLast();
    }
    localecodec.clear();
#ifndef QT_NO_DEBUG
    u_cleanup();
#endif
}
Q_GLOBAL_STATIC(QTextCodecCleanup, qGlobalQTextCodec)

/*!
    \class QTextCodec
    \brief The QTextCodec class provides conversions between text encodings.
    \reentrant
    \ingroup i18n

    Katie uses Unicode to store, draw and manipulate strings. In many
    situations you may wish to deal with data that uses a different
    encoding. For example, most Japanese documents are still stored
    in Shift-JIS or ISO 2022-JP, while Russian users often have their
    documents in KOI8-R or Windows-1251.

    Katie provides QTextCodec class to help with converting non-Unicode
    formats to and from Unicode. Some of the supported encodings are:

    \list
    \o Adobe-Standard-Encoding
    \o ANSI_X3.4-1968
    \o Big5
    \o Big5-HKSCS
    \o BOCU-1
    \o CESU-8
    \o EUC-KR
    \o Extended_UNIX_Code_Packed_Format_for_Japanese
    \o gb18030
    \o GB2312
    \o GB_2312-80
    \o GBK
    \o hp-roman8
    \o HZ-GB-2312
    \o IBM00858
    \o IBM01140
    \o IBM01141
    \o IBM01142
    \o IBM01143
    \o IBM01144
    \o IBM01145
    \o IBM01146
    \o IBM01147
    \o IBM01148
    \o IBM01149
    \o IBM037
    \o IBM1026
    \o IBM1047
    \o IBM273
    \o IBM277
    \o IBM278
    \o IBM280
    \o IBM284
    \o IBM285
    \o IBM290
    \o IBM297
    \o IBM420
    \o IBM424
    \o IBM437
    \o IBM500
    \o IBM775
    \o IBM850
    \o IBM851
    \o IBM852
    \o IBM855
    \o IBM857
    \o IBM860
    \o IBM861
    \o IBM862
    \o IBM863
    \o IBM864
    \o IBM865
    \o IBM866
    \o IBM868
    \o IBM869
    \o IBM870
    \o IBM871
    \o IBM918
    \o IBM-Thai
    \o ISO-2022-CN
    \o ISO-2022-CN-EXT
    \o ISO-2022-JP
    \o ISO-2022-JP-2
    \o ISO-2022-KR
    \o ISO-8859-10
    \o ISO_8859-1:1987
    \o ISO-8859-13
    \o ISO-8859-14
    \o ISO-8859-15
    \o ISO_8859-2:1987
    \o ISO_8859-3:1988
    \o ISO_8859-4:1988
    \o ISO_8859-5:1988
    \o ISO_8859-6:1987
    \o ISO_8859-7:1987
    \o ISO_8859-8:1988
    \o ISO_8859-9:1989
    \o JIS_Encoding
    \o KOI8-R
    \o KOI8-U
    \o KS_C_5601-1987
    \o macintosh
    \o SCSU
    \o Shift_JIS
    \o TIS-620
    \o UTF-16
    \o UTF-16BE
    \o UTF-16LE
    \o UTF-32
    \o UTF-32BE
    \o UTF-32LE
    \o UTF-7
    \o UTF-8
    \o windows-1250
    \o windows-1251
    \o windows-1252
    \o windows-1253
    \o windows-1254
    \o windows-1255
    \o windows-1256
    \o windows-1257
    \o windows-1258

    \o System Any of the above depending on the system locale
    \endlist

    QTextCodecs can be used as follows to convert some locally encoded
    string to Unicode. Suppose you have some string encoded in Russian
    KOI8-R encoding, and want to convert it to Unicode. The simple way
    to do it is like this:

    \snippet doc/src/snippets/code/src_corelib_codecs_qtextcodec.cpp 0

    After this, \c string holds the text converted to Unicode.
    Converting a string from Unicode to the local encoding is just as
    easy:

    \snippet doc/src/snippets/code/src_corelib_codecs_qtextcodec.cpp 1

    To read or write files in various encodings, use QTextStream and
    its \l{QTextStream::setCodec()}{setCodec()} function. See the
    \l{tools/codecs}{Codecs} example for an application of QTextCodec
    to file I/O.

    \sa QTextStream, QTextConverter, {Codecs Example}
*/

/*!
    \internal
*/
QTextCodec::QTextCodec(const QByteArray &codec)
    : d_ptr(new QTextCodecPrivate(codec))
{
}

/*!
    \internal
*/
QTextCodec::QTextCodec(const int codec)
    : d_ptr(new QTextCodecPrivate(codec))
{
}

/*!
    \internal
*/
QTextCodec::~QTextCodec()
{
    delete d_ptr;
}

/*!
    Searches all installed QTextCodec objects and returns the one
    which best matches \a name; the match is case-insensitive. Returns
    0 if no codec matching the name \a name could be found.
*/
QTextCodec *QTextCodec::codecForName(const QByteArray &name)
{
    if (name.isEmpty())
        return nullptr;

    if (name == "System")
        return QTextCodec::codecForLocale();

    {
#ifndef QT_NO_THREAD
        QMutexLocker locker(textCodecsMutex());
#endif
        for (int i = 0; i < qGlobalQTextCodec()->size(); ++i) {
            QTextCodec *cursor = qGlobalQTextCodec()->at(i);
            if (nameMatch(name, cursor->name())) {
                return cursor;
            }
            foreach (const QByteArray &alias, cursor->aliases()) {
                if (nameMatch(name, alias)) {
                    return cursor;
                }
            }
        }
    }

    foreach(const QByteArray &codec, QTextCodec::availableCodecs()) {
        if (nameMatch(name, codec)) {
#ifndef QT_NO_THREAD
            QMutexLocker locker(textCodecsMutex());
#endif
            QTextCodec* newcodec = new QTextCodec(codec);
            qGlobalQTextCodec()->append(newcodec);
            return newcodec;
        }
    }

    return nullptr;
}


/*!
    Returns the QTextCodec which matches the \link
    QTextCodec::mibEnum() MIBenum\endlink \a mib.
*/
QTextCodec* QTextCodec::codecForMib(int mib)
{
    {
#ifndef QT_NO_THREAD
        QMutexLocker locker(textCodecsMutex());
#endif
        for (int i = 0; i < qGlobalQTextCodec()->size(); ++i) {
            QTextCodec *cursor = qGlobalQTextCodec()->at(i);
            if (cursor->mibEnum() == mib) {
                return cursor;
            }
        }
    }

    foreach(const int codec, QTextCodec::availableMibs()) {
        if (mib == codec) {
#ifndef QT_NO_THREAD
            QMutexLocker locker(textCodecsMutex());
#endif
            QTextCodec* newcodec = new QTextCodec(codec);
            qGlobalQTextCodec()->append(newcodec);
            return newcodec;
        }
    }

    return nullptr;
}

/*!
    Returns the list of all available codecs, by name. Call
    QTextCodec::codecForName() to obtain the QTextCodec for the name.

    The list may contain many mentions of the same codec
    if the codec has aliases.

    \sa availableMibs(), name(), aliases()
*/
QList<QByteArray> QTextCodec::availableCodecs()
{
    static const QList<QByteArray> allcodecs = QList<QByteArray>()
        << "System"
        << QTextCodecPrivate::allCodecs();
    return allcodecs;
}

/*!
    Returns the list of MIBs for all available codecs. Call
    QTextCodec::codecForMib() to obtain the QTextCodec for the MIB.

    \sa availableCodecs(), mibEnum()
*/
QList<int> QTextCodec::availableMibs()
{
    static const QList<int> allmibs = QTextCodecPrivate::allMibs();
    return allmibs;
}

/*!
    Set the codec to \a c; this will be returned by
    codecForLocale(). If \a c is a null pointer, the codec is reset to
    the default.

    This might be needed for some applications that want to use their
    own mechanism for setting the locale.

    \sa codecForLocale()
*/
void QTextCodec::setCodecForLocale(QTextCodec *c)
{
    if (c) {
        localecodec = c->name();
    } else {
        localecodec = qt_locale_codec();
    }
}

/*!
    Returns a pointer to the codec most suitable for this locale.
*/

QTextCodec* QTextCodec::codecForLocale()
{
    if (localecodec.isEmpty()) {
        localecodec = qt_locale_codec();
    }
    return QTextCodec::codecForName(localecodec);
}

/*!
    Returns \l{IANA character-sets encoding file} mime name for the
    codec.
*/
QByteArray QTextCodec::name() const
{
    return d_ptr->name;
}

/*!
    Returns the MIBenum (see \l{IANA character-sets encoding file}
    for more information).
*/
int QTextCodec::mibEnum() const
{
    // QTextCodecPrivate::allCodecs() returns IANA names so it should be in the table
    for (qint16 i = 0; i < MIBTblSize; i++) {
        if (ucnv_compareNames(d_ptr->name.constData(), MIBTbl[i].name) == 0) {
            return MIBTbl[i].mib;
        }
    }

    // 2 is for unknown, see https://www.iana.org/assignments/ianacharset-mib/ianacharset-mib
    return 2;
}

/*!
    Returns standard aliases for codecs can be found in the
    \l{IANA character-sets encoding file}.
*/
QList<QByteArray> QTextCodec::aliases() const
{
    QList<QByteArray> aliases;

    UErrorCode error = U_ZERO_ERROR;
    const int count = ucnv_countAliases(d_ptr->name.constData(), &error);
    if (Q_UNLIKELY(U_FAILURE(error))) {
        return aliases;
    }

    for (int i = 0; i < count; i++) {
        error = U_ZERO_ERROR;
        const char *alias = ucnv_getAlias(d_ptr->name.constData(), i, &error);
        if (Q_UNLIKELY(U_FAILURE(error))) {
            continue;
        }
        // aliases contain original
        if (Q_LIKELY(qstrcmp(d_ptr->name.constData(), alias) != 0)) {
            aliases += QByteArray::fromRawData(alias, qstrlen(alias));
        }
    }

    return aliases;
}

/*!
    Converts the first \a length of characters from the \a data array
    from Unicode to the encoding of this codec, and returns the result
    in a QByteArray.
*/
QByteArray QTextCodec::fromUnicode(const QChar *data, int length) const
{
    return QTextCodecPrivate::convertFrom(data, length, d_ptr->name.constData());
}

/*!
    Converts the first \a size characters from the \a data from the
    encoding of this codec to Unicode, and returns the result in a
    QString.
*/
QString QTextCodec::toUnicode(const char *data, int length) const
{
    return QTextCodecPrivate::convertTo(data, length, d_ptr->name.constData());
}

/*!
    Returns true if the Unicode character \a ch can be fully encoded
    with this codec; otherwise returns false.
*/
bool QTextCodec::canEncode(QChar ch) const
{
    return canEncode(QString(ch));
}

/*!
    \overload

    \a s contains the string being tested for encode-ability.
*/
bool QTextCodec::canEncode(const QString& s) const
{
    QTextConverter textconverter = converter();
    textconverter.setFlags(QTextConverter::ConvertInvalidToNull);
    (void)textconverter.fromUnicode(s);
    return !textconverter.hasFailure();
}

/*!
    \since 4.4

    Tries to detect the encoding of the provided snippet of HTML in
    the given byte array, \a ba, by checking the BOM (Byte Order Mark)
    and the content-type meta header and returns a QTextCodec instance
    that is capable of decoding the html to unicode.  If the codec
    cannot be detected from the content provided, \a defaultCodec is
    returned.

    \sa codecForText(), codecForUtfText()
*/
QTextCodec *QTextCodec::codecForHtml(const QByteArray &ba, QTextCodec *defaultCodec)
{
    // determine charset
    QTextCodec *c = QTextCodec::codecForUtfText(ba, nullptr);
    if (!c) {
        QByteArray header = ba.left(512).toLower();
        int pos = header.indexOf("http-equiv=");
        if (pos != -1) {
            pos = header.lastIndexOf("meta ", pos);
            if (pos != -1) {
                pos = header.indexOf("charset=", pos) + qstrlen("charset=");
                if (pos != -1) {
                    int pos2 = header.indexOf('\"', pos+1);
                    QByteArray cs = header.mid(pos, pos2-pos);
                    //            qDebug("found charset: %s", cs.data());
                    return QTextCodec::codecForName(cs);
                }
            }
        }
    }
    return QTextCodec::codecForText(ba, defaultCodec);
}

/*!
    \overload

    Tries to detect the encoding of the provided snippet of HTML in
    the given byte array, \a ba, by checking the BOM (Byte Order Mark)
    and the content-type meta header and returns a QTextCodec instance
    that is capable of decoding the html to unicode. If the codec cannot
    be detected, this overload returns a Latin-1 QTextCodec.

    \sa codecForText(), codecForUtfText()
*/
QTextCodec *QTextCodec::codecForHtml(const QByteArray &ba)
{
    return codecForHtml(ba, QTextCodec::codecForMib(/*Latin 1*/ 4));
}

/*!
    \since 4.6

    Tries to detect the encoding of the provided snippet \a ba by
    using the BOM (Byte Order Mark) and returns a QTextCodec instance
    that is capable of decoding the text to unicode. If the codec
    cannot be detected from the content provided, \a defaultCodec is
    returned.

    \sa codecForText(), codecForHtml()
*/
QTextCodec *QTextCodec::codecForUtfText(const QByteArray &ba, QTextCodec *defaultCodec)
{
    UErrorCode error = U_ZERO_ERROR;
    const char* name = ucnv_detectUnicodeSignature(ba.constData(), ba.size(), nullptr, &error);
    if (Q_UNLIKELY(U_FAILURE(error))) {
        return defaultCodec;
    } else if (name) {
        return QTextCodec::codecForName(name);
    }
    return defaultCodec;
}

/*!
    \overload

    Tries to detect the encoding of the provided snippet \a ba by
    using the BOM (Byte Order Mark) and returns a QTextCodec instance
    that is capable of decoding the text to unicode. If the codec
    cannot be detected, this overload returns a Latin-1 QTextCodec.

    \sa codecForText(), codecForHtml()
*/
QTextCodec *QTextCodec::codecForUtfText(const QByteArray &ba)
{
    return codecForUtfText(ba, QTextCodec::codecForMib(/*Latin 1*/ 4));
}

/*!
    \since 4.9

    Tries to detect the encoding of the provided snippet \a ba and
    returns a QTextCodec instance that is capable of decoding the text
    to unicode. If the codec cannot be detected from the content
    provided, \a defaultCodec is returned. The results can not be
    guaranteed to always be correct.

    \sa codecForUtfText(), codecForHtml()
*/
QTextCodec *QTextCodec::codecForText(const QByteArray &ba, QTextCodec *defaultCodec)
{
    UErrorCode error = U_ZERO_ERROR;
    UCharsetDetector *detector = ucsdet_open(&error);
    if (Q_UNLIKELY(U_FAILURE(error))) {
        return defaultCodec;
    }

    error = U_ZERO_ERROR;
    ucsdet_setText(detector, ba.constData(), ba.size(), &error);
    if (Q_UNLIKELY(U_FAILURE(error))) {
        ucsdet_close(detector);
        return defaultCodec;
    }

    error = U_ZERO_ERROR;
    const UCharsetMatch *match = ucsdet_detect(detector, &error);
    if (Q_UNLIKELY(U_FAILURE(error))) {
        ucsdet_close(detector);
        return defaultCodec;
    }

    error = U_ZERO_ERROR;
    const QByteArray name = ucsdet_getName(match, &error);
    if (Q_UNLIKELY(U_FAILURE(error))) {
        ucsdet_close(detector);
        return defaultCodec;
    }

    ucsdet_close(detector);
    return QTextCodec::codecForName(name.constData());
}

/*!
    \overload

    Tries to detect the encoding of the provided snippet \a ba by
    returns a QTextCodec instance that is capable of decoding the
    text to unicode. If the codec cannot be detected, this overload
    returns a Latin-1 QTextCodec. The results can not be guaranteed
    to always be correct.

    \sa codecForUtfText(), codecForHtml()
*/
QTextCodec *QTextCodec::codecForText(const QByteArray &ba)
{
    return codecForText(ba, QTextCodec::codecForMib(/*Latin 1*/ 4));
}

/*!
    \since 4.12

    Returns QTextConverter for this codec.

    \fn QTextConverter QTextCodec::converter() const
*/

/*!
    \class QTextConverter
    \brief The QTextConverter class provides conversions between text encodings.
    \since 4.12
    \reentrant
    \ingroup i18n

    Some care must be taken when trying to convert the data in chunks,
    for example, when receiving it over a network. In such cases it is
    possible that a multi-byte character will be split over two
    chunks. At best this might result in the loss of a character and
    at worst cause the entire conversion to fail.

    The approach to use in these situations is to create a QTextConverter
    object for the codec and use this QTextConverter for the whole
    decoding process. The QTextConverter object maintains state between
    chunks and therefore works correctly even if a multi-byte character
    is split between chunks.

    \sa QTextStream, QTextCodec, {Codecs Example}
*/

/*!
    \enum QTextConverter::ConversionFlag

    \value DefaultConversion  No flag is set.
    \value ConvertInvalidToNull  If this flag is set, each invalid input
                                 character is output as a null character.
*/

/*!
    Constructs a QTextConverter for the codec with name \a name.
*/
QTextConverter::QTextConverter()
    : d_ptr(new QTextConverterPrivate(4))
{
}

/*!
    Constructs a QTextConverter copy of \a other.
*/
QTextConverter::QTextConverter(const QTextConverter &other)
{
    operator=(other);
}

/*!
    Constructs a QTextConverter for the codec with name \a name.
*/
QTextConverter::QTextConverter(const QByteArray &name)
    : d_ptr(new QTextConverterPrivate(name))
{
}

/*!
    Constructs a QTextConverter for the codec with MIB \a mib.
*/
QTextConverter::QTextConverter(const int mib)
    : d_ptr(new QTextConverterPrivate(mib))
{
}

/*!
    Destroys the QTextConverter.
*/
QTextConverter::~QTextConverter()
{
    delete d_ptr;
}

/*!
    Returns the flags used for decoding and encoding.

    \sa setFlags()
*/
QTextConverter::ConversionFlags QTextConverter::flags() const
{
    return d_ptr->flags;
}

/*!
    Sets the flags used for decoding and encoding.

    \sa flags()
*/
void QTextConverter::setFlags(const QTextConverter::ConversionFlags flags)
{
    d_ptr->flags = flags;
}

/*!
    Determines whether the codec encountered a failure while decoding or
    encoding. If an error was encountered, the produced result is undefined
    and gets converted as according to the conversion flags.
*/
bool QTextConverter::hasFailure() const
{
    return (d_ptr->invalidchars != 0);
}

/*!
    Resets the state of the codec.
*/
void QTextConverter::reset()
{
    if (d_ptr->conv) {
        ucnv_reset(d_ptr->conv);
    }
    d_ptr->invalidchars = 0;
}

/*!
    Assigns \a other to this QTextCodec, returns a reference to this object.
*/
QTextConverter& QTextConverter::operator=(const QTextConverter &other)
{
    if (d_ptr == other.d_ptr) {
        return *this;
    }
    if (d_ptr->conv) {
        ucnv_close(d_ptr->conv);
        d_ptr->conv = nullptr;
    }
    if (other.d_ptr->conv) {
        UErrorCode error = U_ZERO_ERROR;
        d_ptr->conv = ucnv_safeClone(other.d_ptr->conv, nullptr, nullptr, &error);
        if (Q_UNLIKELY(U_FAILURE(error))) {
            d_ptr->conv = nullptr;
        }
    }
    d_ptr->name = other.d_ptr->name;
    d_ptr->flags = other.d_ptr->flags;
    d_ptr->invalidchars = other.d_ptr->invalidchars;
    return *this;
}

/*!
    Converts the first \a length of characters from the \a data array
    from Unicode to the encoding of this codec, and returns the result
    in a QByteArray.
*/
QByteArray QTextConverter::fromUnicode(const QChar *data, int length) const
{
    UConverter *conv = d_ptr->getConverter();
    if (!conv) {
        return QByteArray();
    }

    const int maxbytes = UCNV_GET_MAX_BYTES_FOR_STRING(length, ucnv_getMaxCharSize(conv));
    QStdVector<char> result(maxbytes);
    UErrorCode error = U_ZERO_ERROR;
    const int convresult = ucnv_fromUChars(conv, result.data(), maxbytes,
        reinterpret_cast<const UChar *>(data), length, &error);
    return QByteArray(result.constData(), convresult);
}

/*!
    Converts the first \a size characters from the \a data from the
    encoding of this codec to Unicode, and returns the result in a
    QString.
*/
QString QTextConverter::toUnicode(const char *data, int length) const
{
    UConverter *conv = d_ptr->getConverter();
    if (!conv) {
        return QString();
    }

    const int maxbytes = UCNV_GET_MAX_BYTES_FOR_STRING(length, ucnv_getMaxCharSize(conv));
    QStdVector<UChar> result(maxbytes);
    UErrorCode error = U_ZERO_ERROR;
    const int convresult = ucnv_toUChars(conv, result.data(), maxbytes, data, length, &error);
    // regardless if the data has BOM or not, BOMs shall be generated explicitly only by QTextStream
    if (convresult >= 4 && qstrnicmp("UTF-32", d_ptr->name.constData(), 6) == 0) {
        const uchar* resultuchar = reinterpret_cast<const uchar*>(result.constData());
        Q_ASSERT(sizeof(qt_utf32le_bom) == sizeof(qt_utf32be_bom));
        if (::memcmp(resultuchar, qt_utf32le_bom, sizeof(qt_utf32le_bom)) == 0
            || ::memcmp(resultuchar, qt_utf32be_bom, sizeof(qt_utf32be_bom)) == 0) {
            const int bomoffset = (sizeof(qt_utf32le_bom) / sizeof(QChar));
            return QString(reinterpret_cast<const QChar*>(result.constData()) + bomoffset, convresult - bomoffset);
        }
    } else if (convresult >= 2 && qstrnicmp("UTF-16", d_ptr->name.constData(), 6) == 0) {
        const uchar* resultuchar = reinterpret_cast<const uchar*>(result.constData());
        Q_ASSERT(sizeof(qt_utf16le_bom) == sizeof(qt_utf16be_bom));
        if (::memcmp(resultuchar, qt_utf16le_bom, sizeof(qt_utf16le_bom)) == 0
            || ::memcmp(resultuchar, qt_utf16be_bom, sizeof(qt_utf16be_bom)) == 0) {
            const int bomoffset = (sizeof(qt_utf16le_bom) / sizeof(QChar));
            return QString(reinterpret_cast<const QChar*>(result.constData()) + bomoffset, convresult - bomoffset);
        }
    }
    return QString(reinterpret_cast<const QChar*>(result.constData()), convresult);
}

#endif // QT_NO_TEXTCODEC

QT_END_NAMESPACE

