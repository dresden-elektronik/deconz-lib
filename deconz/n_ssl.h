/*
 * Copyright (c) 2024 dresden elektronik ingenieurtechnik gmbh.
 * All rights reserved.
 *
 * The software in this package is published under the terms of the BSD
 * style license a copy of which has been included with this distribution in
 * the LICENSE.txt file.
 *
 */

#ifndef N_SSL_H
#define N_SSL_H

#include "n_tcp.h"

typedef struct N_SslSocket
{
    N_TcpSocket tcp;
    void *_data[5]; /* buffer internal data structures of word size */

} N_SslSocket;

#ifdef __cplusplus
extern "C" {
#endif

DECONZ_DLLSPEC int N_SslInit(void);
DECONZ_DLLSPEC int N_SslServerInit(N_SslSocket *, N_Address*, unsigned short port, const char *certpath, const char *keypath);
DECONZ_DLLSPEC int N_SslAccept(N_SslSocket *srv, N_SslSocket *cli);
DECONZ_DLLSPEC int N_SslHandshake(N_SslSocket *sock);
DECONZ_DLLSPEC int N_SslRead(N_SslSocket *sock, void *buf, unsigned maxlen);
DECONZ_DLLSPEC int N_SslWrite(N_SslSocket *sock, const void *buf, unsigned len);
DECONZ_DLLSPEC int N_SslRead(N_SslSocket *sock, void *buf, unsigned len);
DECONZ_DLLSPEC int N_SslCanRead(N_SslSocket *sock);
DECONZ_DLLSPEC int N_SslClose(N_SslSocket *sock);

#ifdef __cplusplus
}
#endif

#endif /* N_SSL_H */
