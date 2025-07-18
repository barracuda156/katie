/****************************************************************************
**
** Copyright (C) 2015 The Qt Company Ltd.
** Copyright (C) 2016 Ivailo Monev
**
** This file is part of the QtNetwork module of the Katie Toolkit.
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

// #define QABSTRACTSOCKET_DEBUG

/*!
    \class QAbstractSocket

    \brief The QAbstractSocket class provides the base functionality
    common to all socket types.

    \reentrant
    \ingroup network
    \inmodule QtNetwork

    QAbstractSocket is the base class for QTcpSocket and QUdpSocket
    and contains all common functionality of these two classes. If
    you need a socket, you have two options:

    \list
    \i  Instantiate QTcpSocket or QUdpSocket.
    \i  Create a native socket descriptor, instantiate
        QAbstractSocket, and call setSocketDescriptor() to wrap the
        native socket.
    \endlist

    TCP (Transmission Control Protocol) is a reliable,
    stream-oriented, connection-oriented transport protocol. UDP
    (User Datagram Protocol) is an unreliable, datagram-oriented,
    connectionless protocol. In practice, this means that TCP is
    better suited for continuous transmission of data, whereas the
    more lightweight UDP can be used when reliability isn't
    important.

    QAbstractSocket's API unifies most of the differences between the
    two protocols. For example, although UDP is connectionless,
    connectToHost() establishes a virtual connection for UDP sockets,
    enabling you to use QAbstractSocket in more or less the same way
    regardless of the underlying protocol. Internally,
    QAbstractSocket remembers the address and port passed to
    connectToHost(), and functions like read() and write() use these
    values.

    At any time, QAbstractSocket has a state (returned by
    state()). The initial state is UnconnectedState. After
    calling connectToHost(), the socket first enters
    HostLookupState. If the host is found, QAbstractSocket enters
    ConnectingState and emits the hostFound() signal. When the
    connection has been established, it enters ConnectedState and
    emits connected(). If an error occurs at any stage, error() is
    emitted. Whenever the state changes, stateChanged() is emitted.
    For convenience, isValid() returns true if the socket is ready for
    reading and writing, but note that the socket's state must be
    ConnectedState before reading and writing can occur.

    Read or write data by calling read() or write(), or use the
    convenience functions readLine() and readAll(). QAbstractSocket
    also inherits getChar(), putChar(), and ungetChar() from
    QIODevice, which work on single bytes. The bytesWritten() signal
    is emitted when data has been written to the socket (i.e., when
    the client has read the data). Note that Qt does not limit the
    write buffer size. You can monitor its size by listening to this
    signal.

    The readyRead() signal is emitted every time a new chunk of data
    has arrived. bytesAvailable() then returns the number of bytes
    that are available for reading. Typically, you would connect the
    readyRead() signal to a slot and read all available data there.
    If you don't read all the data at once, the remaining data will
    still be available later, and any new incoming data will be
    appended to QAbstractSocket's internal read buffer. To limit the
    size of the read buffer, call setReadBufferSize().

    To close the socket, call disconnectFromHost(). QAbstractSocket enters
    QAbstractSocket::ClosingState. After all pending data has been written to
    the socket, QAbstractSocket actually closes the socket, enters
    QAbstractSocket::ClosedState, and emits disconnected(). If you want to
    abort a connection immediately, discarding all pending data, call abort()
    instead. If the remote host closes the connection, QAbstractSocket will
    emit error(QAbstractSocket::RemoteHostClosedError), during which the socket
    state will still be ConnectedState, and then the disconnected() signal
    will be emitted.

    The port and address of the connected peer is fetched by calling
    peerPort() and peerAddress(). peerName() returns the host name of
    the peer, as passed to connectToHost(). localPort() and
    localAddress() return the port and address of the local socket.

    QAbstractSocket provides a set of functions that suspend the
    calling thread until certain signals are emitted. These functions
    can be used to implement blocking sockets:

    \list
    \o waitForConnected() blocks until a connection has been established.

    \o waitForReadyRead() blocks until new data is available for
    reading.

    \o waitForBytesWritten() blocks until one payload of data has been
    written to the socket.

    \o waitForDisconnected() blocks until the connection has closed.
    \endlist

    We show an example:

    \snippet doc/src/snippets/network/tcpwait.cpp 0

    If \l{QIODevice::}{waitForReadyRead()} returns false, the
    connection has been closed or an error has occurred.

    Programming with a blocking socket is radically different from
    programming with a non-blocking socket. A blocking socket doesn't
    require an event loop and typically leads to simpler code.
    However, in a GUI application, blocking sockets should only be
    used in non-GUI threads, to avoid freezing the user interface.
    See the \l network/fortuneclient and \l network/blockingfortuneclient
    examples for an overview of both approaches.

    \note We discourage the use of the blocking functions together
    with signals. One of the two possibilities should be used.

    QAbstractSocket can be used with QTextStream and QDataStream's
    stream operators (operator<<() and operator>>()). There is one
    issue to be aware of, though: You must make sure that enough data
    is available before attempting to read it using operator>>().

    \sa QTcpServer
*/

/*!
    \fn void QAbstractSocket::hostFound()

    This signal is emitted after connectToHost() has been called and
    the host lookup has succeeded.

    \note Since Qt 4.6.3 QAbstractSocket may emit hostFound()
    directly from the connectToHost() call since a DNS result could have been
    cached.

    \sa connected()
*/

/*!
    \fn void QAbstractSocket::connected()

    This signal is emitted after connectToHost() has been called and
    a connection has been successfully established.

    \note On some operating systems the connected() signal may
    be directly emitted from the connectToHost() call for connections
    to the localhost.

    \sa connectToHost(), disconnected()
*/

/*!
    \fn void QAbstractSocket::disconnected()

    This signal is emitted when the socket has been disconnected.
    
    \warning If you need to delete the sender() of this signal in a slot connected
    to it, use the \l{QObject::deleteLater()}{deleteLater()} function.

    \sa connectToHost(), disconnectFromHost(), abort()
*/

/*!
    \fn void QAbstractSocket::error(QAbstractSocket::SocketError socketError)

    This signal is emitted after an error occurred. The \a socketError
    parameter describes the type of error that occurred.

    \sa error(), errorString(), {Creating Custom Qt Types}
*/

/*!
    \fn void QAbstractSocket::stateChanged(QAbstractSocket::SocketState socketState)

    This signal is emitted whenever QAbstractSocket's state changes.
    The \a socketState parameter is the new state.

    \sa state(), {Creating Custom Qt Types}
*/

/*!
    \enum QAbstractSocket::NetworkLayerProtocol

    This enum describes the network layer protocol values used in Qt.

    \value IPv4Protocol IPv4
    \value IPv6Protocol IPv6
    \value UnknownNetworkLayerProtocol Other than IPv4 and IPv6

    \sa QHostAddress::protocol()
*/

/*!
    \enum QAbstractSocket::SocketType

    This enum describes the transport layer protocol.

    \value TcpSocket TCP
    \value UdpSocket UDP
    \value UnknownSocketType Other than TCP and UDP

    \sa QAbstractSocket::socketType()
*/

/*!
    \enum QAbstractSocket::SocketError

    This enum describes the socket errors that can occur.

    \value ConnectionRefusedError The connection was refused by the
           peer (or timed out).
    \value RemoteHostClosedError The remote host closed the
           connection. Note that the client socket (i.e., this socket)
           will be closed after the remote close notification has
           been sent.
    \value HostNotFoundError The host address was not found.
    \value SocketAccessError The socket operation failed because the
           application lacked the required privileges.
    \value SocketResourceError The local system ran out of resources
           (e.g., too many sockets).
    \value SocketTimeoutError The socket operation timed out.
    \value DatagramTooLargeError The datagram was larger than the
           operating system's limit (which can be as low as 8192
           bytes).
    \value NetworkError An error occurred with the network (e.g., the
           network cable was accidentally plugged out).
    \value AddressInUseError The address specified to QUdpSocket::bind() is
           already in use and was set to be exclusive.
    \value SocketAddressNotAvailableError The address specified to
           QUdpSocket::bind() does not belong to the host.
    \value UnsupportedSocketOperationError The requested socket operation is
           not supported by the local operating system (e.g., lack of
           IPv6 support).
    \value UnfinishedSocketOperationError Used by QAbstractSocketEngine only,
           The last operation attempted has not finished yet (still in progress in
            the background). (This value was introduced in 4.4.)

    \value UnknownSocketError An unidentified error occurred.
    \sa QAbstractSocket::error()
*/

/*!
    \enum QAbstractSocket::SocketState

    This enum describes the different states in which a socket can be.

    \value UnconnectedState The socket is not connected.
    \value HostLookupState The socket is performing a host name lookup.
    \value ConnectingState The socket has started establishing a connection.
    \value ConnectedState A connection is established.
    \value BoundState The socket is bound to an address and port (for servers).
    \value ClosingState The socket is about to close (data may still
    be waiting to be written).
    \value ListeningState For internal use only.
    \omitvalue Idle
    \omitvalue HostLookup
    \omitvalue Connecting
    \omitvalue Connected
    \omitvalue Closing
    \omitvalue Connection

    \sa QAbstractSocket::state()
*/

