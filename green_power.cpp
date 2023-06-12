/*
 * Copyright (c) 2012-2023 dresden elektronik ingenieurtechnik gmbh.
 * All rights reserved.
 *
 * The software in this package is published under the terms of the BSD
 * style license a copy of which has been included with this distribution in
 * the LICENSE.txt file.
 *
 */

#include <QDebug>
#include <QByteArray>
#include <QDataStream>
#include "deconz/dbg_trace.h"
#include "deconz/green_power.h"

namespace deCONZ
{

enum Constants
{
    GP_NwkProtocolVersion = 3,
    GP_NwkDataFrame = 0,
    GP_NwkMaintenanceFrame = 1,
    GP_MinMsduSize = 1,
    GP_AutoCommissioningFlag = (1 << 6),
    GP_NwkFrameControlExtensionFlag = (1 << 7)
};

class GpDataIndicationPrivate
{
public:
    GpDataIndicationPrivate();

    quint8 nwkFrameControl = 0;

    struct NwkExtFrameControlBits {
        unsigned int applicationId :3;
        unsigned int securityLevel :2;
        unsigned int securityKey   :1;
        unsigned int rxAfterTx     :1;
        unsigned int direction     :1;
    };

    union NwkExtFrameControl
    {
        quint8 byte;
        NwkExtFrameControlBits bits;
    } nwkExtFrameControl;

