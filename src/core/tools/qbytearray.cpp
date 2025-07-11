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

#include "qbytearray.h"
#include "qbytearraymatcher.h"
#include "qstring.h"
#include "qlist.h"
#include "qlocale.h"
#include "qlocale_p.h"
#include "qlocale_tools_p.h"
#include "qdatastream.h"
#include "qplatformdefs.h"
#include "qcorecommon_p.h"

#include <ctype.h>
#include <limits.h>
#include <string.h>
#include <stdlib.h>

#ifndef QT_BOOTSTRAPPED
#  include <libdeflate.h>
#endif

#define IS_RAW_DATA(d) ((d)->data != (d)->array)

QT_BEGIN_NAMESPACE


int qFindByteArray(
    const char *haystack0, int haystackLen, int from,
    const char *needle0, int needleLen);

/*****************************************************************************
  Safe and portable C string functions; extensions to standard string.h
 *****************************************************************************/

/*! \relates QByteArray

    Returns a duplicate string.

    Allocates space for a copy of \a src, copies it, and returns a
    pointer to the copy. If \a src is 0, it immediately returns 0.

    Ownership is passed to the caller, so the returned string must be
    deleted using \c delete[].
*/

char* qstrdup(const char *src)
{
    if (!src)
        return nullptr;
    char *dst = new char[strlen(src) + 1];
    return strcpy(dst, src);
}

/*! \relates QByteArray

    Copies all the characters up to and including the '\\0' from \a
    src into \a dst and returns a pointer to \a dst. If \a src is 0,
    it immediately returns 0.

    This function assumes that \a dst is large enough to hold the
    contents of \a src.

    \sa qstrncpy()
*/

char* qstrcpy(char *dst, const char *src)
{
    if (!src)
        return nullptr;
    return strcpy(dst, src);
}

/*! \relates QByteArray

    A safe \c strncpy() function.

    Copies at most \a len bytes from \a src (stopping at \a len or the
    terminating '\\0' whichever comes first) into \a dst and returns a
    pointer to \a dst. Guarantees that \a dst is '\\0'-terminated. If
    \a src or \a dst is 0, returns 0 immediately.

    This function assumes that \a dst is at least \a len characters
    long.

    \note When compiling with Visual C++ compiler version 14.00
    (Visual C++ 2005) or later, internally the function strncpy_s
    will be used.

    \sa qstrcpy()
*/

char* qstrncpy(char *dst, const char *src, uint len)
{
    if (!src || !dst)
        return nullptr;
    strncpy(dst, src, len);
    if (len > 0)
        dst[len-1] = '\0';
    return dst;
}

/*! \fn uint qstrlen(const char *str)
    \relates QByteArray

    A safe \c strlen() function.

    Returns the number of characters that precede the terminating '\\0',
    or 0 if \a str is 0.

    \sa qstrnlen()
*/

/*! \fn uint qstrnlen(const char *str, uint maxlen)
    \relates QByteArray
    \since 4.2

    A safe \c strnlen() function.

    Returns the number of characters that precede the terminating '\\0', but
    at most \a maxlen. If \a str is 0, returns 0.

    \sa qstrlen()
*/

/*! \fn int qstrcmp(const char *str1, const char *str2);

    \relates QByteArray

    A safe \c strcmp() function.

    Compares \a str1 and \a str2. Returns a negative value if \a str1
    is less than \a str2, 0 if \a str1 is equal to \a str2 or a
    positive value if \a str1 is greater than \a str2.

    Special case 1: Returns 0 if \a str1 and \a str2 are both 0.

    Special case 2: Returns an arbitrary non-zero value if \a str1 is 0
    or \a str2 is 0 (but not both).

    \sa qstrncmp(), qstricmp(), qstrnicmp(), {8-bit Character Comparisons}
*/

/*! \fn int qstrncmp(const char *str1, const char *str2, uint len);

    \relates QByteArray

    A safe \c strncmp() function.

    Compares at most \a len bytes of \a str1 and \a str2.

    Returns a negative value if \a str1 is less than \a str2, 0 if \a
    str1 is equal to \a str2 or a positive value if \a str1 is greater
    than \a str2.

    Special case 1: Returns 0 if \a str1 and \a str2 are both 0.

    Special case 2: Returns a random non-zero value if \a str1 is 0
    or \a str2 is 0 (but not both).

    \sa qstrcmp(), qstricmp(), qstrnicmp(), {8-bit Character Comparisons}
*/

/*! \fn int qstricmp(const char *str1, const char *str2)

   \relates QByteArray

    A safe \c stricmp() function.

    Compares \a str1 and \a str2 ignoring the case of the
    characters. The encoding of the strings is assumed to be Latin-1.

    Returns a negative value if \a str1 is less than \a str2, 0 if \a
    str1 is equal to \a str2 or a positive value if \a str1 is greater
    than \a str2.

    Special case 1: Returns 0 if \a str1 and \a str2 are both 0.

    Special case 2: Returns a random non-zero value if \a str1 is 0
    or \a str2 is 0 (but not both).

    \sa qstrcmp(), qstrncmp(), qstrnicmp(), {8-bit Character Comparisons}
*/

/*! \fn int qstrnicmp(const char *str1, const char *str2, uint len)

   \relates QByteArray

    A safe \c strnicmp() function.

    Compares at most \a len bytes of \a str1 and \a str2 ignoring the
    case of the characters. The encoding of the strings is assumed to
    be Latin-1.

    Returns a negative value if \a str1 is less than \a str2, 0 if \a str1
    is equal to \a str2 or a positive value if \a str1 is greater than \a
    str2.

    Special case 1: Returns 0 if \a str1 and \a str2 are both 0.

    Special case 2: Returns a random non-zero value if \a str1 is 0
    or \a str2 is 0 (but not both).

    \sa qstrcmp(), qstrncmp(), qstricmp(), {8-bit Character Comparisons}
*/

/*!
    \relates QByteArray

    Returns pseudo-randomly generated UUID.
*/
QByteArray qRandomUuid()
{
    static const ushort randiterratio = (sizeof(int) / sizeof(char));
    Q_ASSERT(randiterratio > 1);
    QSTACKARRAY(char, randombuf, 16);
    int *randombufiter = reinterpret_cast<int*>(randombuf);
    for (ushort i = 0; i < (16 / randiterratio); i++) {
        *randombufiter = qrand();
        randombufiter++;
    }

#define UUID_TOHEX(bufi, randi) \
    uuidbuf[bufi] = tohex[(randombuf[randi] >> 4) & 0xf]; \
    uuidbuf[bufi + 1] = tohex[randombuf[randi] & 0xf];

    static const char tohex[] = "0123456789abcdef";
    QSTACKARRAY(char, uuidbuf, 36);
    UUID_TOHEX(0, 0);
    UUID_TOHEX(2, 1);
    UUID_TOHEX(4, 2);
    UUID_TOHEX(6, 3);
    uuidbuf[8] = '-';
    UUID_TOHEX(9, 4);
    UUID_TOHEX(11, 5);
    uuidbuf[13] = '-';
    UUID_TOHEX(14, 6);
    UUID_TOHEX(16, 7);
    uuidbuf[18] = '-';
    UUID_TOHEX(19, 8);
    UUID_TOHEX(21, 9);
    uuidbuf[23] = '-';
    UUID_TOHEX(24, 10);
    UUID_TOHEX(26, 11);
    UUID_TOHEX(28, 12);
    UUID_TOHEX(30, 13);
    UUID_TOHEX(32, 14);
    UUID_TOHEX(34, 15);

#undef UUID_TOHEX

    return QByteArray(uuidbuf, sizeof(uuidbuf));
}

QByteArray::Data QByteArray::shared_null = {
    QAtomicInt(1), 0, 0, shared_null.array, {0}
};
QByteArray::Data QByteArray::shared_empty = {
    QAtomicInt(1), 0, 0, shared_empty.array, {0}
};

/*!
    \class QByteArray
    \brief The QByteArray class provides an array of bytes.

    \ingroup tools
    \ingroup shared
    \ingroup string-processing

    \reentrant

    QByteArray can be used to store both raw bytes (including '\\0's)
    and traditional 8-bit '\\0'-terminated strings. Using QByteArray
    is much more convenient than using \c{const char *}. Behind the
    scenes, it always ensures that the data is followed by a '\\0'
    terminator, and uses \l{implicit sharing} (copy-on-write) to
    reduce memory usage and avoid needless copying of data.

    In addition to QByteArray, Qt also provides the QString class to
    store string data. For most purposes, QString is the class you
    want to use. It stores 16-bit Unicode characters, making it easy
    to store non-ASCII/non-Latin-1 characters in your application.
    Furthermore, QString is used throughout in the Qt API. The two
    main cases where QByteArray is appropriate are when you need to
    store raw binary data, and when memory conservation is critical
    (e.g., with Qt for Embedded Linux).

    One way to initialize a QByteArray is simply to pass a \c{const
    char *} to its constructor. For example, the following code
    creates a byte array of size 5 containing the data "Hello":

    \snippet doc/src/snippets/code/src_corelib_tools_qbytearray.cpp 0

    Although the size() is 5, the byte array also maintains an extra
    '\\0' character at the end so that if a function is used that
    asks for a pointer to the underlying data (e.g. a call to
    data()), the data pointed to is guaranteed to be
    '\\0'-terminated.

    QByteArray makes a deep copy of the \c{const char *} data, so you
    can modify it later without experiencing side effects. (If for
    performance reasons you don't want to take a deep copy of the
    character data, use QByteArray::fromRawData() instead.)

    Another approach is to set the size of the array using resize()
    and to initialize the data byte per byte. QByteArray uses 0-based
    indexes, just like C++ arrays. To access the byte at a particular
    index position, you can use operator[](). On non-const byte
    arrays, operator[]() returns a reference to a byte that can be
    used on the left side of an assignment. For example:

    \snippet doc/src/snippets/code/src_corelib_tools_qbytearray.cpp 1

    For read-only access, an alternative syntax is to use at():

    \snippet doc/src/snippets/code/src_corelib_tools_qbytearray.cpp 2

    at() can be faster than operator[](), because it never causes a
    \l{deep copy} to occur.

    To extract many bytes at a time, use left(), right(), or mid().

    A QByteArray can embed '\\0' bytes. The size() function always
    returns the size of the whole array, including embedded '\\0'
    bytes. If you want to obtain the length of the data up to and
    excluding the first '\\0' character, call qstrlen() on the byte
    array.

    After a call to resize(), newly allocated bytes have undefined
    values. To set all the bytes to a particular value, call fill().

    To obtain a pointer to the actual character data, call data() or
    constData(). These functions return a pointer to the beginning of the data.
    The pointer is guaranteed to remain valid until a non-const function is
    called on the QByteArray. It is also guaranteed that the data ends with a
    '\\0' byte unless the QByteArray was created from a \l{fromRawData()}{raw
    data}. This '\\0' byte is automatically provided by QByteArray and is not
    counted in size().

    QByteArray provides the following basic functions for modifying
    the byte data: append(), prepend(), insert(), replace(), and
    remove(). For example:

    \snippet doc/src/snippets/code/src_corelib_tools_qbytearray.cpp 3

    The replace() and remove() functions' first two arguments are the
    position from which to start erasing and the number of bytes that
    should be erased.

    When you append() data to a non-empty array, the array will be
    reallocated and the new data copied to it. You can avoid this
    behavior by calling reserve(), which preallocates a certain amount
    of memory. You can also call capacity() to find out how much
    memory QByteArray actually allocated. Data appended to an empty
    array is not copied.

    A frequent requirement is to remove whitespace characters from a
    byte array ('\\n', '\\t', ' ', etc.). If you want to remove
    whitespace from both ends of a QByteArray, use trimmed(). If you
    want to remove whitespace from both ends and replace multiple
    consecutive whitespaces with a single space character within the
    byte array, use simplified().

    If you want to find all occurrences of a particular character or
    substring in a QByteArray, use indexOf() or lastIndexOf(). The
    former searches forward starting from a given index position, the
    latter searches backward. Both return the index position of the
    character or substring if they find it; otherwise, they return -1.
    For example, here's a typical loop that finds all occurrences of a
    particular substring:

    \snippet doc/src/snippets/code/src_corelib_tools_qbytearray.cpp 4

    If you simply want to check whether a QByteArray contains a
    particular character or substring, use contains(). If you want to
    find out how many times a particular character or substring
    occurs in the byte array, use count(). If you want to replace all
    occurrences of a particular value with another, use one of the
    two-parameter replace() overloads.

    QByteArrays can be compared using overloaded operators such as
    operator<(), operator<=(), operator==(), operator>=(), and so on.
    The comparison is based exclusively on the numeric values
    of the characters and is very fast, but is not what a human would
    expect. QString::localeAwareCompare() is a better choice for
    sorting user-interface strings.

    For historical reasons, QByteArray distinguishes between a null
    byte array and an empty byte array. A \e null byte array is a
    byte array that is initialized using QByteArray's default
    constructor or by passing (const char *)0 to the constructor. An
    \e empty byte array is any byte array with size 0. A null byte
    array is always empty, but an empty byte array isn't necessarily
    null:

    \snippet doc/src/snippets/code/src_corelib_tools_qbytearray.cpp 5

    All functions except isNull() treat null byte arrays the same as
    empty byte arrays. For example, data() returns a pointer to a
    '\\0' character for a null byte array (\e not a null pointer),
    and QByteArray() compares equal to QByteArray(""). We recommend
    that you always use isEmpty() and avoid isNull().

    \section1 Notes on Locale

    \section2 Number-String Conversions

    Functions that perform conversions between numeric data types and
    strings are performed in the C locale, irrespective of the user's
    locale settings. Use QString to perform locale-aware conversions
    between numbers and strings.

    \section2 8-bit Character Comparisons

    In QByteArray, the notion of uppercase and lowercase and of which
    character is greater than or less than another character is
    locale dependent. This affects functions that support a case
    insensitive option or that compare or lowercase or uppercase
    their arguments. Case insensitive operations and comparisons will
    be accurate if both strings contain only ASCII characters. (If \c
    $LC_CTYPE is set, most Unix systems do "the right thing".)
    Functions that this affects include contains(), indexOf(),
    lastIndexOf(), operator<(), operator<=(), operator>(),
    operator>=(), toLower() and toUpper().

    This issue does not apply to QStrings since they represent
    characters using Unicode.

    \sa QString, QBitArray
*/

