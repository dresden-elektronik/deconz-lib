#ifndef DECONZ_ZDP_DESCRIPTORS_H
#define DECONZ_ZDP_DESCRIPTORS_H

/*
 * Copyright (c) 2012-2023 dresden elektronik ingenieurtechnik gmbh.
 * All rights reserved.
 *
 * The software in this package is published under the terms of the BSD
 * style license a copy of which has been included with this distribution in
 * the LICENSE.txt file.
 *
 */

#include <QObject>
#include <deconz/zcl.h>
#include <deconz/types.h>

/*!
    \defgroup zdp ZDP
    \brief ZigBee Device Profile (ZDP).
*/

namespace deCONZ {

/*! IEEE 802.15.4 MAC capabilities. */
enum MacCapability : unsigned char
{
    MacAlternatePanCoordinator = (1 << 0), //!< Can be a alternate pan coordinator
    MacDeviceIsFFD             = (1 << 1), //!< Is a full function device
    MacIsMainsPowered          = (1 << 2), //!< Is mains powered
    MacReceiverOnWhenIdle      = (1 << 3), //!< Has receiver on when idle
    MacSecuritySupport         = (1 << 6), //!< Has (high) security support
    MacAllocateAddress         = (1 << 7)  //!< Allocates address
};

Q_DECLARE_FLAGS(MacCapabilities, MacCapability)
Q_DECLARE_OPERATORS_FOR_FLAGS(deCONZ::MacCapabilities)

/*!
    Current power mode in node power descriptor.
 */
enum PowerMode
{
    ModeOnWhenIdle  = 0x00, //!< Receiver synchronized with the receiver on when idle sub-field of the node descriptor.
    ModePeriodic    = 0x01, //!< Receiver comes on periodically as defined by the node power descriptor.
    ModeStimulated  = 0x02  //!< Receiver comes on when stimulated, e.g. by a user pressing a button.
};

/*!
    Available power sources in node power descriptor.
 */
enum PowerSource
{
    PowerSourceUnknown       = 0x00,
    PowerSourceMains         = 0x01, //!< Constant (mains) power
    PowerSourceRechargeable  = 0x02, //!< Rechargeable battery
    PowerSourceDisposable    = 0x04  //!< Disposable battery
};

Q_DECLARE_FLAGS(PowerSources, PowerSource)
Q_DECLARE_OPERATORS_FOR_FLAGS(PowerSources)

/*!
    Current power source level in node power descriptor.
 */
enum PowerSourceLevel
{
    PowerLevelCritical = 0x00, //!< Critical
    PowerLevel33       = 0x04, //!< 33%
    PowerLevel66       = 0x08, //!< 66%
    PowerLevel100      = 0x0c  //!< 100%
};

class NodeDescriptorPrivate;

/*!
    \ingroup zdp
    \class NodeDescriptor
    \brief Represents a ZigBee node descriptor.
    \see Node::nodeDescriptor()
 */
class DECONZ_DLLSPEC NodeDescriptor
{

public:
    /*! Constructor. */
    NodeDescriptor();
    /*! Copy constructor. */
    NodeDescriptor(const NodeDescriptor &other);
    /*! Copy assignment constructor. */
    NodeDescriptor &operator=(const NodeDescriptor &other);
    /*! Deconstructor. */
    ~NodeDescriptor();
    /*! Reads a ZigBee standard conform node descriptor from stream. */
    void readFromStream(QDataStream &stream);
    /*! Returns the device type. */
    deCONZ::DeviceType deviceType() const;
    /*! Sets the device type. */
    void setDeviceType(deCONZ::DeviceType deviceType);
    /*! Returns the manufacturer code. */
    uint16_t manufacturerCode() const;
    /*! Returns strong typed manufacturer code.
        \since v2.6.1
     */
    ManufacturerCode_t manufacturerCode_t() const;
    /*! Sets the manufacturer code. */
    void setManufacturerCode(quint16 code);
    /*! Sets the strong typed manufacturer code.
        \since v2.6.1
     */
    void setManufacturerCode(ManufacturerCode_t code);
    /*! Returns the mac capabilities bitmap. */
    MacCapabilities macCapabilities() const;
    /*! Sets the mac capabilities bitmap. */
    void setMacCapabilities(MacCapabilities cap);
    /*! Returns true if the node provides a complex descriptor. */
    bool hasComplexDescriptor() const;
    /*! Sets the has complex descriptor flag. */
    void setHasComplexDescriptor(bool hasComplex);
    /*! Returns true if the node provides a user descriptor. */
    bool hasUserDescriptor() const;
    /*! Sets the has user descriptor flag. */
    void setHasUserDescriptor(bool hasUser);
    /*! Returns the nodes operating frequency band. */
    deCONZ::FrequencyBand frequencyBand() const;
    /*! Sets the nodes operating frequency band. */
    void setFrequenzyBand(deCONZ::FrequencyBand freq);
    /*! Returns the nodes operating frequency band as string. */
    const char *frequencyBandString() const;
    /*! Returns true if the node is a alternate pan coordinator. */
    bool isAlternatePanCoordinator() const;
    /*! Sets the node is a alternate pan coordinator flag. */
    void setIsAlternatePanCoordinator(bool isAlt);
    /*! Returns true if the node is a full function device. */
    bool isFullFunctionDevice() const;
    /*! Sets the node is a full function device flag. */
    void setIsFFD(bool isFFD);
    /*! Returns true if the node is mains powered. */
    bool isMainsPowered() const;
    /*! Sets the node is mains powered flag. */
    void setIsMainsPowered(bool isMains);
    /*! Returns true if the node has its tranceiver on when idle. */
    bool receiverOnWhenIdle() const;
    /*! Sets the node has its tranceiver on when idle flag. */
    void setRxOnWhenIdle(bool on);
    /*! Returns true if the node supports (high) security. */
    bool securitySupport() const;
    /*! Sets the node supports (high) security flag. */
    void setSecuritySupport(bool supported);
    /*! Returns true if the node allocates addresses. */
    bool allocateAddress() const;
    /*! Returns true if the node has a extended endpoint list. */
    bool hasEndpointList() const;
    /*! Returns true if the node has a extended simple descriptor list. */
    bool hasSimpleDescriptorList() const;
    /*! Returns the nodes server mask. */
    const zme::NodeServerFlags serverMask() const;
    /*! Returns the max buffer size. */
    uint8_t maxBufferSize() const;
    /*! Returns the max incoming transfer size. */
    uint16_t maxIncomingTransferSize() const;
    /*! Returns the max outgoing transfer size. */
    uint16_t maxOutgoingTransferSize() const;
    /*! Returns true if valid data is set. */
    bool isNull() const;
    /*! Sets the valid data flag. */
    void setIsNull(bool isNull);
    /*! Returns a array with the ZigBee standard conform node descriptor. */
    QByteArray toByteArray() const;
    /*! Returns server mask stack compliance revision (bits: 9-15). */
    unsigned stackRevision() const;

private:
    NodeDescriptorPrivate *d = nullptr;
};

class PowerDescriptorPrivate;

/*!
    \ingroup zdp
    \class PowerDescriptor
    \brief Represents a ZigBee power descriptor.
    \see Node::powerDescriptor()
 */
class DECONZ_DLLSPEC PowerDescriptor
{
public:
    /*! Constructor. */
    PowerDescriptor();
    /*! Copy constructor. */
    PowerDescriptor(const PowerDescriptor &other);
    /*! Copy assignment constructor. */
    PowerDescriptor &operator=(const PowerDescriptor &other);
    /*! Constructor from raw power descriptor in ZigBee standard conform format (2 bytes). */
    PowerDescriptor(const QByteArray &data);
    /*! Deconstructor. */
    ~PowerDescriptor();
    /*! Returns the current power mode. */
    PowerMode currentPowerMode() const;
    /*! Returns the available power sources. */
    PowerSources availablePowerSources() const;
    /*! Returns the current power source. */
    PowerSource currentPowerSource() const;
    /*! Returns the current power level. */
    PowerSourceLevel currentPowerLevel() const;
    /*! Returns true if this power descriptor has valid data. */
    bool isValid() const;
    /*! Returns the raw power descriptor in ZigBee standard conform format. */
    QByteArray toByteArray() const;

private:
    PowerDescriptorPrivate *d = nullptr;
};

class SimpleDescriptorPrivate;

/*!
    \ingroup zdp

    \class SimpleDescriptor
    \brief Represents a ZigBee simple descriptor.

    Each active application endpoint on a node has its own simple descriptor which provides
    all informations about the endpoints profile and clusters.
    \see Node::simpleDescriptors()
 */
class DECONZ_DLLSPEC SimpleDescriptor
{
public:
    /*! Constructor. */
    SimpleDescriptor();
    /*! Copy constructor. */
    SimpleDescriptor(const SimpleDescriptor &other);
    /*! Copy assignment constructor. */
    SimpleDescriptor &operator=(const SimpleDescriptor &other);
    /*! Deconstructor. */
    ~SimpleDescriptor();
    /*! Reads a ZigBee standard conform simple descriptor from stream. */
    void readFromStream(QDataStream &stream, quint16 mfcode);
    /*! Writes a ZigBee standard conform simple descriptor from stream. */
    void writeToStream(QDataStream &stream) const;
    /*! Returns the endpoint number. */
    uint8_t endpoint() const;
    /*! Sets the endpoint number. */
    void setEndpoint(uint8_t endpoint);
    /*! Returns the profile identifier. */
    uint16_t profileId() const;
    /*! Sets the profile identifier. */
    void setProfileId(uint16_t profileId);
    /*! Returns the device identifier. */
    uint16_t deviceId() const;
    /*! Sets the device identifier. */
    void setDeviceId(uint16_t deviceId);
    /*! Returns the device version. */
    uint8_t deviceVersion() const;
    /*! Sets the device version. */
    void setDeviceVersion(uint8_t version);
    /*! Returns true if this simple descriptor contains valid data. */
    bool isValid() const;
    /*! Returns the modifiable list of in (server) clusters. */
    std::vector<ZclCluster> &inClusters();
    /*! Returns the const list of in (server) clusters. */
    const std::vector<ZclCluster> &inClusters() const;
    /*! Returns the modifiable list of out (client) clusters. */
    std::vector<ZclCluster> &outClusters();
    /*! Returns the const list of out (client) clusters. */
    const std::vector<ZclCluster> &outClusters() const;
    /* \cond INTERNAL_SYMBOLS */ // TODO: move in SimpleDescriptorPrivate
    ZclCluster *cluster(uint16_t id, ZclClusterSide side);
    std::vector<ZclCluster> &clusters(ZclClusterSide side);
    const std::vector<ZclCluster> &clusters(ZclClusterSide side) const;
    /* \endcond */

private:
    SimpleDescriptorPrivate *d = nullptr;
};

} // namespace deCONZ

#endif // DECONZ_ZDP_DESCRIPTORS_H