    quint32 gpdSrcId = 0;
    quint32 frameCounter = 0;
    quint8 gpdCommandId = 0;
    QByteArray gpdCommandPayload;
    quint16 gppNwkAddress = 0;
    quint8 gpGpdLink = 0;
};

GpDataIndicationPrivate::GpDataIndicationPrivate()
{
    nwkExtFrameControl.byte = 0;
}

GpDataIndication::GpDataIndication() :
    d_ptr(new GpDataIndicationPrivate)
{
}

GpDataIndication::GpDataIndication(const GpDataIndication &other) :
    d_ptr(new GpDataIndicationPrivate(*other.d_ptr))
{
}

GpDataIndication &GpDataIndication::operator=(const GpDataIndication &other)
{
    // Self assignment?
    if (this==&other)
    {
        return *this;
    }

    *this->d_ptr = *other.d_ptr;
    return *this;
}

GpDataIndication::~GpDataIndication()
{
    delete d_ptr;
    d_ptr = 0;
}

bool GpDataIndication::readFromStream(QDataStream &stream)
{
    Q_D(GpDataIndication);

    if (stream.atEnd()) { return false; }
    stream >> d->nwkFrameControl;

    // check frame type
    quint8 frameType = d->nwkFrameControl & 0x03;

    if ((frameType != GP_NwkDataFrame) &&
        (frameType != GP_NwkMaintenanceFrame))
    {
        return false;
    }

    // check green power protocol version
    if (((d->nwkFrameControl >> 2)& 0x03) != GP_NwkProtocolVersion)
    {
        return false;
    }

    // extended frame control
    if (d->nwkFrameControl & GP_NwkFrameControlExtensionFlag)
    {
        if (stream.atEnd()) { return false; }
        stream >> d->nwkExtFrameControl.byte;
    }
    else
    {
        d->nwkExtFrameControl.byte = 0;
    }

    // check supported application IDs
    switch (d->nwkExtFrameControl.bits.applicationId)
    {
    case 0: // 0b000 (GP)
    case 2: // 0b010 (GP)
    case 1: // 0b001 (LPED)
        break;

    default:
        return false;
    }

    // GPD SrcID field
    if ((frameType == GP_NwkDataFrame) && (d->nwkExtFrameControl.bits.applicationId == 0))
    {
        if (stream.atEnd()) { return false; }
        stream >> d->gpdSrcId;
    }
    else if ((frameType == GP_NwkMaintenanceFrame) && (d->nwkFrameControl & GP_NwkFrameControlExtensionFlag) && (d->nwkExtFrameControl.bits.applicationId == 0))
    {
        if (stream.atEnd()) { return false; }
        stream >> d->gpdSrcId;
    }
    else
    {
        d->gpdSrcId = 0; // unspecified
    }

    // frame counter filed
    // The presence and length of the Security frame counter field is dependent on the value of ApplicationID
    // and SecurityLevel (see A.1.4.1.3).
    d->frameCounter = 0;
    if (d->nwkFrameControl & GP_NwkFrameControlExtensionFlag)
    {
        switch (d->nwkExtFrameControl.bits.applicationId)
        {
        case 0: // 0b000 (GP)
        case 2: // 0b010 (GP)
        {
            switch (d->nwkExtFrameControl.bits.securityLevel)
            {
            case 0: // 0b00 No security
            case 1: // 0b01 1LSB of frame counter and short (2B) MIC only
                break;

            case 2: // 0b10 Full (4B) frame counter and full (4B) MIC only
            case 3: // 0b11 Encryption & full (4B) frame counter and full (4B) MIC
            {
                if (stream.atEnd()) { return false; }
                stream >> d->frameCounter;
            }
                break;

            default:
                break;
            }
        }
            break;

        case 1: // 0b001 (LPED)
            break;

        default:
            break;
        }
    }

    d->gpdCommandPayload.clear();

    switch (d->nwkExtFrameControl.bits.applicationId)
    {
    case 0: // 0b000 (GP)
    case 2: // 0b010 (GP)
    {
        if (stream.atEnd()) { return false; }
        stream >> d->gpdCommandId;

        // TODO remove trailing MIC and non payload fields
        quint8 byte;
        while (!stream.atEnd())
        {
            stream >> byte;
            d->gpdCommandPayload.append(byte);
        }
    }
        break;

    default:
        d->gpdCommandId = 0;
        break;
    }

    return true;
}

bool GpDataIndication::readFromStreamGpNotification(QDataStream &stream)
{
    Q_D(GpDataIndication);

    quint16 options;

    stream >> options;

    if ((options & 0x3) == 0)
    {
        stream >> d->gpdSrcId;
    }
    else
    {
        return false; // not supported
    }

    stream >> d->frameCounter;
    stream >> d->gpdCommandId;

    quint8 payloadLength;
    stream >> payloadLength;

    while (!stream.atEnd() && payloadLength > 0 && payloadLength < 0xff)
    {
        quint8 byte;
        stream >> byte;
        payloadLength--;
        d->gpdCommandPayload.append(byte);
    }

    if (stream.status() != QDataStream::ReadPastEnd) // assume proxy info present, spec is not clear
    {
        stream >> d->gppNwkAddress;
        stream >> d->gpGpdLink;
    }

    return stream.status() != QDataStream::ReadPastEnd;
}

quint32 GpDataIndication::gpdSrcId() const
{
    Q_D(const GpDataIndication);
    return d->gpdSrcId;
}

quint8 GpDataIndication::gpdCommandId() const
{
    Q_D(const GpDataIndication);
    return d->gpdCommandId;
}

quint32 GpDataIndication::frameCounter() const
{
    Q_D(const GpDataIndication);
    return d->frameCounter;
}

const QByteArray &GpDataIndication::payload() const
{
    Q_D(const GpDataIndication);
    return d->gpdCommandPayload;
}

quint16 GpDataIndication::gppShortAddress() const
{
    Q_D(const GpDataIndication);
    return d->gppNwkAddress;
}

quint8 GpDataIndication::gppGpdLink() const
{
    Q_D(const GpDataIndication);
    return d->gpGpdLink;
}

qint8 GpDataIndication::gppRssi() const
{
    Q_D(const GpDataIndication);

    qint8 result = INT8_MIN;

    if (d->gpGpdLink)
    {
        /*
            - The RSSI parameter value as supplied by the dGP-DATA.indication primitive SHALL be capped
              to the range <+8 ; -109> [dBm], i.e. any value higher than +8dBm is represented as +8 dBm; any value lower than -109dBm is
              represented as -109dBm, the values within the range remain unmodified;
            - 110 SHALL be added to the capped RSSI value, to obtain a non-negative value;
            - The obtained non-negative RSSI value SHALL be divided by 2.
        */
        int rssi = d->gpGpdLink & 0x3f; // bits 0-5
        rssi *= 2;
        rssi -= 110;
        DBG_Assert(rssi >= INT8_MIN);
        DBG_Assert(rssi <= INT8_MAX);
        if (rssi >= INT8_MIN && rssi <= INT8_MAX)
        {
            result = static_cast<qint8>(rssi);
        }
    }

    return result;
}

GppGpdLqi GpDataIndication::gppLqi() const
{
    Q_D(const GpDataIndication);
    return static_cast<GppGpdLqi>((d->gpGpdLink >> 6) & 0x3);
}

} // namespace deCONZ
