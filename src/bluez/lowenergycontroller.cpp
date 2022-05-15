/***************************************************************************
**
** Copyright (C) 2022 David Llewellyn-Jones.
**
** You may use this file under the terms of the BSD license as follows:
**
** "Redistribution and use in source and binary forms, with or without
** modification, are permitted provided that the following conditions are
** met:
**   * Redistributions of source code must retain the above copyright
**     notice, this list of conditions and the following disclaimer.
**   * Redistributions in binary form must reproduce the above copyright
**     notice, this list of conditions and the following disclaimer in
**     the documentation and/or other materials provided with the
**     distribution.
**   * Neither the name of The Qt Company Ltd nor the names of its
**     contributors may be used to endorse or promote products derived
**     from this software without specific prior written permission.
**
**
** THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
** "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
** LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
** A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT
** OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
** SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
** LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
** DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
** THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
** (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
** OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE."
**
****************************************************************************/

#include <QDBusPendingReply>
#include <QDBusArgument>
#include <QDBusPendingCallWatcher>
#include <QDebug>

#include "bluetoothdevicediscoveryagent.h"
#include "lowenergycontroller.h"

LowEnergyController::LowEnergyController(const BluetoothDeviceInfo &remoteDeviceInfo, QObject *parent)
    : QObject(parent)
    , m_remoteDeviceInfo(remoteDeviceInfo)
    , m_type(RandomAddress)
    , m_device(nullptr)
    , m_servicesResolved(false)
    , m_services()
    , m_characteristics()
    , m_descriptors()
{
}

LowEnergyController::~LowEnergyController()
{
}

void LowEnergyController::connectToDevice()
{
    // For Bluez DBus documentation on the Device1 interface, see:
    // https://git.kernel.org/pub/scm/bluetooth/bluez.git/tree/doc/device-api.txt

    if (!m_device) {
        qDebug() << "Connecting to device: " << m_remoteDeviceInfo.dBusPath();
        m_device = new QDBusInterface("org.bluez", m_remoteDeviceInfo.dBusPath(), "org.bluez.Device1", QDBusConnection::systemBus(), this);

        // dbus-send --system --dest=org.bluez --print-reply /org/bluez/hci0/dev_XX_XX_XX_XX_XX_XX org.bluez.Device1.Connect
        QDBusPendingCall async = m_device->asyncCall("Connect");

        QDBusPendingCallWatcher *watcher = new QDBusPendingCallWatcher(async, this);

        connect(watcher, &QDBusPendingCallWatcher::finished, this, [this](QDBusPendingCallWatcher* call) {
            QDBusPendingReply<> reply = *call;
            if (reply.isError()) {
                qDebug() << "Error connecting: " << reply.error().name() << " (" << reply.error().type() << ")";
                qDebug() << reply.error().message();
                // TODO: Report a more usefeul error type
                emit error(UnknownError);
            }
            else {
                qDebug() << "Connected to device";
                // dbus-monitor --system "type='signal',sender='org.bluez', interface='org.freedesktop.DBus.Properties',member='PropertiesChanged', path='/org/bluez/hci0/dev_XX_XX_XX_XX_XX_XX'"
                QDBusConnection::systemBus().connect("org.bluez", m_remoteDeviceInfo.dBusPath(), "org.freedesktop.DBus.Properties", "PropertiesChanged", this, SLOT(onPropertiesChanged(QString, QVariantMap)));
                emit connected();
            }
            call->deleteLater();
        });
    }
    m_services.clear();
    m_characteristics.clear();
    m_descriptors.clear();
    m_servicesResolved = false;
}

void LowEnergyController::disconnectFromDevice()
{
    // For Bluez DBus documentation on the Device1 interface, see:
    // https://git.kernel.org/pub/scm/bluetooth/bluez.git/tree/doc/device-api.txt

    if (m_device) {
        qDebug() << "Disconnecting from device";
        // dbus-send --system --dest=org.bluez --print-reply /org/bluez/hci0/dev_XX_XX_XX_XX_XX_XX org.bluez.Device1.Disconnect
        QDBusPendingCall async = m_device->asyncCall("Disconnect");

        QDBusPendingCallWatcher *watcher = new QDBusPendingCallWatcher(async, this);

        connect(watcher, &QDBusPendingCallWatcher::finished, this, [this](QDBusPendingCallWatcher* call) {
            QDBusPendingReply<> reply = *call;
            if (reply.isError()) {
                qDebug() << "Error disconnecting: " << reply.error().name() << " (" << reply.error().type() << ")";
                qDebug() << reply.error().message();
                // TODO: Report a more usefeul error type
                emit error(UnknownError);
            }
            else {
                qDebug() << "Disconnected from device";
                //delete m_characteristic;
                //m_characteristic = nullptr;
                emit disconnected();
            }
            call->deleteLater();
            // dbus-monitor --system "type='signal',sender='org.bluez', interface='org.freedesktop.DBus.Properties',member='PropertiesChanged', path='/org/bluez/hci0/dev_XX_XX_XX_XX_XX_XX'"
            QDBusConnection::systemBus().disconnect("org.bluez", m_remoteDeviceInfo.dBusPath(), "org.freedesktop.DBus.Properties", "PropertiesChanged", this, SLOT(onPropertiesChanged(QString, QVariantMap)));
            delete m_device;
            m_device = nullptr;
        });
    }
    m_services.clear();
    m_characteristics.clear();
    m_descriptors.clear();
    m_servicesResolved = false;
}

