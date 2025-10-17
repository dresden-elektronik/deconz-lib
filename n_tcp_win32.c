/*
 * Copyright (c) 2024 dresden elektronik ingenieurtechnik gmbh.
 * All rights reserved.
 *
 * The software in this package is published under the terms of the BSD
 * style license a copy of which has been included with this distribution in
 * the LICENSE.txt file.
 *
 */

#include <winsock2.h>
#include <ws2tcpip.h>

#include "deconz/n_tcp.h"
#include "deconz/u_assert.h"
#include "deconz/u_memory.h"
#include "deconz/u_sstream.h"

static int winSockInit = 0;

int N_TcpInit(N_TcpSocket *tcp, int af)
{
    tcp->fd = INVALID_SOCKET;
    tcp->addr.af = N_AF_UNKNOWN;

    if (winSockInit == 0)
    {
        WSADATA wsaData;
        U_memset(&wsaData, 0, sizeof(wsaData));
        int res = WSAStartup(MAKEWORD(2, 2), &wsaData);
        if (res != 0)
            return 0;

        winSockInit = 1;
    }

    if (af == N_AF_IPV4)
    {
        tcp->addr.af = af;
        af = AF_INET;
    }
    else if (af == N_AF_IPV6)
    {
        tcp->addr.af = af;
        af = AF_INET6;
    }
    else
    {
        return 0;
    }

    U_ASSERT(sizeof(SOCKET) <= sizeof(tcp->fd));

    tcp->fd = socket(af, SOCK_STREAM, IPPROTO_TCP);
    if (tcp->fd == INVALID_SOCKET)
    {
        tcp->fd = 0;
        tcp->addr.af = N_AF_UNKNOWN;
        return 0;
    }

    return 1;
}

int N_TcpConnect(N_TcpSocket *tcp, const char *host, unsigned short port)
{
    struct addrinfo hints, *res, *p;
    U_SStream ss;
    char portstr[16];

    U_sstream_init(&ss, portstr, sizeof(portstr));
    U_sstream_put_long(&ss, (long)port);

    U_memset(&hints, 0, sizeof hints);
    hints.ai_family = AF_UNSPEC;

    if      (tcp->addr.af == N_AF_IPV4) { hints.ai_family = AF_INET; }
    else if (tcp->addr.af == N_AF_IPV6) { hints.ai_family = AF_INET6; }
    else                                { return 0; }

    if (tcp->fd == 0)
        return 0;

    hints.ai_socktype = SOCK_STREAM;
    if (getaddrinfo(host, portstr, &hints, &res) != 0)
        return 0;

    for (p = res; p != NULL; p = p->ai_next)
    {
        size_t addrsz = 0;

        if (p->ai_family == AF_INET)
        {
            addrsz = sizeof(struct sockaddr_in);
        }
        else if (p->ai_family == AF_INET6)
        {
            addrsz = sizeof(struct sockaddr_in6);
        }

        if (addrsz)
        {
            if (connect(tcp->fd, p->ai_addr, addrsz) == 0)
                return 1;
        }
    }

    N_TcpClose(tcp);
    return 0;
}

int N_TcpBind(N_TcpSocket *tcp, const N_Address *addr, unsigned short port)
{
    BOOL reuse_addr = TRUE;
    BOOL disable_dual_sock = FALSE;
    struct sockaddr_in addr4;
    struct sockaddr_in6 addr6;

    if (tcp->fd == INVALID_SOCKET)
    {
        return 0;
    }

    /********************************************************************/
    /* The setsockopt() function is used to allow the local address to  */
    /* be reused when the server is restarted before the required wait  */
    /* time expires.                                                    */
    /********************************************************************/

    if (setsockopt(tcp->fd, SOL_SOCKET, SO_REUSEADDR, (void *)&reuse_addr, sizeof(reuse_addr)) != 0)
    {
    }

    /* Make dual stack socket (disabled on Windows by default)
     * Only since Windows Vista.
     */
    if (addr->af == N_AF_IPV6)
    {
        if (setsockopt(tcp->fd, IPPROTO_IPV6, IPV6_V6ONLY, (void *)&disable_dual_sock, sizeof(disable_dual_sock)) != 0)
        {
        }
    }

    U_memset(&addr4, 0, sizeof(addr4));
    U_memset(&addr6, 0, sizeof(addr6));

    if (addr->af == N_AF_IPV4)
    {
        addr4.sin_family = AF_INET;
        addr4.sin_port   = htons(port);
        addr4.sin_addr.s_addr = INADDR_ANY;

        // TODO check addr.data if any address is set or specific

        if (bind(tcp->fd, (SOCKADDR *)&addr4, sizeof(addr4)) == 0)
        {
            return 1;
        }
    }
    else if (addr->af == N_AF_IPV6)
    {
        addr6.sin6_family = AF_INET6;
        addr6.sin6_port   = htons(port);
        addr6.sin6_addr   = in6addr_any;

        // TODO check addr.data if any address is set or specific

        if (bind(tcp->fd, (struct sockaddr *)&addr6, sizeof(addr6)) == 0)
        {
            return 1;
        }
    }

    return 0;
}

