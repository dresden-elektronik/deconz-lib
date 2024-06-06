/*
 * Copyright (c) 2024 dresden elektronik ingenieurtechnik gmbh.
 * All rights reserved.
 *
 * The software in this package is published under the terms of the BSD
 * style license a copy of which has been included with this distribution in
 * the LICENSE.txt file.
 *
 */

#include "deconz/n_tcp.h"
#include "deconz/u_assert.h"
#include "deconz/u_memory.h"


int N_TcpInit(N_TcpSocket *tcp, int af)
{
    return 0;
}

int N_TcpConnect(N_TcpSocket *tcp)
{
    return 0;
}

int N_TcpBind(N_TcpSocket *tcp, const N_Address *addr, unsigned short port)
{
    return 0;
}

int N_TcpListen(N_TcpSocket *tcp, int backlog)
{
    return 0;
}

int N_TcpAccept(N_TcpSocket *tcp, N_TcpSocket *client)
{
    return 0;
}

int N_TcpClose(N_TcpSocket *tcp)
{
    return 0;
}

int N_TcpCanRead(N_TcpSocket *tcp)
{
    return 0;
}

int N_TcpCanWrite(N_TcpSocket *tcp)
{
    return 0;
}

int N_TcpFlush(N_TcpSocket *tcp)
{
    return 0;
}

int N_TcpRead(N_TcpSocket *tcp, void *buf, unsigned maxlen)
{
    return 0;
}

int N_TcpWrite(N_TcpSocket *tcp, const void *buf, unsigned len)
{
    return 0;
}