/*! \fn QByteArray::iterator QByteArray::begin()

    \internal
*/

/*! \fn QByteArray::const_iterator QByteArray::begin() const

    \internal
*/

/*! \fn QByteArray::const_iterator QByteArray::constBegin() const

    \internal
*/

/*! \fn QByteArray::iterator QByteArray::end()

    \internal
*/

/*! \fn QByteArray::const_iterator QByteArray::end() const

    \internal
*/

/*! \fn QByteArray::const_iterator QByteArray::constEnd() const

    \internal
*/

/*! \fn void QByteArray::push_back(const QByteArray &other)

    This function is provided for STL compatibility. It is equivalent
    to append(\a other).
*/

/*! \fn void QByteArray::push_back(const char *str)

    \overload

    Same as append(\a str).
*/

/*! \fn void QByteArray::push_back(char ch)

    \overload

    Same as append(\a ch).
*/

/*! \fn void QByteArray::push_front(const QByteArray &other)

    This function is provided for STL compatibility. It is equivalent
    to prepend(\a other).
*/

/*! \fn void QByteArray::push_front(const char *str)

    \overload

    Same as prepend(\a str).
*/

/*! \fn void QByteArray::push_front(char ch)

    \overload

    Same as prepend(\a ch).
*/

/*! \fn QByteArray QByteArray::fromStdString(const std::string &str)
 \since 5.4

 Returns a copy of the \a str string as a QByteArray.

 \sa toStdString(), QString::fromStdString()
*/

/*!
 \fn std::string QByteArray::toStdString() const
 \since 5.4

 Returns a std::string object with the data contained in this
 QByteArray.

 This operator is mostly useful to pass a QByteArray to a function
 that accepts a std::string object.

 \sa fromStdString(), QString::toStdString()
*/

/*! \fn QByteArray::QByteArray(const QByteArray &other)

    Constructs a copy of \a other.

    This operation takes \l{constant time}, because QByteArray is
    \l{implicitly shared}. This makes returning a QByteArray from a
    function very fast. If a shared instance is modified, it will be
    copied (copy-on-write), taking \l{linear time}.

    \sa operator=()
*/

/*! \fn QByteArray::~QByteArray()
    Destroys the byte array.
*/

/*!
    Assigns \a other to this byte array and returns a reference to
    this byte array.
*/
QByteArray &QByteArray::operator=(const QByteArray &other)
{
    Data *x;
    if (other.isNull()) {
        x = &shared_null;
    } else if (other.isEmpty()) {
        x = &shared_empty;
    } else {
        int len = other.d->size;
        if (d->ref != 1 || len > d->alloc || (len < d->size && len < d->alloc >> 1))
            reallocData(len);
        x = d;
        memcpy(x->data, other.d->data, len );
        x->size = len;
        x->data[len] = '\0';
    }
    x->ref.ref();
    if (!d->ref.deref())
         freeData(d);
    d = x;
    return *this;
}


/*!
    \overload

    Assigns \a str to this byte array.
*/

QByteArray &QByteArray::operator=(const char *str)
{
    Data *x;
    if (!str) {
        x = &shared_null;
    } else if (!*str) {
        x = &shared_empty;
    } else {
        int len = qstrlen(str);
        if (d->ref != 1 || len > d->alloc || (len < d->size && len < d->alloc >> 1))
            reallocData(len);
        x = d;
        memcpy(x->data, str, len);
        x->size = len;
        x->data[len] = '\0';
    }
    x->ref.ref();
    if (!d->ref.deref())
         freeData(d);
    d = x;
    return *this;
}

/*! \fn void QByteArray::swap(QByteArray &other)
    \since 4.8

    Swaps byte array \a other with this byte array. This operation is very
    fast and never fails.
*/

/*! \fn int QByteArray::size() const

    Returns the number of bytes in this byte array.

    The last byte in the byte array is at position size() - 1. In addition,
    QByteArray ensures that the byte at position size() is always '\\0', so
    that you can use the return value of data() and constData() as arguments to
    functions that expect '\\0'-terminated strings. If the QByteArray object
    was created from a \l{fromRawData()}{raw data} that didn't include the
    trailing null-termination character then QByteArray doesn't add it
    automaticall unless the \l{deep copy} is created.

    Example:
    \snippet doc/src/snippets/code/src_corelib_tools_qbytearray.cpp 6

    \sa isEmpty(), resize()
*/

/*! \fn bool QByteArray::isEmpty() const

    Returns true if the byte array has size 0; otherwise returns false.

    Example:
    \snippet doc/src/snippets/code/src_corelib_tools_qbytearray.cpp 7

    \sa size()
*/

/*! \fn int QByteArray::capacity() const

    Returns the maximum number of bytes that can be stored in the
    byte array without forcing a reallocation.

    The sole purpose of this function is to provide a means of fine
    tuning QByteArray's memory usage. In general, you will rarely
    ever need to call this function. If you want to know how many
    bytes are in the byte array, call size().

    \sa reserve(), squeeze()
*/

/*! \fn void QByteArray::reserve(int size)

    Attempts to allocate memory for at least \a size bytes. If you
    know in advance how large the byte array will be, you can call
    this function, and if you call resize() often you are likely to
    get better performance. If \a size is an underestimate, the worst
    that will happen is that the QByteArray will be a bit slower.

    The sole purpose of this function is to provide a means of fine
    tuning QByteArray's memory usage. In general, you will rarely
    ever need to call this function. If you want to change the size
    of the byte array, call resize().

    \sa squeeze(), capacity()
*/

/*! \fn void QByteArray::squeeze()

    Releases any memory not required to store the array's data.

    The sole purpose of this function is to provide a means of fine
    tuning QByteArray's memory usage. In general, you will rarely
    ever need to call this function.

    \sa reserve(), capacity()
*/

/*! \fn QByteArray::operator const char *() const
    \fn QByteArray::operator const void *() const

    Returns a pointer to the data stored in the byte array. The
    pointer can be used to access the bytes that compose the array.
    The data is '\\0'-terminated. The pointer remains valid as long
    as the array isn't reallocated or destroyed.

    This operator is mostly useful to pass a byte array to a function
    that accepts a \c{const char *}.

    You can disable this operator by defining \c
    QT_NO_CAST_FROM_BYTEARRAY when you compile your applications.

    Note: A QByteArray can store any byte values including '\\0's,
    but most functions that take \c{char *} arguments assume that the
    data ends at the first '\\0' they encounter.

    \sa constData()
*/

/*!
  \macro QT_NO_CAST_FROM_BYTEARRAY
  \relates QByteArray

  Disables automatic conversions from QByteArray to
  const char * or const void *.

  \sa QT_NO_CAST_TO_ASCII, QT_NO_CAST_FROM_ASCII
*/

/*! \fn char *QByteArray::data()

    Returns a pointer to the data stored in the byte array. The
    pointer can be used to access and modify the bytes that compose
    the array. The data is '\\0'-terminated, i.e. the number of
    bytes in the returned character string is size() + 1 for the
    '\\0' terminator.

    Example:
    \snippet doc/src/snippets/code/src_corelib_tools_qbytearray.cpp 8

    The pointer remains valid as long as the byte array isn't
    reallocated or destroyed. For read-only access, constData() is
    faster because it never causes a \l{deep copy} to occur.

    This function is mostly useful to pass a byte array to a function
    that accepts a \c{const char *}.

    The following example makes a copy of the char* returned by
    data(), but it will corrupt the heap and cause a crash because it
    does not allocate a byte for the '\\0' at the end:

    \snippet doc/src/snippets/code/src_corelib_tools_qbytearray.cpp 46

    This one allocates the correct amount of space:

    \snippet doc/src/snippets/code/src_corelib_tools_qbytearray.cpp 47

    Note: A QByteArray can store any byte values including '\\0's,
    but most functions that take \c{char *} arguments assume that the
    data ends at the first '\\0' they encounter.

    \sa constData(), operator[]()
*/

/*! \fn const char *QByteArray::data() const

    \overload
*/

/*! \fn const char *QByteArray::constData() const

    Returns a pointer to the data stored in the byte array. The pointer can be
    used to access the bytes that compose the array. The data is
    '\\0'-terminated unless the QByteArray object was created from raw data.
    The pointer remains valid as long as the byte array isn't reallocated or
    destroyed.

    This function is mostly useful to pass a byte array to a function
    that accepts a \c{const char *}.

    Note: A QByteArray can store any byte values including '\\0's,
    but most functions that take \c{char *} arguments assume that the
    data ends at the first '\\0' they encounter.

    \sa data(), operator[](), fromRawData()
*/

/*! \fn void QByteArray::detach()

    \internal
*/

/*! \fn char QByteArray::at(int i) const

    Returns the character at index position \a i in the byte array.

    \a i must be a valid index position in the byte array (i.e., 0 <=
    \a i < size()).

    \sa operator[]()
*/

/*! \fn QByteRef QByteArray::operator[](int i)

    Returns the byte at index position \a i as a modifiable reference.

    If an assignment is made beyond the end of the byte array, the
    array is extended with resize() before the assignment takes
    place.

    Example:
    \snippet doc/src/snippets/code/src_corelib_tools_qbytearray.cpp 9

    The return value is of type QByteRef, a helper class for
    QByteArray. When you get an object of type QByteRef, you can use
    it as if it were a char &. If you assign to it, the assignment
    will apply to the character in the QByteArray from which you got
    the reference.

    \sa at()
*/

/*! \fn char QByteArray::operator[](int i) const

    \overload

    Same as at(\a i).
*/

/*! \fn QByteRef QByteArray::operator[](uint i)

    \overload
*/

/*! \fn char QByteArray::operator[](uint i) const

    \overload
*/

/*! \fn bool QByteArray::contains(const QByteArray &ba) const

    Returns true if the byte array contains an occurrence of the byte
    array \a ba; otherwise returns false.

    \sa indexOf(), count()
*/

/*! \fn bool QByteArray::contains(const char *str) const

    \overload

    Returns true if the byte array contains the string \a str;
    otherwise returns false.
*/

/*! \fn bool QByteArray::contains(char ch) const

    \overload

    Returns true if the byte array contains the character \a ch;
    otherwise returns false.
*/

/*!

    Truncates the byte array at index position \a pos.

    If \a pos is beyond the end of the array, nothing happens.

    Example:
    \snippet doc/src/snippets/code/src_corelib_tools_qbytearray.cpp 10

    \sa chop(), resize(), left()
*/
void QByteArray::truncate(int pos)
{
    if (pos < d->size)
        resize(pos);
}

/*!

    Removes \a n bytes from the end of the byte array.

    If \a n is greater than size(), the result is an empty byte
    array.

    Example:
    \snippet doc/src/snippets/code/src_corelib_tools_qbytearray.cpp 11

    \sa truncate(), resize(), left()
*/

void QByteArray::chop(int n)
{
    if (n > 0)
        resize(d->size - n);
}


/*! \fn QByteArray &QByteArray::operator+=(const QByteArray &ba)

    Appends the byte array \a ba onto the end of this byte array and
    returns a reference to this byte array.

    Example:
    \snippet doc/src/snippets/code/src_corelib_tools_qbytearray.cpp 12

    Note: QByteArray is an \l{implicitly shared} class. Consequently,
    if \e this is an empty QByteArray, then \e this will just share
    the data held in \a ba. In this case, no copying of data is done,
    taking \l{constant time}. If a shared instance is modified, it will
    be copied (copy-on-write), taking \l{linear time}.

    If \e this is not an empty QByteArray, a deep copy of the data is
    performed, taking \l{linear time}.

    This operation typically does not suffer from allocation overhead,
    because QByteArray preallocates extra space at the end of the data
    so that it may grow without reallocating for each append operation.

    \sa append(), prepend()
*/

/*! \fn QByteArray &QByteArray::operator+=(const QString &str)

    \overload

    Appends the string \a str onto the end of this byte array and
    returns a reference to this byte array. The Unicode data is
    converted into 8-bit characters using QString::toAscii().

    If the QString contains non-ASCII Unicode characters, using this
    operator can lead to loss of information. You can disable this
    operator by defining \c QT_NO_CAST_TO_ASCII when you compile your
    applications. You then need to call QString::toAscii() (or
    QString::toLatin1() or QString::toUtf8() or QString::toLocal8Bit())
    explicitly if you want to convert the data to \c{const char *}.
*/