int N_TcpListen(N_TcpSocket *tcp, int backlog)
{
    if (tcp->fd != INVALID_SOCKET && listen(tcp->fd, backlog) == 0)
    {
        return 1;
    }

    return 0;
}

int N_TcpAccept(N_TcpSocket *tcp, N_TcpSocket *client)
{
    struct sockaddr_in6 addr6;
    struct sockaddr_in *addr4;
    socklen_t addrlen = sizeof(addr6);

    client->addr.af = N_AF_UNKNOWN;

    if (tcp->fd == INVALID_SOCKET)
        return 0;

    client->fd = accept(tcp->fd, (struct sockaddr*)&addr6, &addrlen);
    if (client->fd == INVALID_SOCKET)
    {
        client->fd = INVALID_SOCKET;
        return 0;
    }

    if (addrlen == sizeof(addr6))
    {
        client->port = addr6.sin6_port;
        client->addr.af = N_AF_IPV6; /* this can also be a mapped IPv4 address */
        U_memcpy(&client->addr.data[0], &addr6.sin6_addr, 16);
    }
    else if (addrlen == sizeof(*addr4))
    {
        addr4 = (struct sockaddr_in*)&addr6;
        client->port = addr4->sin_port;
        client->addr.af = N_AF_IPV4;
        U_memcpy(&client->addr.data[0], &addr4->sin_addr.s_addr, 4);
    }
    else
    {
        /* TODO test, shouldn't happen */
        client->addr.af = N_AF_UNKNOWN;
        client->port = 0;
    }

    return 1;
}

int N_TcpClose(N_TcpSocket *tcp)
{
    if (tcp->fd != INVALID_SOCKET)
    {
        shutdown(tcp->fd, SD_BOTH);
        closesocket(tcp->fd);
        tcp->fd = INVALID_SOCKET;
        return 1;
    }
    return 0;
}

int N_TcpCanRead(N_TcpSocket *tcp)
{
    int ret;
    fd_set fds;
    struct timeval tv;

    if (tcp->fd != INVALID_SOCKET)
    {
        FD_ZERO(&fds);
        FD_SET(tcp->fd, &fds);

        tv.tv_sec = 0;
        tv.tv_usec = 0;

        ret = select(0 /* nfds ignored by winsock */, &fds, 0, 0, &tv);
        if (ret > 0)
        {
            return 1;
        }

        if (ret == SOCKET_ERROR)
        {
            ret = WSAGetLastError();
            if (ret == WSAENOTSOCK)
            {
                /*
                 * We may land here if the socket wasn't initialized and fd is 0.
                 * Because this is Windows where INVALID_SOCKET != 0.
                 */
                tcp->fd = INVALID_SOCKET;
            }
        }
    }

    return 0;
}

int N_TcpCanWrite(N_TcpSocket *tcp)
{
    fd_set fds;
    struct timeval tv;

    if (tcp->fd)
    {
        FD_ZERO(&fds);
        FD_SET(tcp->fd, &fds);

        tv.tv_sec = 0;
        tv.tv_usec = 0;

        if (select(0 /* nfds ignored by winsock */, 0, &fds, 0, &tv) > 0)
        {
            return 1;
        }
    }
    return 0;
}

int N_TcpFlush(N_TcpSocket *tcp)
{
    return 0;
}

int N_TcpRead(N_TcpSocket *tcp, void *buf, unsigned maxlen)
{
    int n;

    U_ASSERT(maxlen <= 0x7FFFFFFF); /* positive int range */

    if (tcp->fd != INVALID_SOCKET)
    {
        for (;;)
        {
            n = recv(tcp->fd, buf, (int)maxlen, 0);
            if (n == SOCKET_ERROR )
            {
                n = WSAGetLastError();
                if (n == WSAEINTR)
                    continue;
                if (n == WSAENOTCONN || n == WSAESHUTDOWN)
                    tcp->fd = INVALID_SOCKET;
                if (n == WSAECONNABORTED)
                {
                    N_TcpClose(tcp);
                }
                n = -1;
            }
            break;
        }
        return n;
    }

    return -1;
}

int N_TcpWrite(N_TcpSocket *tcp, const void *buf, unsigned len)
{
    int n;
    int pos;
    const char *data;

    U_ASSERT(len <= 0x7FFFFFFF);

    pos = 0;
    if (tcp->fd != INVALID_SOCKET && len)
    {
        pos = 0;
        data = buf;

        for (;pos < (int)len;)
        {
            n = send(tcp->fd, &data[pos], ((int)len - pos), 0 /* flags */);
            if (n == SOCKET_ERROR)
            {
                n = WSAGetLastError();
                if (n == WSAEINTR)
                    continue;
                if (n == WSAENOTCONN || n == WSAESHUTDOWN)
                    tcp->fd = INVALID_SOCKET;

                return 0;
            }

            if (n > 0)
            {
                pos += n;
                U_ASSERT(pos <= (int)len);
            }
            else
            {
                break;
            }
        }
    }

    return pos;
}
