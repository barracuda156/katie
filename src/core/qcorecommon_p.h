#ifndef QCORECOMMON_P_H
#define QCORECOMMON_P_H

#include "qstring.h"
#include "qstringlist.h"
#include "qfile.h"

#include <time.h>
#include <limits.h>
#include <ctype.h>

QT_BEGIN_NAMESPACE

#define Q_VOID

#define QBYTEARRAY_MAX INT_MAX

// enough space to hold BOM, each char as surrogate pair and terminator
#define QMAXUSTRLEN(X) 4 + (X * 2) + 2

// VLAs are nasty but using them by-passes std::bad_alloc exception for release
// builds (negative size VLAs are a thing, exceptions may even be disabled via
// QT_NO_EXCEPTIONS) and replaces it with assert for debug builds
#define QSTACKARRAY(arraytype, arrayname, arraysize) \
    Q_ASSERT(arraysize >= 1); \
    arraytype arrayname[arraysize]; \
    ::memset(arrayname, 0, arraysize * sizeof(arraytype));

static const qreal q_deg2rad = qreal(0.01745329251994329576923690768489); /* pi/180 */
static const qreal q_rad2deg = qreal(57.295779513082320876798154814105); /* 180/pi */

#ifndef QT_NO_TEXTCODEC
static const uchar qt_utf32le_bom[] = { 0xff, 0xfe, 0x00, 0x00 }; // MIB 1019
static const uchar qt_utf32be_bom[] = { 0x00, 0x00, 0xfe, 0xff }; // MIB 1018
static const uchar qt_utf16le_bom[] = { 0xff, 0xfe }; // MIB 1014
static const uchar qt_utf16be_bom[] = { 0xfe, 0xff }; // MIB 1013
#endif

static inline uint foldCase(const ushort *ch, const ushort *start)
{
    uint c = *ch;
    if (QChar::isLowSurrogate(c) && ch > start && QChar::isHighSurrogate(*(ch - 1)))
        c = QChar::surrogateToUcs4(*(ch - 1), c);
    return QChar::toCaseFolded(c);
}

static inline uint foldCase(const uint ch, uint &last)
{
    uint c = ch;
    if (QChar::isLowSurrogate(c) && QChar::isHighSurrogate(last))
        c = QChar::surrogateToUcs4(last, c);
    last = c;
    return QChar::toCaseFolded(c);
}

static inline QString timeZone()
{
    // posix compliant system
    time_t ltime;
    ::time(&ltime);

    ::tzset();
#if !defined(QT_NO_THREAD)
    // use the reentrant version of localtime() where available
    struct tm res;
    struct tm *t = ::localtime_r(&ltime, &res);
#else
    struct tm *t = ::localtime(&ltime);
#endif // !QT_NO_THREAD

#if defined(QT_HAVE_TM_TM_ZONE)
    return QString::fromLocal8Bit(t->tm_zone);
#else
    return QString::fromLocal8Bit(tzname[t->tm_isdst]);
#endif
}

// Returns a human readable representation of the first \a len
// characters in \a data.
static inline QByteArray qt_prettyDebug(const char *data, int len, int maxSize)
{
    if (!data) return "(null)";
    QByteArray out;
    for (int i = 0; i < len; ++i) {
        char c = data[i];
        if (isprint(int(c))) {
            out += c;
        } else {
            switch (c) {
                case '\n': out += "\\n"; break;
                case '\r': out += "\\r"; break;
                case '\t': out += "\\t"; break;
                default: {
                    QSTACKARRAY(char, snprintfbuf, 5);
                    ::snprintf(snprintfbuf, sizeof(snprintfbuf), "\\%3o", c);
                    out += QByteArray(snprintfbuf, sizeof(snprintfbuf) - 1);
                }
            }
        }
    }

    if (len < maxSize)
        out += "...";

    return out;
}

static inline QString qGetEnv(const char* const name)
{
    return QFile::decodeName(qgetenv(name));
}

static inline QStringList qGetEnvList(const char* const name)
{
    QStringList result;
    const QByteArray location(qgetenv(name));
    foreach (const QByteArray &path, location.split(':')) {
        if (path.isEmpty()) {
            continue;
        }
        result.append(QFile::decodeName(path));
    }
    return result;
}

// Get the first non-empty value from $LC_ALL, $LC_CTYPE and $LANG environment
// variables.
static inline QByteArray qGetLang()
{
    QByteArray result = qgetenv("LC_ALL");
    if (result.isEmpty()) {
        result = qgetenv("LC_CTYPE");
    }
    if (result.isEmpty()) {
        result = qgetenv("LANG");
    }
    return result;
}

QT_END_NAMESPACE

#endif // QCORECOMMON_P_H
