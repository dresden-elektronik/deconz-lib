#ifndef DECONZ_APS_H
#define DECONZ_APS_H

/*
 * Copyright (c) 2012-2023 dresden elektronik ingenieurtechnik gmbh.
 * All rights reserved.
 *
 * The software in this package is published under the terms of the BSD
 * style license a copy of which has been included with this distribution in
 * the LICENSE.txt file.
 *
 */

/*! \mainpage deCONZ Reference Manual

   \section intro_sec Introduction

   The %deCONZ C++ API allows the development of plugins for the %deCONZ main application.
   Plugins are free to communicate with any node in the ZigBee network.

   \section features Features
   - Send and receive packets to and from any node in the network
   - Full node discovery is done in the background and available through the node cache
   - Full access to \ref aps_intro (no limitation to specific profiles)
   - Convenience classes for \ref zcl_intro handling

   \section Installation

   The installation of %deCONZ and the development package is described on Github <a href="https://github.com/dresden-elektronik/deconz-rest-plugin#installation">README.md</a>.

   \section basics Basic Usage

   Include the main %deCONZ header in your Qt plugin project.

   `#include <deconz.h>`

   Link your project with `-ldeCONZ`
 */

/*! \defgroup aps APS
    \section aps_intro Application Support Layer (APS)
    \brief Services to send and receive arbitrary data.

    The Application Support Layer (APS) as defined in ZigBee specification (ZigBee Document 053474r19),
    provides an asynchronous interface for sending and receiving data to and from other nodes
    in the network.

    Applications can use the deCONZ::ApsController service in order to send deCONZ::ApsDataRequest to
    the network as well as register for incoming deCONZ::ApsDataConfirm and deCONZ::ApsDataIndication
    events via the Qt signal/slot mechanism.

    \msc
     width=900;
     deCONZ,Firmware,Device;
     deCONZ=>Firmware [label="deCONZ::ApsDataRequest", URL="\ref deCONZ::ApsDataRequest()"];
     Firmware=>>Device [label="APSDE-DATA.request"];
     Firmware=>deCONZ [label="deCONZ::ApsDataConfirm", URL="\ref deCONZ::ApsDataConfirm()"];
     Device=>>Firmware [label="APSDE-DATA.indication"];
     Firmware=>deCONZ [label="deCONZ::ApsDataIndication", URL="\ref deCONZ::ApsDataIndication()"];
   \endmsc

    The APS is not limited to any specific ZigBee profile like Home Automation or Smart Energy, the application
    is responsible to specify related profile ids, cluster ids and payload data.

    It is possible to send and receive ZCL based packets as well as non-ZCL based proprietary ones.
*/

#include "deconz/types.h"
#include "deconz/timeref.h"
#include "deconz/declspec.h"

