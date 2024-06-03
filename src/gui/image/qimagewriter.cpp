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

/*!
    \class QImageWriter
    \brief The QImageWriter class provides a format independent interface
    for writing images to files or other devices.

    \reentrant
    \ingroup painting
    \ingroup io

    QImageWriter supports setting format specific options, such as the
    compression level and quality, prior to storing the image. If you
    do not need such options, you can use QImage::save() or
    QPixmap::save() instead.

    To store an image, you start by constructing a QImageWriter
    object.  Pass either a file name or a device pointer, and the
    image format to QImageWriter's constructor. You can then set
    several options, such as the quality (by calling setQuality()).
    canWrite() returns true if QImageWriter can write the image (i.e.,
    the image format is supported and the device is open for writing).
    Call write() to write the image to the device.

    If any error occurs when writing the image, write() will return
    false. You can then call error() to find the type of error that
    occurred, or errorString() to get a human readable description of
    what went wrong.

    Call supportedImageFormats() for a list of formats that
    QImageWriter can write. QImageWriter supports all built-in image
    formats, in addition to any image format plugins that support
    writing.

    \sa QImageReader, QImageIOHandler, QImageIOPlugin
*/

/*!
    \enum QImageWriter::ImageWriterError

    This enum describes errors that can occur when writing images with
    QImageWriter.

    \value DeviceError QImageWriter encountered a device error when
    writing the image data. Consult your device for more details on
    what went wrong.

    \value UnsupportedFormatError Qt does not support the requested
    image format.

    \value UnknownError An unknown error occurred. If you get this
    value after calling write(), it is most likely caused by a bug in
    QImageWriter.
*/

#include "qimagewriter.h"
#include "qbytearray.h"
#include "qfile.h"
#include "qimageiohandler.h"
#include "qset.h"
#include "qvariant.h"
#include "qfileinfo.h"
#include "qapplication.h"
#include "qcoreapplication.h"
#include "qguicommon_p.h"

// image handlers
#include "qkathandler_p.h"
#include "qpnghandler_p.h"

QT_BEGIN_NAMESPACE

static QImageIOHandler *createWriteHandlerHelper(QIODevice *device, const QByteArray &format)
{
    QByteArray form = format.toLower();
    QImageIOHandler *handler = nullptr;

    // check if any built-in handlers can write the image
    if (form == "png") {
        handler = new QPngHandler();
#ifndef QT_NO_IMAGEFORMAT_KAT
    } else if (form == "kat") {
        handler = new QKatHandler();
#endif
    }

#ifndef QT_NO_LIBRARY
    // check if any plugins can write the image
    if (!handler) {
        QFactoryLoader *l = imageloader();
        foreach (const QString &key, l->keys()) {
            QImageIOPlugin *plugin = qobject_cast<QImageIOPlugin *>(l->instance(key));
            if (plugin && (plugin->capabilities(device, form) & QImageIOPlugin::CanWrite)) {
                handler = plugin->create(device, key.toLatin1());
                break;
            }
        }
    }
#endif // QT_NO_LIBRARY

    if (!handler) {
        return nullptr;
    }

    handler->setDevice(device);
    return handler;
}

class QImageWriterPrivate
{
public:
    QImageWriterPrivate();
    ~QImageWriterPrivate();

    // device
    QByteArray format;
    QIODevice *device;
    bool deleteDevice;
    QImageIOHandler *handler;

    // image options
    int quality;
    int compression;

    // error
    QImageWriter::ImageWriterError imageWriterError;
};

/*!
    \internal
*/
QImageWriterPrivate::QImageWriterPrivate()
    : device(nullptr),
    deleteDevice(false),
    handler(nullptr),
    quality(-1),
    compression(1),
    imageWriterError(QImageWriter::UnknownError)
{
}

QImageWriterPrivate::~QImageWriterPrivate()
{
    if (deleteDevice)
        delete device;
    delete handler;
}

/*!
    Constructs an empty QImageWriter object. Before writing, you must
    call setFormat() to set an image format, then setDevice() or
    setFileName().
*/
QImageWriter::QImageWriter()
    : d(new QImageWriterPrivate())
{
}

/*!
    Constructs a QImageWriter object using the device \a device and
    image format \a format.
*/
QImageWriter::QImageWriter(QIODevice *device, const QByteArray &format)
    : d(new QImageWriterPrivate())
{
    d->device = device;
    d->format = format;
}

/*!
    Constructs a QImageWriter objects that will write to a file with
    the name \a fileName, using the image format \a format. If \a
    format is not provided, QImageWriter will detect the image format
    by inspecting the extension of \a fileName.
*/
QImageWriter::QImageWriter(const QString &fileName, const QByteArray &format)
    : d(new QImageWriterPrivate())
{
    QFile *file = new QFile(fileName);
    d->device = file;
    d->deleteDevice = true;
    d->format = format;
    if (d->format.isEmpty()) {
        const QFileInfo formatinfo(fileName);
        d->format = formatinfo.suffix().toLocal8Bit();
    }
}

