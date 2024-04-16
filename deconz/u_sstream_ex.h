/*
 * Copyright (c) 2024 dresden elektronik ingenieurtechnik gmbh.
 * All rights reserved.
 *
 * The software in this package is published under the terms of the BSD
 * style license a copy of which has been included with this distribution in
 * the LICENSE.txt file.
 *
 */

#ifndef U_SSTREAM_EX_H
#define U_SSTREAM_EX_H

#include "u_sstream.h"

/*
 * Extension module for upstream u_sstream.
 */

#ifdef __cplusplus
extern "C" {
#endif

/*! Writes MAc address to stream:  00:11:22:33:44:55:66:77 */
U_LIBAPI void U_sstream_put_mac_address(U_SStream *ss, unsigned long long mac);
/*! Parses a hex value (without 0x prefix) and returns as byte: for example FF -> 255 */
U_LIBAPI unsigned char U_sstream_get_hex_byte(U_SStream *ss);

#ifdef __cplusplus
}
#endif

#endif /* U_SSTREAM_EX_H */
