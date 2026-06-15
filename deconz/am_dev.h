#ifndef AM_DEV_H
#define AM_DEV_H

#define AM_ACTOR_ID_CORE_DEV    2008

// Notifications: CORE_DEV → GUI_MAINWINDOW
// payload: u32 packed state
//   bits 0-7:   flags (bit 0 = open, bit 1 = connected, bits 2-7 reserved)
//   bits 8-15:  netState (deCONZ::State enum 0-5)
//   bits 16-23: reason (disconnect reason, 0 if not disconnecting)
//   bits 24-31: reserved for future extensions
#define M_ID_DEV_STATE              AM_MESSAGE_ID_SPECIFIC_NOTIFY(10)
#define M_ID_DEV_ACTIVITY           AM_MESSAGE_ID_SPECIFIC_NOTIFY(13)
#define M_ID_DEV_TIMEOUT            AM_MESSAGE_ID_SPECIFIC_NOTIFY(14)

// Requests: GUI_MAINWINDOW → CORE_DEV
#define M_ID_DEV_CONNECT_REQ        AM_MESSAGE_ID_SPECIFIC_REQUEST(20)
#define M_ID_DEV_DISCONNECT_REQ     AM_MESSAGE_ID_SPECIFIC_REQUEST(21)
#define M_ID_DEV_REBOOT_REQ         AM_MESSAGE_ID_SPECIFIC_REQUEST(22)
#define M_ID_DEV_JOIN_REQ           AM_MESSAGE_ID_SPECIFIC_REQUEST(23)
#define M_ID_DEV_LEAVE_REQ          AM_MESSAGE_ID_SPECIFIC_REQUEST(24)
#ifdef QT_DEBUG
#define M_ID_DEV_FACTORY_RESET_REQ  AM_MESSAGE_ID_SPECIFIC_REQUEST(25)
#endif

// VFS paths for device info queries
#define VFS_DEV_INFO_FIRMWARE       "device/info/firmware"
#define VFS_DEV_INFO_PROTOCOL       "device/info/protocol"
#define VFS_DEV_INFO_NAME           "device/info/name"
#define VFS_DEV_INFO_PATH           "device/info/path"

// VFS path for combined device state (returns packed u32, same format as M_ID_DEV_STATE)
#define VFS_DEV_STATE               "device/state"

#endif