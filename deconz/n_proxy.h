#ifndef N_PROXY_H
#define N_PROXY_H

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

typedef enum
{
    N_ProxyTypeNone = 0,
    N_ProxyTypeHttp,
    N_ProxyTypeSocks5
} N_ProxyType;

typedef struct
{
    N_ProxyType type;
    char host[256];
    unsigned short port;
} N_Proxy;

#ifdef __cplusplus
extern "C" {
#endif

/*! \returns the current configured proxy or ::type = N_ProxyNone
 */
DECONZ_DLLSPEC int N_GetProxy(N_Proxy *proxy);

#ifdef __cplusplus
}
#endif


#endif /* N_PROXY_H */
