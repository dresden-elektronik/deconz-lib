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
#include <array>
#include "deconz/dbg_trace.h"
#include "deconz/aps.h"
#include "aps_private.h"
#include "deconz/mem_pool.h"

namespace deCONZ
{

const char *ApsStatusToString(unsigned char status)
{
    switch (status)
    {
    case ApsSuccessStatus:               return "SUCCESS";
    case ApsAsduTooLongStatus:           return "ASDU_TOO_LONG";
    case ApsDefragDeferredStatus:        return "DEFRAG_DEFERRED";
    case ApsDefragUnsupportedStatus:     return "DEFRAG_UNSUPPORTED";
    case ApsIllegalRequestStatus:        return "ILLEGAL_REQUEST";
    case ApsInvalidBindingStatus:        return "INVALID_BINDING";
    case ApsInvalidGroupStatus:          return "INVALID_GROUP";
    case ApsInvalidParameterStatus:      return "INVALID_PARAMETER";
    case ApsNoAckStatus:                 return "NO_ACK";
    case ApsNoBoundDeviceStatus:         return "NO_BOUND_DEVICE";
    case ApsNoShortAddressStatus:        return "NO_SHORT_ADDRESS";
    case ApsNotSupportedStatus:          return "NOT_SUPPORTED";
    case ApsSecuredLinkKeyStatus:        return "SECURED_LINK_KEY";
    case ApsSecuredNwkKeyStatus:         return "SECURED_NWK_KEY";
    case ApsSecurityFailStatus:          return "SECURITY_FAIL";
    case ApsTableFullStatus:             return "TABLE_FULL";
    case ApsUnsecuredStatus:             return "UNSECURED";
    case ApsUnsupportedAttributeStatus:  return "UNSUPPORTED_ATTRIBUTE";
    case MacInvalidParameterStatus:      return "INVALID_PARAMETER";
    case MacNoAckStatus:                 return "MAC_NO_ACK";
    case MacNoBeaconStatus:              return "NO_BEACON";
    case MacTransactionExpiredStatus:    return "TRANSACTION_EXPIRED";
    default:
        break;
    }

    return "";
}

class ApsDataRequestPrivate
{
public:
    static constexpr int PoolSize = 16; // for ApsMemory

    Address dstAddr;
    std::array<quint16, 9> sourceRoute{};
    ApsAddressMode dstAddrMode = ApsNoAddress;
    deCONZ::SteadyTimeRef sendAfter;
    deCONZ::SteadyTimeRef timeout;
    QByteArray asdu;
    uint sourceRouteUuidHash = 0;
#if QT_VERSION < QT_VERSION_CHECK(5,14,0)
    ApsTxOptions txOptions = nullptr;
#else
    ApsTxOptions txOptions = {};
#endif
    int sendDelay = 0;
    uint16_t profileId = 0xFFFF;
    uint16_t clusterId = 0xFFFF;
    uint16_t responseClusterId = 0xFFFF;
    uint16_t nodeId = APS_INVALID_NODE_ID;
    CommonState state = IdleState;
    uint8_t dstEndpoint = 0xFF;
    uint8_t srcEndpoint = 0xFF;
    uint8_t id = 0;
    uint8_t version = 1;
    uint8_t radius = 0;
    uint8_t relayCount = 0;

    // service
    bool confirmed = false;
};

static ApsMemory *apsMem = nullptr;
static ApsMemoryPrivate *apsMemPriv = nullptr;

class ApsMemoryPrivate
{
public:
    ~ApsMemoryPrivate();