/*!
    Destructs the QImageWriter object.
*/
QImageWriter::~QImageWriter()
{
    delete d;
}

/*!
    Sets the format QImageWriter will use when writing images, to \a
    format. \a format is a case insensitive text string. Example:

    \snippet doc/src/snippets/code/src_gui_image_qimagewriter.cpp 0

    You can call supportedImageFormats() for the full list of formats
    QImageWriter supports and defaultImageFormat() for the prefered
    format.

    \sa format()
*/
void QImageWriter::setFormat(const QByteArray &format)
{
    d->format = format;
}

/*!
    Returns the format QImageWriter uses for writing images.

    \sa setFormat()
*/
QByteArray QImageWriter::format() const
{
    return d->format;
}

/*!
    Sets QImageWriter's device to \a device. If a device has already
    been set, the old device is removed from QImageWriter and is
    otherwise left unchanged.

    If the device is not already open, QImageWriter will attempt to
    open the device in \l QIODevice::WriteOnly mode by calling
    open(). Note that this does not work for certain devices, such as
    QProcess, QTcpSocket and QUdpSocket, where more logic is required
    to open the device.

    \sa device(), setFileName()
*/
void QImageWriter::setDevice(QIODevice *device)
{
    if (d->device && d->deleteDevice)
        delete d->device;

    d->device = device;
    d->deleteDevice = false;
    delete d->handler;
    d->handler = 0;
}

/*!
    Returns the device currently assigned to QImageWriter, or 0 if no
    device has been assigned.
*/
QIODevice *QImageWriter::device() const
{
    return d->device;
}

/*!
    Sets the file name of QImageWriter to \a fileName. Internally,
    QImageWriter will create a QFile and open it in \l
    QIODevice::WriteOnly mode, and use this file when writing images.

    \sa fileName(), setDevice()
*/
void QImageWriter::setFileName(const QString &fileName)
{
    setDevice(new QFile(fileName));
    d->deleteDevice = true;
}

/*!
    If the currently assigned device is a QFile, or if setFileName()
    has been called, this function returns the name of the file
    QImageWriter writes to. Otherwise (i.e., if no device has been
    assigned or the device is not a QFile), an empty QString is
    returned.

    \sa setFileName(), setDevice()
*/
QString QImageWriter::fileName() const
{
    QFile *file = qobject_cast<QFile *>(d->device);
    return file ? file->fileName() : QString();
}

/*!
    This is an image format specific function that sets the quality
    level of the image to \a quality. For image formats that do not
    support setting the quality, this value is ignored.

    The value range of \a quality should be between 0 and 100.

    \sa quality()
*/
void QImageWriter::setQuality(int quality)
{
    if (Q_UNLIKELY(quality < 0 || quality > 100)) {
        qWarning("QImageWriter::setQuality: invalid quality value %d", quality);
        quality = 100;
    } else {
        d->quality = quality;
    }
}

/*!
    Returns the quality level of the image. Default value is -1.

    \sa setQuality()
*/
int QImageWriter::quality() const
{
    return d->quality;
}

/*!
    This is an image format specific function that set the compression
    of an image. For image formats that do not support setting the
    compression, this value is ignored.

    The value range of \a compression depends on the image format.

    \sa compression()
*/
void QImageWriter::setCompression(int compression)
{
    d->compression = compression;
}

/*!
    Returns the compression of the image. Default value is 1.

    \sa setCompression()
*/
int QImageWriter::compression() const
{
    return d->compression;
}

/*!
    Returns true if QImageWriter can write the image; i.e., the image
    format is supported and the assigned device is open for reading.

    \sa write(), setDevice(), setFormat()
*/
bool QImageWriter::canWrite() const
{
    if (d->device && !d->handler && (d->handler = createWriteHandlerHelper(d->device, d->format)) == 0) {
        d->imageWriterError = QImageWriter::UnsupportedFormatError;
        return false;
    }
    if (d->device && !d->device->isOpen())
        d->device->open(QIODevice::WriteOnly);
    if (!d->device || !d->device->isWritable()) {
        d->imageWriterError = QImageWriter::DeviceError;
        return false;
    }
    return true;
}

/*!
    Writes the image \a image to the assigned device or file
    name. Returns true on success; otherwise returns false. If the
    operation fails, you can call error() to find the type of error
    that occurred, or errorString() to get a human readable
    description of the error.

    \sa canWrite(), error(), errorString()
*/
bool QImageWriter::write(const QImage &image)
{
    if (!canWrite())
        return false;

    if (d->handler->supportsOption(QImageIOHandler::Quality))
        d->handler->setOption(QImageIOHandler::Quality, d->quality);
    if (d->handler->supportsOption(QImageIOHandler::CompressionLevel))
        d->handler->setOption(QImageIOHandler::CompressionLevel, d->compression);

    if (!d->handler->write(image))
        return false;
    if (QFile *file = qobject_cast<QFile *>(d->device))
        file->flush();
    return true;
}