/*!
    \enum QAbstractSocket::SocketOption
    \since 4.6

    This enum represents the options that can be set on a socket.
    If desired, they can be set after having received the connected() signal from
    the socket or after having received a new socket from a QTcpServer.

    \value LowDelayOption Try to optimize the socket for low latency. For a QTcpSocket
    this would set the TCP_NODELAY option and disable Nagle's algorithm. Set this to 1
    to enable.
    \value KeepAliveOption Set this to 1 to enable the SO_KEEPALIVE socket option

    \value MulticastTtlOption Set this to an integer value to set IP_MULTICAST_TTL (TTL for multicast datagrams) socket option.

    \value MulticastLoopbackOption Set this to 1 to enable the IP_MULTICAST_LOOP (multicast loopback) socket option.

    \sa QAbstractSocket::setSocketOption(), QAbstractSocket::socketOption()
*/

#include "qabstractsocket.h"
#include "qabstractsocket_p.h"
#include "qhostinfo_p.h"
#include "qabstracteventdispatcher.h"
#include "qhostaddress.h"
#include "qhostinfo.h"
#include "qmetaobject.h"
#include "qpointer.h"
#include "qtimer.h"
#include "qelapsedtimer.h"
#include "qscopedvaluerollback.h"
#include "qdebug.h"
#include "qthread_p.h"
#include "qcore_unix_p.h"
#include "qcorecommon_p.h"
#include "qnetworkcommon_p.h"

#include <time.h>

#define Q_CHECK_SOCKETENGINE(returnValue) do { \
    if (!d->socketEngine) { \
        return returnValue; \
    } } while (0)

QT_BEGIN_NAMESPACE

/*! \internal

    Constructs a QAbstractSocketPrivate. Initializes all members.
*/
QAbstractSocketPrivate::QAbstractSocketPrivate()
    : readSocketNotifierCalled(false),
      readSocketNotifierState(false),
      readSocketNotifierStateSet(false),
      emittedReadyRead(false),
      emittedBytesWritten(false),
      abortCalled(false),
      closeCalled(false),
      pendingClose(false),
      port(0),
      localPort(0),
      peerPort(0),
      socketEngine(0),
      cachedSocketDescriptor(-1),
      readBufferMaxSize(0),
      isBuffered(false),
      connectTimer(0),
      disconnectTimer(0),
      socketType(QAbstractSocket::UnknownSocketType),
      state(QAbstractSocket::UnconnectedState),
      socketError(QAbstractSocket::UnknownSocketError)
{
    qRegisterMetaType<QAbstractSocket::SocketState>();
    qRegisterMetaType<QAbstractSocket::SocketError>();
}

/*! \internal

    Destructs the QAbstractSocket. If the socket layer is open, it
    will be reset.
*/
QAbstractSocketPrivate::~QAbstractSocketPrivate()
{
}

/*! \internal

    Resets the socket layer, clears the read and write buffers and
    deletes any socket notifiers.
*/
void QAbstractSocketPrivate::resetSocketLayer()
{
#if defined (QABSTRACTSOCKET_DEBUG)
    qDebug("QAbstractSocketPrivate::resetSocketLayer()");
#endif

    if (socketEngine) {
        socketEngine->close();
        socketEngine->disconnect();
        delete socketEngine;
        socketEngine = 0;
        cachedSocketDescriptor = -1;
    }
    if (connectTimer)
        connectTimer->stop();
    if (disconnectTimer)
        disconnectTimer->stop();
}

/*! \internal

    Initializes the socket layer to by of type \a type, using the
    network layer protocol \a protocol. Resets the socket layer first
    if it's already initialized. Sets up the socket notifiers.
*/
bool QAbstractSocketPrivate::initSocketLayer(QAbstractSocket::NetworkLayerProtocol protocol)
{
    Q_Q(QAbstractSocket);
#if defined (QABSTRACTSOCKET_DEBUG)
    QString typeStr;
    if (q->socketType() == QAbstractSocket::TcpSocket) typeStr = QLatin1String("TcpSocket");
    else if (q->socketType() == QAbstractSocket::UdpSocket) typeStr = QLatin1String("UdpSocket");
    else typeStr = QLatin1String("UnknownSocketType");
    QString protocolStr;
    if (protocol == QAbstractSocket::IPv4Protocol) protocolStr = QLatin1String("IPv4Protocol");
    else if (protocol == QAbstractSocket::IPv6Protocol) protocolStr = QLatin1String("IPv6Protocol");
    else protocolStr = QLatin1String("UnknownNetworkLayerProtocol");
#endif

    resetSocketLayer();
    socketEngine = new QAbstractSocketEngine(q);
    if (!socketEngine) {
        socketError = QAbstractSocket::UnsupportedSocketOperationError;
        q->setErrorString(QAbstractSocket::tr("Operation on socket is not supported"));
        return false;
    }
    if (!socketEngine->initialize(q->socketType(), protocol)) {
#if defined (QABSTRACTSOCKET_DEBUG)
        qDebug("QAbstractSocketPrivate::initSocketLayer(%s, %s) failed (%s)",
               typeStr.toLatin1().constData(), protocolStr.toLatin1().constData(),
               socketEngine->errorString().toLatin1().constData());
#endif
        socketError = socketEngine->error();
        q->setErrorString(socketEngine->errorString());
        return false;
    }

    if (threadData->eventDispatcher)
        socketEngine->setReceiver(this);

#if defined (QABSTRACTSOCKET_DEBUG)
    qDebug("QAbstractSocketPrivate::initSocketLayer(%s, %s) success",
           typeStr.toLatin1().constData(), protocolStr.toLatin1().constData());
#endif
    return true;
}

/*! \internal

    Slot connected to the read socket notifier. This slot is called
    when new data is available for reading, or when the socket has
    been closed. Handles recursive calls.
*/
bool QAbstractSocketPrivate::canReadNotification()
{
    Q_Q(QAbstractSocket);
#if defined (QABSTRACTSOCKET_DEBUG)
    qDebug("QAbstractSocketPrivate::canReadNotification()");
#endif

    // Prevent recursive calls
    if (readSocketNotifierCalled) {
        if (!readSocketNotifierStateSet) {
            readSocketNotifierStateSet = true;
            readSocketNotifierState = socketEngine->isReadNotificationEnabled();
            socketEngine->setReadNotificationEnabled(false);
        }
    }
    QScopedValueRollback<bool> rsncrollback(readSocketNotifierCalled);
    readSocketNotifierCalled = true;

    if (!isBuffered)
        socketEngine->setReadNotificationEnabled(false);

    // If buffered, read data from the socket into the read buffer
    qint64 newBytes = 0;
    if (isBuffered) {
        // Return if there is no space in the buffer
        if (readBufferMaxSize && readBuffer.size() >= readBufferMaxSize) {
#if defined (QABSTRACTSOCKET_DEBUG)
            qDebug("QAbstractSocketPrivate::canReadNotification() buffer is full");
#endif
            return false;
        }

        // If reading from the socket fails after getting a read
        // notification, close the socket.
        newBytes = readBuffer.size();
        if (!readFromSocket()) {
#if defined (QABSTRACTSOCKET_DEBUG)
            qDebug("QAbstractSocketPrivate::canReadNotification() disconnecting socket");
#endif
            q->disconnectFromHost();
            return false;
        }
        newBytes = readBuffer.size() - newBytes;

        // If read buffer is full, disable the read socket notifier.
        if (readBufferMaxSize && readBuffer.size() == readBufferMaxSize) {
            socketEngine->setReadNotificationEnabled(false);
        }
    }

    // only emit readyRead() when not recursing, and only if there is data available
    bool hasData = newBytes > 0
#ifndef QT_NO_UDPSOCKET
        || (!isBuffered && socketType != QAbstractSocket::TcpSocket && socketEngine && socketEngine->hasPendingDatagrams())
#endif
        || (!isBuffered && socketType == QAbstractSocket::TcpSocket && socketEngine)
        ;

    if (!emittedReadyRead && hasData) {
        QScopedValueRollback<bool> r(emittedReadyRead);
        emittedReadyRead = true;
        emit q->readyRead();
    }

    // If we were closed as a result of the readyRead() signal,
    // return.
    if (state == QAbstractSocket::UnconnectedState || state == QAbstractSocket::ClosingState) {
#if defined (QABSTRACTSOCKET_DEBUG)
        qDebug("QAbstractSocketPrivate::canReadNotification() socket is closing - returning");
#endif
        return true;
    }

    if (!hasData && socketEngine)
        socketEngine->setReadNotificationEnabled(true);

    // reset the read socket notifier state if we reentered inside the
    // readyRead() connected slot.
    if (readSocketNotifierStateSet && socketEngine &&
        readSocketNotifierState != socketEngine->isReadNotificationEnabled()) {
        socketEngine->setReadNotificationEnabled(readSocketNotifierState);
        readSocketNotifierStateSet = false;
    }
    return true;
}

/*! \internal

    Slot connected to the write socket notifier. It's called during a
    delayed connect or when the socket is ready for writing.
*/
bool QAbstractSocketPrivate::canWriteNotification()
{
#if defined (QABSTRACTSOCKET_DEBUG)
    qDebug("QAbstractSocketPrivate::canWriteNotification() flushing");
#endif
    int tmp = writeBuffer.size();
    flush();

    if (socketEngine && writeBuffer.isEmpty() && socketEngine->bytesToWrite() == 0)
        socketEngine->setWriteNotificationEnabled(false);

    return (writeBuffer.size() < tmp);
}

/*! \internal

    Slot connected to a notification of connection status
    change. Either we finished connecting or we failed to connect.
*/
void QAbstractSocketPrivate::connectionNotification()
{
    // If in connecting state, check if the connection has been
    // established, otherwise flush pending data.
    if (state == QAbstractSocket::ConnectingState) {
#if defined (QABSTRACTSOCKET_DEBUG)
        qDebug("QAbstractSocketPrivate::connectionNotification() testing connection");
#endif
        _q_testConnection();
    }
}

