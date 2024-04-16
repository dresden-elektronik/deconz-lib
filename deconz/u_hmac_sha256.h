/*
 * Copyright (c) 2024 dresden elektronik ingenieurtechnik gmbh.
 * All rights reserved.
 *
 * The software in this package is published under the terms of the BSD
 * style license a copy of which has been included with this distribution in
 * the LICENSE.txt file.
 *
 */

#ifndef U_HMAC_SHA256_H
#define U_HMAC_SHA256_H

#include "deconz/declspec.h"

#ifdef __cplusplus
extern "C" {
#endif

/*! Creates a HMAC-SHA256

    \p key - secret key
    \p keysize - length of key in bytes, at least 32 bytes
    \p msg - message
    \p msgsize - length of message in bytes, must be non zero
    \p wrk - a temporary work buffer
    \p wrksize - size of work buffer in bytes
    \p result - buffer of 32 bytes

    \returns 1 on success
*/

DECONZ_DLLSPEC int U_HmacSha256(const void *key,
                                unsigned keysize,
                                const void *msg,
                                unsigned msgsize,
                                unsigned char *wrk,
                                unsigned wrksize,
                                unsigned char *result);

DECONZ_DLLSPEC void U_HmacSha256Test(void);

#ifdef __cplusplus
}
#endif

#endif /* U_HMAC_SHA256_H */
