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
    \class QMovie

    \brief The QMovie class is a convenience class for playing movies
    with QImageReader.

    \ingroup painting

    This class is used to show simple animations without sound. If you want
    to display video and media content, use other multimedia framework.

    First, create a QMovie object by passing either the name of a file or a
    pointer to a QIODevice containing an animated image format to QMovie's
    constructor. You can call isValid() to check if the image data is valid,
    before starting the movie. To start the movie, call start(). QMovie will
    enter \l Running state, and emit started() and stateChanged(). To get the
    current state of the movie, call state().

    To display the movie in your application, you can pass your QMovie object
    to QLabel::setMovie(). Example:

    \snippet doc/src/snippets/code/src_gui_image_qmovie.cpp 0

    Whenever the frame changes, QMovie will emit frameChanged(). currentImage()
    can be used to get a copy of the current frame. When the movie is done,
    QMovie emits finished(). If any error occurs during playback (i.e, the image
    file is corrupt), QMovie will emit error(). When QMovie state changes, QMovie
    will emit stateChanged(). To stop the movie, call stop().

    Certain animation formats allow you to set the background color. You can
    call setBackgroundColor() to set the color, or backgroundColor() to
    retrieve the current background color.

    currentFrameNumber() returns the sequence number of the current frame. The
    first frame in the animation has the sequence number 0. frameCount()
    returns the total number of frames in the animation, if the image format
    supports this. You can call loopCount() to get the number of times the
    movie should loop before finishing. nextFrameDelay() returns the number of
    milliseconds the current frame should be displayed.

    QMovie can be instructed to cache frames of an animation by calling
    setCacheMode().

    Call supportedFormats() for a list of formats that QMovie supports.

    \sa QLabel, QImageReader, {Movie Example}
*/

/*! \enum QMovie::MovieState

    This enum describes the different states of QMovie.

    \value NotRunning The movie is not running. This is QMovie's initial
    state, and the state it enters after stop() has been called or the movie
    is finished.

    \value Running The movie is running.
*/

/*! \fn void QMovie::started()

    This signal is emitted after QMovie::start() has been called, and QMovie
    has entered QMovie::Running state.
*/

/*! \fn void QMovie::frameChanged(int frameNumber)
    \since 4.1

    This signal is emitted when the frame number has changed to
    \a frameNumber.  You can call currentImage() or currentPixmap() to get a
    copy of the frame.
*/

/*! 
    \fn void QMovie::stateChanged(QMovie::MovieState state)

    This signal is emitted every time the state of the movie changes. The new
    state is specified by \a state.

    \sa QMovie::state()
*/

/*! \fn void QMovie::error(QImageReader::ImageReaderError error)

    This signal is emitted by QMovie when the error \a error occurred during
    playback.  QMovie will stop the movie, and enter QMovie::NotRunning state.
*/

/*! \fn void QMovie::finished()

    This signal is emitted when the movie has finished.

    \sa QMovie::stop()
*/

#include "qglobal.h"

#ifndef QT_NO_MOVIE

#include "qmovie.h"
#include "qimage.h"
#include "qimagereader.h"
#include "qtimer.h"
#include "qcolor.h"
#include "qlist.h"
#include "qbuffer.h"
#include "qobject_p.h"
#include "qdebug.h"

#define QMOVIE_INVALID_DELAY -1

QT_BEGIN_NAMESPACE

class QMoviePrivate : public QObjectPrivate
{
    Q_DECLARE_PUBLIC(QMovie)

public:
    QMoviePrivate(QMovie *qq);

    void begin();
    void done();
    void readerError(const QImageReader::ImageReaderError error);
    void _q_loadNextFrame();

    int loopCount;
    QImageReader *reader;
    QMovie::MovieState movieState;
    QImage currentImage;
    QTimer nextImageTimer;
};

QMoviePrivate::QMoviePrivate(QMovie *qq)
    : loopCount(-1),
    reader(nullptr),
    movieState(QMovie::NotRunning)
{
    q_ptr = qq;
}

void QMoviePrivate::begin()
{
    Q_Q(QMovie);
    loopCount = reader->loopCount();
    _q_loadNextFrame();
    nextImageTimer.start();
    movieState = QMovie::Running;
    emit q->started();
    emit q->stateChanged(QMovie::Running);
}

void QMoviePrivate::done()
{
    Q_Q(QMovie);
    nextImageTimer.stop();
    movieState = QMovie::NotRunning;
    emit q->finished();
    emit q->stateChanged(QMovie::NotRunning);
}

