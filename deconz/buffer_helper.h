#ifndef DECONZ_BUFFER_HELPER_H_
#define DECONZ_BUFFER_HELPER_H_

/*
 * Copyright (c) 2012-2023 dresden elektronik ingenieurtechnik gmbh.
 * All rights reserved.
 *
 * The software in this package is published under the terms of the BSD
 * style license a copy of which has been included with this distribution in
 * the LICENSE.txt file.
 *
 */

#include <stdint.h>
#include "deconz/declspec.h"

typedef union
{
    uint8_t u8;
    uint16_t u16;
    uint32_t u32;
    uint64_t u64;
    uint8_t raw[8];
} NumericBuffer_t;

#ifdef __cplusplus
extern "C" {
#endif

DECONZ_DLLSPEC uint8_t * put_u8_le(uint8_t *out, const uint8_t *in);
DECONZ_DLLSPEC uint8_t * put_u16_le(uint8_t *out, const uint16_t *in);
DECONZ_DLLSPEC uint8_t * put_u32_le(uint8_t *out, const uint32_t *in);
DECONZ_DLLSPEC uint8_t * put_u64_le(uint8_t *out, const uint64_t *in);
DECONZ_DLLSPEC const uint8_t * get_u8_le(const uint8_t *in, uint8_t *out);
DECONZ_DLLSPEC const uint8_t * get_u16_le(const uint8_t *in, uint16_t *out);
DECONZ_DLLSPEC const uint8_t * get_u32_le(const uint8_t *in, uint32_t *out);
DECONZ_DLLSPEC const uint8_t * get_u64_le(const uint8_t *in, uint64_t *out);

#ifdef __cplusplus
}
#endif
#endif /* DECONZ_BUFFER_HELPER_H_ */
