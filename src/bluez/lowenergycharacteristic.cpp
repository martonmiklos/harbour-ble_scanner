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

#include "lowenergycharacteristic.h"

LowEnergyCharacteristic::LowEnergyCharacteristic()
    : m_path()
    , m_name()
    , m_uuid()
    , m_value()
    , m_handle(0u)
    , m_properties(Unknown)
    , m_descriptors()
{
}

QString LowEnergyCharacteristic::path() const
{
    return m_path;
}

QString LowEnergyCharacteristic::name() const
{
    return m_name;
}

QString LowEnergyCharacteristic::uuid() const
{
    return m_uuid;
}

QByteArray LowEnergyCharacteristic::value() const
{
    return m_value;
}

quint16 LowEnergyCharacteristic::handle() const
{
    return m_handle;
}

LowEnergyCharacteristic::PropertyTypes LowEnergyCharacteristic::properties() const
{
    return m_properties;
}

QList<LowEnergyDescriptor> LowEnergyCharacteristic::descriptors() const
{
    return m_descriptors;
}

void LowEnergyCharacteristic::setPath(QString const &path)
{
    m_path = path;
}

void LowEnergyCharacteristic::setName(QString const &name)
{
    m_name = name;
}

void LowEnergyCharacteristic::setUuid(QString const &uuid)
{
    m_uuid = uuid;
}

void LowEnergyCharacteristic::setValue(QByteArray const &value)
{
    m_value = value;
}

void LowEnergyCharacteristic::setHandle(quint16 handle)
{
    m_handle = handle;
}

void LowEnergyCharacteristic::setProperties(PropertyTypes properties)
{
    m_properties = properties;
}

void LowEnergyCharacteristic::setProperties(QStringList const &properties)
{
    m_properties = Unknown;
    for (QString const &property : properties) {
        if (property == QStringLiteral("broadcast")) {
            m_properties |= Broadcasting;
        } else if (property == QStringLiteral("read")) {
            m_properties |= Read;
        } else if (property == QStringLiteral("write-without-response")) {
            m_properties |= WriteNoResponse;
        } else if (property == QStringLiteral("write")) {
            m_properties |= Write;
        } else if (property == QStringLiteral("notify")) {
            m_properties |= Notify;
        } else if (property == QStringLiteral("indicate")) {
            m_properties |= Indicate;
        } else if (property == QStringLiteral("authenticated-signed-writes")) {
            m_properties |= WriteSigned;
        } else if (property == QStringLiteral("extended-properties")) {
            m_properties |= ExtendedProperty;
        }
    }
}

void LowEnergyCharacteristic::setDescriptors(QList<LowEnergyDescriptor> const &descriptors)
{
    m_descriptors = descriptors;
}