QString LowEnergyController::localAddress() const
{
    return m_remoteDeviceInfo.address();
}

QString LowEnergyController::remoteAddress() const
{
    return m_remoteDeviceInfo.address();
}

QString LowEnergyController::remoteName() const
{
    return m_remoteDeviceInfo.name();
}

void LowEnergyController::setRemoteAddressType(LowEnergyController::RemoteAddressType type)
{
    m_type = type;
}

LowEnergyController::RemoteAddressType LowEnergyController::remoteAddressType() const
{
    return m_type;
}

void LowEnergyController::discoverServices()
{
    // For Bluez DBus documentation on the Device1 interface, see:
    // https://git.kernel.org/pub/scm/bluetooth/bluez.git/tree/doc/device-api.txt

    if (m_device) {
        qDebug() << "Discovering services";

        // Check whether services have been resolved
        // dbus-monitor --system "type='signal',sender='org.bluez', interface='org.freedesktop.DBus.Properties',member='PropertiesChanged', path='/org/bluez/hci0/dev_XX_XX_XX_XX_XX_XX'"
        QDBusInterface *properties = new QDBusInterface("org.bluez", m_remoteDeviceInfo.dBusPath(), "org.freedesktop.DBus.Properties", QDBusConnection::systemBus(), this);

        // dbus-send --system --dest=org.bluez --print-reply /org/bluez/hci0/dev_XX_XX_XX_XX_XX_XX org.freedesktop.DBus.Properties.Get string:org.bluez.Device1 string:ServicesResolved
        QDBusPendingCall async = properties->asyncCall("Get", "org.bluez.Device1", "ServicesResolved");
        QDBusPendingCallWatcher *watcher = new QDBusPendingCallWatcher(async, this);
        connect(watcher, &QDBusPendingCallWatcher::finished, this, [this, properties](QDBusPendingCallWatcher* call) {
            qDebug() << "DBus ServicesResolved check returned";

            QDBusPendingReply<QVariant> reply = *call;
            if (reply.isError()) {
                qDebug() << "Error discoering services: " << reply.error().name() << " (" << reply.error().type() << ")";
                qDebug() << reply.error().message();
                // TODO: Report a more usefeul error type
                emit error(UnknownError);
            }
            else {
                if (reply.value().toBool()) {
                    // Services resolved
                    qDebug() << "Services resolved";
                    servicesResolved();
                }
                else {
                    // Wait for them to be resolved
                    qDebug() << "Services not yet resolved";
                }
            }
            call->deleteLater();
            properties->deleteLater();
        });
    }
}

void LowEnergyController::onPropertiesChanged(QString interface, QVariantMap properties)
{
    // For Bluez DBus documentation on the Device1 interface, see:
    // https://git.kernel.org/pub/scm/bluetooth/bluez.git/tree/doc/device-api.txt

    qDebug() << "Properties changed: " << interface;
    if (interface == QStringLiteral("org.bluez.Device1")) {
        if (properties.contains("ServicesResolved")) {
            if (properties.value("ServicesResolved").toBool()) {
                qDebug() << "Services resolved";
                servicesResolved();
            }
            else {
                qDebug() << "Services not yet resolved";
            }
        }
    }
}