namespace deCONZ
{

/*! Address modes used to specify source and destination addresses. */
enum ApsAddressMode : unsigned char
{
    ApsNoAddress    = 0x0, //!< No addressing specified
    ApsGroupAddress = 0x1, //!< 16-bit group address mode
    ApsNwkAddress   = 0x2, //!< 16-bit network address mode
    ApsExtAddress   = 0x3, //!< 64-bit extended IEEE address mode
    ApsNwkExtAddress   = 0x4  //!< 16-bit network address mode and 64-bit extended IEEE address mode (since protocol version 0x010B)
};

/*! Flags used in the ApsDataRequest. */
enum ApsTxOption : unsigned char
{
    ApsTxSecurityEnabledTransmission = 0x01, //!< APS layer security enabled transmission
    ApsTxUseNwk                      = 0x02, //!< Use network key security
    ApsTxAcknowledgedTransmission    = 0x04, //!< Enable APS acknowledged transmission
    ApsTxFragmentationPermitted      = 0x08  //!< Allow fragmentation
};

Q_DECLARE_FLAGS(ApsTxOptions, ApsTxOption)
Q_DECLARE_OPERATORS_FOR_FLAGS(deCONZ::ApsTxOptions)

class AddressPrivate;

/*!
    \ingroup aps
    \class Address
    \brief Convenience class to work with network, extended and group addresses.
 */
class DECONZ_DLLSPEC Address
{
public:
    /*! Constructor. */
    Address() = default;
    /*! Copy constructor. */
    Address(const Address &other);
    /*! Copy assignment constructor. */
    Address &operator= (const Address &other);
    /*! Deconstructor. */
    ~Address() = default;
    /*! Returns true if network address is a unicast address. */
    bool isNwkUnicast() const;
    /*! Returns true if network address is a broadcast address. */
    bool isNwkBroadcast() const;
    /*! Returns true if network address is set. */
    bool hasNwk() const;
    /*! Returns true if the extended IEEE aka MAC address is set. */
    bool hasExt() const;
    /*! Returns true if the group address is set. */
    bool hasGroup() const;
    /*! Returns the 16-bit network address. */
    uint16_t nwk() const;
    /*! Returns 64-bit extended IEEE aka MAC address. */
    uint64_t ext() const;
    /*! Returns 16-bit group address. */
    uint16_t group() const;
    /*! Sets the 16-bit network address which might be a unicast or broadcast address.
        \param addr the network address
     */
    void setNwk(uint16_t addr);
    /*! Sets the 64-bit extended IEEE aka MAC address.
        \param addr the extended IEEE address
     */
    void setExt(uint64_t addr);
    /*! Sets the 16-bit group address.
        \param addr the group address
     */
    void setGroup(uint16_t addr);
    /*! Clears all address values to 0. */
    void clear();
    /*! Returns the exended IEEE address as string in form or 0x001122334455667788. */
    QString toStringExt() const;
    /*! Returns the network address as string in form or 0x0011. */
    QString toStringNwk() const;
    /*! Returns the groups address as string in form or 0x0011. */
    QString toStringGroup() const;
    /*! Returns true if \p str could be parsed and was set as extended IEEE address.
        \param str a string holding a hexadecimal address
     */
    bool fromStringExt(const QString &str);
    /*! Returns true if \p str could be parsed and was set as network address.
        \param str a string holding a hexadecimal address
     */
    bool fromStringNwk(const QString &str);
    /*! Returns true if nwk(), ext() and group() addresses are equal.
        \param other the address to compare against
     */
    bool operator==(const Address &other) const;
    /*! Returns true if nwk(), ext() and group() addresses are not equal.
        \param other the address to compare against
     */
    bool operator !=(const Address &other) const;

private:
    uint64_t m_ext{0};
    uint16_t m_nwk{0};
    uint16_t m_group{0};
    uint8_t m_addrModes{deCONZ::NoAddress};
    uint8_t _pad[3];
};

class ApsDataRequestPrivate;

/*!
    \ingroup aps
    \class ApsDataRequest
    \brief APSDE-DATA.request primitive.

    The ApsDataRequest class can be used so send arbitrary messages via ZigBee APS
    layer.

    The destination can be a single node (unicast) or a group of nodes (groupcast, broadcast).

    The following code creates and sends a ZigBee Device Profile (ZDP) <em>Match Descriptor Request</em> via
    broadcast<br/> to search the network for ZigBee Light Link (ZLL) devices like Philips Hue which provide a OnOff cluster.

    \code{.cpp}
    deCONZ::ApsDataRequest req;

    // set destination addressing
    req.setDstAddressMode(deCONZ::ApsNwkAddress);
    req.dstAddress().setNwk(deCONZ::BroadcastRxOnWhenIdle);
    req.setDstEndpoint(ZDO_ENDPOINT);
    req.setSrcEndpoint(ZDO_ENDPOINT);
    req.setProfileId(ZDP_PROFILE_ID);
    req.setClusterId(ZDP_MATCH_DESCRIPTOR_CLID);

    // prepare payload
    QDataStream stream(&req.asdu(), QIODevice::WriteOnly);
    stream.setByteOrder(QDataStream::LittleEndian);

    // write payload according to ZigBee specification (section 2.4.3.1.7, Match_Descr_req)
    stream << static_cast<quint8>(qrand());         // ZDP transaction sequence number
    stream << static_cast<quint16>(deCONZ::BroadcastRxOnWhenIdle); // NWKAddrOfInterest
    stream << static_cast<quint16>(ZLL_PROFILE_ID); // ProfileID
    stream << static_cast<quint8>(0x01);            // NumInClusters
    stream << static_cast<quint16>(0x0006);         // OnOff ClusterID
    stream << static_cast<quint8>(0x00);            // NumOutClusters

    if (deCONZ::ApsController::instance()->apsdeDataRequest(req) == deCONZ::Success))
    {
        ...
    }
    \endcode
 */
class DECONZ_DLLSPEC ApsDataRequest
{
public:
    /*! Constructor. */
    ApsDataRequest();
    /*! Copy constructor. */
    ApsDataRequest(const ApsDataRequest &other);
    /*! Move constructor. */
    ApsDataRequest(ApsDataRequest &&other) noexcept;
    /*! Copy assignment operator. */
    ApsDataRequest& operator=(const ApsDataRequest &other);
    /*! Move assignment operator. */
    ApsDataRequest& operator=(ApsDataRequest &&other) noexcept;
    /*! Deconstructor. */
    ~ApsDataRequest();
    /*! Returns the uniqe id for this request.
        The id is set in the constructor.
        It allows to match a ApsDataConfirm to the request.
     */
    uint8_t id() const;
    /*! Returns the modifiable destination address. */
    Address &dstAddress();
    /*! Returns the const destination address. */
    const Address &dstAddress() const;
    /*! Returns the destination address mode. */
    deCONZ::ApsAddressMode dstAddressMode() const;
    /*! Sets the destination address mode. */
    void setDstAddressMode(ApsAddressMode mode);
    /*! Returns the source endpoint. */
    uint8_t srcEndpoint() const;
    /*! Sets the source endpoint. */
    void setSrcEndpoint(uint8_t ep);
    /*! Returns the destination endpoint. */
    uint8_t dstEndpoint() const;
    /*! Sets the destination endpoint. */
    void setDstEndpoint(uint8_t ep);
    /*! Returns the profile identifier. */
    uint16_t profileId() const;
    /*! Sets the profile identifier. */
    void setProfileId(uint16_t profileId);
    /*! Returns the cluster identifier. */
    uint16_t clusterId() const;
    /*! Sets the cluster identifier. */
    void setClusterId(uint16_t clusterId);
    /*! Returns the response cluster identifier. */
    uint16_t responseClusterId() const;
    /*! Sets the response cluster identifier. */
    void setResponseClusterId(uint16_t clusterId);
    /*! Returns the const ASDU payload. */
    const QByteArray &asdu() const;
    /*! Returns the writeable ASDU payload. */
    QByteArray &asdu();
    /*! Sets the ASDU payload. */
    void setAsdu(const QByteArray &asdu);
    /*! Returns the send radius. */
    uint8_t radius() const;
    /*! Sets the send radius. A value of 0 means the stack will use the default value. */
    void setRadius(uint8_t radius);
    /*! Returns the transmit options.
        \returns a a logical OR combined value of deCONZ::ApsTxOption flags
     */
    ApsTxOptions txOptions() const;
    /*! Sets the transmit options.
        \param txOptions a logical OR combined value of deCONZ::ApsTxOption flags
     */
    void setTxOptions(ApsTxOptions txOptions);
    /*! Writes the request to the stream in a ZigBee standard conform format.
        \returns 1 on success
        \code{.cpp}
          deCONZ::ApsDataRequest req;
          // ... setup request
          QByteArray arr; // buffer into which the plain request will be written
          QDataStream stream(&arr, QIODevice::WriteOnly);
          stream.setByteOrder(QDataStream::LittleEndian); // everything in ZigBee is little endian

          // serialize request to buffer
          if (req.writeToStream(stream))
          { }
        \endcode
     */
    int writeToStream(QDataStream &stream) const;
    /*! Reads a request from the stream which must be in a ZigBee standard conform format.
        \code{.cpp}
          QByteArray arr; // buffer which contains the plain request
          deCONZ::ApsDataRequest req;
          QDataStream stream(&arr, QIODevice::ReadOnly);
          stream.setByteOrder(QDataStream::LittleEndian); // everything in ZigBee is little endian

          // deserialize request from buffer
          req.readFromStream(stream);
        \endcode
     */
    void readFromStream(QDataStream &stream);
    /*! Resets the request parameters. */
    void clear();

