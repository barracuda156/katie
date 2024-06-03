/****************************************************************************
**
** Copyright (C) 2021 Ivailo Monev
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

#include "qxpmhandler_p.h"

#ifndef QT_NO_XPM

#include "qimage.h"
#include "qt_x11_p.h"
#include "qcorecommon_p.h"

QT_BEGIN_NAMESPACE

static inline void qt_ximage_to_qimage(XImage *ximage, QImage &image)
{
    bool freedata = false;
    QX11Data::copyXImageToQImage(ximage, image, &freedata);
    QX11Data::destroyXImage(ximage, freedata);
}

static inline QImage::Format qt_xpm_qimage_format(const XpmAttributes *xpmattributes)
{
    for (int i = 0; i < xpmattributes->ncolors; i++) {
        if (qstricmp(xpmattributes->colorTable[i].c_color, "None") == 0) {
            return QImage::Format_ARGB32_Premultiplied;
        }
    }
    return QImage::systemFormat();
}

static inline QImage qt_xpm_to_qimage(XImage *ximage, XImage *ximagemask, const QImage::Format format)
{
    QImage qimage(ximage->width, ximage->height, format);
    qt_ximage_to_qimage(ximage, qimage);
    if (ximagemask) {
        QImage qimagemask(ximagemask->width, ximagemask->height, QImage::Format_Mono);
        qt_ximage_to_qimage(ximagemask, qimagemask);
        qimage.setAlphaChannel(qimagemask);
    }
    return qimage;
}

QXpmHandler::QXpmHandler()
{
}

bool QXpmHandler::canRead() const
{
    if (QXpmHandler::canRead(device())) {
        setFormat("xpm");
        return true;
    }
    return false;
}

bool QXpmHandler::read(QImage *image)
{
    if (!canRead()) {
        return false;
    }

    QByteArray data = device()->readAll();
    XImage *ximage = nullptr;
    XImage *ximagemask = nullptr;
    XpmAttributes xpmattributes;
    ::memset(&xpmattributes, 0, sizeof(XpmAttributes));
    const int xpmresult = XpmCreateImageFromBuffer(
        qt_x11Data->display,
        data.data(), &ximage, &ximagemask, &xpmattributes
    );
    if (xpmresult != XpmSuccess) {
        qWarning("QXpmHandler::read: %s", XpmGetErrorString(xpmresult));
        XpmFreeAttributes(&xpmattributes);
        return false;
    } else if (!ximage) {
        qWarning("QXpmHandler::read: null XImage");
        XpmFreeAttributes(&xpmattributes);
        return false;
    }
    const QImage::Format format = qt_xpm_qimage_format(&xpmattributes);
    *image = qt_xpm_to_qimage(ximage, ximagemask, format);
    XpmFreeAttributes(&xpmattributes);
    return true;
}

QByteArray QXpmHandler::name() const
{
    return "xpm";
}

bool QXpmHandler::canRead(QIODevice *device)
{
    if (Q_UNLIKELY(!device)) {
        qWarning("QXpmHandler::canRead() called with no device");
        return false;
    }

    QSTACKARRAY(char, head, 6);
    if (device->peek(head, sizeof(head)) != sizeof(head))
        return false;

    return (qstrncmp(head, "/* XPM", 6) == 0 || qstrncmp(head, "! XPM2", 6) == 0);
}

QT_END_NAMESPACE

#endif // QT_NO_XPM