/*! \internal

    Writes pending data in the write buffers to the socket. The
    function writes as much as it can without blocking.

    It is usually invoked by canWriteNotification after one or more
    calls to write().

    Emits bytesWritten().
*/
bool QAbstractSocketPrivate::flush()
{
    Q_Q(QAbstractSocket);
    if (!socketEngine || !socketEngine->isValid() || (writeBuffer.isEmpty()
        && socketEngine->bytesToWrite() == 0)) {
#if defined (QABSTRACTSOCKET_DEBUG)
    qDebug("QAbstractSocketPrivate::flush() nothing to do: valid ? %s, writeBuffer.isEmpty() ? %s",
           socketEngine->isValid() ? "yes" : "no", writeBuffer.isEmpty() ? "yes" : "no");
#endif

        // this covers the case when the buffer was empty, but we had to wait for the socket engine to finish
        if (state == QAbstractSocket::ClosingState)
            q->disconnectFromHost();

        return false;
    }

    int nextSize = writeBuffer.nextDataBlockSize();
    const char *ptr = writeBuffer.readPointer();

    // Attempt to write it all in one chunk.
    qint64 written = socketEngine->write(ptr, nextSize);
    if (written < 0) {
        socketError = socketEngine->error();
        q->setErrorString(socketEngine->errorString());
#if defined (QABSTRACTSOCKET_DEBUG)
        qDebug() << "QAbstractSocketPrivate::flush() write error, aborting." << socketEngine->errorString();
#endif
        emit q->error(socketError);
        // an unexpected error so close the socket.
        q->abort();
        return false;
    }

#if defined (QABSTRACTSOCKET_DEBUG)
    qDebug("QAbstractSocketPrivate::flush() %lld bytes written to the network",
           written);
#endif

    // Remove what we wrote so far.
    writeBuffer.free(written);
    if (written > 0) {
        // Don't emit bytesWritten() recursively.
        if (!emittedBytesWritten) {
            QScopedValueRollback<bool> r(emittedBytesWritten);
            emittedBytesWritten = true;
            emit q->bytesWritten(written);
        }
    }

    if (writeBuffer.isEmpty() && socketEngine && socketEngine->isWriteNotificationEnabled()
        && !socketEngine->bytesToWrite())
        socketEngine->setWriteNotificationEnabled(false);
    if (state == QAbstractSocket::ClosingState)
        q->disconnectFromHost();

    return true;
}

/*! \internal

    Slot connected to QHostInfo::lookupHost() in connectToHost(). This
    function starts the process of connecting to any number of
    candidate IP addresses for the host, if it was found. Calls
    _q_connectToNextAddress().
*/
void QAbstractSocketPrivate::_q_startConnecting(const QHostInfo &hostInfo)
{
    Q_Q(QAbstractSocket);
    if (state != QAbstractSocket::HostLookupState) {
        return;
    }

    addresses = hostInfo.addresses();

#if defined(QABSTRACTSOCKET_DEBUG)
    QString s = QLatin1String("{");
    for (int i = 0; i < addresses.count(); ++i) {
        if (i != 0) s += QLatin1String(", ");
        s += addresses.at(i).toString();
    }
    s += QLatin1Char('}');
    qDebug("QAbstractSocketPrivate::_q_startConnecting(hostInfo == %s)", s.toLatin1().constData());
#endif

    // Try all addresses twice.
    addresses += addresses;

    // If there are no addresses in the host list, report this to the
    // user.
    if (addresses.isEmpty()) {
#if defined(QABSTRACTSOCKET_DEBUG)
        qDebug("QAbstractSocketPrivate::_q_startConnecting(), host not found");
#endif
        state = QAbstractSocket::UnconnectedState;
        socketError = QAbstractSocket::HostNotFoundError;
        q->setErrorString(QAbstractSocket::tr("Host not found"));
        emit q->stateChanged(state);
        emit q->error(QAbstractSocket::HostNotFoundError);
        return;
    }

    // Enter Connecting state (see also sn_write, which is called by
    // the write socket notifier after connect())
    state = QAbstractSocket::ConnectingState;
    emit q->stateChanged(state);

    // Report the successful host lookup
    emit q->hostFound();

    // The addresses returned by the lookup will be tested one after
    // another by _q_connectToNextAddress().
    _q_connectToNextAddress();
}

/*! \internal

    Called by a queued or direct connection from _q_startConnecting() or
    _q_testConnection(), this function takes the first address of the
    pending addresses list and tries to connect to it. If the
    connection succeeds, QAbstractSocket will emit
    connected(). Otherwise, error(ConnectionRefusedError) or
    error(SocketTimeoutError) is emitted.
*/
void QAbstractSocketPrivate::_q_connectToNextAddress()
{
    Q_Q(QAbstractSocket);
    do {
        // Check for more pending addresses
        if (addresses.isEmpty()) {
#if defined(QABSTRACTSOCKET_DEBUG)
            qDebug("QAbstractSocketPrivate::_q_connectToNextAddress(), all addresses failed.");
#endif
            state = QAbstractSocket::UnconnectedState;
            if (socketEngine) {
                if (socketEngine->error() == QAbstractSocket::UnknownSocketError
                    && socketEngine->state() == QAbstractSocket::ConnectingState) {
                    socketError = QAbstractSocket::ConnectionRefusedError;
                    q->setErrorString(QAbstractSocket::tr("Connection refused"));
                } else {
                    socketError = socketEngine->error();
                    q->setErrorString(socketEngine->errorString());
                }
            } else {
//                socketError = QAbstractSocket::ConnectionRefusedError;
//                q->setErrorString(QAbstractSocket::tr("Connection refused"));
            }
            emit q->stateChanged(state);
            emit q->error(socketError);
            return;
        }

        // Pick the first host address candidate
        host = addresses.takeFirst();
#if defined(QABSTRACTSOCKET_DEBUG)
        qDebug("QAbstractSocketPrivate::_q_connectToNextAddress(), connecting to %s:%i, %d left to try",
               host.toString().toLatin1().constData(), port, addresses.count());
#endif

#if defined(QT_NO_IPV6)
        if (host.protocol() == QAbstractSocket::IPv6Protocol) {
            // If we have no IPv6 support, then we will not be able to
            // connect. So we just pretend we didn't see this address.
#if defined(QABSTRACTSOCKET_DEBUG)
            qDebug("QAbstractSocketPrivate::_q_connectToNextAddress(), skipping IPv6 entry");
#endif
            continue;
        }
#endif

        if (!initSocketLayer(host.protocol())) {
            // hope that the next address is better
#if defined(QABSTRACTSOCKET_DEBUG)
            qDebug("QAbstractSocketPrivate::_q_connectToNextAddress(), failed to initialize sock layer");
#endif
            continue;
        }

        // Tries to connect to the address. If it succeeds immediately
        // (localhost address on BSD or any UDP connect), emit
        // connected() and return.
        if (socketEngine->connectToHost(host, port)) {
            //_q_testConnection();
            fetchConnectionParameters();
            return;
        }

        // cache the socket descriptor even if we're not fully connected yet
        cachedSocketDescriptor = socketEngine->socketDescriptor();

        // Check that we're in delayed connection state. If not, try
        // the next address
        if (socketEngine->state() != QAbstractSocket::ConnectingState) {
#if defined(QABSTRACTSOCKET_DEBUG)
            qDebug("QAbstractSocketPrivate::_q_connectToNextAddress(), connection failed (%s)",
                   socketEngine->errorString().toLatin1().constData());
#endif
            continue;
        }

        // Start the connect timer.
        if (threadData->eventDispatcher) {
            if (!connectTimer) {
                connectTimer = new QTimer(q);
                QObject::connect(connectTimer, SIGNAL(timeout()),
                                 q, SLOT(_q_abortConnectionAttempt()),
                                 Qt::DirectConnection);
            }
            connectTimer->start(QT_CONNECT_TIMEOUT);
        }

        // Wait for a write notification that will eventually call
        // _q_testConnection().
        socketEngine->setWriteNotificationEnabled(true);
        break;
    } while (state != QAbstractSocket::ConnectedState);
}

/*! \internal

    Tests if a connection has been established. If it has, connected()
    is emitted. Otherwise, _q_connectToNextAddress() is invoked.
*/
void QAbstractSocketPrivate::_q_testConnection()
{
    if (socketEngine) {
        if (threadData->eventDispatcher) {
            if (connectTimer)
                connectTimer->stop();
        }

        if (socketEngine->state() == QAbstractSocket::ConnectedState) {
            // Fetch the parameters if our connection is completed;
            // otherwise, fall out and try the next address.
            fetchConnectionParameters();
            if (pendingClose) {
                q_func()->disconnectFromHost();
                pendingClose = false;
            }
            return;
        }
    }

    if (threadData->eventDispatcher) {
        if (connectTimer)
            connectTimer->stop();
    }

#if defined(QABSTRACTSOCKET_DEBUG)
    qDebug("QAbstractSocketPrivate::_q_testConnection() connection failed,"
           " checking for alternative addresses");
#endif
    _q_connectToNextAddress();
}

/*! \internal

    This function is called after a certain number of seconds has
    passed while waiting for a connection. It simply tests the
    connection, and continues to the next address if the connection
    failed.
*/
void QAbstractSocketPrivate::_q_abortConnectionAttempt()
{
    Q_Q(QAbstractSocket);
#if defined(QABSTRACTSOCKET_DEBUG)
    qDebug("QAbstractSocketPrivate::_q_abortConnectionAttempt() (timed out)");
#endif
    if (socketEngine)
        socketEngine->setWriteNotificationEnabled(false);

    connectTimer->stop();

    if (addresses.isEmpty()) {
        state = QAbstractSocket::UnconnectedState;
        socketError = QAbstractSocket::SocketTimeoutError;
        q->setErrorString(QAbstractSocket::tr("Connection timed out"));
        emit q->stateChanged(state);
        emit q->error(socketError);
    } else {
        _q_connectToNextAddress();
    }
}

