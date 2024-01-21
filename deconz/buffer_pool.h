/*
 * Copyright (c) 2024 dresden elektronik ingenieurtechnik gmbh.
 * All rights reserved.
 *
 * The software in this package is published under the terms of the BSD
 * style license a copy of which has been included with this distribution in
 * the LICENSE.txt file.
 *
 */

#ifndef BP_BUFFER_POOL_H
#define BP_BUFFER_POOL_H

#include "deconz/declspec.h"
#include "deconz/file.h"

#define BP_PAGE_SIZE 4096

#define BP_PAGE_FLAG_ACCESS  1
#define BP_PAGE_FLAG_DIRTY   2
/*#define BP_PAGE_FLAG_LOCKED  4*/
#define BP_PAGE_FLAG_LOADED  8

typedef unsigned short bp_page_id;

typedef struct BP_Frame
{
    unsigned char data[BP_PAGE_SIZE];
} BP_Frame;

typedef struct BP_Page
{
    unsigned short frame;
    unsigned short page;
    unsigned short flags;
} BP_Page;

typedef struct BP_PageData
{
    bp_page_id page_id;
    unsigned char *data;
} BP_PageData;

typedef struct BP_BufferPool
{
    FS_File file;
    BP_Frame *frames;
    BP_Page *pages;
    unsigned n_frames;
    unsigned clock_cursor;
    unsigned n_pages_in_file;
} BP_BufferPool;

#ifdef __cplusplus
extern "C" {
#endif

DECONZ_DLLSPEC int BP_Init(BP_BufferPool *bp, const char *path, BP_Frame *frames, BP_Page *pages, unsigned n_frames);
DECONZ_DLLSPEC void BP_Destroy(BP_BufferPool *bp);
DECONZ_DLLSPEC void BP_Flush(BP_BufferPool *bp);
DECONZ_DLLSPEC int BP_LoadPage(BP_BufferPool *, bp_page_id, BP_PageData *);
DECONZ_DLLSPEC void BP_MarkPageDirty(BP_BufferPool *, bp_page_id);
DECONZ_DLLSPEC int BP_AllocPage(BP_BufferPool *, BP_PageData *);
DECONZ_DLLSPEC int BP_Truncate(BP_BufferPool *, unsigned);

#ifdef __cplusplus
}
#endif

#endif /* BP_BUFFER_POOL_H */