/*! \fn QByteArray &QByteArray::operator+=(const char *str)

    \overload

    Appends the string \a str onto the end of this byte array and
    returns a reference to this byte array.
*/

/*! \fn QByteArray &QByteArray::operator+=(char ch)

    \overload

    Appends the character \a ch onto the end of this byte array and
    returns a reference to this byte array.
*/

/*! \fn int QByteArray::length() const

    Same as size().
*/

/*! \fn bool QByteArray::isNull() const

    Returns true if this byte array is null; otherwise returns false.

    Example:
    \snippet doc/src/snippets/code/src_corelib_tools_qbytearray.cpp 13

    Qt makes a distinction between null byte arrays and empty byte
    arrays for historical reasons. For most applications, what
    matters is whether or not a byte array contains any data,
    and this can be determined using isEmpty().

    \sa isEmpty()
*/

/*! \fn QByteArray::QByteArray()

    Constructs an empty byte array.

    \sa isEmpty()
*/

/*! \fn QByteArray::QByteArray(const char *str)

    Constructs a byte array initialized with the string \a str.

    QByteArray makes a deep copy of the string data.
*/

QByteArray::QByteArray(const char *data)
{
    int size = qstrlen(data);
    if (!data) {
        d = &shared_null;
    } else if (size <= 0) {
        d = &shared_empty;
    } else {
        d = static_cast<Data *>(::malloc(sizeof(Data) + size));
        Q_CHECK_PTR(d);
        d->ref = 0;
        d->alloc = d->size = size;
        d->data = d->array;
        memcpy(d->array, data, size);
        d->array[size] = '\0';
    }
    d->ref.ref();
}

/*!
    Constructs a byte array containing the first \a size bytes of
    array \a data.

    If \a data is 0, a null byte array is constructed.

    QByteArray makes a deep copy of the string data.

    \sa fromRawData()
*/

QByteArray::QByteArray(const char *data, int size)
{
    if (!data) {
        d = &shared_null;
    } else if (size <= 0) {
        d = &shared_empty;
    } else {
        d = static_cast<Data *>(::malloc(sizeof(Data) + size));
        Q_CHECK_PTR(d);
        d->ref = 0;
        d->alloc = d->size = size;
        d->data = d->array;
        memcpy(d->array, data, size);
        d->array[size] = '\0';
    }
    d->ref.ref();
}

/*!
    Constructs a byte array of size \a size with every byte set to
    character \a ch.

    \sa fill()
*/

QByteArray::QByteArray(int size, char ch)
{
    if (size <= 0) {
        d = &shared_null;
    } else {
        d = static_cast<Data *>(::malloc(sizeof(Data) + size));
        Q_CHECK_PTR(d);
        d->ref = 0;
        d->alloc = d->size = size;
        d->data = d->array;
        d->array[size] = '\0';
        memset(d->array, ch, size);
    }
    d->ref.ref();
}

/*!
    \internal 

    Constructs a byte array of size \a size with uninitialized contents.
*/

QByteArray::QByteArray(int size, Qt::Initialization)
{
    if (size <= 0) {
        d = &shared_empty;
    } else {
        d = static_cast<Data *>(::malloc(sizeof(Data)+size));
        Q_CHECK_PTR(d);
        d->ref = 0;
        d->alloc = d->size = size;
        d->data = d->array;
        d->array[size] = '\0';
    }
    d->ref.ref();
}

/*!
    Sets the size of the byte array to \a size bytes.

    If \a size is greater than the current size, the byte array is
    extended to make it \a size bytes with the extra bytes added to
    the end. The new bytes are uninitialized.

    If \a size is less than the current size, bytes are removed from
    the end.

    \sa size(), truncate()
*/

void QByteArray::resize(int size)
{
    if (size <= 0) {
        Data *x = &shared_empty;
        x->ref.ref();
        if (!d->ref.deref())
            freeData(d);
        d = x;
    } else {
        if (d->ref != 1 || size > d->alloc || (size < d->size && size < d->alloc >> 1))
            reallocData(size);
        if (d->alloc >= size) {
            d->size = size;
            if (!IS_RAW_DATA(d)) {
                d->array[size] = '\0';
            }
         }
    }
}

/*!
    Sets every byte in the byte array to character \a ch. If \a size
    is different from -1 (the default), the byte array is resized to
    size \a size beforehand.

    Example:
    \snippet doc/src/snippets/code/src_corelib_tools_qbytearray.cpp 14

    \sa resize()
*/

QByteArray &QByteArray::fill(char ch, int size)
{
    resize(size < 0 ? d->size : size);
    if (d->size)
        memset(d->data, ch, d->size);
    return *this;
}

void QByteArray::freeData(Data *d)
{
    if (d != &shared_empty && d != &shared_null)
        ::free(d);
}

void QByteArray::reallocData(int alloc)
{
    if (d->ref != 1 || d->data != d->array || d->size <= 0) {
        Data *x = static_cast<Data *>(::malloc(sizeof(Data) + alloc));
        Q_CHECK_PTR(x);
        x->size = qMin(alloc, d->size);
        ::memcpy(x->array, d->data, x->size);
        x->array[x->size] = '\0';
        x->ref = 1;
        x->alloc = alloc;
        x->data = x->array;
        if (!d->ref.deref())
            freeData(d);
        d = x;
    } else {
        Data *x = static_cast<Data *>(::realloc(d, sizeof(Data) + alloc));
        Q_CHECK_PTR(x);
        x->alloc = alloc;
        x->data = x->array;
        d = x;
    }
}

/*!
   \internal
   Return a QByteArray that is sure to be NUL-terminated.

   By default, all QByteArray have an extra NUL at the end,
   guaranteeing that assumption. However, if QByteArray::fromRawData
   is used, then the NUL is there only if the user put it there. We
   can't be sure.
*/
QByteArray QByteArray::nulTerminated() const
{
    if (!IS_RAW_DATA(d))
        return *this; // it is zero terminated

    QByteArray copy(*this);
    copy.detach();
    return copy;
}

/*!
    Prepends the byte array \a ba to this byte array and returns a
    reference to this byte array.

    Example:
    \snippet doc/src/snippets/code/src_corelib_tools_qbytearray.cpp 15

    This is the same as insert(0, \a ba).

    Note: QByteArray is an \l{implicitly shared} class. Consequently,
    if \e this is an empty QByteArray, then \e this will just share
    the data held in \a ba. In this case, no copying of data is done,
    taking \l{constant time}. If a shared instance is modified, it will
    be copied (copy-on-write), taking \l{linear time}.

    If \e this is not an empty QByteArray, a deep copy of the data is
    performed, taking \l{linear time}.

    \sa append(), insert()
*/

QByteArray &QByteArray::prepend(const QByteArray &ba)
{
    if (isEmpty()) {
        *this = ba;
    } else if (!ba.isEmpty()) {
        prepend(ba.constData(), ba.size());
    }
    return *this;
}

/*!
    \overload

    Prepends the string \a str to this byte array.
*/

QByteArray &QByteArray::prepend(const char *str)
{
    return prepend(str, qstrlen(str));
}

/*!
    \overload
    \since 4.6

    Prepends \a len bytes of the string \a str to this byte array.
*/

QByteArray &QByteArray::prepend(const char *str, int len)
{
    if (str && len > 0) {
        if (d->ref != 1 || d->size + len > d->alloc)
            reallocData(d->size + len);
        memmove(d->data+len, d->data, d->size);
        memcpy(d->data, str, len);
        d->size += len;
        d->data[d->size] = '\0';
    }
    return *this;
}

/*!
    \overload

    Prepends the character \a ch to this byte array.
*/

QByteArray &QByteArray::prepend(char ch)
{
    if (d->ref != 1 || d->size + 1 > d->alloc)
        reallocData(d->size + 1);
    memmove(d->data+1, d->data, d->size);
    d->data[0] = ch;
    ++d->size;
    d->data[d->size] = '\0';
    return *this;
}

/*!
    Appends the byte array \a ba onto the end of this byte array.

    Example:
    \snippet doc/src/snippets/code/src_corelib_tools_qbytearray.cpp 16

    This is the same as insert(size(), \a ba).

    Note: QByteArray is an \l{implicitly shared} class. Consequently,
    if \e this is an empty QByteArray, then \e this will just share
    the data held in \a ba. In this case, no copying of data is done,
    taking \l{constant time}. If a shared instance is modified, it will
    be copied (copy-on-write), taking \l{linear time}.

    If \e this is not an empty QByteArray, a deep copy of the data is
    performed, taking \l{linear time}.

    This operation typically does not suffer from allocation overhead,
    because QByteArray preallocates extra space at the end of the data
    so that it may grow without reallocating for each append operation.

    \sa operator+=(), prepend(), insert()
*/

QByteArray &QByteArray::append(const QByteArray &ba)
{
    if (isEmpty()) {
        *this = ba;
    } else if (!ba.isEmpty()) {
        append(ba.constData(), ba.size());
    }
    return *this;
}

/*! \fn QByteArray &QByteArray::append(const QString &str)

    \overload

    Appends the string \a str to this byte array. The Unicode data is
    converted into 8-bit characters using QString::toAscii().

    If the QString contains non-ASCII Unicode characters, using this
    function can lead to loss of information. You can disable this
    function by defining \c QT_NO_CAST_TO_ASCII when you compile your
    applications. You then need to call QString::toAscii() (or
    QString::toLatin1() or QString::toUtf8() or QString::toLocal8Bit())
    explicitly if you want to convert the data to \c{const char *}.
*/

/*!
    \overload

    Appends the string \a str to this byte array.
*/

QByteArray& QByteArray::append(const char *str)
{
    return append(str, qstrlen(str));
}

/*!
    \overload append()

    Appends the first \a len characters of the string \a str to this byte
    array and returns a reference to this byte array.

    If \a len is negative, the length of the string will be determined
    automatically using qstrlen(). If \a len is zero or \a str is
    null, nothing is appended to the byte array. Ensure that \a len is
    \e not longer than \a str.
*/

QByteArray &QByteArray::append(const char *str, int len)
{
    if (len < 0)
        len = qstrlen(str);
    if (str && len) {
        if (d->ref != 1 || d->size + len > d->alloc)
            reallocData(d->size + len);
        memcpy(d->data + d->size, str, len);
        d->size += len;
        d->data[d->size] = '\0';
    }
    return *this;
}

/*!
    \overload

    Appends the character \a ch to this byte array.
*/

QByteArray& QByteArray::append(char ch)
{
    if (d->ref != 1 || d->size + 1 > d->alloc)
        reallocData(d->size + 1);
    d->data[d->size++] = ch;
    d->data[d->size] = '\0';
    return *this;
}

/*!
    Inserts the byte array \a ba at index position \a i and returns a
    reference to this byte array.

    Example:
    \snippet doc/src/snippets/code/src_corelib_tools_qbytearray.cpp 17

    \sa append(), prepend(), replace(), remove()
*/

QByteArray &QByteArray::insert(int i, const QByteArray &ba)
{
    return insert(i, ba.d->data, ba.d->size);
}

/*!
    \fn QByteArray &QByteArray::insert(int i, const QString &str)

    \overload

    Inserts the string \a str at index position \a i in the byte
    array. The Unicode data is converted into 8-bit characters using
    QString::toAscii().

    If \a i is greater than size(), the array is first extended using
    resize().

    If the QString contains non-ASCII Unicode characters, using this
    function can lead to loss of information. You can disable this
    function by defining \c QT_NO_CAST_TO_ASCII when you compile your
    applications. You then need to call QString::toAscii() (or
    QString::toLatin1() or QString::toUtf8() or QString::toLocal8Bit())
    explicitly if you want to convert the data to \c{const char *}.
*/

/*!
    \overload

    Inserts the string \a str at position \a i in the byte array.

    If \a i is greater than size(), the array is first extended using
    resize().
*/

QByteArray &QByteArray::insert(int i, const char *str)
{
    return insert(i, str, qstrlen(str));
}

/*!
    \overload
    \since 4.6

    Inserts \a len bytes of the string \a str at position
    \a i in the byte array.

    If \a i is greater than size(), the array is first extended using
    resize().
*/

QByteArray &QByteArray::insert(int i, const char *str, int len)
{
    Q_ASSERT(i >= 0);
    if (i < 0 || len <= 0 || !str) {
        return *this;
    }
    int oldsize = d->size;
    resize(qMax(i, oldsize) + len);
    char *dst = this->data();
    if (i > oldsize) {
        ::memset(dst + oldsize, 0x20, i - oldsize);
    } else {
        ::memmove(dst + i + len, dst + i, oldsize - i);
    }
    ::memcpy(dst + i, str, len);
    return *this;
}

/*!
    \overload

    Inserts character \a ch at index position \a i in the byte array.
    If \a i is greater than size(), the array is first extended using
    resize().
*/

QByteArray &QByteArray::insert(int i, char ch)
{
    return insert(i, &ch, 1);
}