void QAbstractSocketPrivate::_q_forceDisconnect()
{
    Q_Q(QAbstractSocket);
    if (socketEngine && socketEngine->isValid() && state == QAbstractSocket::ClosingState) {
        socketEngine->close();
        q->disconnectFromHost();
    }
}

/*! \internal

    Reads data from the socket layer into the read buffer. Returns
    true on success; otherwise false.
*/
bool QAbstractSocketPrivate::readFromSocket()
{
    Q_Q(QAbstractSocket);
    // Find how many bytes we can read from the socket layer.
    qint64 bytesToRead = socketEngine->bytesAvailable();
    if (bytesToRead == 0) {
        // Under heavy load, certain conditions can trigger read notifications
        // for socket notifiers on which there is no activity. If we continue
        // to read 0 bytes from the socket, we will trigger behavior similar
        // to that which signals a remote close. When we hit this condition,
        // we try to read 4k of data from the socket, which will give us either
        // an EAGAIN/EWOULDBLOCK if the connection is alive (i.e., the remote
        // host has _not_ disappeared).
        bytesToRead = 4096;
    }
    if (readBufferMaxSize && bytesToRead > (readBufferMaxSize - readBuffer.size()))
        bytesToRead = readBufferMaxSize - readBuffer.size();

#if defined(QABSTRACTSOCKET_DEBUG)
    qDebug("QAbstractSocketPrivate::readFromSocket() about to read %d bytes",
           int(bytesToRead));
#endif

    // Read from the socket, store data in the read buffer.
    char *ptr = readBuffer.reserve(bytesToRead);
    qint64 readBytes = socketEngine->read(ptr, bytesToRead);
    if (readBytes == -2) {
        // No bytes currently available for reading.
        readBuffer.chop(bytesToRead);
        return true;
    }
    readBuffer.chop(int(bytesToRead - (readBytes < 0 ? qint64(0) : readBytes)));
#if defined(QABSTRACTSOCKET_DEBUG)
    qDebug("QAbstractSocketPrivate::readFromSocket() got %d bytes, buffer size = %d",
           int(readBytes), readBuffer.size());
#endif

    if (!socketEngine->isValid()) {
        socketError = socketEngine->error();
        q->setErrorString(socketEngine->errorString());
        emit q->error(socketError);
#if defined(QABSTRACTSOCKET_DEBUG)
        qDebug("QAbstractSocketPrivate::readFromSocket() read failed: %s",
               q->errorString().toLatin1().constData());
#endif
        resetSocketLayer();
        return false;
    }

    return true;
}

/*! \internal

    Sets up the internal state after the connection has succeeded.
*/
void QAbstractSocketPrivate::fetchConnectionParameters()
{
    Q_Q(QAbstractSocket);

    peerName = hostName;
    if (socketEngine) {
        socketEngine->setReadNotificationEnabled(true);
        socketEngine->setWriteNotificationEnabled(true);
        localPort = socketEngine->localPort();
        peerPort = socketEngine->peerPort();
        localAddress = socketEngine->localAddress();
        peerAddress = socketEngine->peerAddress();
        cachedSocketDescriptor = socketEngine->socketDescriptor();
    }

    state = QAbstractSocket::ConnectedState;
    emit q->stateChanged(state);
    emit q->connected();

#if defined(QABSTRACTSOCKET_DEBUG)
    qDebug("QAbstractSocketPrivate::fetchConnectionParameters() connection to %s:%i established",
           host.toString().toLatin1().constData(), port);
#endif
}

/*! \internal

    Constructs a new abstract socket of type \a socketType. The \a
    parent argument is passed to QObject's constructor.
*/
QAbstractSocket::QAbstractSocket(SocketType socketType,
                                 QAbstractSocketPrivate &dd, QObject *parent)
    : QIODevice(dd, parent)
{
    Q_D(QAbstractSocket);
#if defined(QABSTRACTSOCKET_DEBUG)
    qDebug("QAbstractSocket::QAbstractSocket(%sSocket, QAbstractSocketPrivate == %p, parent == %p)",
           socketType == TcpSocket ? "Tcp" : socketType == UdpSocket
           ? "Udp" : "Unknown", &dd, parent);
#endif
    d->socketType = socketType;
}

/*!
    Creates a new abstract socket of type \a socketType. The \a
    parent argument is passed to QObject's constructor.

    \sa socketType(), QTcpSocket, QUdpSocket
*/
QAbstractSocket::QAbstractSocket(SocketType socketType, QObject *parent)
    : QIODevice(*new QAbstractSocketPrivate, parent)
{
    Q_D(QAbstractSocket);
#if defined(QABSTRACTSOCKET_DEBUG)
    qDebug("QAbstractSocket::QAbstractSocket(%p)", parent);
#endif
    d->socketType = socketType;
}

/*!
    Destroys the socket.
*/
QAbstractSocket::~QAbstractSocket()
{
    Q_D(QAbstractSocket);
#if defined(QABSTRACTSOCKET_DEBUG)
    qDebug("QAbstractSocket::~QAbstractSocket()");
#endif
    if (d->state != UnconnectedState)
        abort();
}

/*!
    Returns true if the socket is valid and ready for use; otherwise
    returns false.

    \bold{Note:} The socket's state must be ConnectedState before reading and
    writing can occur.

    \sa state()
*/
bool QAbstractSocket::isValid() const
{
    return d_func()->socketEngine ? d_func()->socketEngine->isValid() : isOpen();
}

/*!
    Attempts to make a connection to \a hostName on the given \a port.

    The socket is opened in the given \a openMode and first enters
    HostLookupState, then performs a host name lookup of \a hostName.
    If the lookup succeeds, hostFound() is emitted and QAbstractSocket
    enters ConnectingState. It then attempts to connect to the address
    or addresses returned by the lookup. Finally, if a connection is
    established, QAbstractSocket enters ConnectedState and
    emits connected().

    At any point, the socket can emit error() to signal that an error
    occurred.

    \a hostName may be an IP address in string form (e.g.,
    "43.195.83.32"), or it may be a host name (e.g.,
    "example.com"). QAbstractSocket will do a lookup only if
    required. \a port is in native byte order.

    \sa state(), peerName(), peerAddress(), peerPort(), waitForConnected()
*/
void QAbstractSocket::connectToHost(const QString &hostName, quint16 port,
                                    OpenMode openMode)
{
    Q_D(QAbstractSocket);
#if defined(QABSTRACTSOCKET_DEBUG)
    qDebug("QAbstractSocket::connectToHost(\"%s\", %i, %i)...", qPrintable(hostName), port,
           (int) openMode);
#endif

    if (Q_UNLIKELY(d->state == ConnectedState || d->state == ConnectingState
        || d->state == ClosingState || d->state == HostLookupState)) {
        qWarning("QAbstractSocket::connectToHost() called when already looking up or connecting/connected to \"%s\"", qPrintable(hostName));
        return;
    }

    d->hostName = hostName;
    d->port = port;
    d->state = UnconnectedState;
    d->readBuffer.clear();
    d->writeBuffer.clear();
    d->abortCalled = false;
    d->closeCalled = false;
    d->pendingClose = false;
    d->localPort = 0;
    d->peerPort = 0;
    d->localAddress.clear();
    d->peerAddress.clear();
    d->peerName = hostName;

    if (openMode & QIODevice::Unbuffered)
        d->isBuffered = false; // Unbuffered QTcpSocket
    else if (!d_func()->isBuffered)
        openMode |= QAbstractSocket::Unbuffered; // QUdpSocket

    QIODevice::open(openMode);
    d->state = HostLookupState;
    emit stateChanged(d->state);

    QHostAddress temp;
    if (temp.setAddress(hostName.toLatin1())) {
        QHostInfo info;
        info.d->err = QHostInfo::NoError;
        info.d->errorStr = QCoreApplication::translate("QHostInfo", "Unknown error");
        info.d->addrs.append(temp);
        d->_q_startConnecting(info);
    } else {
        if (d->threadData->eventDispatcher) {
            d->_q_startConnecting(QHostInfo::fromName(hostName));
        }
    }

#if defined(QABSTRACTSOCKET_DEBUG)
    qDebug("QAbstractSocket::connectToHost(\"%s\", %i) == %s%s", hostName.toLatin1().constData(), port,
           (d->state == ConnectedState) ? "true" : "false",
           (d->state == ConnectingState || d->state == HostLookupState)
           ? " (connection in progress)" : "");
#endif
}

/*! \overload

    Attempts to make a connection to \a address on port \a port.
*/
void QAbstractSocket::connectToHost(const QHostAddress &address, quint16 port,
                                    OpenMode openMode)
{
#if defined(QABSTRACTSOCKET_DEBUG)
    qDebug("QAbstractSocket::connectToHost([%s], %i, %i)...",
           address.toString().toLatin1().constData(), port, (int) openMode);
#endif
    connectToHost(address.toString(), port, openMode);
}

/*!
    Returns the number of bytes that are waiting to be written. The
    bytes are written when control goes back to the event loop or
    when flush() is called.

    \sa bytesAvailable(), flush()
*/
qint64 QAbstractSocket::bytesToWrite() const
{
    Q_D(const QAbstractSocket);
#if defined(QABSTRACTSOCKET_DEBUG)
    qDebug("QAbstractSocket::bytesToWrite() == %i", d->writeBuffer.size());
#endif
    return (qint64)d->writeBuffer.size();
}

