/*
 * Copyright (c) 2024 dresden elektronik ingenieurtechnik gmbh.
 * All rights reserved.
 *
 * The software in this package is published under the terms of the BSD
 * style license a copy of which has been included with this distribution in
 * the LICENSE.txt file.
 *
 */

#ifndef FS_FILE_H
#define FS_FILE_H

#include "deconz/declspec.h"

#define FS_MODE_R 1
#define FS_MODE_RW 2

enum
{
    FS_SEEK_SET = 10,
    FS_SEEK_CUR = 11,
    FS_SEEK_END = 12
};

typedef struct FS_File
{
    void *fd;
    int flags;
} FS_File;

enum FS_EntryType
{
    FS_TYPE_UNKNOWN,
    FS_TYPE_BLOCK,
    FS_TYPE_CHARACTER_DEVICE,
    FS_TYPE_DIRECTORY,
    FS_TYPE_FIFO,
    FS_TYPE_LINK,
    FS_TYPE_FILE,
    FS_TYPE_SOCKET
};

typedef struct FS_DirEntry
{
    enum FS_EntryType type;
    char name[256];
} FS_DirEntry;

typedef struct FS_Dir
{
    void *p;
    int state;
    FS_DirEntry entry;
} FS_Dir;

#ifdef __cplusplus
extern "C" {
#endif

DECONZ_DLLSPEC int FS_OpenFile(FS_File *fp, int flags, const char *path);
DECONZ_DLLSPEC int FS_CloseFile(FS_File *fp);
DECONZ_DLLSPEC long FS_GetFileSize(FS_File *fp);
DECONZ_DLLSPEC long FS_ReadFile(FS_File *fp, void *buf, long max);
DECONZ_DLLSPEC long FS_WriteFile(FS_File *fp, const void *buf, long size);
DECONZ_DLLSPEC int FS_SeekFile(FS_File *fp, long offset, int whence);
DECONZ_DLLSPEC int FS_TruncateFile(FS_File *fp, long size);
DECONZ_DLLSPEC int FS_DeleteFile(const char *path);
DECONZ_DLLSPEC int FS_FileExists(const char *path);

DECONZ_DLLSPEC int FS_OpenDir(FS_Dir *dir, const char *path);
DECONZ_DLLSPEC int FS_ReadDir(FS_Dir *dir);
DECONZ_DLLSPEC int FS_CloseDir(FS_Dir *dir);

#ifdef __cplusplus
}
#endif


#endif /* FS_FILE_H */
