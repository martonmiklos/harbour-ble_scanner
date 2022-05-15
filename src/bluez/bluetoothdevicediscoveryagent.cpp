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

#include <QDebug>
#include <QDBusPendingReply>
#include <QDBusArgument>
#include <QDBusMetaType>

#include "bluetoothdevicediscoveryagent.h"

// TODO: Make the scan time configurable
// Scan time measured in milliseconds
#define BLUETOOTH_SCAN_TIME (5000)

BluetoothDeviceDiscoveryAgent::BluetoothDeviceDiscoveryAgent(QObject *parent)
    : QObject(parent)
    , m_scanState(Inactive)
{
    // For Bluez DBus documentation on the Adapter1 interface, see:
    // https://git.kernel.org/pub/scm/bluetooth/bluez.git/tree/doc/adapter-api.txt

    // TODO: Identify the correct adaptor to use
    QString path = QStringLiteral("/org/bluez/hci0");

    qDebug() << "Registering interface";
    m_adapterInterface = new QDBusInterface("org.bluez", path, "org.bluez.Adapter1", QDBusConnection::systemBus(), this);

    qDBusRegisterMetaType<ObjectStuff>();
    qDBusRegisterMetaType<DeviceInterface>();

    // dbus-send --system --dest=org.bluez --print-reply / org.freedesktop.DBus.ObjectManager.GetManagedObjects
    // dbus-monitor --system "type='signal',sender='org.bluez', interface='org.freedesktop.DBus.ObjectManager',member='InterfacesAdded', path='/'"
    QDBusConnection::systemBus().connect("org.bluez", "/", "org.freedesktop.DBus.ObjectManager", "InterfacesAdded", this, SLOT(onInterfaceAdded(QDBusObjectPath, ObjectStuff)));
}

BluetoothDeviceDiscoveryAgent::~BluetoothDeviceDiscoveryAgent()
{
    QDBusConnection::systemBus().disconnect("org.bluez", "/", "org.freedesktop.DBus.ObjectManager", "InterfacesAdded", this, SLOT(onInterfaceAdded(QDBusObjectPath, ObjectStuff)));
}

void BluetoothDeviceDiscoveryAgent::start()
{
    qDebug() << "Starting discovery";
    m_scanState = StartingDiscovery;

    // dbus-send --system --dest=org.bluez --print-reply / org.freedesktop.DBus.ObjectManager.GetManagedObjects
    QDBusInterface *adapterInterface = new QDBusInterface("org.bluez", "/", "org.freedesktop.DBus.ObjectManager", QDBusConnection::systemBus(), this);
    QDBusPendingCall async = adapterInterface->asyncCall("GetManagedObjects");
    QDBusPendingCallWatcher *watcher = new QDBusPendingCallWatcher(async, this);
    connect(watcher, &QDBusPendingCallWatcher::finished, this, [this, adapterInterface](QDBusPendingCallWatcher* call) {
        qDebug() << "DBus GetManagedObjects returned";

        QDBusPendingReply<DeviceInterface> reply = *call;
        if (reply.isError()) {
            dbusError(reply.error());
        }
        else {
            m_scanState = Active;
            for (QDBusObjectPath const &path : reply.value().keys()) {
                qDebug() << "Cached object: " << path.path();
                onInterfaceAdded(path, reply.value().value(path));
            }
            this->continueScan();
        }
        call->deleteLater();
        adapterInterface->deleteLater();
    });
}

void BluetoothDeviceDiscoveryAgent::continueScan() {
    qDebug() << "Continuing discovery";

    // dbus-send --system --dest=org.bluez --print-reply /org/bluez/hci0 org.bluez.Adapter1.StartDiscovery
    QDBusPendingCall async = m_adapterInterface->asyncCall("StartDiscovery");

    QDBusPendingCallWatcher *watcher = new QDBusPendingCallWatcher(async, this);

    connect(watcher, &QDBusPendingCallWatcher::finished, this, [this](QDBusPendingCallWatcher* call) {
        qDebug() << "DBus StartDiscovery returned";

        QDBusPendingReply<> reply = *call;
        if (reply.isError()) {
            dbusError(reply.error());
        }
        else {
            qDebug() << "No error";
            m_scanState = Active;
            m_timer.setInterval(BLUETOOTH_SCAN_TIME);
            m_timer.setSingleShot(true);
            connect(&m_timer, &QTimer::timeout, this, [this]() {
                stop();
            });
            m_timer.start();
        }
        call->deleteLater();
    });
}

void BluetoothDeviceDiscoveryAgent::onInterfaceAdded(QDBusObjectPath path, ObjectStuff interfaces_and_properties)
{
    // For Bluez DBus documentation on the Device1 interface, see:
    // https://git.kernel.org/pub/scm/bluetooth/bluez.git/tree/doc/device-api.txt

    bool result = true;
    QVariantMap properties;
    QString name;
    QString address;

    result = interfaces_and_properties.contains("org.bluez.Device1");

    if (result) {
        properties = interfaces_and_properties.value("org.bluez.Device1");
        if (properties.contains("Address")) {
            address = properties.value("Address").toString();
        }

        if (properties.contains("Alias")) {
            name = properties.value("Alias").toString();
        }
        else {
            name = address;
        }
        qDebug() << "Found bluetooth interface: " << name;

        BluetoothDeviceInfo info(address, name, 0);
        // TODO: Set the core configuration properly
        info.setCoreConfigurations(BluetoothDeviceInfo::LowEnergyCoreConfiguration);
        info.setDBusPath(path.path());

        emit deviceDiscovered(info);
    }
}

void BluetoothDeviceDiscoveryAgent::stop()
{
    qDebug() << "Stopping discovery";

    // dbus-send --system --dest=org.bluez --print-reply /org/bluez/hci0 org.bluez.Adapter1.StopDiscovery
    QDBusPendingCall async = m_adapterInterface->asyncCall("StopDiscovery");

    m_scanState = StoppingDiscovery;
    QDBusPendingCallWatcher *watcher = new QDBusPendingCallWatcher(async, this);

    connect(watcher, &QDBusPendingCallWatcher::finished, this, [this](QDBusPendingCallWatcher* call) {
        qDebug() << "DBus StopDiscovery returned";

        QDBusPendingReply<> reply = *call;
        if (reply.isError()) {
            dbusError(reply.error());
        }
        else {
            qDebug() << "Scan finished without error";
            m_scanState = Inactive;
            emit finished();
        }
        call->deleteLater();
    });
}

bool BluetoothDeviceDiscoveryAgent::isActive() const
{
    return m_scanState != Inactive;
}

void BluetoothDeviceDiscoveryAgent::dbusError(QDBusError error)
{
    qDebug() << "Error during discovery: " << error.name() << " (" << error.type() << ")";
    qDebug() << error.message();
    m_scanState = Inactive;
    // TODO: Report a more usefeul error type
    this->emit error(UnknownError);
}
