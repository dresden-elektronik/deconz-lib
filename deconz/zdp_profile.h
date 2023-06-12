#ifndef ZDP_PROFILE_H_
#define ZDP_PROFILE_H_

/*
 * Copyright (c) 2012-2023 dresden elektronik ingenieurtechnik gmbh.
 * All rights reserved.
 *
 * The software in this package is published under the terms of the BSD
 * style license a copy of which has been included with this distribution in
 * the LICENSE.txt file.
 *
 */

#define ZDP_PROFILE_ID  0x0000
#define ZDO_ENDPOINT    0x00

// ZDP_CLUSTER_BEGIN
enum
{
// Device and Service Discovery commands
    ZDP_NWK_ADDR_CLID                = 0x0000,//!< Request for the 16-bit address of a remote device based on its known IEEE address.
    ZDP_IEEE_ADDR_CLID               = 0x0001,//!< Request for the 64-bit IEEE address of a remote device based on its known 16-bit address.
    ZDP_NODE_DESCRIPTOR_CLID         = 0x0002,//!< Request for the node descriptor of a remote device.
    ZDP_POWER_DESCRIPTOR_CLID        = 0x0003,//!< Request for the power descriptor of a remote device.
    ZDP_SIMPLE_DESCRIPTOR_CLID       = 0x0004,//!< Request for the simple descriptor of a remote device on the specified endpoint.
    ZDP_ACTIVE_ENDPOINTS_CLID        = 0x0005,//!< Request for the list of endpoints on a remote device with simple descriptors.
    ZDP_MATCH_DESCRIPTOR_CLID        = 0x0006,//!< Request for remote devices supporting a specific simple descriptor match criterion.
    ZDP_COMPLEX_DESCRIPTOR_CLID      = 0x0010,//!< Request for the complex descriptor of a remote device.
    ZDP_USER_DESCRIPTOR_CLID         = 0x0011,//!< Request for the user descriptor of a remote device.
    ZDP_DEVICE_ANNCE_CLID            = 0x0013,//!< Device_annce indication.
    ZDP_USER_DESCRIPTOR_SET_CLID     = 0x0014,//!< User_Desc_req.
    ZDP_PARENT_ANNOUNCE_CLID         = 0x001F,//!< Parent_annce indication.
    ZDP_END_DEVICE_BIND_REQ_CLID     = 0x0020,//!< Request from a end device to bind.
    ZDP_BIND_REQ_CLID                = 0x0021,//!< Request to bind two remote devices.
    ZDP_UNBIND_REQ_CLID              = 0x0022,//!< Request to unbind two remote devices.
    ZDP_MGMT_LQI_REQ_CLID            = 0x0031,//!< Request generated from a Local Device wishing to obtain a neighbor list for the Remote Device along with associated LQI values to each neighbor.
    ZDP_MGMT_RTG_REQ_CLID            = 0x0032,//!< Request for routing table.
    ZDP_MGMT_BIND_REQ_CLID           = 0x0033,//!< Request generated from a Local Device wishing to obtain a binding table of the Remote Device.
    ZDP_MGMT_LEAVE_REQ_CLID          = 0x0034,//!< Request a remote device to leave the network.
    ZDP_MGMT_PERMIT_JOINING_REQ_CLID = 0x0036,//!< Request to allow or disallow joining.
    ZDP_MGMT_NWK_UPDATE_REQ_CLID     = 0x0038,//!< Request mgmt nwk update.
    ZDP_NWK_ADDR_RSP_CLID            = 0x8000,//!<
    ZDP_IEEE_ADDR_RSP_CLID           = 0x8001,//!<
    ZDP_NODE_DESCRIPTOR_RSP_CLID     = 0x8002,//!<
    ZDP_POWER_DESCRIPTOR_RSP_CLID    = 0x8003,//!<
    ZDP_SIMPLE_DESCRIPTOR_RSP_CLID   = 0x8004,//!<
    ZDP_ACTIVE_ENDPOINTS_RSP_CLID    = 0x8005,//!<
    ZDP_MATCH_DESCRIPTOR_RSP_CLID    = 0x8006,//!<
    ZDP_USER_DESCRIPTOR_RSP_CLID     = 0x8011,//!< User descriptor response.
    ZDP_USER_DESCRIPTOR_CONF_CLID    = 0x8014,//!< User descriptor configuration response.
    ZDP_END_DEVICE_BIND_RSP_CLID     = 0x8020,//!< End device bind response.
    ZDP_BIND_RSP_CLID                = 0x8021,//!< Bind response.
    ZDP_UNBIND_RSP_CLID              = 0x8022,//!< Unbind response.
    ZDP_MGMT_LQI_RSP_CLID            = 0x8031,//!<
    ZDP_MGMT_BIND_RSP_CLID           = 0x8033,//!< Mgmt bind response.
    ZDP_MGMT_RTG_RSP_CLID            = 0x8032,//!< Mgtm routing table response.
    ZDP_MGMT_LEAVE_RSP_CLID          = 0x8034,//!< Mgmt leave response.
    ZDP_MGMT_PERMIT_JOINING_RSP_CLID = 0x8036,//!< Mgmt permit joining response.
    ZDP_MGMT_NWK_UPDATE_RSP_CLID     = 0x8038 //!< Mgmt nwk update response.
};
// ZDP_CLUSTER_END

enum
{
    ZDP_SUCCESS            = 0x00,
    ZDP_INV_REQUESTTYPE    = 0x80,
    ZDP_DEVICE_NOT_FOUND   = 0x81,
    ZDP_INVALID_EP         = 0x82,
    ZDP_NOT_ACTIVE         = 0x83,
    ZDP_NOT_SUPPORTED      = 0x84,
    ZDP_TIMEOUT            = 0x85,
    ZDP_NO_MATCH           = 0x86,
    ZDP_NO_ENTRY           = 0x88,
    ZDP_NO_DESCRIPTOR      = 0x89,
    ZDP_INSUFFICIENT_SPACE = 0x8a,
    ZDP_NOT_PERMITTED      = 0x8b,
    ZDP_TABLE_FULL         = 0x8c,
    ZDP_NOT_AUTHORIZED     = 0x8d,
};

/*!
    Current Power Mode in Node Power Descriptor.
 */
enum ZM_POWER_MODE
{
    ZM_POWER_MODE_ON_WHEN_IDLE =       0,  //!< Receiver synchronized with the receiver on when idle sub-field of the node descriptor.
    ZM_POWER_MODE_PERIODIC     = (1 << 0), //!< Receiver comes on periodically as defined by the node power descriptor.
    ZM_POWER_MODE_STIMULATED   = (1 << 1)  //!< Receiver comes on when stimulated, e.g. by a user pressing a button.
};

/*!
    Available/current Power Sources in Node Power Descriptor.
 */
enum ZM_POWER_SOURCE
{
    ZM_POWER_SOURCE_MAINS    = (1 << 0), //!< Constant (mains) power
    ZM_POWER_SOURCE_RECHARGE = (1 << 1), //!< Rechargeable battery
    ZM_POWER_SOURCE_DISPOSE  = (1 << 2)  //!< Disposable battery
};

/*!
    Current Power Source Level in Node Power Descriptor.
 */
enum ZM_POWER_LEVEL
{
    ZM_POWER_LEVEL_CRITICAL =  0, //!< Critical
    ZM_POWER_LEVEL_33       =  4, //!< 33%
    ZM_POWER_LEVEL_66       =  8, //!< 66%
    ZM_POWER_LEVEL_100      = 12  //!< 100%
};

#endif /* ZDP_PROFILE_H_ */