/*!
    Removes \a len bytes from the array, starting at index position \a
    pos, and returns a reference to the array.

    If \a pos is out of range, nothing happens. If \a pos is valid,
    but \a pos + \a len is larger than the size of the array, the
    array is truncated at position \a pos.

    Example:
    \snippet doc/src/snippets/code/src_corelib_tools_qbytearray.cpp 18

    \sa insert(), replace()
*/

QByteArray &QByteArray::remove(int pos, int len)
{
    if (len <= 0  || pos >= d->size || pos < 0)
        return *this;
    detach();
    if (pos + len >= d->size) {
        resize(pos);
    } else {
        memmove(d->data + pos, d->data + pos + len, d->size - pos - len);
        resize(d->size - len);
    }
    return *this;
}

/*!
    Replaces \a len bytes from index position \a pos with the byte
    array \a after, and returns a reference to this byte array.

    Example:
    \snippet doc/src/snippets/code/src_corelib_tools_qbytearray.cpp 19

    \sa insert(), remove()
*/

QByteArray &QByteArray::replace(int pos, int len, const QByteArray &after)
{
    return replace(pos, len, after.constData(), after.size());
}

/*! \fn QByteArray &QByteArray::replace(int pos, int len, const char *after)

    \overload

    Replaces \a len bytes from index position \a pos with the zero terminated
    string \a after.

    Notice: this can change the length of the byte array.
*/
QByteArray &QByteArray::replace(int pos, int len, const char *after)
{
    return replace(pos,len, after, qstrlen(after));
}

/*! \fn QByteArray &QByteArray::replace(int pos, int len, const char *after, int alen)

    \overload

    Replaces \a len bytes from index position \a pos with \a alen bytes
    from the string \a after. \a after is allowed to have '\0' characters.

    \since 4.7
*/
QByteArray &QByteArray::replace(int pos, int len, const char *after, int alen)
{
    if (len == alen && (pos + len <= d->size)) {
        detach();
        memcpy(d->data + pos, after, len*sizeof(char));
        return *this;
    }
    remove(pos, len);
    return insert(pos, after, alen);
}

/*!
    \overload

    Replaces every occurrence of the byte array \a before with the
    byte array \a after.

    Example:
    \snippet doc/src/snippets/code/src_corelib_tools_qbytearray.cpp 20
*/

QByteArray &QByteArray::replace(const QByteArray &before, const QByteArray &after)
{
    if (isNull() || before.d == after.d) {
        return *this;
    }
    return replace(before.constData(), before.size(), after.constData(), after.size());
}

/*!
    \fn QByteArray &QByteArray::replace(const char *before, const QByteArray &after)
    \overload

    Replaces every occurrence of the string \a before with the
    byte array \a after.
*/

QByteArray &QByteArray::replace(const char *c, const QByteArray &after)
{
    return replace(c, qstrlen(c), after.constData(), after.size());
}

/*!
    \fn QByteArray &QByteArray::replace(const char *before, int bsize, const char *after, int asize)
    \overload

    Replaces every occurrence of the string \a before with the string \a after.
    Since the sizes of the strings are given by \a bsize and \a asize, they
    may contain zero characters and do not need to be zero-terminated.
*/

QByteArray &QByteArray::replace(const char *before, int bsize, const char *after, int asize)
{
    if (isEmpty() || !before || !bsize || (before == after && bsize == asize))
        return *this;

    QByteArrayMatcher matcher(before, bsize);
    int index = 0;
    int len = d->size;
    char *d = data();

    if (bsize == asize) {
        if (bsize) {
            while ((index = matcher.indexIn(*this, index)) != -1) {
                memcpy(d + index, after, asize);
                index += bsize;
            }
        }
    } else if (asize < bsize) {
        uint to = 0;
        uint movestart = 0;
        uint num = 0;
        while ((index = matcher.indexIn(*this, index)) != -1) {
            if (num) {
                int msize = index - movestart;
                if (msize > 0) {
                    memmove(d + to, d + movestart, msize);
                    to += msize;
                }
            } else {
                to = index;
            }
            if (asize) {
                memcpy(d + to, after, asize);
                to += asize;
            }
            index += bsize;
            movestart = index;
            num++;
        }
        if (num) {
            int msize = len - movestart;
            if (msize > 0)
                memmove(d + to, d + movestart, msize);
            resize(len - num*(bsize-asize));
        }
    } else {
        // the most complex case. We don't want to lose performance by doing repeated
        // copies and reallocs of the string.
        while (index != -1) {
            QSTACKARRAY(uint, indices, 4096);
            uint pos = 0;
            while(pos < 4095) {
                index = matcher.indexIn(*this, index);
                if (index == -1)
                    break;
                indices[pos++] = index;
                index += bsize;
                // avoid infinite loop
                if (!bsize)
                    index++;
            }
            if (!pos)
                break;

            // we have a table of replacement positions, use them for fast replacing
            int adjust = pos*(asize-bsize);
            // index has to be adjusted in case we get back into the loop above.
            if (index != -1)
                index += adjust;
            int newlen = len + adjust;
            int moveend = len;
            if (newlen > len) {
                resize(newlen);
                len = newlen;
            }
            d = this->d->data;

            while(pos) {
                pos--;
                int movestart = indices[pos] + bsize;
                int insertstart = indices[pos] + pos*(asize-bsize);
                int moveto = insertstart + asize;
                memmove(d + moveto, d + movestart, (moveend - movestart));
                if (asize)
                    memcpy(d + insertstart, after, asize);
                moveend = movestart - bsize;
            }
        }
    }
    
    return *this;
}


/*!
    \fn QByteArray &QByteArray::replace(const QByteArray &before, const char *after)
    \overload

    Replaces every occurrence of the byte array \a before with the
    string \a after.
*/

/*! \fn QByteArray &QByteArray::replace(const QString &before, const QByteArray &after)

    \overload

    Replaces every occurrence of the string \a before with the byte
    array \a after. The Unicode data is converted into 8-bit
    characters using QString::toAscii().

    If the QString contains non-ASCII Unicode characters, using this
    function can lead to loss of information. You can disable this
    function by defining \c QT_NO_CAST_TO_ASCII when you compile your
    applications. You then need to call QString::toAscii() (or
    QString::toLatin1() or QString::toUtf8() or QString::toLocal8Bit())
    explicitly if you want to convert the data to \c{const char *}.
*/

/*! \fn QByteArray &QByteArray::replace(const QString &before, const char *after)
    \overload

    Replaces every occurrence of the string \a before with the string
    \a after.
*/

/*! \fn QByteArray &QByteArray::replace(const char *before, const char *after)

    \overload

    Replaces every occurrence of the string \a before with the string
    \a after.
*/

/*!
    \overload

    Replaces every occurrence of the character \a before with the
    byte array \a after.
*/

QByteArray &QByteArray::replace(char before, const QByteArray &after)
{
    const char b[2] = { before, '\0' };
    QByteArray cb = fromRawData(b, 1);
    return replace(cb, after);
}

/*! \fn QByteArray &QByteArray::replace(char before, const QString &after)

    \overload

    Replaces every occurrence of the character \a before with the
    string \a after. The Unicode data is converted into 8-bit
    characters using QString::toAscii().

    If the QString contains non-ASCII Unicode characters, using this
    function can lead to loss of information. You can disable this
    function by defining \c QT_NO_CAST_TO_ASCII when you compile your
    applications. You then need to call QString::toAscii() (or
    QString::toLatin1() or QString::toUtf8() or QString::toLocal8Bit())
    explicitly if you want to convert the data to \c{const char *}.
*/

/*! \fn QByteArray &QByteArray::replace(char before, const char *after)

    \overload

    Replaces every occurrence of the character \a before with the
    string \a after.
*/

/*!
    \overload

    Replaces every occurrence of the character \a before with the
    character \a after.
*/

QByteArray &QByteArray::replace(char before, char after)
{
    if (d->size) {
        char *i = data();
        char *e = i + d->size;
        for (; i != e; ++i)
            if (*i == before)
                * i = after;
    }
    return *this;
}

/*!
    Splits the byte array into subarrays wherever \a sep occurs, and
    returns the list of those arrays. If \a sep does not match
    anywhere in the byte array, split() returns a single-element list
    containing this byte array.
*/

QList<QByteArray> QByteArray::split(char sep) const
{
    QList<QByteArray> list;
    int start = 0;
    int end;
    while ((end = indexOf(sep, start)) != -1) {
        list.append(mid(start, end - start));
        start = end + 1;
    }
    list.append(mid(start));
    return list;
}

/*!
    \since 4.5

    Returns a copy of this byte array repeated the specified number of \a times.

    If \a times is less than 1, an empty byte array is returned.

    Example:

    \code
        QByteArray ba("ab");
        ba.repeated(4);             // returns "abababab"
    \endcode
*/
QByteArray QByteArray::repeated(const int times) const
{
    if (d->size == 0 || times == 1) {
        return *this;
    }
    if (times < 1) {
        return QByteArray();
    }

    const int resultsize = (times * d->size);
    QByteArray result(resultsize, Qt::Uninitialized);
    for (int i = 0; i < times; i++) {
        ::memcpy(result.d->data + (i * d->size), d->data, d->size);
    }
    result.d->data[resultsize] = '\0';
    return result;
}

/*!
    Returns the index position of the first occurrence of the byte
    array \a ba in this byte array, searching forward from index
    position \a from. Returns -1 if \a ba could not be found.

    Example:
    \snippet doc/src/snippets/code/src_corelib_tools_qbytearray.cpp 21

    \sa lastIndexOf(), contains(), count()
*/

int QByteArray::indexOf(const QByteArray &ba, const int from) const
{
    const int ol = ba.d->size;
    if (ol == 0)
        return from;
    if (ol == 1)
        return indexOf(*ba.d->data, from);

    const int l = d->size;
    if (from > d->size || ol + from > l)
        return -1;

    return qFindByteArray(d->data, d->size, from, ba.d->data, ol);
}

/*! \fn int QByteArray::indexOf(const QString &str, int from) const

    \overload

    Returns the index position of the first occurrence of the string
    \a str in the byte array, searching forward from index position
    \a from. Returns -1 if \a str could not be found.

    The Unicode data is converted into 8-bit characters using
    QString::toAscii().

    If the QString contains non-ASCII Unicode characters, using this
    function can lead to loss of information. You can disable this
    function by defining \c QT_NO_CAST_TO_ASCII when you compile your
    applications. You then need to call QString::toAscii() (or
    QString::toLatin1() or QString::toUtf8() or QString::toLocal8Bit())
    explicitly if you want to convert the data to \c{const char *}.
*/

/*! \fn int QByteArray::indexOf(const char *str, int from) const

    \overload

    Returns the index position of the first occurrence of the string
    \a str in the byte array, searching forward from index position \a
    from. Returns -1 if \a str could not be found.
*/
int QByteArray::indexOf(const char *c, const int from) const
{
    const int ol = qstrlen(c);
    if (ol == 1)
        return indexOf(*c, from);
    
    const int l = d->size;
    if (from > d->size || ol + from > l)
        return -1;
    if (ol == 0)
        return from;

    return qFindByteArray(d->data, d->size, from, c, ol);
}

/*!
    \overload

    Returns the index position of the first occurrence of the
    character \a ch in the byte array, searching forward from index
    position \a from. Returns -1 if \a ch could not be found.

    Example:
    \snippet doc/src/snippets/code/src_corelib_tools_qbytearray.cpp 22

    \sa lastIndexOf(), contains()
*/

int QByteArray::indexOf(char ch, int from) const
{
    if (from < 0)
        from = qMax(from + d->size, 0);
    if (from < d->size) {
        const char *n = d->data + from - 1;
        const char *e = d->data + d->size;
        while (++n != e) {
            if (*n == ch)
                return  n - d->data;
        }
    }
    return -1;
}

#define REHASH(a) \
    if (ol_minus_1 < sizeof(uint) * CHAR_BIT) \
        hashHaystack -= (a) << ol_minus_1; \
    hashHaystack <<= 1

static int lastIndexOfHelper(const char *haystack, int l, const char *needle, int ol, int from)
{
    int delta = l - ol;
    if (from < 0)
        from = delta;
    if (from < 0 || from > l)
        return -1;
    if (from > delta)
        from = delta;

    const char *end = haystack;
    haystack += from;
    const uint ol_minus_1 = ol - 1;
    const char *n = needle + ol_minus_1;
    const char *h = haystack + ol_minus_1;
    uint hashNeedle = 0, hashHaystack = 0;
    int idx;
    for (idx = 0; idx < ol; ++idx) {
        hashNeedle = ((hashNeedle<<1) + *(n-idx));
        hashHaystack = ((hashHaystack<<1) + *(h-idx));
    }
    hashHaystack -= *haystack;
    while (haystack >= end) {
        hashHaystack += *haystack;
        if (hashHaystack == hashNeedle && memcmp(needle, haystack, ol) == 0)
            return haystack - end;
        --haystack;
        REHASH(*(haystack + ol));
    }
    return -1;

}
#undef REHASH

/*!
    \fn int QByteArray::lastIndexOf(const QByteArray &ba, int from) const

    Returns the index position of the last occurrence of the byte
    array \a ba in this byte array, searching backward from index
    position \a from. If \a from is -1 (the default), the search
    starts at the last byte. Returns -1 if \a ba could not be found.

    Example:
    \snippet doc/src/snippets/code/src_corelib_tools_qbytearray.cpp 23

    \sa indexOf(), contains(), count()
*/