    std::tuple<
        std::array<ApsDataRequestPrivate*, ApsDataRequestPrivate::PoolSize>
    > mem{};
};

ApsMemoryPrivate::~ApsMemoryPrivate()
{
    for (auto &i : MEM_GetAllocContainer<ApsDataRequestPrivate>(apsMemPriv->mem))
    {
        if (i) { delete i; }
    }
}

ApsMemory::ApsMemory() :
    d(new ApsMemoryPrivate)
{
    Q_ASSERT_X(apsMem == nullptr, "ApsMemory::ApsMemory()", "Already initialized");
    apsMem = this; // singleton
    apsMemPriv = d; // quick ref
}

ApsMemory::~ApsMemory()
{
    Q_ASSERT(apsMem);
    delete d;
    apsMem = nullptr;
    apsMemPriv = nullptr;
}

ApsDataRequest::ApsDataRequest()
{
    d_ptr = MEM_AllocItem<ApsDataRequestPrivate>(&apsMemPriv->mem);
    *d_ptr = {};
    d_ptr->id = APS_NextApsRequestId();
}

ApsDataRequest::ApsDataRequest(const ApsDataRequest &other)
{
    d_ptr = MEM_AllocItem<ApsDataRequestPrivate>(&apsMemPriv->mem);
    *d_ptr = *other.d_ptr;
}

ApsDataRequest::ApsDataRequest(ApsDataRequest &&other) noexcept :
    d_ptr(other.d_ptr)
{
    Q_ASSERT(&other != this);
    other.d_ptr = nullptr;
    Q_ASSERT(d_ptr);
}

ApsDataRequest& ApsDataRequest::operator=(const ApsDataRequest &other)
{
    // Self assignment?
    if (this == &other)
    {
        return *this;
    }

    Q_ASSERT(d_ptr);
    Q_ASSERT(other.d_ptr);
    *d_ptr = *other.d_ptr;
    return *this;
}

ApsDataRequest& ApsDataRequest::operator=(ApsDataRequest &&other) noexcept
{
    // Self assignment?
    if (this == &other)
    {
        return *this;
    }

    Q_ASSERT(other.d_ptr);

    if (d_ptr)
    {
        MEM_DeallocItem<ApsDataRequestPrivate>(d_ptr, &apsMemPriv->mem);
        d_ptr = nullptr;
    }

    d_ptr = other.d_ptr;
    other.d_ptr = nullptr;
    Q_ASSERT(d_ptr);
    return *this;
}

ApsDataRequest::~ApsDataRequest()
{
    if (d_ptr)
    {
        MEM_DeallocItem<ApsDataRequestPrivate>(d_ptr, &apsMemPriv->mem);
        d_ptr = nullptr;
    }
}

uint8_t ApsDataRequest::id() const
{
    return d_ptr->id;
}

Address &ApsDataRequest::dstAddress()
{
    return d_ptr->dstAddr;
}

const Address &ApsDataRequest::dstAddress() const
{
    return d_ptr->dstAddr;
}

ApsAddressMode ApsDataRequest::dstAddressMode() const
{
    return d_ptr->dstAddrMode;
}

void ApsDataRequest::setDstAddressMode(ApsAddressMode mode)
{
    d_ptr->dstAddrMode = mode;
}

uint8_t ApsDataRequest::srcEndpoint() const
{
    return d_ptr->srcEndpoint;
}

void ApsDataRequest::setSrcEndpoint(uint8_t ep)
{
    d_ptr->srcEndpoint = ep;
}

uint8_t ApsDataRequest::dstEndpoint() const
{
    return d_ptr->dstEndpoint;
}

void ApsDataRequest::setDstEndpoint(uint8_t ep)
{
    d_ptr->dstEndpoint = ep;
}

uint16_t ApsDataRequest::profileId() const
{
    return d_ptr->profileId;
}

void ApsDataRequest::setProfileId(uint16_t profileId)
{
    d_ptr->profileId = profileId;
}

uint16_t ApsDataRequest::clusterId() const
{
    return d_ptr->clusterId;
}

void ApsDataRequest::setClusterId(uint16_t clusterId)
{
    d_ptr->clusterId = clusterId;
}

uint16_t ApsDataRequest::responseClusterId() const
{
    return d_ptr->responseClusterId;
}

void ApsDataRequest::setResponseClusterId(uint16_t clusterId)
{
    d_ptr->responseClusterId = clusterId;
}

const QByteArray &ApsDataRequest::asdu() const
{
    return d_ptr->asdu;
}

QByteArray &ApsDataRequest::asdu()
{
    return d_ptr->asdu;
}

void ApsDataRequest::setAsdu(const QByteArray &asdu)
{
    d_ptr->asdu = asdu;
}

uint8_t ApsDataRequest::radius() const
{
    return d_ptr->radius;
}

void ApsDataRequest::setRadius(uint8_t radius)
{
    d_ptr->radius = radius;
}

ApsTxOptions ApsDataRequest::txOptions() const
{
    return d_ptr->txOptions;
}

void ApsDataRequest::setTxOptions(ApsTxOptions txOptions)
{
    d_ptr->txOptions = txOptions;
}

int ApsDataRequest::writeToStream(QDataStream &stream) const
{
    uint8_t flags = 0;

    stream << id();

    if (version() > 1)
    {
        if (nodeId() != APS_INVALID_NODE_ID)
        {
            flags |= 0x01; // include node id
        }

        if (d_ptr->relayCount > 0)
        {
            flags |= 0x02;
        }

        stream << flags; // flags are supported since version 2
    }

    if (flags & 0x01)
    {
        stream << nodeId();
    }

    stream << (uint8_t)dstAddressMode();
    switch (dstAddressMode())
    {
    case ApsNoAddress:
    {
        DBG_Printf(DBG_APS, "write APS.req no address mode specified\n");
        return 0;
    }
        break;

    case ApsGroupAddress:
        if (!dstAddress().hasGroup())
        {
            DBG_Printf(DBG_APS, "write APS.req no group address\n");
            return 0;
        }
        stream << dstAddress().group();
        break;

    case ApsNwkAddress:
        if (!dstAddress().hasNwk())
        {
            DBG_Printf(DBG_APS, "write APS.req no nwk address\n");
            return 0;
        }
        stream << dstAddress().nwk();
        stream << dstEndpoint();
        break;

    case ApsExtAddress:
        if (!dstAddress().hasExt())
        {
            DBG_Printf(DBG_APS, "write APS.req no ext address\n");
            return 0;
        }
        stream << (quint64)dstAddress().ext();
        stream << dstEndpoint();
        break;

    default:
        DBG_Printf(DBG_APS, "write APS.req invalid address mode\n");
        return 0;
    }

    stream << profileId();
    stream << clusterId();
    stream << srcEndpoint();
    stream << (uint16_t)asdu().size();
    for (int i = 0; i < asdu().size(); i++)
    {
        stream << (uint8_t)asdu()[i];
    }
    stream << (uint8_t)static_cast<int>(txOptions());
    stream << radius();

    if (flags & 0x02)
    {
        stream << d_ptr->relayCount;

        for (uint8_t i = 0; i < d_ptr->relayCount; i++)
        {
            stream << d_ptr->sourceRoute.at(i);
        }
    }

    return 1;
}

void ApsDataRequest::readFromStream(QDataStream &stream)
{
    quint8 u8;
    quint16 u16;
    quint64 u64;

    stream >> d_ptr->id;

    stream >> u8; // dst address mode
    d_ptr->dstAddrMode = (ApsAddressMode)u8;

    switch (u8)
    {
    case ApsNoAddress:
        break;

    case ApsGroupAddress:
        stream >> u16;
        dstAddress().setGroup(u16);
        break;

    case ApsNwkAddress:
        stream >> u16;
        dstAddress().setNwk(u16);
        stream >> u8;
        setDstEndpoint(u8);
        break;

    case ApsExtAddress:
        stream >> u64;
        dstAddress().setExt(u64);
        stream >> u8;
        setDstEndpoint(u8);
        break;

    default:
        return;
    }

    stream >> u16;
    setProfileId(u16);
    stream >> u16;
    setClusterId(u16);
    stream >> u8;
    setSrcEndpoint(u8);

    stream >> u16; // asdu size
    QByteArray asdu((int)u16, 0x00);
    for (quint16 i = 0; i < u16; i++)
    {
        stream >> u8;
        asdu[i] = u8;
    }

    setAsdu(asdu);
    stream >> u8;
    //ApsTxOptions txOptions(ApsTxAcknowledgedTransmission | ApsTxFragmentationPermitted | ApsTxSecurityEnabledTransmission | ApsTxUseNwk);
    u8 &= 0x0F;

    // fugly but can't cast to QFlags
    for (uint8_t i = 0; i < 4; i++)
    {
        ApsTxOption opt = (ApsTxOption)(1 << i);
        bool set = (u8 & (1 << i)) != 0;
        d_ptr->txOptions.setFlag(opt, set);
    }

    stream >> u8;
    setRadius(u8);
}

void ApsDataRequest::clear()
{
    d_ptr->sourceRoute = {};
    d_ptr->sourceRouteUuidHash = 0;
    dstAddress().clear();
    asdu().clear();
}

void ApsDataRequest::setSendAfter(SteadyTimeRef t)
{
    d_ptr->sendAfter = t;
}

SteadyTimeRef ApsDataRequest::sendAfter() const
{
    return d_ptr->sendAfter;
}

uint8_t ApsDataRequest::version() const
{
    return d_ptr->version;
}

void ApsDataRequest::setVersion(uint8_t version) const
{
    d_ptr->version = version;
}

deCONZ::SteadyTimeRef ApsDataRequest::timeout() const
{
    return d_ptr->timeout;
}

uint16_t ApsDataRequest::nodeId() const
{
    return d_ptr->nodeId;
}

void ApsDataRequest::setNodeId(uint16_t id) const
{
    d_ptr->nodeId = id;
}

void ApsDataRequest::setTimeout(deCONZ::SteadyTimeRef timeout)
{
    d_ptr->timeout = timeout;
}

CommonState ApsDataRequest::state() const
{
    return d_ptr->state;
}

void ApsDataRequest::setState(CommonState state)
{
    if (state == deCONZ::FireAndForgetState)
    {
        DBG_Assert(state != deCONZ::FireAndForgetState); // warn in logs deprecated
        state = deCONZ::IdleState;
    }

    d_ptr->state = state;
}

int ApsDataRequest::sendDelay() const
{
    return d_ptr->sendDelay;
}

void ApsDataRequest::setSendDelay(int delayMs)
{
    d_ptr->sendDelay = delayMs;
}

bool ApsDataRequest::confirmed() const
{
    return d_ptr->confirmed;
}

void ApsDataRequest::setConfirmed(bool confirmed)
{
    d_ptr->confirmed = confirmed;
}

void ApsDataRequest::setSourceRoute(const std::array<quint16, 9> &relays, size_t size, const uint srHash)
{
    DBG_Assert(relays.size() <= d_ptr->sourceRoute.size());

    if (relays.size() <= d_ptr->sourceRoute.size())
    {
        d_ptr->relayCount = static_cast<uint8_t>(size);

        for (size_t i = 0; i < size; i++)
        {
            d_ptr->sourceRoute[i] = relays.at(i);
        }
        d_ptr->sourceRouteUuidHash = srHash;
    }
    else
    {
        d_ptr->sourceRouteUuidHash = 0;
    }
}

uint ApsDataRequest::sourceRouteUuidHash() const
{
    return d_ptr->sourceRouteUuidHash;
}

ApsDataConfirm::ApsDataConfirm(uint8_t reqId, uint8_t status) :
    m_id(reqId),
    m_status(status)
{
}

/*! Constructor for error reporting */
ApsDataConfirm::ApsDataConfirm(const ApsDataRequest &req, uint8_t status) :
    m_dstAddr(req.dstAddress()),
    m_dstAddrMode(req.dstAddressMode()),
    m_id(req.id()),
    m_dstEndpoint(req.dstEndpoint()),
    m_srcEndpoint(req.srcEndpoint()),
    m_status(status)
{
}

uint8_t ApsDataConfirm::id() const
{
    return m_id;
}

void ApsDataConfirm::setId(uint8_t id)
{
    m_id = id;
}

Address &ApsDataConfirm::dstAddress()
{
    return m_dstAddr;
}

const Address &ApsDataConfirm::dstAddress() const
{
    return m_dstAddr;
}

ApsAddressMode ApsDataConfirm::dstAddressMode() const
{
    return m_dstAddrMode;
}

void ApsDataConfirm::setDstAddressMode(ApsAddressMode mode)
{
    m_dstAddrMode = mode;
}

uint8_t ApsDataConfirm::dstEndpoint() const
{
    return m_dstEndpoint;
}

uint8_t ApsDataConfirm::srcEndpoint() const
{
    return m_srcEndpoint;
}

uint8_t ApsDataConfirm::status() const
{
    return m_status;
}

uint32_t ApsDataConfirm::txTime() const
{
    return ~0; // not used
}

void ApsDataConfirm::readFromStream(QDataStream &stream)
{
    // id from request
    stream >> m_id;

    {
        quint8  u8;
        stream >> u8;
        m_dstAddrMode = (ApsAddressMode)u8;
    }

    switch (m_dstAddrMode)
    {
    case ApsNwkAddress:
    {
        quint16 u16;
        stream >> u16;
        m_dstAddr.setNwk(u16);
        stream >> m_dstEndpoint;
    }
        break;

    case ApsGroupAddress:
    {
        quint16 u16;
        stream >> u16;
        m_dstAddr.setGroup(u16);
    }
        break;

    case ApsExtAddress:
    {
        quint64 u64;
        stream >> u64;
        m_dstAddr.setExt(u64);
        stream >> m_dstEndpoint;
    }
        break;

    case ApsNoAddress:
        break;

    default:
        break;
    }

    stream >> m_srcEndpoint;
    stream >> m_status;

    DBG_Assert(stream.status() == QDataStream::Ok && "read APS confirm invalid");

//    quint32 txTime_;
//    stream >> txTime_;
//    Q_UNUSED(txTime_)
}

class ApsDataIndicationPrivate
{
public:
    ApsDataIndicationPrivate();
    ApsDataIndicationPrivate(const ApsDataIndicationPrivate &other);
    ApsDataIndicationPrivate &operator=(const ApsDataIndicationPrivate &other);
    ApsAddressMode dstAddrMode = ApsNoAddress;
    Address dstAddr{};
    uint8_t dstEndpoint = 0xFF;
    ApsAddressMode srcAddrMode = ApsNoAddress;
    Address srcAddr{};
    uint8_t srcEndpoint = 0xFF;
    uint16_t profileId = 0xFFFF;
    uint16_t clusterId = 0xFFFF;
    QByteArray asdu;
    quint16 previousHop = 0xFFFF;
    uint8_t status = 0xFF;
    uint8_t securityStatus = 0xFF;
    uint8_t linkQuality = 0xFF;
    uint32_t rxTime = 0;
    int8_t rssi = 0;
    int version = 1;
    std::array<uint8_t, 118> asduBuf;
    void reset();
};

ApsDataIndicationPrivate::ApsDataIndicationPrivate() :
    dstEndpoint(0xFF),
    srcEndpoint(0xFF),
    profileId(0xFFFF),
    clusterId(0xFFFF),
    previousHop(0xFFFF),
    status(0xFF),
    securityStatus(0xFF),
    linkQuality(0xFF),
    rxTime(0x0000),
    rssi(0),
    version(1)
{

}

ApsDataIndicationPrivate::ApsDataIndicationPrivate(const ApsDataIndicationPrivate &other)
{
    dstAddrMode = other.dstAddrMode;
    dstAddr = other.dstAddr;
    dstEndpoint = other.dstEndpoint;

    srcAddrMode = other.srcAddrMode;
    srcAddr = other.srcAddr;
    srcEndpoint = other.srcEndpoint;

    profileId = other.profileId;
    clusterId = other.clusterId;

    previousHop = other.previousHop;
    status = other.status;
    securityStatus = other.securityStatus;
    linkQuality = other.linkQuality;
    rxTime = other.rxTime;
    rssi = other.rssi;
    version = other.version;
    asduBuf = other.asduBuf;

    asdu.setRawData(reinterpret_cast<char*>(asduBuf.data()), other.asdu.size());
}

ApsDataIndicationPrivate &ApsDataIndicationPrivate::operator=(const ApsDataIndicationPrivate &other)
{
    if (&other == this)
    {
        return *this;
    }

    dstAddrMode = other.dstAddrMode;
    dstAddr = other.dstAddr;
    dstEndpoint = other.dstEndpoint;

    srcAddrMode = other.srcAddrMode;
    srcAddr = other.srcAddr;
    srcEndpoint = other.srcEndpoint;

    profileId = other.profileId;
    clusterId = other.clusterId;

    previousHop = other.previousHop;
    status = other.status;
    securityStatus = other.securityStatus;
    linkQuality = other.linkQuality;
    rxTime = other.rxTime;
    rssi = other.rssi;
    version = other.version;
    asduBuf = other.asduBuf;

    asdu.setRawData(reinterpret_cast<char*>(asduBuf.data()), other.asdu.size());
    return *this;
}

void ApsDataIndicationPrivate::reset()
{
    dstAddr = {};
    srcAddr = {};
    dstEndpoint = 0xFF;
    srcEndpoint = 0xFF;
    profileId = 0xFFFF;
    clusterId = 0xFFFF;
    previousHop = 0xFFFF;
    status = 0xFF;
    securityStatus = 0xFF;
    linkQuality = 0xFF;
    rxTime = 0x0000;
    rssi = 0;
    version = 1;
}

ApsDataIndication::ApsDataIndication() :
    d_ptr(new ApsDataIndicationPrivate)
{
}

ApsDataIndication::ApsDataIndication(const ApsDataIndication &other) :
    d_ptr(new ApsDataIndicationPrivate(*other.d_ptr))
{
}

ApsDataIndication &ApsDataIndication::operator=(const ApsDataIndication &other)
{
    // Self assignment?
    if (this==&other)
    {
        return *this;
    }

    Q_ASSERT(d_ptr);
    Q_ASSERT(other.d_ptr);
    *d_ptr = *other.d_ptr;
    return *this;
}

ApsDataIndication::~ApsDataIndication()
{
    Q_ASSERT(d_ptr);
    delete d_ptr;
    d_ptr = nullptr;
}

ApsAddressMode ApsDataIndication::dstAddressMode() const
{
    return d_ptr->dstAddrMode;
}

void ApsDataIndication::setDstAddressMode(ApsAddressMode mode)
{
    d_ptr->dstAddrMode = mode;
}

Address &ApsDataIndication::dstAddress()
{
    return d_ptr->dstAddr;
}

const Address &ApsDataIndication::dstAddress() const
{
    return d_ptr->dstAddr;
}

uint8_t ApsDataIndication::dstEndpoint() const
{
    return d_ptr->dstEndpoint;
}

void ApsDataIndication::setDstEndpoint(uint8_t ep)
{
    d_ptr->dstEndpoint = ep;
}

ApsAddressMode ApsDataIndication::srcAddressMode() const
{
    return d_ptr->srcAddrMode;
}

void ApsDataIndication::setSrcAddressMode(ApsAddressMode mode)
{
    d_ptr->srcAddrMode = mode;
}

Address &ApsDataIndication::srcAddress()
{
    return d_ptr->srcAddr;
}

const Address &ApsDataIndication::srcAddress() const
{
    return d_ptr->srcAddr;
}

uint8_t ApsDataIndication::srcEndpoint() const
{
    return d_ptr->srcEndpoint;
}

void ApsDataIndication::setSrcEndpoint(uint8_t ep)
{
    d_ptr->srcEndpoint = ep;
}

uint16_t ApsDataIndication::profileId() const
{
    return d_ptr->profileId;
}

void ApsDataIndication::setProfileId(uint16_t profileId)
{
    d_ptr->profileId = profileId;
}

uint16_t ApsDataIndication::clusterId() const
{
    return d_ptr->clusterId;
}

void ApsDataIndication::setClusterId(uint16_t clusterId)
{
    d_ptr->clusterId = clusterId;
}

const QByteArray &ApsDataIndication::asdu() const
{
    return d_ptr->asdu;
}

QByteArray &ApsDataIndication::asdu()
{
    return d_ptr->asdu;
}

void ApsDataIndication::setAsdu(const QByteArray &asdu)
{
    d_ptr->asdu = asdu;
}

uint8_t ApsDataIndication::status() const
{
    return d_ptr->status;
}

void ApsDataIndication::setStatus(uint8_t status)
{
    d_ptr->status = status;
}

uint8_t ApsDataIndication::securityStatus() const
{
    return d_ptr->securityStatus;
}

void ApsDataIndication::setSecurityStatus(uint8_t status)
{
    d_ptr->securityStatus = status;
}

uint8_t ApsDataIndication::linkQuality() const
{
    return d_ptr->linkQuality;
}

void ApsDataIndication::setLinkQuality(uint8_t lqi)
{
    d_ptr->linkQuality = lqi;
}

uint32_t ApsDataIndication::rxTime() const
{
    return d_ptr->rxTime;
}

void ApsDataIndication::setRxTime(uint32_t time)
{
    d_ptr->rxTime = time;
}

int8_t ApsDataIndication::rssi() const
{
    return d_ptr->rssi;
}

void ApsDataIndication::setRssi(int8_t rssi)
{
    d_ptr->rssi = rssi;
}

quint16 ApsDataIndication::previousHop() const
{
    return d_ptr->previousHop;
}

void ApsDataIndication::readFromStream(QDataStream &stream)
{
    quint8  u8;
    quint16 u16;
    quint64 u64;

    stream >> u8;
    d_ptr->dstAddrMode = static_cast<ApsAddressMode>(u8);

    switch (dstAddressMode())
    {
    case ApsNoAddress:
        break;

    case ApsGroupAddress:
        stream >> u16;
        dstAddress().setGroup(u16);
        break;

    case ApsNwkAddress:
        stream >> u16;
        dstAddress().setNwk(u16);
        break;

    case ApsExtAddress:
        stream >> u64;
        dstAddress().setExt(u64);
        break;

    default:
        DBG_Printf(DBG_APS, "APSDE-DATA.indication invalid dst address mode 0x%02X\n", dstAddressMode());
        return;
    }

    stream >> d_ptr->dstEndpoint;

    stream >> u8;
    d_ptr->srcAddrMode = static_cast<ApsAddressMode>(u8);

    switch (srcAddressMode())
    {
    case ApsNoAddress:
        break;

    case ApsGroupAddress:
        stream >> u16;
        srcAddress().setGroup(u16);
        break;

    case ApsNwkAddress:
        stream >> u16;
        srcAddress().setNwk(u16);
        break;

    case ApsExtAddress:
        stream >> u64;
        srcAddress().setExt(u64);
        break;

    case ApsNwkExtAddress:
        d_ptr->srcAddrMode = ApsNwkAddress; // keep it simple
        stream >> u16;
        srcAddress().setNwk(u16);
        stream >> u64;
        if (0 != u64) // 0 means invalid ieee address
        {
            srcAddress().setExt(u64);
        }
        break;

    default:
        DBG_Printf(DBG_APS, "APSDE-DATA.indication invalid src address mode 0x%02X\n", srcAddressMode());
        return;
    }

    stream >> d_ptr->srcEndpoint;
    stream >> d_ptr->profileId;
    stream >> d_ptr->clusterId;
    stream >> u16; // asdu length

    for (unsigned i = 0; i < u16 && i < d_ptr->asduBuf.size(); i++)
    {
        stream >> d_ptr->asduBuf[i];
    }
    asdu().setRawData(reinterpret_cast<const char*>(d_ptr->asduBuf.data()), u16);

    if (version() >= 3)
    {
        stream >> d_ptr->previousHop;
        d_ptr->status = 0x00; // success
    }
    else
    {
        stream >> d_ptr->status;
        stream >> d_ptr->securityStatus;
    }
    stream >> d_ptr->linkQuality;
    stream >> d_ptr->rxTime;

    if (version() >= 2)
    {
        DBG_Assert(stream.atEnd() == false);
        if (stream.atEnd())
        {
            return;
        }
        stream >> d_ptr->rssi;
    }
}

void ApsDataIndication::writeToStream(QDataStream &stream) const
{
    stream << (quint8)dstAddressMode();

    switch (dstAddressMode())
    {
    case ApsNoAddress:
        break;

    case ApsGroupAddress:
        DBG_Assert(dstAddress().hasGroup());
        stream << dstAddress().group();
        break;

    case ApsNwkAddress:
        DBG_Assert(dstAddress().hasNwk());
        stream << dstAddress().nwk();
        break;

    case ApsExtAddress:
        DBG_Assert(dstAddress().hasExt());
        stream << (quint64)dstAddress().ext();
        break;

    default:
        DBG_Printf(DBG_APS, "invalid dst address mode");
        return;
    }

    stream << dstEndpoint();

    stream << (quint8)srcAddressMode();

    switch (srcAddressMode())
    {
    case ApsNoAddress:
        break;

    case ApsGroupAddress:
        DBG_Assert(srcAddress().hasGroup());
        stream << srcAddress().group();
        break;

    case ApsNwkAddress:
        DBG_Assert(srcAddress().hasNwk());
        stream << srcAddress().nwk();
        break;

    case ApsExtAddress:

        DBG_Assert(srcAddress().hasExt());
        stream << (quint64)srcAddress().ext();
        break;

    default:
        DBG_Printf(DBG_APS, "invalid src address mode");
        break;
    }

    stream << srcEndpoint();
    stream << profileId();
    stream << clusterId();
    stream << (quint16)asdu().size();

    for (int i = 0, end = asdu().size(); i < end; i++)
    {
        stream << d_ptr->asduBuf[i];
    }

    stream << status();
    stream << securityStatus();
    stream << linkQuality();
    stream << rxTime();

    if (version() >= 2)
    {
        stream << rssi();
    }
}

int ApsDataIndication::version() const
{
    return d_ptr->version;
}

void ApsDataIndication::setVersion(int version) const
{
    d_ptr->version = version;
}

void ApsDataIndication::reset()
{
    d_ptr->reset();
}

Address::Address(const Address &other)
{
    *this = other;
}

Address &Address::operator= (const Address &other)
{
    // Self assignment?
    if (this != &other)
    {
        m_ext = other.m_ext;
        m_nwk = other.m_nwk;
        m_group = other.m_group;
        m_addrModes = other.m_addrModes;
    }

    return *this;
}

bool Address::isNwkUnicast() const
{
    return (hasNwk() && (nwk() < 0xFFFA));
}

bool Address::isNwkBroadcast() const
{
    return (hasNwk() && (nwk() >= 0xFFFA));
}

bool Address::hasNwk() const
{
    return m_addrModes & deCONZ::NwkAddress;
}

bool Address::hasExt() const
{
    return m_addrModes & deCONZ::ExtAddress;
}

bool Address::hasGroup() const
{
    return m_addrModes & deCONZ::GroupAddress;
}

uint16_t Address::nwk() const
{
    return m_nwk;
}

uint64_t Address::ext() const
{
    return m_ext;
}

uint16_t Address::group() const
{
    return m_group;
}

void Address::setNwk(uint16_t addr)
{
    m_nwk = addr;
    m_addrModes |= deCONZ::NwkAddress;
}

void Address::setExt(uint64_t addr)
{
    m_ext = addr;
    m_addrModes |= deCONZ::ExtAddress;
}

void Address::setGroup(uint16_t addr)
{
    m_group = addr;
    m_addrModes |= deCONZ::GroupAddress;
}

void Address::clear()
{
    *this = {};
}

QString Address::toStringExt() const
{
    return QString("0x%1").arg(ext(), int(16), int(16), QChar('0'));
}

QString Address::toStringNwk() const
{
    return QString("0x%1").arg(nwk(), int(4), int(16), QChar('0'));
}

QString Address::toStringGroup() const
{
    return QString("0x%1").arg(group(), int(4), int(16), QChar('0'));
}

bool Address::fromStringExt(const QString &str)
{
    bool ok = false;
    if (!str.isEmpty())
    {
        const uint64_t addr = str.toULongLong(&ok, 16);
        if (ok)
        {
            setExt(addr);
        }
    }
    return ok;
}

bool Address::fromStringNwk(const QString &str)
{
    bool ok = false;
    if (!str.isEmpty())
    {
        const uint16_t addr = str.toUShort(&ok, 16);
        if (ok)
        {
            setNwk(addr);
        }
    }
    return ok;
}

bool Address::operator==(const Address &other) const
{
    return hasNwk() == other.hasNwk() &&
           hasExt() == other.hasExt() &&
           hasGroup() == other.hasGroup() &&
           nwk() == other.nwk() &&
           ext() == other.ext() &&
           group() == other.group();
}

bool Address::operator!=(const Address &other) const
{
    return !(*this == other);
}

} // namespace deCONZ