    /* \cond INTERNAL_SYMBOLS */
    void setSendAfter(deCONZ::SteadyTimeRef t);
    deCONZ::SteadyTimeRef sendAfter() const;
    uint8_t version() const;
    void setVersion(uint8_t version) const;
    uint16_t nodeId() const;
    void setNodeId(uint16_t id) const;
    deCONZ::SteadyTimeRef timeout() const;
    void setTimeout(deCONZ::SteadyTimeRef timeout);
    CommonState state() const;
    void setState(CommonState state);
    int sendDelay() const;
    void setSendDelay(int delayMs);
    bool confirmed() const;
    void setConfirmed(bool confirmed);
    void setSourceRoute(const std::array<quint16, 9> &relays, size_t size, const uint srHash);
    uint sourceRouteUuidHash() const;
    /* \endcond */

private:
    ApsDataRequestPrivate *d_ptr = nullptr;
    Q_DECLARE_PRIVATE(ApsDataRequest)
};

/*!
    \ingroup aps
    \class ApsDataConfirm
    \brief APSDE-DATA.confirm primitive.
 */
class DECONZ_DLLSPEC ApsDataConfirm
{
public:
    /*! Constructor */
    ApsDataConfirm() = default;
    /*! Constructor for error reporting */
    ApsDataConfirm(uint8_t reqId, uint8_t status);
    /*! Constructor for error reporting */
    ApsDataConfirm(const ApsDataRequest &req, uint8_t status);
    /*! Copy constructor */
    ApsDataConfirm(const ApsDataConfirm &other) = default;
    /*! Copy assignment constructor */
    ApsDataConfirm &operator=(const ApsDataConfirm &other) = default;
    /*! Deconstructor */
    ~ApsDataConfirm() = default;
    /*! Returns a id to match this confirm to the related ApsDataRequest. */
    uint8_t id() const;
    /*! Sets a id to match this confirm to the related ApsDataRequest. */
    void setId(uint8_t id);
    /*! Returns the modifiable destination address. */
    Address &dstAddress();
    /*! Returns the const destination address. */
    const Address &dstAddress() const;
    /*! Returns the destination address mode. */
    ApsAddressMode dstAddressMode() const;
    /*! Sets the destination address mode. */
    void setDstAddressMode(ApsAddressMode mode);
    /*! Returns the destination endpoint. */
    uint8_t dstEndpoint() const;
    /*! Returns the source endpoint. */
    uint8_t srcEndpoint() const;
    /*! Returns the sending status.
        \sa deCONZ::ApsStatus, deCONZ::NwkStatus, deCONZ::MacStatus
     */
    uint8_t status() const;
    /*! Returns the transmission time. */
    uint32_t txTime() const;
    /*!  Reads a confirm from the \p stream which must be in a ZigBee standard conform format. */
    void readFromStream(QDataStream &stream);

private:
    Address m_dstAddr{};
    ApsAddressMode m_dstAddrMode = deCONZ::ApsNoAddress;
    uint8_t m_id = 0;
    uint8_t m_dstEndpoint = 0xFF;
    uint8_t m_srcEndpoint = 0xFF;
    uint8_t m_status = 0xFF;
};

class ApsDataIndicationPrivate;

/*!
    \ingroup aps
    \class ApsDataIndication
    \brief APSDE-DATA.indication primitive.
 */
class DECONZ_DLLSPEC ApsDataIndication
{
public:
    /*! Constructor. */
    ApsDataIndication();
    /*! Copy constructor. */
    ApsDataIndication(const ApsDataIndication &other);
    /*! Copy constructor. */
    ApsDataIndication &operator=(const ApsDataIndication &other);
    /*! Deconstructor. */
    ~ApsDataIndication();
    /*! Returns the destination address mode. */
    ApsAddressMode dstAddressMode() const;
    /*! Sets the destination address mode. */
    void setDstAddressMode(ApsAddressMode mode);
    /*! Returns the modifiable destination address. */
    Address &dstAddress();
    /*! Returns the const destination address. */
    const Address &dstAddress() const;
    /*! Returns the destination endpoint. */
    uint8_t dstEndpoint() const;
    /*! Sets the destination endpoint. */
    void setDstEndpoint(uint8_t ep);
    /*! Returns the source address mode. */
    ApsAddressMode srcAddressMode() const;
    /*! Sets the source address mode. */
    void setSrcAddressMode(ApsAddressMode mode);
    /*! Returns the modifiable source address. */
    Address &srcAddress();
    /*! Returns the const source address. */
    const Address &srcAddress() const;
    /*! Returns the source endpoint. */
    uint8_t srcEndpoint() const;
    /*! Sets the source endpoint. */
    void setSrcEndpoint(uint8_t ep);
    /*! Returns the profile identifier. */
    uint16_t profileId() const;
    /*! Sets the profile identifier. */
    void setProfileId(uint16_t profileId);
    /*! Returns the cluster identifier. */
    uint16_t clusterId() const;
    /*! Sets the cluster identifier. */
    void setClusterId(uint16_t clusterId);
    /*! Returns the const ASDU payload. */
    const QByteArray &asdu() const;
    /*! Returns the modifiable ASDU payload. */
    QByteArray &asdu();
    /*! Sets the ASDU payload. */
    void setAsdu(const QByteArray &asdu);
    /*! Returns the indication status. */
    uint8_t status() const;
    /*! Sets the indication status. */
    void setStatus(uint8_t status);
    /*! Returns the indication security status. */
    uint8_t securityStatus() const;
    /*! Sets the indication security status. */
    void setSecurityStatus(uint8_t status);
    /*! Returns the link quality indicatior (LQI). */
    uint8_t linkQuality() const;
    /*! Sets the link quality indicatior (LQI). */
    void setLinkQuality(uint8_t lqi);
    /*! Returns the receive time. */
    uint32_t rxTime() const;
    /*! Sets the receive time. */
    void setRxTime(uint32_t time);
    /*! Returns the received signal strength indication (RSSI). */
    int8_t rssi() const;
    /*! Sets the received signal strength indication (RSSI). */
    void setRssi(int8_t rssi);
    /*! Returns the previous hop nwk address (version >= 3). */
    quint16 previousHop() const;
    /*! Reads a ZigBee standard conform indication from stream. */
    void readFromStream(QDataStream &stream);
    /*! Writes a ZigBee standard conform indication to stream. */
    void writeToStream(QDataStream &stream) const;
    /* \cond INTERNAL_SYMBOLS */
    int version() const;
    void setVersion(int version) const;
    /* \endcond */
    /*! Resets the object to initial state, allowing it's reuse without extra allocation. */
    void reset();

private:
    ApsDataIndicationPrivate *d_ptr = nullptr;
    Q_DECLARE_PRIVATE(ApsDataIndication)
};

class ApsMemoryPrivate;

class DECONZ_DLLSPEC ApsMemory
{
public:
    ApsMemory();
    ~ApsMemory();

    /* \cond INTERNAL_SYMBOLS */
    ApsMemoryPrivate *d = nullptr; // opaque pointer only used internally
    /* \endcond */
};

} // namespace deCONZ

uint8_t DECONZ_DLLSPEC APS_NextApsRequestId();

Q_DECLARE_METATYPE(deCONZ::ApsDataRequest)
Q_DECLARE_METATYPE(deCONZ::ApsDataConfirm)
Q_DECLARE_METATYPE(deCONZ::ApsDataIndication)

#endif // DECONZ_APS_H
