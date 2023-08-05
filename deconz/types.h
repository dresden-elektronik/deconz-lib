#ifndef DECONZ_TYPES_H_
#define DECONZ_TYPES_H_

/*
 * Copyright (c) 2012-2023 dresden elektronik ingenieurtechnik gmbh.
 * All rights reserved.
 *
 * The software in this package is published under the terms of the BSD
 * style license a copy of which has been included with this distribution in
 * the LICENSE.txt file.
 *
 */

/*!
  \file types.h
  Declaration of the most common deCONZ library types.


  \namespace deCONZ

  The <em>deCONZ</em> namespace.
 */

#include <stdint.h>
#include <inttypes.h>

#ifdef __cplusplus

#define CL_URL_SCHEME "cluster"
#define CL_ITEM_ENDPOINT "ep"
#define CL_ITEM_EXT_ADDR "ieee"
#define CL_ITEM_CLUSTER_ID "cid"
#define CL_ITEM_NAME "name"
#define CL_ITEM_CLUSTER_SIDE "side"
#define CL_ITEM_PROFILE_ID "prf"
#define CL_ITEM_DEVICE_ID "dev"
#define EP_URL_SCHEME "endpoint"

#define ZLL_PROFILE_ID 0xC05E
#define GP_PROFILE_ID 0xa1e0
#define HA_PROFILE_ID 0x0104
#define SE_PROFILE_ID 0x0109

#define HA_DEFAULT_TC_LINK_KEY 0x5a6967426565416c6c69616e63653039LL  //! Zigbee HA default trust center link key

#define BROADCAST_SOCKET -1

#define FW_ONLY_AVR_BOOTLOADER 1

class zmgNode;

namespace deCONZ
{
    class zmNode;

    /*! Return codes of the deCONZ library. */
    enum LibraryReturnCodes
    {
        Success = 0,       //!< Success
        ErrorNotConnected, //!< Not connected (device or network)
        ErrorQueueIsFull,  //!< Queue is full
        ErrorNodeIsZombie, //!< Node is not reachable
        ErrorNotFound      //!< Not found
    };

    /*! \brief Special network layer broadcast addresses.
        \ingroup aps
     */
    enum NwkBroadcastAddress
    {
        BroadcastAll             = 0xFFFF, //!< Broadcast to all nodes including end devices
        BroadcastLowPowerRouters = 0xFFFB, //!< Broadcast only to low power routers
        BroadcastRouters         = 0xFFFC, //!< Broadcast only to routers
        BroadcastRxOnWhenIdle    = 0xFFFD  //!< Broadcast to all nodes which have the RxOnWhenIdle flag set
    };

    /*! Security modes. */
    enum SecurityMode
    {
        SecModeNoSecurity                      = 0x00,
        SecModePreconfiguredNetworkKey         = 0x01,
        SecModeNetworkKeyFromTrustCenter       = 0x02,
        SecModeNoMasterButTrustCeneterLinkKey  = 0x03,
        SecModeMasterKey                       = 0x04
    };

    enum Indication
    {
        IndicateNone,
        IndicateReceive,
        IndicateSend,
        IndicateSendDone,
        IndicateDataUpdate,
        IndicateError
    };

    /*! \brief Common ZigBee Device Profile (ZDP) status codes.
        \ingroup zdp
     */
    enum ZdpState
    {
        ZdpSuccess            = 0x00,
        ZdpInvalidRequestType = 0x80,
        ZdpDeviceNotFound     = 0x81,
        ZdpInvalidEndpoint    = 0x82,
        ZdpNotActive          = 0x83,
        ZdpNotSupported       = 0x84,
        ZdpTimeout            = 0x85,
        ZdpNoMatch            = 0x86,
        // reserved             0x87
        ZdpNoEntry            = 0x88,
        ZdpNoDescriptor       = 0x89,
        ZdpInsufficientSpace  = 0x8a,
        ZdpNotPermitted       = 0x8b,
        ZdpTableFull          = 0x8c,
        ZdpNotAuthorized      = 0x8d
    };

    /*! \brief Common ZigBee Cluster Library (ZCL) status codes.
        \ingroup zcl
     */
    enum ZclStatus
    {
        ZclSuccessStatus                   = 0x00,
        ZclFailureStatus                   = 0x01,
        ZclNotAuthorizedStatus             = 0x7e,
        ZclReservedFieldNotZeroStatus      = 0x7f,
        ZclMalformedCommandStatus          = 0x80,
        ZclUnsupClusterCommandStatus       = 0x81,
        ZclUnsupGeneralCommandStatus       = 0x82,
        ZclUnsupManufClusterCommandStatus  = 0x83,
        ZclUnsupManufGeneralCommandStatus  = 0x84,
        ZclInvalidFieldStatus              = 0x85,
        ZclUnsupportedAttributeStatus      = 0x86,
        ZclInvalidValueStatus              = 0x87,
        ZclReadOnlyStatus                  = 0x88,
        ZclInsufficientSpaceStatus         = 0x89,
        ZclInconstistentStartupStateStatus = 0x90,
        ZclDefinedOutOfBandStatus          = 0x91,
        ZclHardwareFailureStatus           = 0xc0,
        ZclSoftwareFailureStatus           = 0xc1,
        ZclCalibrationErrorStatus          = 0xc2,
        ZclClusterNotSupportedErrorStatus  = 0xc3
    };

