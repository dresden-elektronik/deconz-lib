#ifndef N_DOWNLOADER_H
#define N_DOWNLOADER_H

/*
 * Copyright (c) 2026 dresden elektronik ingenieurtechnik gmbh.
 * All rights reserved.
 *
 * The software in this package is published under the terms of the BSD
 * style license a copy of which has been included with this distribution in
 * the LICENSE.txt file.
 *
 */

#include "deconz/declspec.h"

typedef enum {
    N_DownloadStarting,
    N_DownloadRunning,
    N_DownloadDone,
    N_DownloadError,
    N_DownloadErrorNotFound, /* 404 */
    N_DownloadErrorSizeMismatch,
    N_DownloadErrorSendRequest,
    N_DownloadErrorHandshake,
    N_DownloadErrorTooLarge
} N_DownloadStatus;

typedef struct
{
    int handle;
    void *user;
    N_DownloadStatus status;
    const char *hdr;
    unsigned hdr_size;
    const unsigned char *data;
    unsigned data_size;
} N_DownloadData;

typedef void (*N_DownloadCallback)(N_DownloadData *);

#ifdef __cplusplus
extern "C" {
#endif

/*! Downloads data from a URL
    \returns a handle or 0 on failure
 */
DECONZ_DLLSPEC int N_Download(const char *url, unsigned maxsize, N_DownloadCallback cb, void *user);

DECONZ_DLLSPEC void N_DownloadInit(void);
DECONZ_DLLSPEC void N_DownloadShutDown(void);
DECONZ_DLLSPEC void N_DownloadStep(void);

#ifdef __cplusplus
}
#endif


#endif /* N_DOWNLOADER_H */
