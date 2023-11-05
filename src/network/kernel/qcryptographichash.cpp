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
#include "qcryptographichash.h"
#include "qiodevice.h"
#include "qcorecommon_p.h"

#include "xxhash.h"

QT_BEGIN_NAMESPACE

static const int xxh3len = sizeof(XXH128_hash_t);

class QKatHash
{
public:
    QKatHash();
    ~QKatHash();

    void reset();
    void update(const char *data, const int length);
    QByteArray result() const;

private:
    Q_DISABLE_COPY(QKatHash);

    XXH3_state_t* m_xxh3;
    XXH3_state_t* m_xxh32;
};

QKatHash::QKatHash()
    : m_xxh3(XXH3_createState()),
    m_xxh32(XXH3_createState())
{
}

QKatHash::~QKatHash()
{
    XXH3_freeState(m_xxh3);
    XXH3_freeState(m_xxh32);
}

void QKatHash::reset()
{
    XXH3_128bits_reset(m_xxh3);
    XXH3_128bits_reset(m_xxh32);
}

void QKatHash::update(const char *data, const int length)
{
    if (Q_UNLIKELY(length == 1)) {
        XXH3_128bits_update(m_xxh3, data, length);
        XXH3_128bits_update(m_xxh32, "K", 1);
    } else if (Q_LIKELY(length > 1)) {
        const size_t halflength = (length / 2);
        XXH3_128bits_update(m_xxh3, data, halflength);
        XXH3_128bits_update(m_xxh32, data + halflength, halflength + (length % 2));
    }
}

QByteArray QKatHash::result() const
{
    QByteArray result(xxh3len * 2, char(0));
    char* resultdata = result.data();
    XXH128_canonicalFromHash(
        reinterpret_cast<XXH128_canonical_t*>(resultdata),
        XXH3_128bits_digest(m_xxh3)
    );
    XXH128_canonicalFromHash(
        reinterpret_cast<XXH128_canonical_t*>(resultdata + xxh3len),
        XXH3_128bits_digest(m_xxh32)
    );
    return result;
}

class QCryptographicHashPrivate
{
public:
    QCryptographicHashPrivate();

    QKatHash katContext;
    bool rehash;
};

QCryptographicHashPrivate::QCryptographicHashPrivate()
    : rehash(false)
{
}

/*!
    \class QCryptographicHash
    \inmodule QtCore

    \brief The QCryptographicHash class provides a way to generate
    cryptographic hashes.

    \since 4.3

    \ingroup tools
    \reentrant

    QCryptographicHash can be used to generate cryptographic hashes of binary
    or text data.

    Currently a custom algorithm is used.

    \warning The custom algorithm will not produce same result from the static
    and the incremental methods. Use either to compute hash sums. Do not feed
    QCryptographicHash with chunks of data that have different sizes either -
    the chunks size must be fixed, i.e. QT_BUFFSIZE. In other words, if you
    cannot tell what the optimal buffer size or when to use it then use a
    generic algorithm.
*/

/*!
    Constructs an object that can be used to create a cryptographic hash from
    data using \a method.
*/
QCryptographicHash::QCryptographicHash()
    : d(new QCryptographicHashPrivate())
{
    reset();
}

/*!
    Destroys the object.
*/
QCryptographicHash::~QCryptographicHash()
{
    delete d;
}

/*!
    Resets the object.
*/
void QCryptographicHash::reset()
{
    d->rehash = false;
    d->katContext.reset();
}

/*!
    Adds the first \a length chars of \a data to the cryptographic hash.
*/
void QCryptographicHash::addData(const char *data, int length)
{
    d->rehash = true;
    d->katContext.update(data, length);
}

/*!
    Reads the data from the open QIODevice \a device until it ends and hashes
    it. Returns \c true if reading was successful.

    \since 4.9
 */
bool QCryptographicHash::addData(QIODevice* device)
{
    if (!device || !device->isReadable() || !device->isOpen()) {
        return false;
    }

    QSTACKARRAY(char, buffer, QT_BUFFSIZE);
    int length;
    while ((length = device->read(buffer, QT_BUFFSIZE)) > 0) {
        addData(buffer, length);
    }

    return device->atEnd();
}


/*!
    Returns the final hash value.

    \sa QByteArray::toHex()
*/
QByteArray QCryptographicHash::result() const
{
    if (Q_UNLIKELY(!d->rehash)) {
        qWarning("QCryptographicHash::result called without any data");
        return QByteArray();
    }
    return d->katContext.result();
}

/*!
    Returns the hash of \a data.
*/
QByteArray QCryptographicHash::hash(const QByteArray &data)
{
    QKatHash kathash;
    kathash.reset();
    kathash.update(data.constData(), data.length());
    return kathash.result();
}

QT_END_NAMESPACE
