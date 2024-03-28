/*
 * Copyright (c) 2024 dresden elektronik ingenieurtechnik gmbh.
 * All rights reserved.
 *
 * The software in this package is published under the terms of the BSD
 * style license a copy of which has been included with this distribution in
 * the LICENSE.txt file.
 *
 */

#ifndef U_SHA1_H
#define U_SHA1_H

#include "deconz/declspec.h"

#define U_SHA1_HASH_SIZE 20

#ifdef __cplusplus
extern "C" {
#endif

DECONZ_DLLSPEC int U_Sha1(const void *data, unsigned size, unsigned char *hash);

#ifdef __cplusplus
}
#endif

#endif /* U_SHA1_H */