void LowEnergyController::servicesResolved()
{
    // For Bluez DBus documentation on the GattService1, GattCharacteristic1 and GattDescriptor1 interfaces, see:
    // https://git.kernel.org/pub/scm/bluetooth/bluez.git/tree/doc/gatt-api.txt

    if (!m_servicesResolved) {
        QDBusInterface *adapterInterface = new QDBusInterface("org.bluez", "/", "org.freedesktop.DBus.ObjectManager", QDBusConnection::systemBus(), this);
        // dbus-send --system --dest=org.bluez --print-reply / org.freedesktop.DBus.ObjectManager.GetManagedObjects
        QDBusPendingCall async = adapterInterface->asyncCall("GetManagedObjects");
        QDBusPendingCallWatcher *watcher = new QDBusPendingCallWatcher(async, this);
        connect(watcher, &QDBusPendingCallWatcher::finished, this, [this, adapterInterface](QDBusPendingCallWatcher* call) {
            qDebug() << "Services resolution completed";

            QDBusPendingReply<DeviceInterface> reply = *call;
            if (reply.isError()) {
                qDebug() << "Error resolving services: " << reply.error().name() << " (" << reply.error().type() << ")";
                qDebug() << reply.error().message();
                // TODO: Report a more usefeul error type
                emit error(UnknownError);
            }
            else {
                for (QDBusObjectPath const &path : reply.value().keys()) {
                    qDebug() << "Cached object: " << path.path();
                    ObjectStuff interfaces = reply.value().value(path);
                    // If it's a GATT service collect the service info
                    if (interfaces.contains("org.bluez.GattService1")) {
                        QVariantMap properties = interfaces.value("org.bluez.GattService1");
                        QString uuid = properties.value("UUID").toString();
                        qDebug() << "Service: " << uuid;
                        ServiceSummary service;
                        service.name = uuid;
                        service.primary = properties.value("Primary").toBool();
                        service.path = path.path();
                        m_services.insert(uuid, service);
                    }

                    // If it's a GATT characteristic collect the characteristic info
                    if (interfaces.contains("org.bluez.GattCharacteristic1")) {
                        QVariantMap properties = interfaces.value("org.bluez.GattCharacteristic1");
                        QString uuid = properties.value("UUID").toString();
                        qDebug() << "Characteristic: " << uuid;
                        CharacteristicSummary characteristic;
                        characteristic.path = path.path();
                        characteristic.name = uuid;
                        characteristic.servicePath = properties.value("Service").value<QDBusObjectPath>().path();
                        characteristic.flags = properties.value("Flags", QStringList()).toStringList();
                        characteristic.handle = properties.value("Handle", 0u).toUInt();
                        characteristic.value = properties.value("Value", QByteArray()).toByteArray();
                        m_characteristics.insert(uuid, characteristic);
                    }

                    // If it's a GATT descriptor collect the descriptor info
                    if (interfaces.contains("org.bluez.GattDescriptor1")) {
                        QVariantMap properties = interfaces.value("org.bluez.GattDescriptor1");
                        QString uuid = properties.value("UUID").toString();
                        qDebug() << "Descriptor: " << uuid;
                        DescriptorSummary descriptor;
                        descriptor.path = path.path();
                        descriptor.characteristicPath = properties.value("Characteristic").value<QDBusObjectPath>().path();
                        descriptor.value = properties.value("Value", QByteArray()).toByteArray();
                        m_descriptors.insert(uuid, descriptor);
                    }
                }
                m_servicesResolved = true;
            }
            for (QString uuid : m_services.keys()) {
                emit serviceDiscovered(uuid);
            }
            call->deleteLater();
            adapterInterface->deleteLater();
            emit discoveryFinished();
        });
    }
}

LowEnergyController::ControllerState LowEnergyController::state() const
{
    ControllerState state;
    if (m_device) {
        state = ConnectedState;
    }
    else {
        state = UnconnectedState;
    }
    return state;
}

LowEnergyController::Error LowEnergyController::error() const
{
    return NoError;
}

QString LowEnergyController::errorString() const
{
    return QStringLiteral("Error");
}

LowEnergyService *LowEnergyController::createServiceObject(const QString &serviceUuid)
{
    LowEnergyService *service = nullptr;

    if (m_services.contains(serviceUuid)) {
        ServiceSummary summary = m_services.value(serviceUuid);
        QList<LowEnergyCharacteristic> characteristics;
        // Add characteristics that are part of the service
        for (QString uuid : m_characteristics.keys()) {
            CharacteristicSummary charSummary = m_characteristics.value(uuid);
            if (charSummary.servicePath == summary.path) {
                LowEnergyCharacteristic characteristic;
                characteristic.setPath(charSummary.path);
                characteristic.setName(charSummary.name);
                characteristic.setUuid(uuid);
                characteristic.setValue(charSummary.value);
                characteristic.setHandle(charSummary.handle);
                characteristic.setProperties(charSummary.flags);

                // Add descriptors that are part of the characteristic
                QList<LowEnergyDescriptor> descriptors;
                for (QString descriptorUUID : m_descriptors.keys()) {
                    DescriptorSummary descriptorSummary = m_descriptors.value(descriptorUUID);
                    if (descriptorSummary.characteristicPath == charSummary.path) {
                        LowEnergyDescriptor descriptor;
                        descriptor.setPath(descriptorSummary.path);
                        descriptor.setUuid(descriptorUUID);
                        descriptor.setValue(descriptorSummary.value);
                        descriptors.append(descriptor);
                    }
                }
                characteristic.setDescriptors(descriptors);
                characteristics.append(characteristic);
                qDebug() << "Creating characteristic with " << descriptors.length() << " descriptors";
            }
        }

        qDebug() << "Creating service with " << characteristics.length() << " characteristics";

        LowEnergyService::ServiceType type = summary.primary ? LowEnergyService::PrimaryService : LowEnergyService::IncludedService;
        service = new LowEnergyService(serviceUuid, summary.path, summary.name, type, characteristics);
    }
    else {
        qDebug() << "Service not found: " << serviceUuid;
    }

    return service;
}

