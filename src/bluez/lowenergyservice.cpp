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
#include <QDBusInterface>
#include <QDebug>

#include "lowenergyservice.h"

LowEnergyService::LowEnergyService(QString const &uuid, QString const &path, QString const &name, ServiceType type, QList<LowEnergyCharacteristic> characteristics, QObject *parent)
    : QObject(parent)
    , m_uuid(uuid)
    , m_path(path)
    , m_name(name)
    , m_type(type)
    , m_characteristics(characteristics)
    , m_state(DiscoveryRequired)
    , m_discoveryCount(0)
{

}

QString LowEnergyService::serviceName() const
{
    return m_name;
}

LowEnergyService::ServiceType LowEnergyService::type() const
{
    return m_type;
}

QString LowEnergyService::serviceUuid() const
{
    return m_uuid;
}

void LowEnergyService::discoverDetails()
{
    qDebug() << "Discovering details";
    m_state = DiscoveringServices;
    m_discoveryCount = m_characteristics.length();
    for (LowEnergyCharacteristic characteristic : m_characteristics) {
        m_discoveryCount += characteristic.descriptors().length();
    }

    for (LowEnergyCharacteristic characteristic : m_characteristics) {
        readCharacteristicValue(characteristic);
        QList<LowEnergyDescriptor> descriptors = characteristic.descriptors();
        for (LowEnergyDescriptor descriptor : descriptors) {
            readDescriptorValue(descriptor);
        }
    }
    qDebug() << "Reading value for " << m_discoveryCount << " characteristics and descriptors";
    if (m_discoveryCount == 0) {
        m_state = ServiceDiscovered;
        emit stateChanged(m_state);
    }
}

LowEnergyService::ServiceState LowEnergyService::state() const
{
    return m_state;
}

QList<LowEnergyCharacteristic> LowEnergyService::characteristics() const
{
    return m_characteristics;
}

void LowEnergyService::readCharacteristicValue(LowEnergyCharacteristic const &characteristic)
{
    // For Bluez DBus documentation on the GattCharacteristic1 interface, see:
    // https://git.kernel.org/pub/scm/bluetooth/bluez.git/tree/doc/gatt-api.txt

    if (characteristic.properties() | LowEnergyCharacteristic::Read) {
        QDBusInterface *gattCharacteristic = new QDBusInterface("org.bluez", characteristic.path(), "org.bluez.GattCharacteristic1", QDBusConnection::systemBus(), this);
        // dbus-send --system --dest=org.bluez --print-reply /org/bluez/hci0/dev_XX_XX_XX_XX_XX_XX/serviceXXXX/charXXXX org.bluez.GattCharacteristic1.ReadValue
        // Note: dbus-send doesn't support sending a{sv} (QVariantMap) types
        QDBusPendingCall async = gattCharacteristic->asyncCall("ReadValue", QVariantMap());
        QDBusPendingCallWatcher *watcher = new QDBusPendingCallWatcher(async, this);
        connect(watcher, &QDBusPendingCallWatcher::finished, this, [this, gattCharacteristic, characteristic](QDBusPendingCallWatcher* call) {
            qDebug() << "DBus characteristic ReadValue returned";

            QDBusPendingReply<QByteArray> reply = *call;
            if (reply.isError()) {
                qDebug() << "Error reading characteristic value: " << reply.error().name() << " (" << reply.error().type() << ")";
                qDebug() << reply.error().message();
                // TODO: Emit an error signal
            }
            else {
                for (int pos = 0; pos < m_characteristics.length(); ++pos) {
                    if (m_characteristics[pos].uuid() == characteristic.uuid()) {
                        LowEnergyCharacteristic replacement = m_characteristics[pos];
                        qDebug() << "Updating characteristic value";
                        replacement.setValue(reply.value());
                        m_characteristics.replace(pos, replacement);
                    }
                }
            }
            --m_discoveryCount;
            if (m_discoveryCount == 0) {
                m_state = ServiceDiscovered;
                emit stateChanged(m_state);
            }
            call->deleteLater();
            gattCharacteristic->deleteLater();
        });
    }
    else {
        --m_discoveryCount;
        if (m_discoveryCount == 0) {
            m_state = ServiceDiscovered;
            emit stateChanged(m_state);
        }
    }
}

void LowEnergyService::readDescriptorValue(LowEnergyDescriptor const &descriptor)
{
    // For Bluez DBus documentation on the GattDescriptor1 interface, see:
    // https://git.kernel.org/pub/scm/bluetooth/bluez.git/tree/doc/gatt-api.txt

    QDBusInterface *gattDescriptor = new QDBusInterface("org.bluez", descriptor.path(), "org.bluez.GattDescriptor1", QDBusConnection::systemBus(), this);
    // dbus-send --system --dest=org.bluez --print-reply /org/bluez/hci0/dev_XX_XX_XX_XX_XX_XX/serviceXXXX/charXXXX/descXXXX org.bluez.GattDescriptor1.ReadValue
    // Note: dbus-send doesn't support sending a{sv} (QVariantMap) types
    QDBusPendingCall async = gattDescriptor->asyncCall("ReadValue", QVariantMap());
    QDBusPendingCallWatcher *watcher = new QDBusPendingCallWatcher(async, this);
    connect(watcher, &QDBusPendingCallWatcher::finished, this, [this, gattDescriptor, descriptor](QDBusPendingCallWatcher* call) {
        qDebug() << "DBus descriptor ReadValue returned";

        QDBusPendingReply<QByteArray> reply = *call;
        if (reply.isError()) {
            qDebug() << "Error reading descriptor value: " << reply.error().name() << " (" << reply.error().type() << ")";
            qDebug() << reply.error().message();
            // TODO: Emit an error signal
        }
        else {
            for (int charPos = 0; charPos < m_characteristics.length(); ++charPos) {
                QList<LowEnergyDescriptor> descriptors = m_characteristics[charPos].descriptors();
                bool changed = false;
                for (int descPos = 0; descPos < descriptors.length(); ++descPos) {
                    if (descriptors[descPos].uuid() == m_characteristics[charPos].uuid()) {
                        LowEnergyDescriptor replacement = descriptors[descPos];
                        qDebug() << "Updating descriptor value";
                        replacement.setValue(reply.value());
                        descriptors.replace(descPos, replacement);
                    }
                }
                if (changed) {
                    LowEnergyCharacteristic replacement = m_characteristics.value(charPos);
                    replacement.setDescriptors(descriptors);
                    m_characteristics.replace(charPos, replacement);
                }
            }
        }
        --m_discoveryCount;
        if (m_discoveryCount == 0) {
            m_state = ServiceDiscovered;
            emit stateChanged(m_state);
        }
        call->deleteLater();
        gattDescriptor->deleteLater();
    });
}