/*!
    Returns the number of incoming bytes that are waiting to be read.

    \sa bytesToWrite(), read()
*/
qint64 QAbstractSocket::bytesAvailable() const
{
    Q_D(const QAbstractSocket);
    qint64 available = QIODevice::bytesAvailable();

    available += (qint64) d->readBuffer.size();

    if (!d->isBuffered && d->socketEngine && d->socketEngine->isValid())
        available += d->socketEngine->bytesAvailable();

#if defined(QABSTRACTSOCKET_DEBUG)
    qDebug("QAbstractSocket::bytesAvailable() == %llu", available);
#endif
    return available;
}

/*!
    Returns the host port number (in native byte order) of the local
    socket if available; otherwise returns 0.

    \sa localAddress(), peerPort(), setLocalPort()
*/
quint16 QAbstractSocket::localPort() const
{
    Q_D(const QAbstractSocket);
    return d->localPort;
}

/*!
    Returns the host address of the local socket if available;
    otherwise returns QHostAddress::Null.

    This is normally the main IP address of the host, but can be
    QHostAddress::LocalHost (127.0.0.1) for connections to the
    local host.

    \sa localPort(), peerAddress(), setLocalAddress()
*/
QHostAddress QAbstractSocket::localAddress() const
{
    Q_D(const QAbstractSocket);
    return d->localAddress;
}

/*!
    Returns the port of the connected peer if the socket is in
    ConnectedState; otherwise returns 0.

    \sa peerAddress(), localPort(), setPeerPort()
*/
quint16 QAbstractSocket::peerPort() const
{
    Q_D(const QAbstractSocket);
    return d->peerPort;
}

/*!
    Returns the address of the connected peer if the socket is in
    ConnectedState; otherwise returns QHostAddress::Null.

    \sa peerName(), peerPort(), localAddress(), setPeerAddress()
*/
QHostAddress QAbstractSocket::peerAddress() const
{
    Q_D(const QAbstractSocket);
    return d->peerAddress;
}

/*!
    Returns the name of the peer as specified by connectToHost(), or
    an empty QString if connectToHost() has not been called.

    \sa peerAddress(), peerPort(), setPeerName()
*/
QString QAbstractSocket::peerName() const
{
    Q_D(const QAbstractSocket);
    return d->peerName.isEmpty() ? d->hostName : d->peerName;
}

/*!
    Returns true if a line of data can be read from the socket;
    otherwise returns false.

    \sa readLine()
*/
bool QAbstractSocket::canReadLine() const
{
    bool hasLine = d_func()->readBuffer.canReadLine();
#if defined (QABSTRACTSOCKET_DEBUG)
    qDebug("QAbstractSocket::canReadLine() == %s, buffer size = %d, size = %lld", hasLine ? "true" : "false",
           d_func()->readBuffer.size(), d_func()->buffer.size());
#endif
    return hasLine || QIODevice::canReadLine();
}

/*!
    Returns the native socket descriptor of the QAbstractSocket object
    if this is available; otherwise returns -1.

    The socket descriptor is not available when QAbstractSocket is in
    UnconnectedState.

    \sa setSocketDescriptor()
*/
int QAbstractSocket::socketDescriptor() const
{
    Q_D(const QAbstractSocket);
    return d->cachedSocketDescriptor;
}

/*!
    Initializes QAbstractSocket with the native socket descriptor \a
    socketDescriptor. Returns true if \a socketDescriptor is accepted
    as a valid socket descriptor; otherwise returns false.
    The socket is opened in the mode specified by \a openMode, and
    enters the socket state specified by \a socketState.

    \bold{Note:} It is not possible to initialize two abstract sockets
    with the same native socket descriptor.

    \sa socketDescriptor()
*/
bool QAbstractSocket::setSocketDescriptor(int socketDescriptor, SocketState socketState,
                                          OpenMode openMode)
{
    Q_D(QAbstractSocket);
    d->resetSocketLayer();
    d->socketEngine = new QAbstractSocketEngine(this);
    bool result = d->socketEngine->initialize(socketDescriptor, socketState);
    if (!result) {
        d->socketError = d->socketEngine->error();
        setErrorString(d->socketEngine->errorString());
        return false;
    }

    if (d->threadData->eventDispatcher)
        d->socketEngine->setReceiver(d);

    QIODevice::open(openMode);

    if (d->state != socketState) {
        d->state = socketState;
        emit stateChanged(d->state);
    }

    d->pendingClose = false;
    d->socketEngine->setReadNotificationEnabled(true);
    d->localPort = d->socketEngine->localPort();
    d->peerPort = d->socketEngine->peerPort();
    d->localAddress = d->socketEngine->localAddress();
    d->peerAddress = d->socketEngine->peerAddress();
    d->cachedSocketDescriptor = socketDescriptor;

    return true;
}

/*!
    \since 4.6
    Sets the given \a option to the value described by \a value.

    \sa socketOption()
*/
void QAbstractSocket::setSocketOption(QAbstractSocket::SocketOption option, const int value)
{
    if (!d_func()->socketEngine)
        return;

    switch (option) {
        case LowDelayOption:
            d_func()->socketEngine->setOption(QAbstractSocketEngine::LowDelayOption, value);
            break;
        case KeepAliveOption:
            d_func()->socketEngine->setOption(QAbstractSocketEngine::KeepAliveOption, value);
            break;
        case MulticastTtlOption:
            d_func()->socketEngine->setOption(QAbstractSocketEngine::MulticastTtlOption, value);
            break;
        case MulticastLoopbackOption:
            d_func()->socketEngine->setOption(QAbstractSocketEngine::MulticastLoopbackOption, value);
            break;
    }
}

/*!
    \since 4.6
    Returns the value of the \a option option.

    \sa setSocketOption()
*/
int QAbstractSocket::socketOption(QAbstractSocket::SocketOption option)
{
    int ret = -1;
    if (!d_func()->socketEngine)
        return ret;

    switch (option) {
        case LowDelayOption:
            ret = d_func()->socketEngine->option(QAbstractSocketEngine::LowDelayOption);
            break;
        case KeepAliveOption:
            ret = d_func()->socketEngine->option(QAbstractSocketEngine::KeepAliveOption);
            break;
        case MulticastTtlOption:
            ret = d_func()->socketEngine->option(QAbstractSocketEngine::MulticastTtlOption);
            break;
        case MulticastLoopbackOption:
            ret = d_func()->socketEngine->option(QAbstractSocketEngine::MulticastLoopbackOption);
            break;
    }
    return ret;
}

/*!
    Waits until the socket is connected, up to \a msecs
    milliseconds. If the connection has been established, this
    function returns true; otherwise it returns false. In the case
    where it returns false, you can call error() to determine
    the cause of the error.

    The following example waits up to one second for a connection
    to be established:

    \snippet doc/src/snippets/code/src_network_socket_qabstractsocket.cpp 0

    If msecs is -1, this function will not time out.

    \note This function may wait slightly longer than \a msecs,
    depending on the time it takes to complete the host lookup.

    \note Multiple calls to this functions do not accumulate the time.
    If the function times out, the connecting process will be aborted.

    \sa connectToHost(), connected()
*/
bool QAbstractSocket::waitForConnected(int msecs)
{
    Q_D(QAbstractSocket);
#if defined (QABSTRACTSOCKET_DEBUG)
    qDebug("QAbstractSocket::waitForConnected(%i)", msecs);
#endif

    if (state() == ConnectedState) {
#if defined (QABSTRACTSOCKET_DEBUG)
        qDebug("QAbstractSocket::waitForConnected(%i) already connected", msecs);
#endif
        return true;
    }

    bool wasPendingClose = d->pendingClose;
    d->pendingClose = false;
    QElapsedTimer stopWatch;
    stopWatch.start();

    if (d->state == HostLookupState) {
#if defined (QABSTRACTSOCKET_DEBUG)
        qDebug("QAbstractSocket::waitForConnected(%i) doing host name lookup", msecs);
#endif
        d->_q_startConnecting(QHostInfo::fromName(d->hostName));
    }
    if (state() == UnconnectedState)
        return false; // connect not in progress anymore!

    bool timedOut = true;
#if defined (QABSTRACTSOCKET_DEBUG)
    int attempt = 1;
#endif
    while (state() == ConnectingState && (msecs == -1 || stopWatch.elapsed() < msecs)) {
        int timeout = qt_timeout_value(msecs, stopWatch.elapsed());
        if (msecs != -1 && timeout > QT_CONNECT_TIMEOUT)
            timeout = QT_CONNECT_TIMEOUT;
#if defined (QABSTRACTSOCKET_DEBUG)
        qDebug("QAbstractSocket::waitForConnected(%i) waiting %.2f secs for connection attempt #%i",
               msecs, timeout / 1000.0, attempt++);
#endif
        timedOut = false;

        if (d->socketEngine && d->socketEngine->waitForWrite(timeout, &timedOut) && !timedOut) {
            d->_q_testConnection();
        } else {
            d->_q_connectToNextAddress();
        }
    }

    if ((timedOut && state() != ConnectedState) || state() == ConnectingState) {
        d->socketError = SocketTimeoutError;
        d->state = UnconnectedState;
        emit stateChanged(d->state);
        d->resetSocketLayer();
        setErrorString(tr("Socket operation timed out"));
    }

#if defined (QABSTRACTSOCKET_DEBUG)
    qDebug("QAbstractSocket::waitForConnected(%i) == %s", msecs,
           state() == ConnectedState ? "true" : "false");
#endif
    if (state() != ConnectedState)
        return false;
    if (wasPendingClose)
        disconnectFromHost();
    return true;
}