void QMoviePrivate::readerError(const QImageReader::ImageReaderError error)
{
    Q_Q(QMovie);
    done();
    emit q->error(reader->error());
}

void QMoviePrivate::_q_loadNextFrame()
{
    Q_Q(QMovie);
    if (!reader->read(&currentImage)) {
        currentImage = QImage();
        readerError(reader->error());
        return;
    }
    const int currentframe = reader->currentImageNumber();
    emit q->frameChanged(currentframe);
    nextImageTimer.setInterval(reader->nextImageDelay());
    if (currentframe >= reader->imageCount()) {
        loopCount++;
        if (loopCount == 0 || loopCount < reader->loopCount()) {
            // infinite loop or not done looping
            if (!reader->jumpToImage(0)) {
                readerError(reader->error());
            }
        } else {
            done();
        }
    }
}

/*!
    Constructs a QMovie object, passing the \a parent object to QObject's
    constructor.

    \sa setFileName(), setDevice(), setFormat()
 */
QMovie::QMovie(QObject *parent)
    : QObject(*new QMoviePrivate(this), parent)
{
    Q_D(QMovie);
    d->reader = new QImageReader();
    connect(&d->nextImageTimer, SIGNAL(timeout()), this, SLOT(_q_loadNextFrame()));
}

/*!
    Constructs a QMovie object. QMovie will use read image data from \a
    device, which it assumes is open and readable. If \a format is not empty,
    QMovie will use the image format \a format for decoding the image
    data. Otherwise, QMovie will attempt to guess the format.

    The \a parent object is passed to QObject's constructor.
 */
QMovie::QMovie(QIODevice *device, const QByteArray &format, QObject *parent)
    : QObject(*new QMoviePrivate(this), parent)
{
    Q_D(QMovie);
    d->reader = new QImageReader(device, format);
    connect(&d->nextImageTimer, SIGNAL(timeout()), this, SLOT(_q_loadNextFrame()));
}

/*!
    Constructs a QMovie object. QMovie will use read image data from \a
    fileName. If \a format is not empty, QMovie will use the image format \a
    format for decoding the image data. Otherwise, QMovie will attempt to
    guess the format.

    The \a parent object is passed to QObject's constructor.
 */
QMovie::QMovie(const QString &fileName, const QByteArray &format, QObject *parent)
    : QObject(*new QMoviePrivate(this), parent)
{
    Q_D(QMovie);
    d->reader = new QImageReader(fileName, format);
    connect(&d->nextImageTimer, SIGNAL(timeout()), this, SLOT(_q_loadNextFrame()));
}

/*!
    Destructs the QMovie object.
*/
QMovie::~QMovie()
{
    Q_D(QMovie);
    delete d->reader;
}

/*!
    Sets the current device to \a device. QMovie will read image data from
    this device when the movie is running.

    \sa device(), setFormat()
*/
void QMovie::setDevice(QIODevice *device)
{
    Q_D(QMovie);
    stop();
    d->reader->setDevice(device);
}

/*!
    Returns the device QMovie reads image data from. If no device has
    currently been assigned, 0 is returned.

    \sa setDevice(), fileName()
*/
QIODevice *QMovie::device() const
{
    Q_D(const QMovie);
    return d->reader->device();
}

/*!
    Sets the name of the file that QMovie reads image data from, to \a
    fileName.

    \sa fileName(), setDevice(), setFormat()
*/
void QMovie::setFileName(const QString &fileName)
{
    Q_D(QMovie);
    stop();
    d->reader->setFileName(fileName);
}

/*!
    Returns the name of the file that QMovie reads image data from. If no file
    name has been assigned, or if the assigned device is not a file, an empty
    QString is returned.

    \sa setFileName(), device()
*/
QString QMovie::fileName() const
{
    Q_D(const QMovie);
    return d->reader->fileName();
}

/*!
    Sets the format that QMovie will use when decoding image data, to \a
    format. By default, QMovie will attempt to guess the format of the image
    data.

    You can call supportedFormats() for the full list of formats
    QMovie supports.

    \sa QImageReader::supportedImageFormats()
*/
void QMovie::setFormat(const QByteArray &format)
{
    Q_D(QMovie);
    stop();
    d->reader->setFormat(format);
}

/*!
    Returns the format that QMovie uses when decoding image data. If no format
    has been assigned, an empty QByteArray() is returned.

    \sa setFormat()
*/
QByteArray QMovie::format() const
{
    Q_D(const QMovie);
    return d->reader->format();
}