int QByteArray::lastIndexOf(const QByteArray &ba, int from) const
{
    const int ol = ba.d->size;
    if (ol == 1)
        return lastIndexOf(*ba.d->data, from);

    return lastIndexOfHelper(d->data, d->size, ba.d->data, ol, from);
}

/*! \fn int QByteArray::lastIndexOf(const QString &str, int from) const

    \overload

    Returns the index position of the last occurrence of the string \a
    str in the byte array, searching backward from index position \a
    from. If \a from is -1 (the default), the search starts at the
    last (size() - 1) byte. Returns -1 if \a str could not be found.

    The Unicode data is converted into 8-bit characters using
    QString::toAscii().

    If the QString contains non-ASCII Unicode characters, using this
    function can lead to loss of information. You can disable this
    function by defining \c QT_NO_CAST_TO_ASCII when you compile your
    applications. You then need to call QString::toAscii() (or
    QString::toLatin1() or QString::toUtf8() or QString::toLocal8Bit())
    explicitly if you want to convert the data to \c{const char *}.
*/

/*! \fn int QByteArray::lastIndexOf(const char *str, int from) const
    \overload

    Returns the index position of the last occurrence of the string \a
    str in the byte array, searching backward from index position \a
    from. If \a from is -1 (the default), the search starts at the
    last (size() - 1) byte. Returns -1 if \a str could not be found.
*/
int QByteArray::lastIndexOf(const char *str, int from) const
{
    const int ol = qstrlen(str);
    if (ol == 1)
        return lastIndexOf(*str, from);

    return lastIndexOfHelper(d->data, d->size, str, ol, from);
}

/*!
    \overload

    Returns the index position of the last occurrence of character \a
    ch in the byte array, searching backward from index position \a
    from. If \a from is -1 (the default), the search starts at the
    last (size() - 1) byte. Returns -1 if \a ch could not be found.

    Example:
    \snippet doc/src/snippets/code/src_corelib_tools_qbytearray.cpp 24

    \sa indexOf(), contains()
*/

int QByteArray::lastIndexOf(char ch, int from) const
{
    if (from < 0)
        from += d->size;
    else if (from > d->size)
        from = d->size-1;
    if (from >= 0) {
        const char *b = d->data;
        const char *n = d->data + from + 1;
        while (n-- != b)
            if (*n == ch)
                return  n - b;
    }
    return -1;
}

/*!
    Returns the number of (potentially overlapping) occurrences of
    byte array \a ba in this byte array.

    \sa contains(), indexOf()
*/

int QByteArray::count(const QByteArray &ba) const
{
    int num = 0;
    int i = -1;
    if (d->size > 500 && ba.d->size > 5) {
        QByteArrayMatcher matcher(ba);
        while ((i = matcher.indexIn(*this, i + 1)) != -1)
            ++num;
    } else {
        while ((i = indexOf(ba, i + 1)) != -1)
            ++num;
    }
    return num;
}

/*!
    \overload

    Returns the number of (potentially overlapping) occurrences of
    string \a str in the byte array.
*/

int QByteArray::count(const char *str) const
{
    return count(fromRawData(str, qstrlen(str)));
}

/*!
    \overload

    Returns the number of occurrences of character \a ch in the byte
    array.

    \sa contains(), indexOf()
*/

int QByteArray::count(char ch) const
{
    int num = 0;
    const char *i = d->data + d->size;
    const char *b = d->data;
    while (i != b)
        if (*--i == ch)
            ++num;
    return num;
}

/*! \fn int QByteArray::count() const

    \overload

    Same as size().
*/

/*!
    Returns true if this byte array starts with byte array \a ba;
    otherwise returns false.

    Example:
    \snippet doc/src/snippets/code/src_corelib_tools_qbytearray.cpp 25

    \sa endsWith(), left()
*/
bool QByteArray::startsWith(const QByteArray &ba) const
{
    if (d == ba.d || ba.isEmpty())
        return true;
    if (d->size < ba.d->size)
        return false;
    return memcmp(d->data, ba.d->data, ba.d->size) == 0;
}

/*! \overload

    Returns true if this byte array starts with string \a str;
    otherwise returns false.
*/
bool QByteArray::startsWith(const char *str) const
{
    if (!str || !*str)
        return true;
    int len = qstrlen(str);
    if (d->size < len)
        return false;
    return qstrncmp(d->data, str, len) == 0;
}

/*! \overload

    Returns true if this byte array starts with character \a ch;
    otherwise returns false.
*/
bool QByteArray::startsWith(char ch) const
{
    if (d->size == 0)
        return false;
    return d->data[0] == ch;
}

/*!
    Returns true if this byte array ends with byte array \a ba;
    otherwise returns false.

    Example:
    \snippet doc/src/snippets/code/src_corelib_tools_qbytearray.cpp 26

    \sa startsWith(), right()
*/
bool QByteArray::endsWith(const QByteArray &ba) const
{
    if (d == ba.d || ba.isEmpty())
        return true;
    if (d->size < ba.d->size)
        return false;
    return memcmp(d->data + d->size - ba.d->size, ba.d->data, ba.d->size) == 0;
}

/*! \overload

    Returns true if this byte array ends with string \a str; otherwise
    returns false.
*/
bool QByteArray::endsWith(const char *str) const
{
    if (!str || !*str)
        return true;
    const int len = qstrlen(str);
    if (d->size < len)
        return false;
    return qstrncmp(d->data + d->size - len, str, len) == 0;
}

/*! \overload

    Returns true if this byte array ends with character \a ch;
    otherwise returns false.
*/
bool QByteArray::endsWith(char ch) const
{
    if (d->size == 0)
        return false;
    return d->data[d->size - 1] == ch;
}

/*!
    Returns a byte array that contains the leftmost \a len bytes of
    this byte array.

    The entire byte array is returned if \a len is greater than
    size().

    Example:
    \snippet doc/src/snippets/code/src_corelib_tools_qbytearray.cpp 27

    \sa right(), mid(), startsWith(), truncate()
*/

QByteArray QByteArray::left(int len)  const
{
    if (len >= d->size)
        return *this;
    if (len < 0)
        len = 0;
    return QByteArray(d->data, len);
}

/*!
    Returns a byte array that contains the rightmost \a len bytes of
    this byte array.

    The entire byte array is returned if \a len is greater than
    size().

    Example:
    \snippet doc/src/snippets/code/src_corelib_tools_qbytearray.cpp 28

    \sa endsWith(), left(), mid()
*/

QByteArray QByteArray::right(int len) const
{
    if (len >= d->size)
        return *this;
    if (len < 0)
        len = 0;
    return QByteArray(d->data + d->size - len, len);
}

/*!
    Returns a byte array containing \a len bytes from this byte array,
    starting at position \a pos.

    If \a len is -1 (the default), or \a pos + \a len >= size(),
    returns a byte array containing all bytes starting at position \a
    pos until the end of the byte array.

    Example:
    \snippet doc/src/snippets/code/src_corelib_tools_qbytearray.cpp 29

    \sa left(), right()
*/

QByteArray QByteArray::mid(int pos, int len) const
{
    if (isEmpty() || pos >= d->size)
        return QByteArray();
    if (len < 0)
        len = d->size - pos;
    if (pos < 0) {
        len += pos;
        pos = 0;
    }
    if (len + pos > d->size)
        len = d->size - pos;
    if (pos == 0 && len == d->size)
        return *this;
    return QByteArray(d->data + pos, len);
}

/*!
    Returns a lowercase copy of the byte array. The bytearray is
    interpreted as a Latin-1 encoded string.

    Example:
    \snippet doc/src/snippets/code/src_corelib_tools_qbytearray.cpp 30

    \sa toUpper(), {8-bit Character Comparisons}
*/
QByteArray QByteArray::toLower() const
{
    QByteArray s(*this);
    char *p = s.data();
    while (*p) {
        *p = qToLower(*p);
        p++;
    }
    return s;
}

/*!
    Returns an uppercase copy of the byte array. The bytearray is
    interpreted as a Latin-1 encoded string.

    Example:
    \snippet doc/src/snippets/code/src_corelib_tools_qbytearray.cpp 31

    \sa toLower(), {8-bit Character Comparisons}
*/

QByteArray QByteArray::toUpper() const
{
    QByteArray s(*this);
    char *p = s.data();
    while (*p) {
        *p = qToUpper(*p);
        p++;
    }
    return s;
}

/*! \fn void QByteArray::clear()

    Clears the contents of the byte array and makes it empty.

    \sa resize(), isEmpty()
*/

void QByteArray::clear()
{
    if (!d->ref.deref())
        freeData(d);
    d = &shared_null;
    d->ref.ref();
}

#if !defined(QT_NO_DATASTREAM)

/*! \relates QByteArray

    Writes byte array \a ba to the stream \a out and returns a reference
    to the stream.

    \sa {Serializing Qt Data Types}
*/

QDataStream &operator<<(QDataStream &out, const QByteArray &ba)
{
    if (ba.isNull()) {
        out << (quint32)0xffffffff;
        return out;
    }
    out << (quint32)ba.size();
    out.writeRawData(ba.constData(), ba.size());
    return out;
}

/*! \relates QByteArray

    Reads a byte array into \a ba from the stream \a in and returns a
    reference to the stream.

    \sa {Serializing Qt Data Types}
*/

QDataStream &operator>>(QDataStream &in, QByteArray &ba)
{
    quint32 len;
    in >> len;
    if (len == 0xffffffff) {
        ba.clear();
        return in;
    }

    ba.resize(len);
    const quint32 readlen = in.readRawData(ba.data(), len);
    if (readlen != len) {
        ba.clear();
        in.setStatus(QDataStream::ReadPastEnd);
        return in;
    }

    return in;
}
#endif // QT_NO_DATASTREAM

/*! \fn bool QByteArray::operator==(const QString &str) const

    Returns true if this byte array is equal to string \a str;
    otherwise returns false.

    The Unicode data is converted into 8-bit characters using
    QString::toAscii().

    The comparison is case sensitive.

    You can disable this operator by defining \c
    QT_NO_CAST_FROM_ASCII when you compile your applications. You
    then need to call QString::fromAscii(), QString::fromLatin1(),
    QString::fromUtf8(), or QString::fromLocal8Bit() explicitly if
    you want to convert the byte array to a QString before doing the
    comparison.
*/

/*! \fn bool QByteArray::operator!=(const QString &str) const

    Returns true if this byte array is not equal to string \a str;
    otherwise returns false.

    The Unicode data is converted into 8-bit characters using
    QString::toAscii().

    The comparison is case sensitive.

    You can disable this operator by defining \c
    QT_NO_CAST_FROM_ASCII when you compile your applications. You
    then need to call QString::fromAscii(), QString::fromLatin1(),
    QString::fromUtf8(), or QString::fromLocal8Bit() explicitly if
    you want to convert the byte array to a QString before doing the
    comparison.
*/

/*! \fn bool QByteArray::operator<(const QString &str) const

    Returns true if this byte array is lexically less than string \a
    str; otherwise returns false.

    The Unicode data is converted into 8-bit characters using
    QString::toAscii().

    The comparison is case sensitive.

    You can disable this operator by defining \c
    QT_NO_CAST_FROM_ASCII when you compile your applications. You
    then need to call QString::fromAscii(), QString::fromLatin1(),
    QString::fromUtf8(), or QString::fromLocal8Bit() explicitly if
    you want to convert the byte array to a QString before doing the
    comparison.
*/

/*! \fn bool QByteArray::operator>(const QString &str) const

    Returns true if this byte array is lexically greater than string
    \a str; otherwise returns false.

    The Unicode data is converted into 8-bit characters using
    QString::toAscii().

    The comparison is case sensitive.

    You can disable this operator by defining \c
    QT_NO_CAST_FROM_ASCII when you compile your applications. You
    then need to call QString::fromAscii(), QString::fromLatin1(),
    QString::fromUtf8(), or QString::fromLocal8Bit() explicitly if
    you want to convert the byte array to a QString before doing the
    comparison.
*/

/*! \fn bool QByteArray::operator<=(const QString &str) const

    Returns true if this byte array is lexically less than or equal
    to string \a str; otherwise returns false.

    The Unicode data is converted into 8-bit characters using
    QString::toAscii().

    The comparison is case sensitive.

    You can disable this operator by defining \c
    QT_NO_CAST_FROM_ASCII when you compile your applications. You
    then need to call QString::fromAscii(), QString::fromLatin1(),
    QString::fromUtf8(), or QString::fromLocal8Bit() explicitly if
    you want to convert the byte array to a QString before doing the
    comparison.
*/

/*! \fn bool QByteArray::operator>=(const QString &str) const

    Returns true if this byte array is greater than or equal to string
    \a str; otherwise returns false.

    The Unicode data is converted into 8-bit characters using
    QString::toAscii().

    The comparison is case sensitive.

    You can disable this operator by defining \c
    QT_NO_CAST_FROM_ASCII when you compile your applications. You
    then need to call QString::fromAscii(), QString::fromLatin1(),
    QString::fromUtf8(), or QString::fromLocal8Bit() explicitly if
    you want to convert the byte array to a QString before doing the
    comparison.
*/

