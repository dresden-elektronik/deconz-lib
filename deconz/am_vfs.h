#ifndef AM_VFS_H
#define AM_VFS_H

/*
 * Copyright (c) 2012-2025 dresden elektronik ingenieurtechnik gmbh.
 * All rights reserved.
 *
 * The software in this package is published under the terms of the BSD
 * style license a copy of which has been included with this distribution in
 * the LICENSE.txt file.
 *
 */

/*
 * The message structures are documented in doc/actor_model_vfs.md
 */

#define VFS_LS_DIR_ENTRY_FLAGS_IS_DIR 0x0001

#define VFS_ENTRY_MODE_READONLY 0
#define VFS_ENTRY_MODE_WRITEABLE 1

/* Bit 16-19 Display */
#define VFS_ENTRY_MODE_DISPLAY_AUTO (0U << 16)
#define VFS_ENTRY_MODE_DISPLAY_HEX  (1U << 16)
#define VFS_ENTRY_MODE_DISPLAY_BIN  (2U << 16)


#define VFS_M_ID_LIST_DIR_REQ   AM_MESSAGE_ID_COMMON_REQUEST(1)
#define VFS_M_ID_LIST_DIR_RSP   AM_MESSAGE_ID_COMMON_RESPONSE(1)
#define VFS_M_ID_READ_ENTRY_REQ AM_MESSAGE_ID_COMMON_REQUEST(2)
#define VFS_M_ID_READ_ENTRY_RSP AM_MESSAGE_ID_COMMON_RESPONSE(2)
#define VFS_M_ID_CHANGED_NTFY   AM_MESSAGE_ID_COMMON_NOTIFY(5)
#define VFS_M_ID_ADDED_NTFY     AM_MESSAGE_ID_COMMON_NOTIFY(6)
#define VFS_M_ID_REMOVED_NTFY   AM_MESSAGE_ID_COMMON_NOTIFY(7)

#endif // AM_VFS_H
