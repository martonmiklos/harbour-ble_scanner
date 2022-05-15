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

#ifndef LOWENERGYCONTROLLER_H
#define LOWENERGYCONTROLLER_H

#include <QObject>
#include <QDBusInterface>
#include "lowenergyservice.h"
#include "bluetoothdevicediscoveryagent.h"
#include "bluetoothdeviceinfo.h"

class LowEnergyController : public QObject
{
    Q_OBJECT
public:
    enum Error {
        NoError,
        UnknownError,
        UnknownRemoteDeviceError,
        NetworkError,
        InvalidBluetoothAdapterError,
        ConnectionError
    };

    enum RemoteAddressType {
        RandomAddress,
        PublicAddress
    };

    enum ControllerState {
        UnconnectedState = 0,
        ConnectingState,
        ConnectedState,
        DiscoveringState,
        DiscoveredState,
        ClosingState
    };

    Q_ENUM(RemoteAddressType)
    Q_ENUM(Error)
    Q_ENUM(ControllerState)

    explicit LowEnergyController(const BluetoothDeviceInfo &remoteDeviceInfo, QObject *parent = nullptr);
    ~LowEnergyController();

    void connectToDevice();
    void disconnectFromDevice();
    QString localAddress() const;
    QString remoteAddress() const;
    QString remoteName() const;
    void setRemoteAddressType(RemoteAddressType type);
    RemoteAddressType remoteAddressType() const;
    void discoverServices();
    ControllerState state() const;
    Error error() const;
    QString errorString() const;
    LowEnergyService *createServiceObject(const QString &serviceUuid);

signals:
    void connected();
    void disconnected();
    void error(LowEnergyController::Error newError);
    void serviceDiscovered(const QString &newService);
    void discoveryFinished();

private:
    void servicesResolved();

public slots:
    void onPropertiesChanged(QString interface, QVariantMap properties);

private:
    struct ServiceSummary {
        QString path;
        QString name;
        bool primary;
    };

    struct CharacteristicSummary {
        QString path;
        QString name;
        QString servicePath;
        QStringList flags;
        quint16 handle;
        QByteArray value;
    };

    struct DescriptorSummary {
        QString path;
        QString characteristicPath;
        QByteArray value;
    };

    BluetoothDeviceInfo m_remoteDeviceInfo;
    RemoteAddressType m_type;
    QDBusInterface *m_device;
    bool m_servicesResolved;
    QMap<QString, ServiceSummary> m_services;
    QMap<QString, CharacteristicSummary> m_characteristics;
    QMap<QString, DescriptorSummary> m_descriptors;
};

#endif // LOWENERGYCONTROLLER_H