/*! \fn bool operator==(const QByteArray &a1, const QByteArray &a2)
    \relates QByteArray

    \overload

    Returns true if byte array \a a1 is equal to byte array \a a2;
    otherwise returns false.
*/

/*! \fn bool operator==(const QByteArray &a1, const char *a2)
    \relates QByteArray

    \overload

    Returns true if byte array \a a1 is equal to string \a a2;
    otherwise returns false.
*/

/*! \fn bool operator==(const char *a1, const QByteArray &a2)
    \relates QByteArray

    \overload

    Returns true if string \a a1 is equal to byte array \a a2;
    otherwise returns false.
*/

/*! \fn bool operator!=(const QByteArray &a1, const QByteArray &a2)
    \relates QByteArray

    \overload

    Returns true if byte array \a a1 is not equal to byte array \a a2;
    otherwise returns false.
*/

/*! \fn bool operator!=(const QByteArray &a1, const char *a2)
    \relates QByteArray

    \overload

    Returns true if byte array \a a1 is not equal to string \a a2;
    otherwise returns false.
*/

/*! \fn bool operator!=(const char *a1, const QByteArray &a2)
    \relates QByteArray

    \overload

    Returns true if string \a a1 is not equal to byte array \a a2;
    otherwise returns false.
*/

/*! \fn bool operator<(const QByteArray &a1, const QByteArray &a2)
    \relates QByteArray

    \overload

    Returns true if byte array \a a1 is lexically less than byte array
    \a a2; otherwise returns false.
*/

/*! \fn inline bool operator<(const QByteArray &a1, const char *a2)
    \relates QByteArray

    \overload

    Returns true if byte array \a a1 is lexically less than string
    \a a2; otherwise returns false.
*/

/*! \fn bool operator<(const char *a1, const QByteArray &a2)
    \relates QByteArray

    \overload

    Returns true if string \a a1 is lexically less than byte array
    \a a2; otherwise returns false.
*/

/*! \fn bool operator<=(const QByteArray &a1, const QByteArray &a2)
    \relates QByteArray

    \overload

    Returns true if byte array \a a1 is lexically less than or equal
    to byte array \a a2; otherwise returns false.
*/

/*! \fn bool operator<=(const QByteArray &a1, const char *a2)
    \relates QByteArray

    \overload

    Returns true if byte array \a a1 is lexically less than or equal
    to string \a a2; otherwise returns false.
*/

/*! \fn bool operator<=(const char *a1, const QByteArray &a2)
    \relates QByteArray

    \overload

    Returns true if string \a a1 is lexically less than or equal
    to byte array \a a2; otherwise returns false.
*/

/*! \fn bool operator>(const QByteArray &a1, const QByteArray &a2)
    \relates QByteArray

    \overload

    Returns true if byte array \a a1 is lexically greater than byte
    array \a a2; otherwise returns false.
*/

/*! \fn bool operator>(const QByteArray &a1, const char *a2)
    \relates QByteArray

    \overload

    Returns true if byte array \a a1 is lexically greater than string
    \a a2; otherwise returns false.
*/

/*! \fn bool operator>(const char *a1, const QByteArray &a2)
    \relates QByteArray

    \overload

    Returns true if string \a a1 is lexically greater than byte array
    \a a2; otherwise returns false.
*/

/*! \fn bool operator>=(const QByteArray &a1, const QByteArray &a2)
    \relates QByteArray

    \overload

    Returns true if byte array \a a1 is lexically greater than or
    equal to byte array \a a2; otherwise returns false.
*/

/*! \fn bool operator>=(const QByteArray &a1, const char *a2)
    \relates QByteArray

    \overload

    Returns true if byte array \a a1 is lexically greater than or
    equal to string \a a2; otherwise returns false.
*/

/*! \fn bool operator>=(const char *a1, const QByteArray &a2)
    \relates QByteArray

    \overload

    Returns true if string \a a1 is lexically greater than or
    equal to byte array \a a2; otherwise returns false.
*/

/*! \fn const QByteArray operator+(const QByteArray &a1, const QByteArray &a2)
    \relates QByteArray

    Returns a byte array that is the result of concatenating byte
    array \a a1 and byte array \a a2.

    \sa QByteArray::operator+=()
*/

/*! \fn const QByteArray operator+(const QByteArray &a1, const char *a2)
    \relates QByteArray

    \overload

    Returns a byte array that is the result of concatenating byte
    array \a a1 and string \a a2.
*/

/*! \fn const QByteArray operator+(const QByteArray &a1, char a2)
    \relates QByteArray

    \overload

    Returns a byte array that is the result of concatenating byte
    array \a a1 and character \a a2.
*/

/*! \fn const QByteArray operator+(const char *a1, const QByteArray &a2)
    \relates QByteArray

    \overload

    Returns a byte array that is the result of concatenating string
    \a a1 and byte array \a a2.
*/

/*! \fn const QByteArray operator+(char a1, const QByteArray &a2)
    \relates QByteArray

    \overload

    Returns a byte array that is the result of concatenating character
    \a a1 and byte array \a a2.
*/

/*!
    Returns a byte array that has whitespace removed from the start
    and the end, and which has each sequence of internal whitespace
    replaced with a single space.

    Whitespace means any character for which the standard C++
    isspace() function returns true. This includes the ASCII
    characters '\\t', '\\n', '\\v', '\\f', '\\r', and ' '.

    Example:
    \snippet doc/src/snippets/code/src_corelib_tools_qbytearray.cpp 32

    \sa trimmed()
*/
QByteArray QByteArray::simplified() const
{
    if (d->size == 0)
        return *this;

    QSTACKARRAY(char, result, d->size)
    const char *from = d->data;
    const char *fromend = from + d->size;
    int outc = 0;
    for (;;) {
        while (from != fromend && isspace(uchar(*from)))
            from++;
        while (from != fromend && !isspace(uchar(*from)))
            result[outc++] = *from++;
        if (from != fromend)
            result[outc++] = ' ';
        else
            break;
    }
    if (outc > 0 && result[outc-1] == ' ')
        outc--;
    return QByteArray(result, outc);
}

/*!
    Returns a byte array that has whitespace removed from the start
    and the end.

    Whitespace means any character for which the standard C++
    isspace() function returns true. This includes the ASCII
    characters '\\t', '\\n', '\\v', '\\f', '\\r', and ' '.

    Example:
    \snippet doc/src/snippets/code/src_corelib_tools_qbytearray.cpp 33

    Unlike simplified(), trimmed() leaves internal whitespace alone.

    \sa simplified()
*/
QByteArray QByteArray::trimmed() const
{
    if (d->size == 0)
        return *this;
    const char *s = d->data;
    if (!isspace(uchar(*s)) && !isspace(uchar(s[d->size-1])))
        return *this;
    int start = 0;
    int end = d->size - 1;
    while (start<=end && isspace(uchar(s[start])))  // skip white space from start
        start++;
    if (start <= end) {                          // only white space
        while (end && isspace(uchar(s[end])))           // skip white space from end
            end--;
    }
    int l = end - start + 1;
    if (l <= 0) {
        return QByteArray();
    }
    return QByteArray(s+start, l);
}

/*!
    Returns a byte array of size \a width that contains this byte
    array padded by the \a fill character.

    If \a truncate is false and the size() of the byte array is more
    than \a width, then the returned byte array is a copy of this byte
    array.

    If \a truncate is true and the size() of the byte array is more
    than \a width, then any bytes in a copy of the byte array
    after position \a width are removed, and the copy is returned.

    Example:
    \snippet doc/src/snippets/code/src_corelib_tools_qbytearray.cpp 34

    \sa rightJustified()
*/

QByteArray QByteArray::leftJustified(int width, char fill, bool truncate) const
{
    QByteArray result;
    int len = d->size;
    int padlen = width - len;
    if (padlen > 0) {
        result.resize(len+padlen);
        if (len)
            memcpy(result.d->data, d->data, len);
        memset(result.d->data+len, fill, padlen);
    } else {
        if (truncate)
            result = left(width);
        else
            result = *this;
    }
    return result;
}

/*!
    Returns a byte array of size \a width that contains the \a fill
    character followed by this byte array.

    If \a truncate is false and the size of the byte array is more
    than \a width, then the returned byte array is a copy of this byte
    array.

    If \a truncate is true and the size of the byte array is more
    than \a width, then the resulting byte array is truncated at
    position \a width.

    Example:
    \snippet doc/src/snippets/code/src_corelib_tools_qbytearray.cpp 35

    \sa leftJustified()
*/

QByteArray QByteArray::rightJustified(int width, char fill, bool truncate) const
{
    QByteArray result;
    int len = d->size;
    int padlen = width - len;
    if (padlen > 0) {
        result.resize(len+padlen);
        if (len)
            memcpy(result.d->data+padlen, data(), len);
        memset(result.d->data, fill, padlen);
    } else {
        if (truncate)
            result = left(width);
        else
            result = *this;
    }
    return result;
}

bool QByteArray::isNull() const { return d == &shared_null; }


/*!
    Returns the byte array converted to a \c {long long} using base \a
    base, which is 10 by default and must be between 2 and 36, or 0.

    If \a base is 0, the base is determined automatically using the
    following rules: If the byte array begins with "0x", it is assumed to
    be hexadecimal; if it begins with "0", it is assumed to be octal;
    otherwise it is assumed to be decimal.

    Returns 0 if the conversion fails.

    If \a ok is not 0: if a conversion error occurs, *\a{ok} is set to
    false; otherwise *\a{ok} is set to true.

    \note The conversion of the number is performed in the default C locale,
    irrespective of the user's locale.

    \sa number()
*/

qlonglong QByteArray::toLongLong(bool *ok, int base) const
{
#if defined(QT_CHECK_RANGE)
    if (Q_UNLIKELY(base != 0 && (base < 2 || base > 36))) {
        qWarning("QByteArray::toLongLong: Invalid base %d", base);
        base = 10;
    }
#endif

    return QLocalePrivate::bytearrayToLongLong(nulTerminated().constData(), base, ok);
}

/*!
    Returns the byte array converted to an \c {unsigned long long}
    using base \a base, which is 10 by default and must be between 2
    and 36, or 0.

    If \a base is 0, the base is determined automatically using the
    following rules: If the byte array begins with "0x", it is assumed to
    be hexadecimal; if it begins with "0", it is assumed to be octal;
    otherwise it is assumed to be decimal.

    Returns 0 if the conversion fails.

    If \a ok is not 0: if a conversion error occurs, *\a{ok} is set to
    false; otherwise *\a{ok} is set to true.

    \note The conversion of the number is performed in the default C locale,
    irrespective of the user's locale.

    \sa number()
*/

qulonglong QByteArray::toULongLong(bool *ok, int base) const
{
#if defined(QT_CHECK_RANGE)
    if (Q_UNLIKELY(base != 0 && (base < 2 || base > 36))) {
        qWarning("QByteArray::toULongLong: Invalid base %d", base);
        base = 10;
    }
#endif

    return QLocalePrivate::bytearrayToUnsLongLong(nulTerminated().constData(), base, ok);
}


/*!
    Returns the byte array converted to an \c int using base \a
    base, which is 10 by default and must be between 2 and 36, or 0.

    If \a base is 0, the base is determined automatically using the
    following rules: If the byte array begins with "0x", it is assumed to
    be hexadecimal; if it begins with "0", it is assumed to be octal;
    otherwise it is assumed to be decimal.

    Returns 0 if the conversion fails.

    If \a ok is not 0: if a conversion error occurs, *\a{ok} is set to
    false; otherwise *\a{ok} is set to true.

    \snippet doc/src/snippets/code/src_corelib_tools_qbytearray.cpp 36

    \note The conversion of the number is performed in the default C locale,
    irrespective of the user's locale.

    \sa number()
*/

int QByteArray::toInt(bool *ok, int base) const
{
    qlonglong v = toLongLong(ok, base);
    if (Q_UNLIKELY(v < INT_MIN || v > INT_MAX)) {
        if (ok)
            *ok = false;
        return 0;
    }
    return int(v);
}

/*!
    Returns the byte array converted to an \c {unsigned int} using base \a
    base, which is 10 by default and must be between 2 and 36, or 0.

    If \a base is 0, the base is determined automatically using the
    following rules: If the byte array begins with "0x", it is assumed to
    be hexadecimal; if it begins with "0", it is assumed to be octal;
    otherwise it is assumed to be decimal.

    Returns 0 if the conversion fails.

    If \a ok is not 0: if a conversion error occurs, *\a{ok} is set to
    false; otherwise *\a{ok} is set to true.

    \note The conversion of the number is performed in the default C locale,
    irrespective of the user's locale.

    \sa number()
*/

uint QByteArray::toUInt(bool *ok, int base) const
{
    qulonglong v = toULongLong(ok, base);
    if (Q_UNLIKELY(v > UINT_MAX)) {
        if (ok)
            *ok = false;
        return 0;
    }
    return uint(v);
}

