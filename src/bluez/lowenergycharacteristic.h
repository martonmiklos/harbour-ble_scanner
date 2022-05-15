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

#ifndef LOWENERGYCHARACTERISTIC_H
#define LOWENERGYCHARACTERISTIC_H

#include <QString>
#include <QList>
#include "lowenergydescriptor.h"

class LowEnergyCharacteristic
{
public:
    enum PropertyType {
        Unknown = 0x00,
        Broadcasting = 0x01,
        Read = 0x02,
        WriteNoResponse = 0x04,
        Write = 0x08,
        Notify = 0x10,
        Indicate = 0x20,
        WriteSigned = 0x40,
        ExtendedProperty = 0x80
    };
    Q_DECLARE_FLAGS(PropertyTypes, PropertyType)

    LowEnergyCharacteristic();

    QString path() const;
    QString name() const;
    QString uuid() const;
    QByteArray value() const;
    quint16 handle() const;
    PropertyTypes properties() const;
    QList<LowEnergyDescriptor> descriptors() const;


    void setPath(QString const &path);
    void setName(QString const &name);
    void setUuid(QString const &uuid);
    void setValue(QByteArray const &value);
    void setHandle(quint16 handle);
    void setProperties(PropertyTypes properties);
    void setProperties(QStringList const &properties);
    void setDescriptors(QList<LowEnergyDescriptor> const &descriptors);

private:
    QString m_path;
    QString m_name;
    QString m_uuid;
    QByteArray m_value;
    quint16 m_handle;
    PropertyTypes m_properties;
    QList<LowEnergyDescriptor> m_descriptors;
};

#endif // LOWENERGYCHARACTERISTIC_H
