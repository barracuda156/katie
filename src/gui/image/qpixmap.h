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

#ifndef QPIXMAP_H
#define QPIXMAP_H

#include <QtGui/qpaintdevice.h>
#include <QtGui/qcolor.h>
#include <QtCore/qnamespace.h>
#include <QtCore/qstring.h> // char*->QString conversion
#include <QtCore/qsharedpointer.h>
#include <QtGui/qimage.h>
#include <QtGui/qtransform.h>



QT_BEGIN_NAMESPACE


class QImageWriter;
class QImageReader;
class QColor;
class QVariant;
class QPixmapData;

class Q_GUI_EXPORT QPixmap : public QPaintDevice
{
public:
    QPixmap();
    explicit QPixmap(QPixmapData *data);
    QPixmap(int w, int h);
    QPixmap(const QSize &);
    QPixmap(const QString& fileName, const char *format = nullptr, Qt::ImageConversionFlags flags = Qt::AutoColor);
    QPixmap(const QPixmap &);
    ~QPixmap();

    QPixmap &operator=(const QPixmap &);
    inline QPixmap &operator=(QPixmap &&other)
    { qSwap(data, other.data); return *this; }
    inline void swap(QPixmap &other) { qSwap(data, other.data); }

    bool operator==(const QPixmap &) const;
    bool operator!=(const QPixmap &) const;
    operator QVariant() const;

    bool isNull() const;
    int devType() const;

    int width() const;
    int height() const;
    QSize size() const;
    QRect rect() const;
    int depth() const;

    void fill(const QColor &fillColor = Qt::white);
    void fill(const QWidget *widget, const QPoint &ofs);
    inline void fill(const QWidget *widget, int xofs, int yofs) { fill(widget, QPoint(xofs, yofs)); }

    QBitmap mask() const;
    void setMask(const QBitmap &);

    QPixmap alphaChannel() const;
    void setAlphaChannel(const QPixmap &);

    bool hasAlpha() const;
    bool hasAlphaChannel() const;

#ifndef QT_NO_IMAGE_HEURISTIC_MASK
    QBitmap createHeuristicMask(bool clipTight = true) const;
#endif
    QBitmap createMaskFromColor(const QColor &maskColor,
        Qt::MaskMode mode = Qt::MaskInColor) const;

    static QPixmap grabWindow(WId, int x=0, int y=0, int w=-1, int h=-1);
    static QPixmap grabWidget(QWidget *widget, const QRect &rect);
    static inline QPixmap grabWidget(QWidget *widget, int x=0, int y=0, int w=-1, int h=-1)
    { return grabWidget(widget, QRect(x, y, w, h)); }


    inline QPixmap scaled(int w, int h, Qt::AspectRatioMode aspectMode = Qt::IgnoreAspectRatio,
                          Qt::TransformationMode mode = Qt::FastTransformation) const
        { return scaled(QSize(w, h), aspectMode, mode); }
    QPixmap scaled(const QSize &s, Qt::AspectRatioMode aspectMode = Qt::IgnoreAspectRatio,
                   Qt::TransformationMode mode = Qt::FastTransformation) const;
    QPixmap scaledToWidth(int w, Qt::TransformationMode mode = Qt::FastTransformation) const;
    QPixmap scaledToHeight(int h, Qt::TransformationMode mode = Qt::FastTransformation) const;
    QPixmap transformed(const QMatrix &, Qt::TransformationMode mode = Qt::FastTransformation) const;
    static QMatrix trueMatrix(const QMatrix &m, int w, int h);
    QPixmap transformed(const QTransform &, Qt::TransformationMode mode = Qt::FastTransformation) const;
    static QTransform trueMatrix(const QTransform &m, int w, int h);

    QImage toImage() const;
    static QPixmap fromImage(const QImage &image, Qt::ImageConversionFlags flags = Qt::AutoColor);
    static QPixmap fromImageReader(QImageReader *imageReader, Qt::ImageConversionFlags flags = Qt::AutoColor);

    bool load(const QString& fileName, const char *format = nullptr, Qt::ImageConversionFlags flags = Qt::AutoColor);
    bool loadFromData(const uchar *buf, uint len, const char* format = nullptr, Qt::ImageConversionFlags flags = Qt::AutoColor);
    inline bool loadFromData(const QByteArray &data, const char* format = nullptr, Qt::ImageConversionFlags flags = Qt::AutoColor);
    bool save(const QString& fileName, const char* format = nullptr, int quality = -1) const;
    bool save(QIODevice* device, const char* format = nullptr, int quality = -1) const;

    inline QPixmap copy(int x, int y, int width, int height) const;
    QPixmap copy(const QRect &rect = QRect()) const;

    inline void scroll(int dx, int dy, int x, int y, int width, int height, QRegion *exposed = nullptr);
    void scroll(int dx, int dy, const QRect &rect, QRegion *exposed = nullptr);

    qint64 cacheKey() const;

    bool isQBitmap() const;

#if defined(Q_WS_X11)
    static QPixmap fromX11Pixmap(Qt::HANDLE pixmap);
    Qt::HANDLE toX11Pixmap() const;
#endif

    QPaintEngine *paintEngine() const;

protected:
    int metric(PaintDeviceMetric) const;

private:
    QExplicitlySharedDataPointer<QPixmapData> data;

    bool doImageIO(QImageWriter *io, int quality) const;

    QPixmap(const QSize &s, int type);
    void init(int, int, int);
    void detach();
    QPixmapData* pixmapData() const;

    friend class QPixmapData;
    friend class QBitmap;
    friend class QPaintDevice;
    friend class QPainter;
    friend class QRasterPaintEngine;
    friend class QX11PaintEngine;
    friend class QWidgetPrivate;
    friend class QRasterBuffer;

#if !defined(QT_NO_DATASTREAM)
    friend Q_GUI_EXPORT QDataStream &operator>>(QDataStream &, QPixmap &);
#endif
};

Q_DECLARE_TYPEINFO(QPixmap, Q_MOVABLE_TYPE);

inline QPixmap QPixmap::copy(int ax, int ay, int awidth, int aheight) const
{
    return copy(QRect(ax, ay, awidth, aheight));
}

inline void QPixmap::scroll(int dx, int dy, int ax, int ay, int awidth, int aheight, QRegion *exposed)
{
    scroll(dx, dy, QRect(ax, ay, awidth, aheight), exposed);
}

inline bool QPixmap::loadFromData(const QByteArray &buf, const char *format,
                                  Qt::ImageConversionFlags flags)
{
    return loadFromData(reinterpret_cast<const uchar *>(buf.constData()), buf.size(), format, flags);
}

/*****************************************************************************
 QPixmap stream functions
*****************************************************************************/

#if !defined(QT_NO_DATASTREAM)
Q_GUI_EXPORT QDataStream &operator<<(QDataStream &, const QPixmap &);
Q_GUI_EXPORT QDataStream &operator>>(QDataStream &, QPixmap &);
#endif

/*****************************************************************************
 QPixmap (and QImage) helper functions
*****************************************************************************/

QT_END_NAMESPACE


#endif // QPIXMAP_H