/*!
    \since 4.1

    Returns the byte array converted to a \c long int using base \a
    base, which is 10 by default and must be between 2 and 36, or 0.

    If \a base is 0, the base is determined automatically using the
    following rules: If the byte array begins with "0x", it is assumed to
    be hexadecimal; if it begins with "0", it is assumed to be octal;
    otherwise it is assumed to be decimal.

    Returns 0 if the conversion fails.

    If \a ok is not 0: if a conversion error occurs, *\a{ok} is set to
    false; otherwise *\a{ok} is set to true.

    \snippet doc/src/snippets/code/src_corelib_tools_qbytearray.cpp 37

    \note The conversion of the number is performed in the default C locale,
    irrespective of the user's locale.

    \sa number()
*/
long QByteArray::toLong(bool *ok, int base) const
{
    qlonglong v = toLongLong(ok, base);
    if (Q_UNLIKELY(v < LONG_MIN || v > LONG_MAX)) {
        if (ok)
            *ok = false;
        return 0;
    }
    return long(v);
}

/*!
    \since 4.1

    Returns the byte array converted to an \c {unsigned long int} using base \a
    base, which is 10 by default and must be between 2 and 36, or 0.

    If \a base is 0, the base is determined automatically using the
    following rules: If the byte array begins with "0x", it is assumed to
    be hexadecimal; if it begins with "0", it is assumed to be octal;
    otherwise it is assumed to be decimal.

    Returns 0 if the conversion fails.

    If \a ok is not 0: if a conversion error occurs, *\a{ok} is set to
    false; otherwise *\a{ok} is set to true.

    \note The conversion of the number is performed in the default C locale,
    irrespective of the user's locale.

    \sa number()
*/
ulong QByteArray::toULong(bool *ok, int base) const
{
    qulonglong v = toULongLong(ok, base);
    if (Q_UNLIKELY(v > ULONG_MAX)) {
        if (ok)
            *ok = false;
        return 0;
    }
    return ulong(v);
}

/*!
    Returns the byte array converted to a \c short using base \a
    base, which is 10 by default and must be between 2 and 36, or 0.

    If \a base is 0, the base is determined automatically using the
    following rules: If the byte array begins with "0x", it is assumed to
    be hexadecimal; if it begins with "0", it is assumed to be octal;
    otherwise it is assumed to be decimal.

    Returns 0 if the conversion fails.

    If \a ok is not 0: if a conversion error occurs, *\a{ok} is set to
    false; otherwise *\a{ok} is set to true.

    \note The conversion of the number is performed in the default C locale,
    irrespective of the user's locale.

    \sa number()
*/

short QByteArray::toShort(bool *ok, int base) const
{
    qlonglong v = toLongLong(ok, base);
    if (Q_UNLIKELY(v < SHRT_MIN || v > SHRT_MAX)) {
        if (ok)
            *ok = false;
        return 0;
    }
    return short(v);
}

/*!
    Returns the byte array converted to an \c {unsigned short} using base \a
    base, which is 10 by default and must be between 2 and 36, or 0.

    If \a base is 0, the base is determined automatically using the
    following rules: If the byte array begins with "0x", it is assumed to
    be hexadecimal; if it begins with "0", it is assumed to be octal;
    otherwise it is assumed to be decimal.

    Returns 0 if the conversion fails.

    If \a ok is not 0: if a conversion error occurs, *\a{ok} is set to
    false; otherwise *\a{ok} is set to true.

    \note The conversion of the number is performed in the default C locale,
    irrespective of the user's locale.

    \sa number()
*/

ushort QByteArray::toUShort(bool *ok, int base) const
{
    qulonglong v = toULongLong(ok, base);
    if (Q_UNLIKELY(v > USHRT_MAX)) {
        if (ok)
            *ok = false;
        return 0;
    }
    return ushort(v);
}


/*!
    Returns the byte array converted to a \c double value.

    Returns 0.0 if the conversion fails.

    If \a ok is not 0: if a conversion error occurs, *\a{ok} is set to
    false; otherwise *\a{ok} is set to true.

    \snippet doc/src/snippets/code/src_corelib_tools_qbytearray.cpp 38

    \note The conversion of the number is performed in the default C locale,
    irrespective of the user's locale.

    \sa number()
*/

double QByteArray::toDouble(bool *ok) const
{
    return QLocalePrivate::bytearrayToDouble(nulTerminated().constData(), ok);
}

/*!
    Returns the byte array converted to a \c float value.

    Returns 0.0 if the conversion fails.

    If \a ok is not 0: if a conversion error occurs, *\a{ok} is set to
    false; otherwise *\a{ok} is set to true.

    \note The conversion of the number is performed in the default C locale,
    irrespective of the user's locale.

    \sa number()
*/

float QByteArray::toFloat(bool *ok) const
{
    return QLocalePrivate::bytearrayToFloat(nulTerminated().constData(), ok);
}

/*!
    Returns a copy of the byte array, encoded as Base64.

    \snippet doc/src/snippets/code/src_corelib_tools_qbytearray.cpp 39

    The algorithm used to encode Base64-encoded data is defined in \l{RFC 2045}.

    \sa fromBase64()
*/

static const char base64chars[] = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/";
static const char base64padchar = '=';

QByteArray QByteArray::toBase64() const
{
    int padlen = 0;

    QByteArray tmp((d->size * 4) / 3 + 3, Qt::Uninitialized);

    int i = 0;
    char *out = tmp.data();
    while (i < d->size) {
        int chunk = int(uchar(d->data[i++])) << 16;
        if (i == d->size) {
            padlen = 2;
        } else {
            chunk |= int(uchar(d->data[i++])) << 8;
            if (i == d->size)
                padlen = 1;
            else
                chunk |= int(uchar(d->data[i++]));
        }

        int j = (chunk & 0x00fc0000) >> 18;
        int k = (chunk & 0x0003f000) >> 12;
        int l = (chunk & 0x00000fc0) >> 6;
        int m = (chunk & 0x0000003f);
        *out++ = base64chars[j];
        *out++ = base64chars[k];
        if (padlen > 1) *out++ = base64padchar;
        else *out++ = base64chars[l];
        if (padlen > 0) *out++ = base64padchar;
        else *out++ = base64chars[m];
    }

    tmp.truncate(out - tmp.data());
    return tmp;
}

/*! 
    \fn QByteArray &QByteArray::setNum(int n, int base)

    Sets the byte array to the printed value of \a n in base \a base (10
    by default) and returns a reference to the byte array. The \a base can
    be any value between 2 and 36.

    Example:
    \snippet doc/src/snippets/code/src_corelib_tools_qbytearray.cpp 40

    \note The format of the number is not localized; the default C locale
    is used irrespective of the user's locale.

    \sa number(), toInt()
*/

/*! 
    \fn QByteArray &QByteArray::setNum(uint n, int base)
    \overload

    \sa toUInt()
*/

/*! 
    \fn QByteArray &QByteArray::setNum(short n, int base)
    \overload

    \sa toShort()
*/

/*! 
    \fn QByteArray &QByteArray::setNum(ushort n, int base)
    \overload

    \sa toUShort()
*/

/*!
    \overload

    \sa toLongLong()
*/

QByteArray &QByteArray::setNum(qlonglong n, int base)
{
    *this = QByteArray::number(n, base);
    return *this;
}

/*!
    \overload

    \sa toULongLong()
*/

QByteArray &QByteArray::setNum(qulonglong n, int base)
{
    *this = QByteArray::number(n, base);
    return *this;
}

/*! 
    \overload

    Sets the byte array to the printed value of \a n, formatted in format
    \a f with precision \a prec, and returns a reference to the
    byte array.

    The format \a f can be any of the following:

    \table
    \header \i Format \i Meaning
    \row \i \c e \i format as [-]9.9e[+|-]999
    \row \i \c E \i format as [-]9.9E[+|-]999
    \row \i \c f \i format as [-]9.9
    \row \i \c g \i use \c e or \c f format, whichever is the most concise
    \row \i \c G \i use \c E or \c f format, whichever is the most concise
    \endtable

    With 'e', 'E', and 'f', \a prec is the number of digits after the
    decimal point. With 'g' and 'G', \a prec is the maximum number of
    significant digits (trailing zeroes are omitted).

    \note The format of the number is not localized; the default C locale
    is used irrespective of the user's locale.

    \sa toDouble()
*/

QByteArray &QByteArray::setNum(double n, char f, int prec)
{
    *this = QByteArray::number(n, f, prec);
    return *this;
}

/*! 
    \fn QByteArray &QByteArray::setNum(float n, char f, int prec)
    \overload

    Sets the byte array to the printed value of \a n, formatted in format
    \a f with precision \a prec, and returns a reference to the
    byte array.

    \note The format of the number is not localized; the default C locale
    is used irrespective of the user's locale.

    \sa toFloat()
*/

/*!
    \overload

    \sa toShort()
*/
QByteArray QByteArray::number(short n, int base)
{
    return QByteArray::number(qlonglong(n), base);
}

/*!
    \overload

    \sa toUShort()
*/
QByteArray QByteArray::number(ushort n, int base)
{
    return QByteArray::number(qulonglong(n), base);
}

/*!
    Returns a byte array containing the string equivalent of the
    number \a n to base \a base (10 by default). The \a base can be
    any value between 2 and 36.

    Example:
    \snippet doc/src/snippets/code/src_corelib_tools_qbytearray.cpp 41

    \note The format of the number is not localized; the default C locale
    is used irrespective of the user's locale.

    \sa setNum(), toInt()
*/
QByteArray QByteArray::number(int n, int base)
{
    return QByteArray::number(qlonglong(n), base);
}

/*!
    \overload

    \sa toUInt()
*/
QByteArray QByteArray::number(uint n, int base)
{
    return QByteArray::number(qulonglong(n), base);
}

/*!
    \overload

    \sa toLongLong()
*/
QByteArray QByteArray::number(qlonglong n, int base)
{
#if defined(QT_CHECK_RANGE)
    if (Q_UNLIKELY(base < 2 || base > 36)) {
        qWarning("QByteArray::number: Invalid base %d", base);
        base = 10;
    }
#endif
    QLocale locale(QLocale::C);
    return locale.d()->longLongToString(n, -1, base).toLatin1();
}

/*!
    \overload

    \sa toULongLong()
*/
QByteArray QByteArray::number(qulonglong n, int base)
{
#if defined(QT_CHECK_RANGE)
    if (Q_UNLIKELY(base < 2 || base > 36)) {
        qWarning("QByteArray::number: Invalid base %d", base);
        base = 10;
    }
#endif
    QLocale locale(QLocale::C);
    return locale.d()->unsLongLongToString(n, -1, base).toLatin1();
}

/*! 
    \overload

    Returns a byte array that contains the printed value of \a n,
    formatted in format \a f with precision \a prec.

    Argument \a n is formatted according to the \a f format specified,
    which is \c g by default, and can be any of the following:

    \table
    \header \i Format \i Meaning
    \row \i \c e \i format as [-]9.9e[+|-]999
    \row \i \c E \i format as [-]9.9E[+|-]999
    \row \i \c f \i format as [-]9.9
    \row \i \c g \i use \c e or \c f format, whichever is the most concise
    \row \i \c G \i use \c E or \c f format, whichever is the most concise
    \endtable

    With 'e', 'E', and 'f', \a prec is the number of digits after the
    decimal point. With 'g' and 'G', \a prec is the maximum number of
    significant digits (trailing zeroes are omitted).

    \snippet doc/src/snippets/code/src_corelib_tools_qbytearray.cpp 42

    \note The format of the number is not localized; the default C locale
    is used irrespective of the user's locale.

    \sa toDouble()
*/
QByteArray QByteArray::number(double n, char f, int prec)
{
    QLocalePrivate::DoubleForm form = QLocalePrivate::DFDecimal;
    uint flags = 0;

    if (qIsUpper(f))
        flags = QLocalePrivate::CapitalEorX;

    switch (f) {
        case 'f':
        case 'F': {
            form = QLocalePrivate::DFDecimal;
            break;
        }
        case 'e':
        case 'E': {
            form = QLocalePrivate::DFExponent;
            break;
        }
        case 'g':
        case 'G': {
            form = QLocalePrivate::DFSignificantDigits;
            break;
        }
        default: {
#if defined(QT_CHECK_RANGE)
            qWarning("QByteArray::number: Invalid format char '%c'", f);
#endif
            break;
        }
    }

    QLocale locale(QLocale::C);
    return locale.d()->doubleToString(n, prec, form, -1, flags).toLatin1();
}

/*!
    Constructs a QByteArray that uses the first \a size bytes of the
    \a data array. The bytes are \e not copied. The QByteArray will
    contain the \a data pointer. The caller guarantees that \a data
    will not be deleted or modified as long as this QByteArray and any
    copies of it exist that have not been modified. In other words,
    because QByteArray is an \l{implicitly shared} class and the
    instance returned by this function contains the \a data pointer,
    the caller must not delete \a data or modify it directly as long
    as the returned QByteArray and any copies exist. However,
    QByteArray does not take ownership of \a data, so the QByteArray
    destructor will never delete the raw \a data, even when the
    last QByteArray referring to \a data is destroyed.

    A subsequent attempt to modify the contents of the returned
    QByteArray or any copy made from it will cause it to create a deep
    copy of the \a data array before doing the modification. This
    ensures that the raw \a data array itself will never be modified
    by QByteArray.

    Here is an example of how to read data using a QDataStream on raw
    data in memory without copying the raw data into a QByteArray:

    \snippet doc/src/snippets/code/src_corelib_tools_qbytearray.cpp 43

    \warning A byte array created with fromRawData() is \e not
    null-terminated, unless the raw data contains a 0 character at
    position \a size. While that does not matter for QDataStream or
    functions like indexOf(), passing the byte array to a function
    accepting a \c{const char *} expected to be '\\0'-terminated will
    fail.

    \sa setRawData(), data(), constData()
*/