/*!
    This function blocks until new data is available for reading and the
    \l{QIODevice::}{readyRead()} signal has been emitted. The function
    will timeout after \a msecs milliseconds; the default timeout is
    30000 milliseconds.

    The function returns true if the readyRead() signal is emitted and
    there is new data available for reading; otherwise it returns false
    (if an error occurred or the operation timed out).

    \sa waitForBytesWritten() 
*/
bool QAbstractSocket::waitForReadyRead(int msecs)
{
    Q_D(QAbstractSocket);
#if defined (QABSTRACTSOCKET_DEBUG)
    qDebug("QAbstractSocket::waitForReadyRead(%i)", msecs);
#endif

    // require calling connectToHost() before waitForReadyRead()
    if (state() == UnconnectedState) {
        /* If all you have is a QIODevice pointer to an abstractsocket, you cannot check
           this, so you cannot avoid this warning. */
//        qWarning("QAbstractSocket::waitForReadyRead() is not allowed in UnconnectedState");
        return false;
    }

    QElapsedTimer stopWatch;
    stopWatch.start();

    // handle a socket in connecting state
    if (state() == HostLookupState || state() == ConnectingState) {
        if (!waitForConnected(msecs))
            return false;
    }

    Q_ASSERT(d->socketEngine);
    do {
        bool readyToRead = false;
        bool readyToWrite = false;
        if (!d->socketEngine->waitForReadOrWrite(&readyToRead, &readyToWrite, true,
                                                 !d->writeBuffer.isEmpty(), msecs)) {
            d->socketError = d->socketEngine->error();
            setErrorString(d->socketEngine->errorString());
#if defined (QABSTRACTSOCKET_DEBUG)
            qDebug("QAbstractSocket::waitForReadyRead(%i) failed (%i, %s)",
                   msecs, d->socketError, errorString().toLatin1().constData());
#endif
            emit error(d->socketError);
            if (d->socketError != SocketTimeoutError)
                close();
            return false;
        }

        if (readyToRead) {
            if (d->canReadNotification())
                return true;
        }

        if (readyToWrite)
            d->canWriteNotification();

        if (state() != ConnectedState)
            return false;
    } while (msecs == -1 || qt_timeout_value(msecs, stopWatch.elapsed()) > 0);
    return false;
}

/*! \reimp
 */
bool QAbstractSocket::waitForBytesWritten(int msecs)
{
    Q_D(QAbstractSocket);
#if defined (QABSTRACTSOCKET_DEBUG)
    qDebug("QAbstractSocket::waitForBytesWritten(%i)", msecs);
#endif

    // require calling connectToHost() before waitForBytesWritten()
    if (Q_UNLIKELY(state() == UnconnectedState)) {
        qWarning("QAbstractSocket::waitForBytesWritten() is not allowed in UnconnectedState");
        return false;
    }

    if (d->writeBuffer.isEmpty())
        return false;

    // handle a socket in connecting state
    if (state() == HostLookupState || state() == ConnectingState) {
        if (!waitForConnected(msecs))
            return false;
    }

    forever {
        bool readyToRead = false;
        bool readyToWrite = false;
        if (!d->socketEngine->waitForReadOrWrite(&readyToRead, &readyToWrite, true,
                                                 !d->writeBuffer.isEmpty(), msecs)) {
            d->socketError = d->socketEngine->error();
            setErrorString(d->socketEngine->errorString());
#if defined (QABSTRACTSOCKET_DEBUG)
            qDebug("QAbstractSocket::waitForBytesWritten(%i) failed (%i, %s)",
                   msecs, d->socketError, errorString().toLatin1().constData());
#endif
            emit error(d->socketError);
            if (d->socketError != SocketTimeoutError)
                close();
            return false;
        }

        if (readyToRead) {
#if defined (QABSTRACTSOCKET_DEBUG)
            qDebug("QAbstractSocket::waitForBytesWritten calls canReadNotification");
#endif
            if(!d->canReadNotification())
                return false;
        }


        if (readyToWrite) {
            if (d->canWriteNotification()) {
#if defined (QABSTRACTSOCKET_DEBUG)
                qDebug("QAbstractSocket::waitForBytesWritten returns true");
#endif
                return true;
            }
        }

        if (state() != ConnectedState)
            return false;
    }
    return false;
}

/*!
    Waits until the socket has disconnected, up to \a msecs
    milliseconds. If the connection has been disconnected, this
    function returns true; otherwise it returns false. In the case
    where it returns false, you can call error() to determine
    the cause of the error.

    The following example waits up to one second for a connection
    to be closed:

    \snippet doc/src/snippets/code/src_network_socket_qabstractsocket.cpp 1

    If msecs is -1, this function will not time out.

    \sa disconnectFromHost(), close()
*/
bool QAbstractSocket::waitForDisconnected(int msecs)
{
    Q_D(QAbstractSocket);
    // require calling connectToHost() before waitForDisconnected()
    if (Q_UNLIKELY(state() == UnconnectedState)) {
        qWarning("QAbstractSocket::waitForDisconnected() is not allowed in UnconnectedState");
        return false;
    }

    // handle a socket in connecting state
    if (state() == HostLookupState || state() == ConnectingState) {
        if (!waitForConnected(msecs))
            return false;
        if (state() == UnconnectedState)
            return true;
    }

    forever {
        bool readyToRead = false;
        bool readyToWrite = false;
        if (!d->socketEngine->waitForReadOrWrite(&readyToRead, &readyToWrite, state() == ConnectedState,
                                               !d->writeBuffer.isEmpty(), msecs)) {
            d->socketError = d->socketEngine->error();
            setErrorString(d->socketEngine->errorString());
#if defined (QABSTRACTSOCKET_DEBUG)
            qDebug("QAbstractSocket::waitForReadyRead(%i) failed (%i, %s)",
                   msecs, d->socketError, errorString().toLatin1().constData());
#endif
            emit error(d->socketError);
            if (d->socketError != SocketTimeoutError)
                close();
            return false;
        }

        if (readyToRead)
            d->canReadNotification();
        if (readyToWrite)
            d->canWriteNotification();

        if (state() == UnconnectedState)
            return true;
    }
    return false;
}

/*!
    Aborts the current connection and resets the socket. Unlike disconnectFromHost(),
    this function immediately closes the socket, discarding any pending data in the
    write buffer.

    \sa disconnectFromHost(), close()
*/
void QAbstractSocket::abort()
{
    Q_D(QAbstractSocket);
#if defined (QABSTRACTSOCKET_DEBUG)
    qDebug("QAbstractSocket::abort()");
#endif
    if (d->state == UnconnectedState)
        return;
    if (d->connectTimer) {
        d->connectTimer->stop();
        delete d->connectTimer;
        d->connectTimer = 0;
    }

    d->writeBuffer.clear();
    d->abortCalled = true;
    close();
}

/*! \reimp
*/
bool QAbstractSocket::isSequential() const
{
    return true;
}

/*! \reimp

     Returns true if no more data is currently
     available for reading; otherwise returns false.

     This function is most commonly used when reading data from the
     socket in a loop. For example:

     \snippet doc/src/snippets/code/src_network_socket_qabstractsocket.cpp 2

     \sa bytesAvailable(), readyRead()
 */
bool QAbstractSocket::atEnd() const
{
    return QIODevice::atEnd() && (!isOpen() || d_func()->readBuffer.isEmpty());
}

/*!
    This function writes as much as possible from the internal write buffer to
    the underlying network socket, without blocking. If any data was written,
    this function returns true; otherwise false is returned.

    Call this function if you need QAbstractSocket to start sending buffered
    data immediately. The number of bytes successfully written depends on the
    operating system. In most cases, you do not need to call this function,
    because QAbstractSocket will start sending data automatically once control
    goes back to the event loop. In the absence of an event loop, call
    waitForBytesWritten() instead.

    \sa write(), waitForBytesWritten()
*/
bool QAbstractSocket::flush()
{
    Q_D(QAbstractSocket);
    Q_CHECK_SOCKETENGINE(false);
    return d->flush();
}