    /*! \brief Common Application Support Layer (APS) status codes.
        \ingroup aps
     */
    enum ApsStatus
    {
        ApsSuccessStatus              = 0x00,
        ApsAsduTooLongStatus          = 0xa0,
        ApsDefragDeferredStatus       = 0xa1,
        ApsDefragUnsupportedStatus    = 0xa2,
        ApsIllegalRequestStatus       = 0xa3,
        ApsInvalidBindingStatus       = 0xa4,
        ApsInvalidGroupStatus         = 0xa5,
        ApsInvalidParameterStatus     = 0xa6,
        ApsNoAckStatus                = 0xa7,
        ApsNoBoundDeviceStatus        = 0xa8,
        ApsNoShortAddressStatus       = 0xa9,
        ApsNotSupportedStatus         = 0xaa,
        ApsSecuredLinkKeyStatus       = 0xab,
        ApsSecuredNwkKeyStatus        = 0xac,
        ApsSecurityFailStatus         = 0xad,
        ApsTableFullStatus            = 0xae,
        ApsUnsecuredStatus            = 0xaf,
        ApsUnsupportedAttributeStatus = 0xb0
    };

    /*! \brief Common Network Layer (NWK) status codes.
        \ingroup aps
     */
    enum NwkStatus
    {
        NwkInvalidParameterStatus     = 0xc1,
        NwkInvalidRequestStatus       = 0xc2,
        NwkNotPermittedStatus         = 0xc3,
        NwkStartupFailureStatus       = 0xc4,
        NwkAlreadyPresentStatus       = 0xc5,
        NwkSyncFailureStatus          = 0xc6,
        NwkNeighborTableFullStatus    = 0xc7,
        NwkNoNetworkStatus            = 0xca,
        NwkRouteDiscoveryFailedStatus = 0xd0,
        NwkRouteErrorStatus           = 0xd1,
        NwkBroadcastTableFullStatus   = 0xd2
    };

    /*! \brief Common Medium Access Control Layer (MAC) status codes.
        \ingroup aps
     */
    enum MacStatus
    {
        MacNoChannelAccess            = 0xe1,
        MacInvalidParameterStatus     = 0xe8,
        MacNoAckStatus                = 0xe9,
        MacNoBeaconStatus             = 0xea,
        MacTransactionExpiredStatus   = 0xf0
    };

    /*!
        Bitcloud's APSME-REQUEST-KEY.confirm status codes. The standard does not
        have a confirm primitive.
    */
    enum ApsRequestKeyStatus
    {
        ApsRequestKeySuccessStatus        = 0x00,
        ApsRequestKeyNoShortAddressStatus = 0x01,
        ApsRequestKeySecurityFailStatus   = 0x02,
        ApsRequestKeyNotSendStatus        = 0x03,
        ApsRequestKeyTimeoutStatus        = 0x04
    };

    /*! ZigBee device types. */
    enum AddressMode
    {
        NoAddress = 0x0,
        NwkAddress = 0x1,
        ExtAddress = 0x2,
        GroupAddress = 0x4
    };

    /*! ZigBee device types. */
    enum DeviceType
    {
        Coordinator,
        Router,
        EndDevice,
        UnknownDevice
    };

    enum NeighborPermitJoin
    {
        NeighborAcceptJoin,
        NeighborNotAcceptJoin,
        NeighborJoinUnknown
    };

    /*! Neighbortable relationship between devices. */
    enum DeviceRelationship
    {
        ParentRelation = 0x0,
        ChildRelation,
        SiblingRelation,
        UnknownRelation,
        PreviousChildRelation,
        UnauthenticatedChildRelation
    };

