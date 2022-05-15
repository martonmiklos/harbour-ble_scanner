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

#ifndef LOWENERGYSERVICE_H
#define LOWENERGYSERVICE_H

#include <QObject>
#include "lowenergycharacteristic.h"

class LowEnergyService : public QObject
{
    Q_OBJECT
public:
    enum ServiceType {
        PrimaryService = 0x0001,
        IncludedService = 0x0002
    };
    Q_ENUM(ServiceType)

    enum ServiceState {
        InvalidService = 0,
        DiscoveryRequired,
        DiscoveringServices,
        ServiceDiscovered
    };
    Q_ENUM(ServiceState)

    explicit LowEnergyService(QString const &uuid, QString const &path, QString const &name, ServiceType type, QList<LowEnergyCharacteristic> characteristics, QObject *parent = nullptr);

    QString serviceName() const;
    ServiceType type() const;
    QString serviceUuid() const;

    void discoverDetails();
    ServiceState state() const;
    QList<LowEnergyCharacteristic> characteristics() const;

signals:
    void stateChanged(LowEnergyService::ServiceState newState);

private:
    void readCharacteristicValue(LowEnergyCharacteristic const &characteristic);
    void readDescriptorValue(LowEnergyDescriptor const &descriptor);

private:
    QString m_uuid;
    QString m_path;
    QString m_name;
    ServiceType m_type;
    QList<LowEnergyCharacteristic> m_characteristics;
    ServiceState m_state;
    qint32 m_discoveryCount;
};

#endif // LOWENERGYSERVICE_H
