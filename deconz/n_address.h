/*
 * Copyright (c) 2024 dresden elektronik ingenieurtechnik gmbh.
 * All rights reserved.
 *
 * The software in this package is published under the terms of the BSD
 * style license a copy of which has been included with this distribution in
 * the LICENSE.txt file.
 *
 */

#ifndef N_ADDRESS_H
#define N_ADDRESS_H

#include "deconz/declspec.h"

#define N_AF_UNKNOWN 0
#define N_AF_IPV4    4
#define N_AF_IPV6    6

/* Ipv4 and IPv6 addresses */
typedef struct N_Address
{
    unsigned char data[16];
    unsigned  char af; /* N_AF_* */
} N_Address;

#ifdef __cplusplus
extern "C" {
#endif

#ifdef __cplusplus
}
#endif

#endif /* N_ADDRESS_H */
