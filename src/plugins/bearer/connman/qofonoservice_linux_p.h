/****************************************************************************
**
** Copyright (C) 2015 The Qt Company Ltd.
** Copyright (C) 2016 Ivailo Monev
**
** This file is part of the plugins of the Katie Toolkit.
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

#ifndef QOFONOSERVICE_H
#define QOFONOSERVICE_H

//
//  W A R N I N G
//  -------------
//
// This file is not part of the Katie API.  It exists purely as an
// implementation detail.  This header file may change from version to
// version without notice, or even be removed.
//
// We mean it.
//

#include <QtDBus/QDBusConnection>
#include <QtDBus/QDBusError>
#include <QtDBus/QDBusInterface>
#include <QtDBus/QDBusMessage>
#include <QtDBus/QDBusReply>

#include <QtDBus/qdbuspendingcall.h>
#include <QtDBus/qdbusextratypes.h>
#include <QtDBus/QDBusContext>
#include <QMap>

#ifndef QT_NO_BEARERMANAGEMENT
#ifndef QT_NO_DBUS

#define OFONO_SERVICE	                         "org.ofono"
#define OFONO_MANAGER_INTERFACE                  "org.ofono.Manager"
#define OFONO_MANAGER_PATH                       "/"
#define OFONO_MODEM_INTERFACE                    "org.ofono.Modem"
#define OFONO_NETWORK_REGISTRATION_INTERFACE     "org.ofono.NetworkRegistration"
#define OFONO_NETWORK_OPERATOR_INTERFACE         "org.ofono.NetworkOperator"
#define OFONO_DATA_CONNECTION_MANAGER_INTERFACE  "org.ofono.DataConnectionManager"
#define OFONO_SIM_MANAGER_INTERFACE              "org.ofono.SimManager"
#define OFONO_DATA_CONTEXT_INTERFACE             "org.ofono.PrimaryDataContext"

#define OFONO_SMS_MANAGER_INTERFACE              "org.ofono.SmsManager"
#define OFONO_PHONEBOOK_INTERFACE                "org.ofono.Phonebook"
#define OFONO_MESSAGE_WAITING_INTERFACE          "org.ofono.MessageWaiting"



QT_BEGIN_NAMESPACE
struct ObjectPathProperties
{
    QDBusObjectPath path;
    QVariantMap properties;
};
typedef QList<ObjectPathProperties> PathPropertiesList;
QT_END_NAMESPACE

Q_DECLARE_METATYPE(ObjectPathProperties)
Q_DECLARE_METATYPE(PathPropertiesList)

QT_BEGIN_NAMESPACE

class QOfonoManagerInterface : public  QDBusAbstractInterface
{
    Q_OBJECT

public:

    QOfonoManagerInterface( QObject *parent = nullptr);
    ~QOfonoManagerInterface();

     QDBusObjectPath path() const;

    QVariantMap getProperties();
    bool setProperty(const QString &name, const QDBusVariant &value);
    QList <QDBusObjectPath> getModems();
    QDBusObjectPath currentModem();

Q_SIGNALS:
    void propertyChanged(const QString &, const QDBusVariant &value);
    void propertyChangedContext(const QString &,const QString &,const QDBusVariant &);
protected:
    void connectNotify(const char *signal);
    QVariant getProperty(const QString &);

};


class QOfonoDBusHelper: public QObject, protected QDBusContext
 {
     Q_OBJECT
 public:
    QOfonoDBusHelper(QObject *parent = nullptr);
    ~QOfonoDBusHelper();

 public slots:
    void propertyChanged(const QString &, const QDBusVariant &);
 Q_SIGNALS:
    void propertyChangedContext(const QString &,const QString &,const QDBusVariant &);
};

class QOfonoModemInterface : public QDBusAbstractInterface
{
    Q_OBJECT

public:

    QOfonoModemInterface(const QString &dbusModemPathName, QObject *parent = nullptr);
    ~QOfonoModemInterface();

    QVariantMap getProperties();
    //properties
    bool isPowered();
    bool isOnline();
    QString getName();
    QString getManufacturer();
    QString getModel();
    QString getRevision();
    QString getSerial();

    QStringList getFeatures(); //sms, sim
    QStringList getInterfaces();
    QString defaultInterface();

protected:
    void connectNotify(const char *signal);
    QVariant getProperty(const QString &);
Q_SIGNALS:
    void propertyChanged(const QString &, const QDBusVariant &value);
    void propertyChangedContext(const QString &,const QString &,const QDBusVariant &);
};


class QOfonoNetworkRegistrationInterface : public QDBusAbstractInterface
{
    Q_OBJECT

public:

    QOfonoNetworkRegistrationInterface(const QString &dbusModemPathName, QObject *parent = nullptr);
    ~QOfonoNetworkRegistrationInterface();

    QVariantMap getProperties();

    //properties
    QString getStatus();
    quint16 getLac();
    quint32 getCellId();
    QString getTechnology();
    QString getOperatorName();
    int getSignalStrength();
    QString getBaseStation();
    QList <QDBusObjectPath> getOperators();

protected:
    void connectNotify(const char *signal);
    QVariant getProperty(const QString &);
Q_SIGNALS:
    void propertyChanged(const QString &, const QDBusVariant &value);
    void propertyChangedContext(const QString &,const QString &,const QDBusVariant &);

};

class QOfonoNetworkOperatorInterface : public QDBusAbstractInterface
{
    Q_OBJECT

public:
//modem or operator paths
    QOfonoNetworkOperatorInterface(const QString &dbusPathName, QObject *parent = nullptr);
    ~QOfonoNetworkOperatorInterface();

    QVariantMap getProperties();

    //properties
    QString getName();
    QString getStatus();// "unknown", "available", "current" and "forbidden"
    QString getMcc();
    QString getMnc();
    QStringList getTechnologies();

protected:
    void connectNotify(const char *signal);
    QVariant getProperty(const QString &);
};

class QOfonoSimInterface : public QDBusAbstractInterface
{
    Q_OBJECT

public:

    QOfonoSimInterface(const QString &dbusModemPathName, QObject *parent = nullptr);
    ~QOfonoSimInterface();

    QVariantMap getProperties();

    //properties
    bool isPresent();
    QString getHomeMcc();
    QString getHomeMnc();
//    QStringList subscriberNumbers();
//    QMap<QString,QString> serviceNumbers();
    QString pinRequired();
    QString lockedPins();
    QString cardIdentifier();

protected:
    void connectNotify(const char *signal);
    QVariant getProperty(const QString &);
};


class QOfonoDataConnectionManagerInterface : public QDBusAbstractInterface
{
    Q_OBJECT

public:

    QOfonoDataConnectionManagerInterface(const QString &dbusPathName, QObject *parent = nullptr);
    ~QOfonoDataConnectionManagerInterface();

    QVariantMap getProperties();

    //properties
    QList<QDBusObjectPath> getPrimaryContexts();
    bool isAttached();
    bool isRoamingAllowed();
    bool isPowered();

    bool setPower(bool on);

protected:
    void connectNotify(const char *signal);
    QVariant getProperty(const QString &);
};


class QOfonoPrimaryDataContextInterface : public QDBusAbstractInterface
{
    Q_OBJECT

public:

    QOfonoPrimaryDataContextInterface(const QString &dbusPathName, QObject *parent = nullptr);
    ~QOfonoPrimaryDataContextInterface();

    QVariantMap getProperties();

    //properties
    bool isActive();
    QString getApName();
    QString getType();
    QString getName();
    QVariantMap getSettings();
    QString getInterface();
    QString getAddress();

    bool setActive(bool on);
    bool setApn(const QString &name);

protected:
    void connectNotify(const char *signal);
    QVariant getProperty(const QString &);
    bool setProp(const QString &, const QVariant &var);
};

class QOfonoSmsInterface : public QDBusAbstractInterface
{
    Q_OBJECT

public:

    QOfonoSmsInterface(const QString &dbusModemPathName, QObject *parent = nullptr);
    ~QOfonoSmsInterface();

    QVariantMap getProperties();
    void sendMessage(const QString &to, const QString &message);

    //properties
    QString  serviceCenterAddress();
    bool useDeliveryReports();
    QString bearer();

protected:
    void connectNotify(const char *signal);
    QVariant getProperty(const QString &);

Q_SIGNALS:
    void propertyChanged(const QString &, const QDBusVariant &value);
    void propertyChangedContext(const QString &,const QString &,const QDBusVariant &);
    void immediateMessage(const QString &message, const QVariantMap &info);
    void incomingMessage(const QString &message, const QVariantMap &info);
};

QT_END_NAMESPACE

#endif // QT_NO_DBUS
#endif // QT_NO_BEARERMANAGEMENT

#endif //QOFONOSERVICE_H
