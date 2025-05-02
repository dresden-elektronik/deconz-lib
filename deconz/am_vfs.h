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

#include "actor/plugin.h"
#include "deconz/declspec.h"

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


#define AM_MAX_URL_ELEMENTS 16

typedef struct
{
    am_string url;
    unsigned element_count;
    unsigned char elements[AM_MAX_URL_ELEMENTS];
} am_url_parse;

typedef struct
{
    unsigned short tag;
    unsigned req_index;
    unsigned max_count;
    am_url_parse url_parse;
} am_ls_dir_req;

typedef struct
{
    unsigned short tag;
    am_url_parse url_parse;
} am_read_entry_req;

#ifdef __cplusplus
extern "C" {
#endif

DECONZ_DLLSPEC am_string AM_UrlElementAt(am_url_parse *up, unsigned idx);
DECONZ_DLLSPEC int AM_ParseUrl(am_url_parse *up);
DECONZ_DLLSPEC int AM_ParseListDirectoryRequest(struct am_api_functions *am, struct am_message *msg, am_ls_dir_req *req);
DECONZ_DLLSPEC int AM_ParseReadEntryRequest(struct am_api_functions *am, struct am_message *msg, am_read_entry_req *req);

#ifdef __cplusplus
}
#endif

#endif // AM_VFS_H
