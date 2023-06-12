/*
 * Copyright (c) 2012-2023 dresden elektronik ingenieurtechnik gmbh.
 * All rights reserved.
 *
 * The software in this package is published under the terms of the BSD
 * style license a copy of which has been included with this distribution in
 * the LICENSE.txt file.
 *
 */

#include "deconz/types.h"
#include "deconz/zdp_descriptors.h"
#include "deconz/buffer_helper.h"
#include "deconz/dbg_trace.h"
#include "zcl_private.h"

namespace deCONZ {

class NodeDescriptorPrivate
{
public:
    NodeDescriptorPrivate();

    uint8_t m_raw[13]{0};
    struct
    {
        uint8_t m_isNull : 1;
        uint8_t m_deviceType : 2;
        uint8_t _pad : 5;
    };

    uint16_t m_serverMask{0};
};

NodeDescriptorPrivate::NodeDescriptorPrivate() :
    m_isNull(1),
    m_deviceType(deCONZ::UnknownDevice)
{
    static_assert (sizeof(NodeDescriptorPrivate) == 16, "");
}


NodeDescriptor::NodeDescriptor() :
    d(new NodeDescriptorPrivate)
{
}

NodeDescriptor::NodeDescriptor(const NodeDescriptor &other) :
    d(new NodeDescriptorPrivate(*other.d))
{
}

NodeDescriptor &NodeDescriptor::operator=(const NodeDescriptor &other)
{
    *this->d = *other.d;
    return *this;
}

NodeDescriptor::~NodeDescriptor()
{
    delete d;
    d = nullptr;
}

void NodeDescriptor::readFromStream(QDataStream &stream)
{
    d->m_isNull = 1;
    for (uint i = 0; i < sizeof(d->m_raw); i++)
    {
        if (stream.atEnd()) // error
        {
            memset(d->m_raw, 0 , sizeof(d->m_raw));
            d->m_deviceType = deCONZ::UnknownDevice;
            return;
        }
        stream >> d->m_raw[i]; // store raw data
    }

    d->m_isNull = 0;

    if      (d->m_raw[0] & 0x01) d->m_deviceType = deCONZ::Router;
    else if (d->m_raw[0] & 0x02) d->m_deviceType = deCONZ::EndDevice;
    else                         d->m_deviceType = deCONZ::Coordinator;

    get_u16_le(&d->m_raw[8], &d->m_serverMask);
}

DeviceType NodeDescriptor::deviceType() const
{
    return deCONZ::DeviceType(d->m_deviceType);
}

void NodeDescriptor::setDeviceType(deCONZ::DeviceType deviceType)
{
    d->m_deviceType = deviceType & 0x3;
    if (deviceType == deCONZ::Coordinator)
    {
        d->m_raw[0] &= ~(0x03) & 0xFF;
    }
    else if (deviceType == deCONZ::Router)
    {
        d->m_raw[0] &= ~(0x02) & 0xFF;
        d->m_raw[0] |= 0x01;
    }
    else if (deviceType == deCONZ::EndDevice)
    {
        d->m_raw[0] &= ~(0x01) & 0xFF;
        d->m_raw[0] |= 0x02;
    }
    else if (deviceType == deCONZ::UnknownDevice)
    {  }
    else
    {
        DBG_Assert(deviceType >= 0);
        DBG_Assert(deviceType <= deCONZ::UnknownDevice);
    }
}

uint16_t NodeDescriptor::manufacturerCode() const
{
    uint16_t mcode;
    get_u16_le(&d->m_raw[3], &mcode);
    return mcode;
}

ManufacturerCode_t NodeDescriptor::manufacturerCode_t() const
{
    return ManufacturerCode_t(manufacturerCode());
}


void NodeDescriptor::setManufacturerCode(quint16 code)
{
    put_u16_le(&d->m_raw[3], &code);
}

void NodeDescriptor::setManufacturerCode(ManufacturerCode_t code)
{
    setManufacturerCode(static_cast<quint16>(code));
}

MacCapabilities NodeDescriptor::macCapabilities() const
{
//    MacCapabilities cap(MacAlternatePanCoordinator |
//                        MacDeviceIsFFD |
//                        MacIsMainsPowered |
//                        MacReceiverOnWhenIdle |
//                        MacSecuritySupport |
//                        MacAllocateAddress);
    //return (cap & d->m_raw[2]);
    MacCapabilities cap(d->m_raw[2]);
    return cap;
}

void NodeDescriptor::setMacCapabilities(MacCapabilities cap)
{
    d->m_raw[2] = static_cast<quint8>(cap);
}

bool NodeDescriptor::hasComplexDescriptor() const
{
    return (d->m_raw[0] & 0x08);
}

void NodeDescriptor::setHasComplexDescriptor(bool hasComplex)
{
    if (hasComplex) d->m_raw[0] |=   0x08;
    else            d->m_raw[0] &= ~(0x08) & 0xFF;
}

bool NodeDescriptor::hasUserDescriptor() const
{
    return (d->m_raw[0] & 0x10);
}

void NodeDescriptor::setHasUserDescriptor(bool hasUser)
{
    if (hasUser) d->m_raw[0] |=   0x10;
    else         d->m_raw[0] &= ~(0x10) & 0xFF;
}

FrequencyBand NodeDescriptor::frequencyBand() const
{
    return (deCONZ::FrequencyBand)(d->m_raw[1] & 0x68);
}

void NodeDescriptor::setFrequenzyBand(FrequencyBand freq)
{
    d->m_raw[1] &= ~(0x68) & 0xFF;
    d->m_raw[1] |= static_cast<quint8>(freq);
}

QByteArray NodeDescriptor::toByteArray() const
{
    QByteArray data = QByteArray(13, 0);
    for (int i = 0; i < 13; i++)
    {
        data[i] = static_cast<char>(d->m_raw[i]);
    }
    return data;
}

unsigned NodeDescriptor::stackRevision() const
{
    unsigned result = d->m_serverMask >> 9;
    return result;
}

class PowerDescriptorPrivate
{
public:
    PowerDescriptorPrivate();
    QByteArray data;
    bool isValid;
    PowerMode currentMode;
    PowerSources availableSources;
    PowerSource currentSource;
    PowerSourceLevel currentLevel;
};

PowerDescriptorPrivate::PowerDescriptorPrivate()  :
    isValid(false),
    currentMode(ModeOnWhenIdle),
    currentSource(PowerSourceMains),
    currentLevel(PowerLevel100)
{

}

PowerDescriptor::PowerDescriptor() :
    d(new PowerDescriptorPrivate)
{
}

PowerDescriptor::PowerDescriptor(const PowerDescriptor &other) :
    d(new PowerDescriptorPrivate(*other.d))
{
}

PowerDescriptor &PowerDescriptor::operator=(const PowerDescriptor &other)
{
    *this->d = *other.d;
    return *this;
}

PowerDescriptor::PowerDescriptor(const QByteArray &data) :
    d(new PowerDescriptorPrivate)
{
    if (data.size() < 2)
    {
        d->isValid = false;
        return;
    }

    d->isValid = true;
    d->data = data.left(2);
    d->currentMode = (PowerMode)(data[0] & 0x0F);

    if (((data[0] & 0xF0) >> 4) & PowerSourceMains)
        d->availableSources |= PowerSourceMains;

    if (((data[0] & 0xF0) >> 4) & PowerSourceRechargeable)
        d->availableSources |= PowerSourceRechargeable;

    if (((data[0] & 0xF0) >> 4) & PowerSourceDisposable)
        d->availableSources |= PowerSourceDisposable;

    switch ((data[1] & 0x0F))
    {
    case PowerSourceMains:        d->currentSource = PowerSourceMains; break;
    case PowerSourceRechargeable: d->currentSource = PowerSourceRechargeable; break;
    case PowerSourceDisposable:  d->currentSource = PowerSourceDisposable; break;
    default:
        d->currentSource = PowerSourceUnknown;
        break;
    }

    d->currentLevel = (PowerSourceLevel)((data[1] & 0xF0) >> 4) ;
}

PowerDescriptor::~PowerDescriptor()
{
    delete d;
    d = nullptr;
}

PowerMode PowerDescriptor::currentPowerMode() const
{
    return d->currentMode;
}

PowerSources PowerDescriptor::availablePowerSources() const
{
    return d->availableSources;
}

PowerSource PowerDescriptor::currentPowerSource() const
{
    return d->currentSource;
}

PowerSourceLevel PowerDescriptor::currentPowerLevel() const
{
    return d->currentLevel;
}

bool PowerDescriptor::isValid() const
{
    return d->isValid;
}

QByteArray PowerDescriptor::toByteArray() const
{
    return d->data;
}

class SimpleDescriptorPrivate
{
public:
    SimpleDescriptorPrivate();
    quint8 m_endpoint;
    quint16 m_appProfileId;
    quint16 m_appDeviceId;
    quint8 m_appDeviceVersion;
    std::vector<ZclCluster> m_appInClusters;
    std::vector<ZclCluster> m_appOutClusters;
};

SimpleDescriptorPrivate::SimpleDescriptorPrivate() :
    m_endpoint(0xFF),
    m_appProfileId(0),
    m_appDeviceId(0),
    m_appDeviceVersion(0)
{

}

SimpleDescriptor::SimpleDescriptor() :
    d(new SimpleDescriptorPrivate)
{
}

SimpleDescriptor::SimpleDescriptor(const SimpleDescriptor &other)
    : d(new SimpleDescriptorPrivate(*other.d))
{
}

SimpleDescriptor &SimpleDescriptor::operator=(const SimpleDescriptor &other)
{
    *this->d = *other.d;
    return *this;
}

SimpleDescriptor::~SimpleDescriptor()
{
    delete d;
    d = nullptr;
}

void SimpleDescriptor::readFromStream(QDataStream &stream, quint16 mfcode)
{
    ZclDataBase *db = deCONZ::zclDataBase();
    stream >> d->m_endpoint;
    stream >> d->m_appProfileId;
    stream >> d->m_appDeviceId;
    stream >> d->m_appDeviceVersion;
    d->m_appDeviceVersion &= 0x0F; // kill reserved

    quint8 count;
    quint16 clusterId;

    // init server clusters
    stream >> count;

    if (stream.status() != QDataStream::Ok)
    {
        d->m_endpoint = 0xFF;
        return;
    }

    for (uint i = 0; i < count; i++)
    {
        stream >> clusterId;

        if (stream.status() == QDataStream::ReadPastEnd)
        {
            d->m_endpoint = 0xFF;
            return;
        }

        if (!cluster(clusterId, ServerCluster))
        {
            d->m_appInClusters.push_back(db->inCluster(profileId(), clusterId, mfcode));
        }
    }

    // init client clusters
    stream >> count;

    if (stream.status() == QDataStream::ReadPastEnd)
    {
        d->m_endpoint = 0xFF;
        return;
    }

    for (uint i = 0; i < count; i++)
    {
        stream >> clusterId;

        if (stream.status() == QDataStream::ReadPastEnd)
        {
            d->m_endpoint = 0xFF;
            return;
        }

        if (!cluster(clusterId, ClientCluster))
        {
            d->m_appOutClusters.push_back(db->outCluster(profileId(), clusterId, mfcode));
        }
    }

}

void SimpleDescriptor::writeToStream(QDataStream &stream) const
{
    stream << endpoint();
    stream << profileId();
    stream << deviceId();
    stream << deviceVersion();

    if (inClusters().size() < 0xFF)
    {
        stream << static_cast<quint8>(inClusters().size());

        for (const ZclCluster &cl : inClusters())
        {
            stream << cl.id();
        }
    }
    else
    {
        stream << static_cast<quint8>(0);
    }

    if (outClusters().size() < 0xFF)
    {
        stream << static_cast<quint8>(outClusters().size());

        for (const ZclCluster &cl : outClusters())
        {
            stream << cl.id();
        }
    }
    else
    {
        stream << static_cast<quint8>(0);
    }
}

uint8_t SimpleDescriptor::endpoint() const
{
    return d->m_endpoint;
}

void SimpleDescriptor::setEndpoint(uint8_t endpoint)
{
    d->m_endpoint = endpoint;
}

uint16_t SimpleDescriptor::profileId() const
{
    return d->m_appProfileId;
}

void SimpleDescriptor::setProfileId(uint16_t profileId)
{
    d->m_appProfileId = profileId;
}

uint16_t SimpleDescriptor::deviceId() const
{
    return d->m_appDeviceId;
}

void SimpleDescriptor::setDeviceId(uint16_t deviceId)
{
    d->m_appDeviceId = deviceId;
}

uint8_t SimpleDescriptor::deviceVersion() const
{
    return d->m_appDeviceVersion;
}

void SimpleDescriptor::setDeviceVersion(uint8_t version)
{
    d->m_appDeviceVersion = version;
}

bool SimpleDescriptor::isValid() const
{
    return (d->m_endpoint != 0xFF);
}

ZclCluster *SimpleDescriptor::cluster(uint16_t id, ZclClusterSide side)
{
    if (side == ServerCluster)
    {
        for (auto &cl : d->m_appInClusters)
        {
            if (cl.id() == id)
                return &cl;
        }
    }
    else
    {
        for (auto &cl : d->m_appOutClusters)
        {
            if (cl.id() == id)
                return &cl;
        }
    }

    return nullptr;
}

std::vector<ZclCluster> &SimpleDescriptor::inClusters()
{
    return d->m_appInClusters;
}

const std::vector<ZclCluster> &SimpleDescriptor::inClusters() const
{
    return d->m_appInClusters;
}

std::vector<ZclCluster> &SimpleDescriptor::outClusters()
{
    return d->m_appOutClusters;
}

const std::vector<ZclCluster> &SimpleDescriptor::outClusters() const
{
    return d->m_appOutClusters;
}

// FIXME: remove
std::vector<ZclCluster> &SimpleDescriptor::clusters(ZclClusterSide side)
{
    return (side == ServerCluster) ? inClusters() : outClusters();
}

// FIXME: remove
const std::vector<ZclCluster> &SimpleDescriptor::clusters(ZclClusterSide side) const
{
    return (side == ServerCluster) ? inClusters() : outClusters();
}

const char *NodeDescriptor::frequencyBandString() const
{
    static const char *freq2400 = "2400 - 2483.5 MHz";
    static const char *freq868 = "868 - 868.6 MHz";
    static const char *freq902 = "902 - 928 MHz";

    switch (frequencyBand())
    {
    case deCONZ::Freq2400: return freq2400;
    case deCONZ::Freq868: return freq868;
    case deCONZ::Freq902: return freq902;

    default:
        break;
    }

    return freq2400;
}

bool NodeDescriptor::isAlternatePanCoordinator() const
{
    return (d->m_raw[2] & 0x01);
}

void NodeDescriptor::setIsAlternatePanCoordinator(bool isAlt)
{
    if (isAlt) d->m_raw[2] |=   0x01;
    else       d->m_raw[2] &= ~(0x01) & 0xFF;
}

bool NodeDescriptor::isFullFunctionDevice() const
{
    return (d->m_raw[2] & 0x02);
}

void NodeDescriptor::setIsFFD(bool isFFD)
{
    if (isFFD) d->m_raw[2] |=   0x02;
    else       d->m_raw[2] &= ~(0x02) & 0xFF;
}

bool NodeDescriptor::isMainsPowered() const
{
    return (d->m_raw[2] & 0x04);
}

void NodeDescriptor::setIsMainsPowered(bool isMains)
{
    if (isMains) d->m_raw[2] |=   0x04;
    else         d->m_raw[2] &= ~(0x04) & 0xFF;
}

bool NodeDescriptor::receiverOnWhenIdle() const
{
    return (d->m_raw[2] & 0x08);
}

void NodeDescriptor::setRxOnWhenIdle(bool on)
{
    if (on) d->m_raw[2] |=   0x08;
    else    d->m_raw[2] &= ~(0x08) & 0xFF;
}

bool NodeDescriptor::securitySupport() const
{
    return (d->m_raw[2] & 0x40);
}

void NodeDescriptor::setSecuritySupport(bool supported)
{
    if (supported) d->m_raw[2] |=   0x40;
    else           d->m_raw[2] &= ~(0x40) & 0xFF;
}

bool NodeDescriptor::allocateAddress() const
{
    return (d->m_raw[2] & 0x80);
}

bool NodeDescriptor::hasEndpointList() const
{
    return (d->m_raw[12] & 0x01);
}

bool NodeDescriptor::hasSimpleDescriptorList() const
{
    return (d->m_raw[12] & 0x02);
}

const zme::NodeServerFlags NodeDescriptor::serverMask() const
{
    return zme::NodeServerFlag(d->m_serverMask);
}

uint8_t NodeDescriptor::maxBufferSize() const
{
    return d->m_raw[5];
}

uint16_t NodeDescriptor::maxIncomingTransferSize() const
{
    uint16_t size;
    get_u16_le(&d->m_raw[6], &size);
    return size;
}

uint16_t NodeDescriptor::maxOutgoingTransferSize() const
{
    uint16_t size;
    get_u16_le(&d->m_raw[10], &size);
    return size;
}

bool NodeDescriptor::isNull() const
{
    return d->m_isNull;
}

void NodeDescriptor::setIsNull(bool isNull)
{
    d->m_isNull = isNull;
}

} // namespace deCONZ
