/*
 * Copyright (c) 2012-2023 dresden elektronik ingenieurtechnik gmbh.
 * All rights reserved.
 *
 * The software in this package is published under the terms of the BSD
 * style license a copy of which has been included with this distribution in
 * the LICENSE.txt file.
 *
 */

#include <QDataStream>
#include <deconz/dbg_trace.h>
#include <deconz/touchlink.h>


namespace deCONZ {

class TouchlinkRequestPrivate
{
public:
    TouchlinkRequestPrivate();
    uint32_t transactionId;
    uint8_t txOptions;
    Address addr;
    ApsAddressMode addrMode;
    uint8_t channel;
    uint16_t panId;
    uint16_t profileId;
    uint16_t clusterId;
    QByteArray asdu;
};

TouchlinkRequestPrivate::TouchlinkRequestPrivate() :
    transactionId(0),
    txOptions(0),
    addrMode(ApsNoAddress),
    channel(0),
    panId(0),
    profileId(0),
    clusterId(0)
{

}

TouchlinkRequest::TouchlinkRequest() :
    d_ptr(new TouchlinkRequestPrivate)
{

}

TouchlinkRequest::TouchlinkRequest(const TouchlinkRequest &other) :
    d_ptr(new TouchlinkRequestPrivate(*other.d_ptr))
{
    DBG_Assert(other.d_ptr != 0);
}

TouchlinkRequest& TouchlinkRequest::operator= (const TouchlinkRequest &other)
{
    // Self assignment?
    if (this == &other)
    {
        return *this;
    }

    DBG_Assert(this->d_ptr != 0);
    DBG_Assert(other.d_ptr != 0);
    *this->d_ptr = *other.d_ptr;
    return *this;
}

TouchlinkRequest::~TouchlinkRequest()
{
    delete d_ptr;
    d_ptr = 0;
}

uint32_t TouchlinkRequest::transactionId() const
{
    Q_D(const TouchlinkRequest);
    return d->transactionId;
}

void TouchlinkRequest::setTransactionId(uint32_t transactionId)
{
    Q_D(TouchlinkRequest);
    d->transactionId = transactionId;
}

Address &TouchlinkRequest::dstAddress()
{
    Q_D(TouchlinkRequest);
    return d->addr;
}

const Address &TouchlinkRequest::dstAddress() const
{
    Q_D(const TouchlinkRequest);
    return d->addr;
}

ApsAddressMode TouchlinkRequest::dstAddressMode() const
{
    Q_D(const TouchlinkRequest);
    return d->addrMode;
}

void TouchlinkRequest::setDstAddressMode(ApsAddressMode mode)
{
    DBG_Assert((mode == ApsNwkAddress) || (mode == ApsExtAddress));

    Q_D(TouchlinkRequest);
    d->addrMode = mode;
}

uint8_t TouchlinkRequest::channel() const
{
    Q_D(const TouchlinkRequest);
    return d->channel;
}

void TouchlinkRequest::setChannel(uint8_t channel)
{
    DBG_Assert((channel >= 11) && (channel <= 26));

    Q_D(TouchlinkRequest);
    d->channel = channel;
}

uint16_t TouchlinkRequest::panId() const
{
    Q_D(const TouchlinkRequest);
    return d->panId;
}

void TouchlinkRequest::setPanId(uint16_t panId)
{
    Q_D(TouchlinkRequest);
    d->panId = panId;
}

uint16_t TouchlinkRequest::profileId() const
{
    Q_D(const TouchlinkRequest);
    return d->profileId;
}

void TouchlinkRequest::setProfileId(uint16_t profileId)
{
    Q_D(TouchlinkRequest);
    d->profileId = profileId;
}

uint16_t TouchlinkRequest::clusterId() const
{
    Q_D(const TouchlinkRequest);
    return d->clusterId;
}

void TouchlinkRequest::setClusterId(uint16_t clusterId)
{
    Q_D(TouchlinkRequest);
    d->clusterId = clusterId;
}

bool TouchlinkRequest::writeToStream(QDataStream &stream) const
{
    Q_D(const TouchlinkRequest);
    DBG_Assert(transactionId() != 0);

    if (transactionId() == 0)
    {
        return false;
    }

    DBG_Assert((dstAddress().hasExt() && dstAddressMode() == ApsExtAddress) ||
               (dstAddress().hasNwk() && dstAddressMode() == ApsNwkAddress));

    if (!((dstAddress().hasExt() && dstAddressMode() == ApsExtAddress) ||
          (dstAddress().hasNwk() && dstAddressMode() == ApsNwkAddress)))
    {
        return false;
    }

    stream << transactionId();
    stream << d->txOptions;
    stream << (quint8)dstAddressMode();

    if (dstAddressMode() == ApsExtAddress)
    {
        stream << (quint64)dstAddress().ext();
    }
    else if (dstAddressMode() == ApsNwkAddress)
    {
        stream << dstAddress().nwk();
    }

    stream << panId();
    stream << profileId();
    stream << clusterId();

    stream << (quint8)d->asdu.size();

    for (int i = 0; i < d->asdu.size(); i++)
    {
        stream << (quint8)d->asdu[i];
    }

    return true;
}

const QByteArray &TouchlinkRequest::asdu() const
{
    Q_D(const TouchlinkRequest);
    return d->asdu;
}

QByteArray &TouchlinkRequest::asdu()
{
    Q_D(TouchlinkRequest);
    return d->asdu;
}

void TouchlinkRequest::setAsdu(const QByteArray &asdu)
{
    Q_D(TouchlinkRequest);
    d->asdu = asdu;
}

} // namespace deCONZ