/*! \reimp
*/
qint64 QAbstractSocket::readData(char *data, qint64 maxSize)
{
    Q_D(QAbstractSocket);

    // This is for a buffered QTcpSocket
    if (d->isBuffered && d->readBuffer.isEmpty())
        // if we're still connected, return 0 indicating there may be more data in the future
        // if we're not connected, return -1 indicating EOF
        return d->state == QAbstractSocket::ConnectedState ? qint64(0) : qint64(-1);

    // short cut for a char read if we have something in the buffer
    if (maxSize == 1 && !d->readBuffer.isEmpty()) {
        *data = d->readBuffer.getChar();
#if defined (QABSTRACTSOCKET_DEBUG)
        qDebug("QAbstractSocket::readData(%p '%c (0x%.2x)', 1) == 1 [char buffer]",
               data, isprint(int(*data)) ? *data : '?', *data);
#endif
        if (d->readBuffer.isEmpty() && d->socketEngine && d->socketEngine->isValid())
            d->socketEngine->setReadNotificationEnabled(true);
        return 1;
    }

    // Special case for an Unbuffered QTcpSocket
    // Re-filling the buffer.
    if (d->socketType == TcpSocket
            && !d->isBuffered
            && d->readBuffer.size() < maxSize
            && d->readBufferMaxSize > 0
            && maxSize < d->readBufferMaxSize
            && d->socketEngine
            && d->socketEngine->isValid()) {
        // Our buffer is empty and a read() was requested for a byte amount that is smaller
        // than the readBufferMaxSize. This means that we should fill our buffer since we want
        // such small reads come from the buffer and not always go to the costly socket engine read()
        qint64 bytesToRead = d->socketEngine->bytesAvailable();
        if (bytesToRead > 0) {
            char *ptr = d->readBuffer.reserve(bytesToRead);
            qint64 readBytes = d->socketEngine->read(ptr, bytesToRead);
            if (readBytes == -2) {
                // No bytes currently available for reading.
                d->readBuffer.chop(bytesToRead);
            } else {
                d->readBuffer.chop(int(bytesToRead - (readBytes < 0 ? qint64(0) : readBytes)));
            }
        }
   }

    // First try to satisfy the read from the buffer
    qint64 bytesToRead = qMin(qint64(d->readBuffer.size()), maxSize);
    qint64 readSoFar = 0;
    while (readSoFar < bytesToRead) {
        const char *ptr = d->readBuffer.readPointer();
        int bytesToReadFromThisBlock = qMin(int(bytesToRead - readSoFar),
                                            d->readBuffer.nextDataBlockSize());
        memcpy(data + readSoFar, ptr, bytesToReadFromThisBlock);
        readSoFar += bytesToReadFromThisBlock;
        d->readBuffer.free(bytesToReadFromThisBlock);
    }

    if (d->socketEngine && !d->socketEngine->isReadNotificationEnabled() && d->socketEngine->isValid())
        d->socketEngine->setReadNotificationEnabled(true);

    if (readSoFar > 0) {
        // we read some data from buffer.
        // Just return, readyRead will be emitted again
#if defined (QABSTRACTSOCKET_DEBUG)
        qDebug("QAbstractSocket::readData(%p '%c (0x%.2x)', %lli) == %lli [buffer]",
               data, isprint(int(*data)) ? *data : '?', *data, maxSize, readSoFar);
#endif

        if (d->readBuffer.isEmpty() && d->socketEngine)
            d->socketEngine->setReadNotificationEnabled(true);
        return readSoFar;
    }

    // This code path is for Unbuffered QTcpSocket or for connected UDP

    if (!d->isBuffered) {
        if (!d->socketEngine)
            return -1;          // no socket engine is probably EOF
        if (!d->socketEngine->isValid())
            return -1; // This is for unbuffered TCP when we already had been disconnected
        if (d->state != QAbstractSocket::ConnectedState)
            return -1; // This is for unbuffered TCP if we're not connected yet
        qint64 readBytes = d->socketEngine->read(data, maxSize);
        if (readBytes == -2) {
            // -2 from the engine means no bytes available (EAGAIN) so read more later
            return 0;
        } else if (readBytes < 0) {
            d->socketError = d->socketEngine->error();
            setErrorString(d->socketEngine->errorString());
            d->resetSocketLayer();
            d->state = QAbstractSocket::UnconnectedState;
        } else if (!d->socketEngine->isReadNotificationEnabled()) {
            // Only do this when there was no error
            d->socketEngine->setReadNotificationEnabled(true);
        }

#if defined (QABSTRACTSOCKET_DEBUG)
        qDebug("QAbstractSocket::readData(%p \"%s\", %lli) == %lld [engine]",
               data, qt_prettyDebug(data, 32, readBytes).data(), maxSize,
               readBytes);
#endif
        return readBytes;
    }


#if defined (QABSTRACTSOCKET_DEBUG)
    qDebug("QAbstractSocket::readData(%p \"%s\", %lli) == %lld [unreachable]",
           data, qt_prettyDebug(data, qMin<qint64>(32, readSoFar), readSoFar).data(),
           maxSize, readSoFar);
#endif
    return readSoFar;
}

/*! \reimp
*/
qint64 QAbstractSocket::writeData(const char *data, qint64 size)
{
    Q_D(QAbstractSocket);
    if (d->state == QAbstractSocket::UnconnectedState) {
        d->socketError = QAbstractSocket::UnknownSocketError;
        setErrorString(tr("Socket is not connected"));
        return -1;
    }

    if (!d->isBuffered && d->socketType == TcpSocket && d->writeBuffer.isEmpty()) {
        // This code is for the new Unbuffered QTcpSocket use case
        qint64 written = d->socketEngine->write(data, size);
        if (written < 0) {
            d->socketError = d->socketEngine->error();
            setErrorString(d->socketEngine->errorString());
            return written;
        } else if (written < size) {
            // Buffer what was not written yet
            d->writeBuffer.append(data + written, size - written);
            if (d->socketEngine)
                d->socketEngine->setWriteNotificationEnabled(true);
        }
        return size; // size=actually written + what has been buffered
    } else if (!d->isBuffered && d->socketType != TcpSocket) {
        // This is for a QUdpSocket that was connect()ed
        qint64 written = d->socketEngine->write(data, size);
        if (written < 0) {
            d->socketError = d->socketEngine->error();
            setErrorString(d->socketEngine->errorString());
        } else if (!d->writeBuffer.isEmpty()) {
            d->socketEngine->setWriteNotificationEnabled(true);
        }

#if defined (QABSTRACTSOCKET_DEBUG)
    qDebug("QAbstractSocket::writeData(%p \"%s\", %lli) == %lli", data,
           qt_prettyDebug(data, qMin((int)size, 32), size).data(),
           size, written);
#endif
        if (written >= 0)
            emit bytesWritten(written);
        return written;
    }

    // This is the code path for normal buffered QTcpSocket or
    // unbuffered QTcpSocket when there was already something in the
    // write buffer and therefore we could not do a direct engine write.
    // We just write to our write buffer and enable the write notifier
    // The write notifier then flush()es the buffer.

    d->writeBuffer.append(data, size);
    qint64 written = size;

    if (d->socketEngine && !d->writeBuffer.isEmpty())
        d->socketEngine->setWriteNotificationEnabled(true);

#if defined (QABSTRACTSOCKET_DEBUG)
    qDebug("QAbstractSocket::writeData(%p \"%s\", %lli) == %lli", data,
           qt_prettyDebug(data, qMin((int)size, 32), size).data(),
           size, written);
#endif
    return written;
}

/*!
    \since 4.1

    Sets the port on the local side of a connection to \a port.

    You can call this function in a subclass of QAbstractSocket to
    change the return value of the localPort() function after a
    connection has been established. This feature is commonly used by
    proxy connections for virtual connection settings.

    Note that this function does not bind the local port of the socket
    prior to a connection (e.g., QUdpSocket::bind()).

    \sa localAddress(), setLocalAddress(), setPeerPort()
*/
void QAbstractSocket::setLocalPort(quint16 port)
{
    Q_D(QAbstractSocket);
    d->localPort = port;
}

/*!
    \since 4.1

    Sets the address on the local side of a connection to
    \a address.

    You can call this function in a subclass of QAbstractSocket to
    change the return value of the localAddress() function after a
    connection has been established. This feature is commonly used by
    proxy connections for virtual connection settings.

    Note that this function does not bind the local address of the socket
    prior to a connection (e.g., QUdpSocket::bind()).

    \sa localAddress(), setLocalPort(), setPeerAddress()
*/
void QAbstractSocket::setLocalAddress(const QHostAddress &address)
{
    Q_D(QAbstractSocket);
    d->localAddress = address;
}

/*!
    \since 4.1

    Sets the port of the remote side of the connection to
    \a port.

    You can call this function in a subclass of QAbstractSocket to
    change the return value of the peerPort() function after a
    connection has been established. This feature is commonly used by
    proxy connections for virtual connection settings.

    \sa peerPort(), setPeerAddress(), setLocalPort()
*/
void QAbstractSocket::setPeerPort(quint16 port)
{
    Q_D(QAbstractSocket);
    d->peerPort = port;
}

/*!
    \since 4.1

    Sets the address of the remote side of the connection
    to \a address.

    You can call this function in a subclass of QAbstractSocket to
    change the return value of the peerAddress() function after a
    connection has been established. This feature is commonly used by
    proxy connections for virtual connection settings.

    \sa peerAddress(), setPeerPort(), setLocalAddress()
*/
void QAbstractSocket::setPeerAddress(const QHostAddress &address)
{
    Q_D(QAbstractSocket);
    d->peerAddress = address;
}

/*!
    \since 4.1

    Sets the host name of the remote peer to \a name.

    You can call this function in a subclass of QAbstractSocket to
    change the return value of the peerName() function after a
    connection has been established. This feature is commonly used by
    proxy connections for virtual connection settings.

    \sa peerName()
*/
void QAbstractSocket::setPeerName(const QString &name)
{
    Q_D(QAbstractSocket);
    d->peerName = name;
}

/*!
    Closes the I/O device for the socket, disconnects the socket's connection with the
    host, closes the socket, and resets the name, address, port number and underlying
    socket descriptor.

    See QIODevice::close() for a description of the actions that occur when an I/O
    device is closed.

    \sa abort()
*/
void QAbstractSocket::close()
{
    Q_D(QAbstractSocket);
#if defined(QABSTRACTSOCKET_DEBUG)
    qDebug("QAbstractSocket::close()");
#endif
    QIODevice::close();
    if (d->state != UnconnectedState) {
        d->closeCalled = true;
        disconnectFromHost();
    }

    d->localPort = 0;
    d->peerPort = 0;
    d->localAddress.clear();
    d->peerAddress.clear();
    d->peerName.clear();
    d->cachedSocketDescriptor = -1;
}