/*!
    For image formats that support it, this function sets the background color
    to \a color.

    \sa backgroundColor()
*/
void QMovie::setBackgroundColor(const QColor &color)
{
    Q_D(QMovie);
    d->reader->setBackgroundColor(color);
}

/*!
    Returns the background color of the movie. If no background color has been
    assigned, an invalid QColor is returned.

    \sa setBackgroundColor()
*/
QColor QMovie::backgroundColor() const
{
    Q_D(const QMovie);
    return d->reader->backgroundColor();
}

/*!
    Returns the current state of QMovie.

    \sa MovieState, stateChanged()
*/
QMovie::MovieState QMovie::state() const
{
    Q_D(const QMovie);
    return d->movieState;
}

/*!
    Returns the current frame as a QImage.
*/
QImage QMovie::currentImage() const
{
    Q_D(const QMovie);
    return d->currentImage;
}

/*!
    Returns true if the movie is valid (e.g., the image data is readable and
    the image format is supported); otherwise returns false.
*/
bool QMovie::isValid() const
{
    Q_D(const QMovie);
    return d->reader->canRead();
}

/*!
    Returns the number of frames in the movie.

    Certain animation formats do not support this feature, in which
    case 0 is returned.
*/
int QMovie::frameCount() const
{
    Q_D(const QMovie);
    return d->reader->imageCount();
}

/*!
    Returns the number of milliseconds QMovie will wait before updating the
    next frame in the animation.
*/
int QMovie::nextFrameDelay() const
{
    Q_D(const QMovie);
    return d->reader->nextImageDelay();
}

/*!
    Returns the sequence number of the current frame. The number of the first
    frame in the movie is 0.
*/
int QMovie::currentFrameNumber() const
{
    Q_D(const QMovie);
    return d->reader->currentImageNumber();
}

/*!
    Jumps to the next frame. Returns true on success; otherwise returns false.
*/
bool QMovie::jumpToNextFrame()
{
    Q_D(QMovie);
    return d->reader->jumpToNextImage();
}

/*!
    Returns the number of times the movie will loop before it finishes.
    If the movie will only play once (no looping), loopCount returns 0.
    If the movie loops forever, loopCount returns -1.

    Note that, if the image data comes from a sequential device (e.g. a
    socket), QMovie can only loop the movie if the cacheMode is set to
    QMovie::CacheAll.
*/
int QMovie::loopCount() const
{
    Q_D(const QMovie);
    return d->reader->loopCount();
}

/*!
    Starts the movie. QMovie will enter \l Running state, and start emitting
    updated() and resized() as the movie progresses.

    If QMovie is already in the \l Running state, this function does nothing.

    \sa stop()
*/
void QMovie::start()
{
    Q_D(QMovie);
    if (d->movieState == QMovie::Running) {
        return;
    }
    d->begin();
}

/*!
    Stops the movie. QMovie enters \l NotRunning state, and stops emitting
    updated() and resized(). If start() is called again, the movie will
    restart from the beginning.

    If QMovie is already in the \l NotRunning state, this function
    does nothing.

    \sa start()
*/
void QMovie::stop()
{
    Q_D(QMovie);
    if (d->movieState == QMovie::NotRunning) {
        return;
    }
    d->done();
}

/*!
    \since 4.1

    Returns the scaled size of frames.

    \sa QImageReader::scaledSize()
*/
QSize QMovie::scaledSize() const
{
    Q_D(const QMovie);
    return d->reader->scaledSize();
}

/*!
    \since 4.1

    Sets the scaled frame size to \a size.

    \sa QImageReader::setScaledSize()
*/
void QMovie::setScaledSize(const QSize &size)
{
    Q_D(QMovie);
    d->reader->setScaledSize(size);
}

/*!
    \since 4.1

    Returns the list of image formats supported by QMovie.

    \sa QImageReader::supportedImageFormats()
*/
QList<QByteArray> QMovie::supportedFormats()
{
    QList<QByteArray> list = QImageReader::supportedImageFormats();
    QMutableListIterator<QByteArray> it(list);
    QBuffer buffer;
    buffer.open(QIODevice::ReadOnly);
    while (it.hasNext()) {
        QImageReader reader(&buffer, it.next());
        if (!reader.supportsAnimation())
            it.remove();
    }
    return list;
}

QT_END_NAMESPACE


#include "moc_qmovie.h"

#endif // QT_NO_MOVIE