QByteArray QByteArray::fromRawData(const char *data, int size)
{
    if (!data || !size) {
        return QByteArray();
    }
    Data *x = static_cast<Data *>(::malloc(sizeof(Data)));
    Q_CHECK_PTR(x);
    x->data = const_cast<char *>(data);
    x->ref = 1;
    x->alloc = x->size = size;
    *x->array = '\0';
    return QByteArray(x, 0, 0);
}

/*!
    \since 4.7

    Resets the QByteArray to use the first \a size bytes of the
    \a data array. The bytes are \e not copied. The QByteArray will
    contain the \a data pointer. The caller guarantees that \a data
    will not be deleted or modified as long as this QByteArray and any
    copies of it exist that have not been modified.

    This function can be used instead of fromRawData() to re-use
    existings QByteArray objects to save memory re-allocations.

    \sa fromRawData(), data(), constData()
*/
QByteArray &QByteArray::setRawData(const char *data, int size)
{
    *this = fromRawData(data, size);
    return *this;
}

/*!
    Returns a decoded copy of the Base64 array \a base64. Input is not checked
    for validity; invalid characters in the input are skipped, enabling the
    decoding process to continue with subsequent characters.

    For example:

    \snippet doc/src/snippets/code/src_corelib_tools_qbytearray.cpp 44

    The algorithm used to decode Base64-encoded data is defined in \l{RFC 2045}.

    \sa toBase64()
*/
QByteArray QByteArray::fromBase64(const QByteArray &base64)
{
    unsigned int buf = 0;
    int nbits = 0;
    QByteArray tmp((base64.size() * 3) / 4, Qt::Uninitialized);

    int offset = 0;
    for (int i = 0; i < base64.size(); ++i) {
        int ch = base64.at(i);
        int d;

        if (ch >= 'A' && ch <= 'Z')
            d = ch - 'A';
        else if (ch >= 'a' && ch <= 'z')
            d = ch - 'a' + 26;
        else if (ch >= '0' && ch <= '9')
            d = ch - '0' + 52;
        else if (ch == '+')
            d = 62;
        else if (ch == '/')
            d = 63;
        else
            d = -1;

        if (d != -1) {
            buf = (buf << 6) | d;
            nbits += 6;
            if (nbits >= 8) {
                nbits -= 8;
                tmp[offset++] = buf >> nbits;
                buf &= (1 << nbits) - 1;
            }
        }
    }

    tmp.truncate(offset);
    return tmp;
}

/*!
    Returns a decoded copy of the hex encoded array \a hexEncoded. Input is checked
    for validity; invalid characters in the input are skipped, enabling the
    decoding process to continue with subsequent characters.

    For example:

    \snippet doc/src/snippets/code/src_corelib_tools_qbytearray.cpp 45

    \sa toHex()
*/
QByteArray QByteArray::fromHex(const QByteArray &hexEncoded)
{
    QByteArray res((hexEncoded.size() + 1)/ 2, Qt::Uninitialized);
    char *result = res.d->data + res.size();

    int invalidcount = 0;
    bool oddhex = true;
    for (int i = hexEncoded.size() - 1; i >= 0; --i) {
        int ch = hexEncoded.at(i);
        int tmp;
        if (ch >= '0' && ch <= '9') {
            tmp = (ch - '0');
        } else if (ch >= 'a' && ch <= 'f') {
            tmp = (ch - 'a' + 10);
        } else if (ch >= 'A' && ch <= 'F') {
            tmp = ch - 'A' + 10;
        } else {
            invalidcount++;
            continue;
        }

        if (oddhex) {
            --result;
            *result = tmp;
            oddhex = false;
        } else {
            *result |= (tmp << 4);
            oddhex = true;
        }
    }

    if (Q_UNLIKELY(invalidcount > 0)) {
        qWarning("QByteArray::fromHex: Invalid character in hex");
        res.remove(0, invalidcount);
    }

    return res;
}

/*!
    Returns a hex encoded copy of the byte array. The hex encoding uses the numbers 0-9 and
    the letters a-f.

    \sa fromHex()
*/
QByteArray QByteArray::toHex() const
{
    QByteArray hex(d->size * 2, Qt::Uninitialized);
    char *hexdata = hex.d->data;
    const uchar *data = (const uchar *)d->data;

    static const char tohex[] = "0123456789abcdef";
    for (int i = 0; i < d->size; ++i) {
        *hexdata++ = tohex[(data[i] >> 4) & 0xf];
        *hexdata++ = tohex[data[i] & 0xf];
    }

    return hex;
}

static void q_fromPercentEncoding(QByteArray *ba, char percent)
{
    if (ba->isEmpty())
        return;

    char *data = ba->data();
    const char *inputPtr = data;

    int i = 0;
    int len = ba->count();
    int outlen = 0;
    int a, b;
    char c;
    while (i < len) {
        c = inputPtr[i];
        if (c == percent && i + 2 < len) {
            a = inputPtr[++i];
            b = inputPtr[++i];

            if (a >= '0' && a <= '9') a -= '0';
            else if (a >= 'a' && a <= 'f') a = a - 'a' + 10;
            else if (a >= 'A' && a <= 'F') a = a - 'A' + 10;

            if (b >= '0' && b <= '9') b -= '0';
            else if (b >= 'a' && b <= 'f') b  = b - 'a' + 10;
            else if (b >= 'A' && b <= 'F') b  = b - 'A' + 10;

            *data++ = (char)((a << 4) | b);
        } else {
            *data++ = c;
        }

        ++i;
        ++outlen;
    }

    if (outlen != len)
        ba->truncate(outlen);
}

void q_fromPercentEncoding(QByteArray *ba)
{
    q_fromPercentEncoding(ba, '%');
}

/*!
    \since 4.4

    Returns a decoded copy of the URI/URL-style percent-encoded \a input.
    The \a percent parameter allows you to replace the '%' character for
    another (for instance, '_' or '=').

    For example:
    \code
        QByteArray text = QByteArray::fromPercentEncoding("Qt%20is%20great%33");
        text.data();            // returns "Qt is great!"
    \endcode

    \sa toPercentEncoding(), QUrl::fromPercentEncoding()
*/
QByteArray QByteArray::fromPercentEncoding(const QByteArray &input, char percent)
{
    if (input.isEmpty()) {
        return input;
    }

    QByteArray tmp = input;
    q_fromPercentEncoding(&tmp, percent);
    return tmp;
}

static inline bool q_strchr(const char str[], char chr)
{
    if (!str) return false;

    const char *ptr = str;
    char c;
    while ((c = *ptr++))
        if (c == chr)
            return true;
    return false;
}

static inline char toHexHelper(char c)
{
    static const char hexnumbers[] = "0123456789ABCDEF";
    return hexnumbers[c & 0xf];
}

static void q_toPercentEncoding(QByteArray *ba, const char *dontEncode, const char *alsoEncode, char percent)
{
    if (ba->isEmpty())
        return;

    QByteArray input = *ba;
    int len = input.count();
    const char *inputData = input.constData();
    char *output = 0;
    int length = 0;

    for (int i = 0; i < len; ++i) {
        unsigned char c = *inputData++;
        if (((c >= 0x61 && c <= 0x7A) // ALPHA
             || (c >= 0x41 && c <= 0x5A) // ALPHA
             || (c >= 0x30 && c <= 0x39) // DIGIT
             || c == 0x2D // -
             || c == 0x2E // .
             || c == 0x5F // _
             || c == 0x7E // ~
             || q_strchr(dontEncode, c))
            && !q_strchr(alsoEncode, c)) {
            if (output)
                output[length] = c;
            ++length;
        } else {
            if (!output) {
                // detach now
                ba->resize(len*3); // worst case
                output = ba->data();
            }
            output[length++] = percent;
            output[length++] = toHexHelper((c & 0xf0) >> 4);
            output[length++] = toHexHelper(c & 0xf);
        }
    }
    if (output)
        ba->truncate(length);
}

void q_toPercentEncoding(QByteArray *ba, const char *exclude, const char *include)
{
    q_toPercentEncoding(ba, exclude, include, '%');
}

void q_normalizePercentEncoding(QByteArray *ba, const char *exclude)
{
    q_fromPercentEncoding(ba, '%');
    q_toPercentEncoding(ba, exclude, 0, '%');
}

/*!
    \since 4.4

    Returns a URI/URL-style percent-encoded copy of this byte array. The
    \a percent parameter allows you to override the default '%'
    character for another.

    By default, this function will encode all characters that are not
    one of the following:

        ALPHA ("a" to "z" and "A" to "Z") / DIGIT (0 to 9) / "-" / "." / "_" / "~"

    To prevent characters from being encoded pass them to \a
    exclude. To force characters to be encoded pass them to \a
    include. The \a percent character is always encoded.

    Example:

    \code
         QByteArray text = "{a fishy string?}";
         QByteArray ba = text.toPercentEncoding("{}", "s");
         qDebug(ba.constData());
         // prints "{a fi%73hy %73tring%3F}"
    \endcode

    The hex encoding uses the numbers 0-9 and the uppercase letters A-F.

    \sa fromPercentEncoding(), QUrl::toPercentEncoding()
*/
QByteArray QByteArray::toPercentEncoding(const QByteArray &exclude, const QByteArray &include,
                                         char percent) const
{
    if (isEmpty()) {
        return QByteArray();
    }

    QByteArray include2 = include;
    if (percent != '%')                        // the default
        if ((percent >= 0x61 && percent <= 0x7A) // ALPHA
            || (percent >= 0x41 && percent <= 0x5A) // ALPHA
            || (percent >= 0x30 && percent <= 0x39) // DIGIT
            || percent == 0x2D // -
            || percent == 0x2E // .
            || percent == 0x5F // _
            || percent == 0x7E) // ~
        include2 += percent;

    QByteArray result = *this;
    q_toPercentEncoding(&result, exclude.nulTerminated().constData(), include2.nulTerminated().constData(), percent);

    return result;
}

/*! \typedef QByteArray::ConstIterator
    \internal
*/

/*! \typedef QByteArray::Iterator
    \internal
*/

/*! \typedef QByteArray::const_iterator
    \internal
*/

/*! \typedef QByteArray::iterator
    \internal
*/

/*! \typedef QByteArray::const_reference
    \internal
*/

/*! \typedef QByteArray::reference
    \internal
*/

/*! \typedef QByteArray::value_type
  \internal
 */

/*!
    \fn QByteArray::QByteArray(int size)

    Use QByteArray(int, char) instead.
*/


/*!
    \fn QByteArray QByteArray::leftJustify(uint width, char fill, bool truncate) const

    Use leftJustified() instead.
*/

/*!
    \fn QByteArray QByteArray::rightJustify(uint width, char fill, bool truncate) const

    Use rightJustified() instead.
*/

/*!
    \fn QByteArray& QByteArray::duplicate(const QByteArray& a)

    \oldcode
        QByteArray bdata;
        bdata.duplicate(original);
    \newcode
        QByteArray bdata;
        bdata = original;
    \endcode

    \note QByteArray uses implicit sharing so if you modify a copy, only the
    copy is changed.
*/

/*!
    \fn QByteArray& QByteArray::duplicate(const char *a, uint n)

    \overload

    \oldcode
        QByteArray bdata;
        bdata.duplicate(ptr, size);
    \newcode
        QByteArray bdata;
        bdata = QByteArray(ptr, size);
    \endcode

    \note QByteArray uses implicit sharing so if you modify a copy, only the
    copy is changed.
*/

/*!
    \fn void QByteArray::resetRawData(const char *data, uint n)

    Use clear() instead.
*/

/*!
    \fn QByteArray QByteArray::lower() const

    Use toLower() instead.
*/

/*!
    \fn QByteArray QByteArray::upper() const

    Use toUpper() instead.
*/

/*!
    \fn QByteArray QByteArray::stripWhiteSpace() const

    Use trimmed() instead.
*/

/*!
    \fn QByteArray QByteArray::simplifyWhiteSpace() const

    Use simplified() instead.
*/

/*!
    \fn int QByteArray::find(char c, int from = 0) const

    Use indexOf() instead.
*/

/*!
    \fn int QByteArray::find(const char *c, int from = 0) const

    Use indexOf() instead.
*/

/*!
    \fn int QByteArray::find(const QByteArray &ba, int from = 0) const

    Use indexOf() instead.
*/

/*!
    \fn int QByteArray::findRev(char c, int from = -1) const

    Use lastIndexOf() instead.
*/

/*!
    \fn int QByteArray::findRev(const char *c, int from = -1) const

    Use lastIndexOf() instead.
*/

/*!
    \fn int QByteArray::findRev(const QByteArray &ba, int from = -1) const

    Use lastIndexOf() instead.
*/

/*!
    \fn int QByteArray::find(const QString &s, int from = 0) const

    Use indexOf() instead.
*/

/*!
    \fn int QByteArray::findRev(const QString &s, int from = -1) const

    Use lastIndexOf() instead.
*/

QT_END_NAMESPACE
