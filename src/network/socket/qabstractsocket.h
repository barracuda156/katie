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

#ifndef QABSTRACTSOCKET_H
#define QABSTRACTSOCKET_H

#include <QtCore/qiodevice.h>
#include <QtCore/qobject.h>
#include <QtCore/qmetatype.h>

#ifndef QT_NO_DEBUG_STREAM
#include <QtCore/qdebug.h>
#endif

QT_BEGIN_NAMESPACE

class QHostAddress;
class QAbstractSocketPrivate;

class Q_NETWORK_EXPORT QAbstractSocket : public QIODevice
{
    Q_OBJECT
    Q_ENUMS(SocketType NetworkLayerProtocol SocketError SocketState SocketOption)
public:
    enum SocketType {
        TcpSocket,
        UdpSocket,
        UnknownSocketType = -1
    };
    enum NetworkLayerProtocol {
        IPv4Protocol,
        IPv6Protocol,
        UnknownNetworkLayerProtocol = -1
    };
    enum SocketError {
        ConnectionRefusedError,
        RemoteHostClosedError,
        HostNotFoundError,
        SocketAccessError,
        SocketResourceError,
        SocketTimeoutError,                     /* 5 */
        DatagramTooLargeError,
        NetworkError,
        AddressInUseError,
        SocketAddressNotAvailableError,
        UnsupportedSocketOperationError,        /* 10 */
        UnfinishedSocketOperationError,

        UnknownSocketError = -1
    };
    enum SocketState {
        UnconnectedState,
        HostLookupState,
        ConnectingState,
        ConnectedState,
        BoundState,
        ListeningState,
        ClosingState
    };
    enum SocketOption {
        LowDelayOption, // TCP_NODELAY
        KeepAliveOption, // SO_KEEPALIVE
        MulticastTtlOption, // IP_MULTICAST_TTL
        MulticastLoopbackOption // IP_MULTICAST_LOOPBACK
    };

    QAbstractSocket(SocketType socketType, QObject *parent);
    virtual ~QAbstractSocket();

    virtual void connectToHost(const QString &hostName, quint16 port, OpenMode mode = ReadWrite);
    virtual void connectToHost(const QHostAddress &address, quint16 port, OpenMode mode = ReadWrite);
    virtual void disconnectFromHost();

    bool isValid() const;

    qint64 bytesAvailable() const;
    qint64 bytesToWrite() const;

    bool canReadLine() const;

    quint16 localPort() const;
    QHostAddress localAddress() const;
    quint16 peerPort() const;
    QHostAddress peerAddress() const;
    QString peerName() const;

    qint64 readBufferSize() const;
    virtual void setReadBufferSize(qint64 size);

    int socketDescriptor() const;
    virtual bool setSocketDescriptor(int socketDescriptor, SocketState state = ConnectedState,
                                     OpenMode openMode = ReadWrite);

    void setSocketOption(QAbstractSocket::SocketOption option, const int value);
    int socketOption(QAbstractSocket::SocketOption option);

    SocketType socketType() const;
    SocketState state() const;
    SocketError error() const;

    // from QIODevice
    void close();
    bool isSequential() const;
    bool atEnd() const;

    virtual bool flush();
    virtual void abort();

    // for synchronous access
    virtual bool waitForConnected(int msecs = 30000);
    bool waitForReadyRead(int msecs = 30000);
    bool waitForBytesWritten(int msecs = 30000);
    virtual bool waitForDisconnected(int msecs = 30000);

Q_SIGNALS:
    void hostFound();
    void connected();
    void disconnected();
    void stateChanged(QAbstractSocket::SocketState);
    void error(QAbstractSocket::SocketError);

protected:
    qint64 readData(char *data, qint64 maxlen);
    qint64 writeData(const char *data, qint64 len);

    void setSocketState(SocketState state);
    void setSocketError(SocketError socketError);
    void setLocalPort(quint16 port);
    void setLocalAddress(const QHostAddress &address);
    void setPeerPort(quint16 port);
    void setPeerAddress(const QHostAddress &address);
    void setPeerName(const QString &name);

    QAbstractSocket(SocketType socketType, QAbstractSocketPrivate &dd, QObject *parent = nullptr);

private:
    Q_DECLARE_PRIVATE(QAbstractSocket)
    Q_DISABLE_COPY(QAbstractSocket)

    Q_PRIVATE_SLOT(d_func(), void _q_connectToNextAddress())
    Q_PRIVATE_SLOT(d_func(), void _q_startConnecting(const QHostInfo &))
    Q_PRIVATE_SLOT(d_func(), void _q_abortConnectionAttempt())
    Q_PRIVATE_SLOT(d_func(), void _q_testConnection())
    Q_PRIVATE_SLOT(d_func(), void _q_forceDisconnect())

    friend class QLocalSocket;
    friend class QLocalSocketPrivate;
};

#ifndef QT_NO_DEBUG_STREAM
Q_NETWORK_EXPORT QDebug operator<<(QDebug, QAbstractSocket::SocketError);
Q_NETWORK_EXPORT QDebug operator<<(QDebug, QAbstractSocket::SocketState);
#endif

QT_END_NAMESPACE

Q_DECLARE_METATYPE(QAbstractSocket::SocketError);
Q_DECLARE_METATYPE(QAbstractSocket::SocketState);

#endif // QABSTRACTSOCKET_H