/*!
    Attempts to close the socket. If there is pending data waiting to
    be written, QAbstractSocket will enter ClosingState and wait
    until all data has been written. Eventually, it will enter
    UnconnectedState and emit the disconnected() signal.

    \sa connectToHost()
*/
void QAbstractSocket::disconnectFromHost()
{
    Q_D(QAbstractSocket);
#if defined(QABSTRACTSOCKET_DEBUG)
    qDebug("QAbstractSocket::disconnectFromHost()");
#endif

    if (d->state == UnconnectedState) {
#if defined(QABSTRACTSOCKET_DEBUG)
        qDebug("QAbstractSocket::disconnectFromHost() was called on an unconnected socket");
#endif
        return;
    }

    if (!d->abortCalled && (d->state == ConnectingState || d->state == HostLookupState)) {
#if defined(QABSTRACTSOCKET_DEBUG)
        qDebug("QAbstractSocket::disconnectFromHost() but we're still connecting");
#endif
        d->pendingClose = true;
        return;
    }


    // Disable and delete read notification
    if (d->socketEngine)
        d->socketEngine->setReadNotificationEnabled(false);

    if (d->abortCalled) {
#if defined(QABSTRACTSOCKET_DEBUG)
        qDebug("QAbstractSocket::disconnectFromHost() aborting immediately");
#endif
    } else {
        // Perhaps emit closing()
        if (d->state != ClosingState) {
            d->state = ClosingState;
#if defined(QABSTRACTSOCKET_DEBUG)
            qDebug("QAbstractSocket::disconnectFromHost() emits stateChanged()(ClosingState)");
#endif
            emit stateChanged(d->state);
        } else {
#if defined(QABSTRACTSOCKET_DEBUG)
            qDebug("QAbstractSocket::disconnectFromHost() return from delayed close");
#endif
        }

        // Wait for pending data to be written.
        if (d->socketEngine && d->socketEngine->isValid() && (d->writeBuffer.size() > 0
            || d->socketEngine->bytesToWrite() > 0)) {
            // hack: when we are waiting for the socket engine to write bytes (only
            // possible when using Socks5 or HTTP socket engine), then close
            // anyway after 2 seconds. This is to prevent a timeout on Mac, where we
            // sometimes just did not get the write notifier from the underlying
            // CFSocket and no progress was made.
            if (d->writeBuffer.size() == 0 && d->socketEngine->bytesToWrite() > 0) {
                if (!d->disconnectTimer) {
                    d->disconnectTimer = new QTimer(this);
                    connect(d->disconnectTimer, SIGNAL(timeout()), this,
                            SLOT(_q_forceDisconnect()), Qt::DirectConnection);
                }
                if (!d->disconnectTimer->isActive())
                    d->disconnectTimer->start(2000);
            }
            d->socketEngine->setWriteNotificationEnabled(true);

#if defined(QABSTRACTSOCKET_DEBUG)
            qDebug("QAbstractSocket::disconnectFromHost() delaying disconnect");
#endif
            return;
        } else {
#if defined(QABSTRACTSOCKET_DEBUG)
            qDebug("QAbstractSocket::disconnectFromHost() disconnecting immediately");
#endif
        }
    }

    SocketState previousState = d->state;
    d->resetSocketLayer();
    d->state = UnconnectedState;
    emit stateChanged(d->state);
    emit readChannelFinished();       // we got an EOF

    // only emit disconnected if we were connected before
    if (previousState == ConnectedState || previousState == ClosingState)
        emit disconnected();

    d->localPort = 0;
    d->peerPort = 0;
    d->localAddress.clear();
    d->peerAddress.clear();

#if defined(QABSTRACTSOCKET_DEBUG)
        qDebug("QAbstractSocket::disconnectFromHost() disconnected!");
#endif

    if (d->closeCalled) {
#if defined(QABSTRACTSOCKET_DEBUG)
        qDebug("QAbstractSocket::disconnectFromHost() closed!");
#endif
        d->readBuffer.clear();
        d->writeBuffer.clear();
        QIODevice::close();
    }
}

/*!
    Returns the size of the internal read buffer. This limits the
    amount of data that the client can receive before you call read()
    or readAll().

    A read buffer size of 0 (the default) means that the buffer has
    no size limit, ensuring that no data is lost.

    \sa setReadBufferSize(), read()
*/
qint64 QAbstractSocket::readBufferSize() const
{
    return d_func()->readBufferMaxSize;
}

/*!
    Sets the size of QAbstractSocket's internal read buffer to be \a
    size bytes.

    If the buffer size is limited to a certain size, QAbstractSocket
    won't buffer more than this size of data. Exceptionally, a buffer
    size of 0 means that the read buffer is unlimited and all
    incoming data is buffered. This is the default.

    This option is useful if you only read the data at certain points
    in time (e.g., in a real-time streaming application) or if you
    want to protect your socket against receiving too much data,
    which may eventually cause your application to run out of memory.

    Only QTcpSocket uses QAbstractSocket's internal buffer; QUdpSocket
    does not use any buffering at all, but rather relies on the
    implicit buffering provided by the operating system.
    Because of this, calling this function on QUdpSocket has no
    effect.

    \sa readBufferSize(), read()
*/
void QAbstractSocket::setReadBufferSize(qint64 size)
{
    Q_D(QAbstractSocket);
    if (d->readBufferMaxSize == size)
        return;
    d->readBufferMaxSize = size;
    if (!d->readSocketNotifierCalled && d->socketEngine) {
        // ensure that the read notification is enabled if we've now got
        // room in the read buffer
        // but only if we're not inside canReadNotification -- that will take care on its own
        if ((size == 0 || d->readBuffer.size() < size) && d->state == QAbstractSocket::ConnectedState) // Do not change the notifier unless we are connected.
            d->socketEngine->setReadNotificationEnabled(true);
    }
}

/*!
    Returns the state of the socket.

    \sa error()
*/
QAbstractSocket::SocketState QAbstractSocket::state() const
{
    return d_func()->state;
}

/*!
    Sets the state of the socket to \a state.

    \sa state()
*/
void QAbstractSocket::setSocketState(SocketState state)
{
    d_func()->state = state;
}

/*!
    Returns the socket type (TCP, UDP, or other).

    \sa QTcpSocket, QUdpSocket
*/
QAbstractSocket::SocketType QAbstractSocket::socketType() const
{
    return d_func()->socketType;
}

/*!
    Returns the type of error that last occurred.

    \sa state(), errorString()
*/
QAbstractSocket::SocketError QAbstractSocket::error() const
{
    return d_func()->socketError;
}

/*!
    Sets the type of error that last occurred to \a socketError.

    \sa setSocketState(), setErrorString()
*/
void QAbstractSocket::setSocketError(SocketError socketError)
{
    d_func()->socketError = socketError;
}

#ifndef QT_NO_DEBUG_STREAM
Q_NETWORK_EXPORT QDebug operator<<(QDebug debug, QAbstractSocket::SocketError error)
{
    switch (error) {
    case QAbstractSocket::ConnectionRefusedError:
        debug << "QAbstractSocket::ConnectionRefusedError";
        break;
    case QAbstractSocket::RemoteHostClosedError:
        debug << "QAbstractSocket::RemoteHostClosedError";
        break;
    case QAbstractSocket::HostNotFoundError:
        debug << "QAbstractSocket::HostNotFoundError";
        break;
    case QAbstractSocket::SocketAccessError:
        debug << "QAbstractSocket::SocketAccessError";
        break;
    case QAbstractSocket::SocketResourceError:
        debug << "QAbstractSocket::SocketResourceError";
        break;
    case QAbstractSocket::SocketTimeoutError:
        debug << "QAbstractSocket::SocketTimeoutError";
        break;
    case QAbstractSocket::DatagramTooLargeError:
        debug << "QAbstractSocket::DatagramTooLargeError";
        break;
    case QAbstractSocket::NetworkError:
        debug << "QAbstractSocket::NetworkError";
        break;
    case QAbstractSocket::AddressInUseError:
        debug << "QAbstractSocket::AddressInUseError";
        break;
    case QAbstractSocket::SocketAddressNotAvailableError:
        debug << "QAbstractSocket::SocketAddressNotAvailableError";
        break;
    case QAbstractSocket::UnsupportedSocketOperationError:
        debug << "QAbstractSocket::UnsupportedSocketOperationError";
        break;
    case QAbstractSocket::UnfinishedSocketOperationError:
        debug << "QAbstractSocket::UnfinishedSocketOperationError";
        break;
    case QAbstractSocket::UnknownSocketError:
        debug << "QAbstractSocket::UnknownSocketError";
        break;
    default:
        debug << "QAbstractSocket::SocketError(" << int(error) << ')';
        break;
    }
    return debug;
}

Q_NETWORK_EXPORT QDebug operator<<(QDebug debug, QAbstractSocket::SocketState state)
{
    switch (state) {
    case QAbstractSocket::UnconnectedState:
        debug << "QAbstractSocket::UnconnectedState";
        break;
    case QAbstractSocket::HostLookupState:
        debug << "QAbstractSocket::HostLookupState";
        break;
    case QAbstractSocket::ConnectingState:
        debug << "QAbstractSocket::ConnectingState";
        break;
    case QAbstractSocket::ConnectedState:
        debug << "QAbstractSocket::ConnectedState";
        break;
    case QAbstractSocket::BoundState:
        debug << "QAbstractSocket::BoundState";
        break;
    case QAbstractSocket::ListeningState:
        debug << "QAbstractSocket::ListeningState";
        break;
    case QAbstractSocket::ClosingState:
        debug << "QAbstractSocket::ClosingState";
        break;
    default:
        debug << "QAbstractSocket::SocketState(" << int(state) << ')';
        break;
    }
    return debug;
}
#endif

QT_END_NAMESPACE


#include "moc_qabstractsocket.h"
