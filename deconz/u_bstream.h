/*
 * Copyright (c) 2023-2024 Manuel Pietschmann.
 * All rights reserved.
 *
 * The software in this package is published under the terms of the BSD
 * style license a copy of which has been included with this distribution in
 * the LICENSE.txt file.
 *
 */

#ifndef U_BSTREAM_H
#define U_BSTREAM_H

#ifndef U_LIBAPI
#ifdef _WIN32
#ifdef USE_ULIB_SHARED
    #define U_LIBAPI  __declspec(dllimport)
  #endif
  #ifdef BUILD_ULIB_SHARED
    #define U_LIBAPI  __declspec(dllexport)
  #endif
#endif
#endif /* ! defined(U_LIBAPI) */

#ifndef U_LIBAPI
#define U_LIBAPI
#endif

/* byte stream */

typedef enum
{
    U_BSTREAM_OK,
    U_BSTREAM_READ_PAST_END,
    U_BSTREAM_WRITE_PAST_END,
    U_BSTREAM_NOT_INITIALISED
} U_BStreamStatus;

typedef struct U_BStream
{
    unsigned char *data;
    unsigned long pos;
    unsigned long size;
    U_BStreamStatus status;
} U_BStream;

#ifdef __cplusplus
extern "C" {
#endif

U_LIBAPI void U_bstream_init(U_BStream *bs, void *data, unsigned long size);
U_LIBAPI void U_bstream_put_u8(U_BStream *bs, unsigned char v);
U_LIBAPI void U_bstream_put_u16_le(U_BStream *bs, unsigned short v);
U_LIBAPI void U_bstream_put_s16_le(U_BStream *bs, signed short v);
U_LIBAPI void U_bstream_put_u32_le(U_BStream *bs, unsigned long v);
U_LIBAPI void U_bstream_put_s32_le(U_BStream *bs, signed long v);
U_LIBAPI unsigned char U_bstream_get_u8(U_BStream *bs);
U_LIBAPI unsigned short U_bstream_get_u16_le(U_BStream *bs);
U_LIBAPI signed short U_bstream_get_s16_le(U_BStream *bs);
U_LIBAPI unsigned short U_bstream_get_u16_be(U_BStream *bs);
U_LIBAPI unsigned long U_bstream_get_u32_le(U_BStream *bs);
U_LIBAPI signed long U_bstream_get_s32_le(U_BStream *bs);
U_LIBAPI unsigned long U_bstream_get_u32_be(U_BStream *bs);

#ifdef __cplusplus
}
#endif

#endif /* U_BSTREAM_H */
