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

#ifndef BLUETOOTHDEVICEINFO_H
#define BLUETOOTHDEVICEINFO_H

#include <QString>
#include <QFlags>
#include <QVariantMap>

class BluetoothDeviceInfo
{
public:
    enum CoreConfiguration {
        UnknownCoreConfiguration = 0x00,
        LowEnergyCoreConfiguration = 0x01,
        BaseRateCoreConfiguration = 0x02,
        BaseRateAndLowEnergyCoreConfiguration = 0x03
    };

    typedef QFlags<CoreConfiguration> CoreConfigurations;

    explicit BluetoothDeviceInfo();
    explicit BluetoothDeviceInfo(const QString &address, const QString &name, quint32 classOfDevice);

    CoreConfigurations coreConfigurations() const;
    void setCoreConfigurations(CoreConfigurations coreConfigs);
    QString address() const;
    QString name() const;
    bool isValid() const;
    void setDBusPath(const QString &path);
    QString dBusPath() const;
    void setSerices(const QStringList &services);
    QStringList services() const;

private:
    QString m_address;
    QString m_name;
    quint32 m_classOfDevice;
    CoreConfigurations m_coreConfigs;
    QString m_path;
    QStringList m_services;
};

#endif // BLUETOOTHDEVICEINFO_H