/*!
    Returns the type of error that last occurred.

    \sa ImageWriterError, errorString()
*/
QImageWriter::ImageWriterError QImageWriter::error() const
{
    return d->imageWriterError;
}

/*!
    Returns a human readable description of the last error that occurred.

    \sa error()
*/
QString QImageWriter::errorString() const
{
    switch (d->imageWriterError) {
        case QImageWriter::UnknownError: {
            return QApplication::translate("QImageWriter", "Unknown error");
        }
        case QImageWriter::DeviceError: {
            return QApplication::translate("QImageWriter", "Device not writable");
        }
        case QImageWriter::UnsupportedFormatError: {
            return QApplication::translate("QImageWriter", "Unsupported image format");
        }
    }
    Q_UNREACHABLE();
    return QString();
}

/*!
    \since 4.2

    Returns true if the writer supports \a option; otherwise returns
    false.

    Different image formats support different options. Call this function to
    determine whether a certain option is supported by the current format. For
    example, the PNG format allows you to embed text into the image's metadata
    (see text()).

    \snippet doc/src/snippets/code/src_gui_image_qimagewriter.cpp 2

    Options can be tested after the writer has been associated with a format.

    \sa QImageReader::supportsOption(), setFormat()
*/
bool QImageWriter::supportsOption(QImageIOHandler::ImageOption option) const
{
    if (!d->handler && (d->handler = createWriteHandlerHelper(d->device, d->format)) == 0) {
        d->imageWriterError = QImageWriter::UnsupportedFormatError;
        return false;
    }

    return d->handler->supportsOption(option);
}

/*!
    \since 4.12

    Returns the default image format to use with QImageWriter.

    \sa supportedImageFormats()
*/
QByteArray QImageWriter::defaultImageFormat()
{
    return QByteArray(qt_imageformat);
}

/*!
    Returns the list of image formats supported by QImageWriter.

    By default, Katie can write the following formats:

    \table
    \header \o Format \o Description
    \row    \o PNG    \o Portable Network Graphics
    \row    \o KAT    \o Katie Image
    \endtable

    Reading and writing SVG files is supported through Katie's
    \l{QtSvg Module}{SVG Module}.

    Note that the QApplication instance must be created before this function is
    called.

    \sa setFormat(), QImageReader::supportedImageFormats(), QImageIOPlugin,
    defaultImageFormat()
*/
QList<QByteArray> QImageWriter::supportedImageFormats()
{
    QList<QByteArray> formats;
    formats << "png";
#ifndef QT_NO_IMAGEFORMAT_KAT
    formats << "kat";
#endif

#ifndef QT_NO_LIBRARY
    QFactoryLoader *l = imageloader();
    foreach (const QString &key, l->keys()) {
        QImageIOPlugin *plugin = qobject_cast<QImageIOPlugin *>(l->instance(key));
        if (plugin && (plugin->capabilities(0, key.toLatin1()) & QImageIOPlugin::CanWrite) != 0)
            formats << key.toLatin1();
    }
#endif // QT_NO_LIBRARY

    qSort(formats);
    return formats;
}

/*!
    \since 4.12

    Returns the format string for image MIME specified by \a mime.

    \sa supportedMimeTypes()
*/
QByteArray QImageWriter::formatForMimeType(const QByteArray &mime)
{
    if (mime == "image/png") {
        return QByteArray("png");
    }
#ifndef QT_NO_IMAGEFORMAT_KAT
    if (mime == "image/katie") {
        return QByteArray("kat");
    }
#endif

#ifndef QT_NO_LIBRARY
    QFactoryLoader *l = imageloader();
    foreach (const QString &key, l->keys()) {
        QImageIOPlugin *plugin = qobject_cast<QImageIOPlugin *>(l->instance(key));
        if (plugin && plugin->capabilities(0, key.toLatin1()) & QImageIOPlugin::CanWrite) {
            if (plugin->mimeTypes().contains(mime)) {
                return key.toLatin1();
            }
        }
    }
#endif // QT_NO_LIBRARY

    return QByteArray();
}

/*!
    \since 4.12

    Returns the list of image MIME types supported by QImageWriter.

    \sa supportedImageFormats(), QImageReader::supportedImageFormats()
*/
QList<QByteArray> QImageWriter::supportedMimeTypes()
{
    QList<QByteArray> mimes = QList<QByteArray>()
        << "image/png"
#ifndef QT_NO_IMAGEFORMAT_KAT
        << "image/katie"
#endif
        ;

#ifndef QT_NO_LIBRARY
    QFactoryLoader *l = imageloader();
    foreach (const QString &key, l->keys()) {
        QImageIOPlugin *plugin = qobject_cast<QImageIOPlugin *>(l->instance(key));
        if (plugin && plugin->capabilities(0, key.toLatin1()) & QImageIOPlugin::CanWrite)
            foreach (const QByteArray &mime, plugin->mimeTypes()) {
                if (!mimes.contains(mime)) {
                    mimes << mime;
                }
            }
    }
#endif // QT_NO_LIBRARY

    qSort(mimes);
    return mimes;
}

QT_END_NAMESPACE