    /*!
        Type of a event send by the zmMaster.

        The event may have associated data as described below.
     */
    enum NetEvent
    {
        UnknownEvent,
        /*! zmNetEvent::value() holds the deCONZ::GeneralFrame. */
        GotGeneralFrame,
        /*! zmNetEvent::value() holds the deCONZ::State. */
        DeviceStateChanged,
        /*! zmNetEvent::value() holds a zmNet */
        GotNetworkConfig,
        /*! zmNetEvent::value() holds the deCONZ::ZdpState. */
        GotBindResponse,
        /*! zmNetEvent::value() holds the deCONZ::ZdpState. */
        GotUnbindResponse,
        /*! zmNetEvent::value() holds a BindingTable if isValid(). */
        GotMgmtBind,
        /*! zmNetEvent::pod() holds a zmNodeDescriptor. */
        GotNodeDescriptor,
        /*! zmNetEvent::pod() holds a zmPowerDescriptor. */
        GotPowerDescriptor,
        /*! zmNetEvent::pod() holds a zmSimpleDescriptor. */
        GotSimpleDescriptor,
        /*! zmNetEvent::pod() holds a zmComplexDescriptor. */
        GotComplexDescriptor,
        /*! zmNetEvent::value() holds a QString. */
        GotUserDescriptor,
        /*! zmNetEvent::pod() holds a zmNeighbor. */
        GotMgmtLqiPart,
        /*! zmNetEvent::value() holds a QByteArray with endpoints. */
        GotActiveEndpoints,
        /*! zmNetEvent::value() holds a QByteArray with (u8 lqi, u8 rssi). */
        GotLqiRssi,
        /*! zmNetEvent::value() holds a QByteArray with the ZCL Read Attributes response. */
        GotZclReadAttributes,
        /*! zmNetEvent::value() holds a zmNet. */
        GotNetDescriptor,
        /*!
          // TODO
         */
        GotNwkAddressList,
        /*! zmNetEvent::value() holds a deCONZ::ApsRequestKeyStatus status code. */
        GotLinkKey,
        /*! zmNetEvent::value() holds a zmApsDataConfirm. */
        GotApsDataConfirm,
        /*! zmNetEvent::value() holds a zmApsDataIndication. */
        GotApsDataIndication,
        /*!
            zmNetEvent::value() holds a zmNeighbor.
            zmNetEvent::listSize() holds the size of the neighbor table.
            zmNetEvent::listIndex() holds the index of this neighbor in the table.
         */
        NeighborUpdated,
        /*! zmNetEvent::value() holds a uint error code. */
        NotifyError,
        /*! zmNetEvent::value() holds a uint error code. */
        NotifyZdpError,
        /*! zmNetEvent::value() holds a uint error code. */
        NotifyZclError,
        /*! zmNetEvent::value() holds a uint error code. */
        NotifyNwkError,
        /*!
            zmNetEvent::value() holds a uint status code.
            zmNetEvent::cookie() holds the associated cookie.
         */
        NotifyStatus,
        /*! zmNetEvent::value() holds a QString. */
        NotifyText,
        /*! zmNetEvent::node() holds the node for which the data has changed. */
        NodeDataChanged,
        /*! zmNetEvent::node() holds the node which is gone. */
        NodeDeleted
    };

    /*! Holds various numeric values 8-64 bit, signed and unsigned. */
    union NumericUnion
    {
        uint8_t u8;
        uint16_t u16;
        uint32_t u32;
        uint64_t u64;
        int8_t s8;
        int16_t s16;
        int32_t s32;
        int64_t s64;
        float real;
    };

    struct BindReq
    {
        bool unbind; //!< true if this is a unbind request
        ZdpState rspState;
        uint64_t srcAddr;
        uint8_t srcEndpoint;
        uint16_t clusterId;
        uint8_t dstAddrMode;
        uint64_t dstExtAddr;
        uint8_t dstEndpoint;
        uint16_t dstGroupAddr; //!< only set then dstAddrMode = 0x1 (group)
        uint64_t binderAddr;
    };

    enum RequestId
    {
        ReqUnknown          = 0,
        ReqNwkAddr,
        ReqIeeeAddr,
        ReqNodeDescriptor,
        ReqPowerDescriptor,
        ReqSimpleDescriptor,
        ReqUserDescriptor,
        ReqActiveEndpoints,
        ReqMgmtLqi,
        ReqMgmtBind,

        ReqMaxItems
    };

    /*!
        \enum State
        The state of a device or node.
     */
    enum State
    {
        NotInNetwork = 0,
        Connecting,
        InNetwork,
        Leaving,
        UnknownState,
        Touchlink
    };

    /*!
        \enum ConnectMode
        Defines how the device connects to the network.
     */
    enum ConnectMode
    {
        ConnectModeManual = 0x00,
        ConnectModeNormal = 0x01,
        ConnectModeZll    = 0x02
    };

    /*!
        Common states for various purposes.
     */
    enum CommonState : unsigned char
    {
        IdleState = 0,
        BusyState,
        WaitState,
        ConfirmedState,
        TimeoutState,
        FailureState,
        FinishState,
        FireAndForgetState // deprecated
    };

    /*! \brief ZigBee frequency band.
        \ingroup zdp
     */
    enum FrequencyBand
    {
        UnknownFrequencyBand = 0,
        Freq868  = 0x08, //!< 868 - 868.6 MHz
        Freq902  = 0x20, //!< 902 - 928 MHz
        Freq2400 = 0x40  //!< 2400 - 2483.5 MHz
    };

    enum GraphicalTypes
    {
        GraphNodeType    = 1,
        GraphLinkType    = 2,
        GraphSocketType  = 3
    };
} // namespace deCONZ

#endif /* __cplusplus */

#endif // DECONZ_TYPES_H_
