/****************************************************************************
**
** Copyright (C) 2015 The Qt Company Ltd.
** Copyright (C) 2016-2019 Ivailo Monev
**
** This file is part of the QtCore module of the Katie Toolkit.
**
** $QT_BEGIN_LICENSE:LGPL$
** GNU Lesser General Public License Usage
** Alternatively, this file may be used under the terms of the GNU Lesser
** General Public License version 2.1 or version 3 as published by the Free
** Software Foundation and appearing in the file LICENSE.LGPLv21 and
** LICENSE.LGPLv3 included in the packaging of this file. Please review the
** following information to ensure the GNU Lesser General Public License
** requirements will be met: https://www.gnu.org/licenses/lgpl.html and
** http://www.gnu.org/licenses/old-licenses/lgpl-2.1.html.
**
** As a special exception, The Qt Company gives you certain additional
** rights. These rights are described in The Qt Company LGPL Exception
** version 1.1, included in the file LGPL_EXCEPTION.txt in this package.
**
** GNU General Public License Usage
** Alternatively, this file may be used under the terms of the GNU
** General Public License version 3.0 as published by the Free Software
** Foundation and appearing in the file LICENSE.GPL included in the
** packaging of this file.  Please review the following information to
** ensure the GNU General Public License version 3.0 requirements will be
** met: http://www.gnu.org/copyleft/gpl.html.
**
** $QT_END_LICENSE$
**
****************************************************************************/

#include "qplatformdefs.h"
#include "qurl.h"
#include "qdataurl_p.h"

QT_BEGIN_NAMESPACE

/*!
    \internal

    Decode a data: URL into its mimetype and payload. Returns a null string if
    the URL could not be decoded.
*/
Q_CORE_EXPORT QPair<QString, QByteArray> qDecodeDataUrl(const QUrl &uri)
{
    QString mimeType;
    QByteArray payload;

    if (uri.scheme() == QLatin1String("data") && uri.host().isEmpty()) {
        mimeType = QLatin1String("text/plain;charset=US-ASCII");

        // the following would have been the correct thing, but
        // reality often differs from the specification. People have
        // data: URIs with ? and #
        //QByteArray data = QByteArray::fromPercentEncoding(uri.encodedPath());
        QByteArray data = QByteArray::fromPercentEncoding(uri.toEncoded());

        // remove the data: scheme
        data.remove(0, 5);

        // parse it:
        int pos = data.indexOf(',');
        if (pos != -1) {
            payload = data.mid(pos + 1);
            data.truncate(pos);
            data = data.trimmed();

            // find out if the payload is encoded in Base64
            if (data.endsWith(";base64")) {
                payload = QByteArray::fromBase64(payload);
                data.chop(7);
            }

            if (data.toLower().startsWith("charset")) {
                int i = 7;      // strlen("charset")
                while (data.at(i) == ' ')
                    ++i;
                if (data.at(i) == '=')
                    data.prepend("text/plain;");
            }

            if (!data.isEmpty())
                mimeType = QString::fromLatin1(data.trimmed().constData());

        }
    }

    return QPair<QString,QByteArray>(mimeType,payload);
}

QT_END_NAMESPACE
