/*
 * Copyright (c) 2024 dresden elektronik ingenieurtechnik gmbh.
 * All rights reserved.
 *
 * The software in this package is published under the terms of the BSD
 * style license a copy of which has been included with this distribution in
 * the LICENSE.txt file.
 *
 */

#ifndef N_TCP_H
#define N_TCP_H

#include "n_address.h"

typedef struct N_TcpSocket
{
    N_Address addr;
    unsigned short port;

#if defined(_WIN32) || defined(_WIN64)
#ifdef _WIN64
        unsigned long long int fd;
#else  /* Windows 32-bit build */
        unsigned int fd;
#endif
#else /* UNIX */
        int fd;
#endif

} N_TcpSocket;

#ifdef __cplusplus
extern "C" {
#endif

DECONZ_DLLSPEC int N_TcpInit(N_TcpSocket *tcp, int af);
DECONZ_DLLSPEC int N_TcpConnect(N_TcpSocket *tcp);
DECONZ_DLLSPEC int N_TcpBind(N_TcpSocket *tcp, const N_Address *addr, unsigned short port);
DECONZ_DLLSPEC int N_TcpListen(N_TcpSocket *tcp, int backlog);
DECONZ_DLLSPEC int N_TcpAccept(N_TcpSocket *tcp, N_TcpSocket *client);
DECONZ_DLLSPEC int N_TcpClose(N_TcpSocket *tcp);
DECONZ_DLLSPEC int N_TcpCanRead(N_TcpSocket *tcp);
DECONZ_DLLSPEC int N_TcpCanWrite(N_TcpSocket *tcp);
DECONZ_DLLSPEC int N_TcpFlush(N_TcpSocket *tcp);
DECONZ_DLLSPEC int N_TcpRead(N_TcpSocket *tcp, void *buf, unsigned maxlen);
DECONZ_DLLSPEC int N_TcpWrite(N_TcpSocket *tcp, const void *buf, unsigned len);

#ifdef __cplusplus
}
#endif

#endif /* N_TCP_H */
