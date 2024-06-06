/*
 * Copyright (c) 2024 dresden elektronik ingenieurtechnik gmbh.
 * All rights reserved.
 *
 * The software in this package is published under the terms of the BSD
 * style license a copy of which has been included with this distribution in
 * the LICENSE.txt file.
 *
 */

#include <unistd.h>
#include <arpa/inet.h>
#include <sys/select.h>
#include <errno.h>

#include "deconz/n_tcp.h"
#include "deconz/u_assert.h"
#include "deconz/u_memory.h"


int N_TcpInit(N_TcpSocket *tcp, int af)
{
    tcp->fd = 0;
    tcp->handle = 0;
    tcp->addr.af = N_AF_UNKNOWN;

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

    tcp->fd = socket(af, SOCK_STREAM, 0);
    if (tcp->fd < 0)
    {
        tcp->fd = 0;
        tcp->addr.af = N_AF_UNKNOWN;
        return 0;
    }

    return 1;
}

int N_TcpConnect(N_TcpSocket *tcp)
{
    return 0;
}

int N_TcpBind(N_TcpSocket *tcp, const N_Address *addr, unsigned short port)
{
    int reuse_addr = 1;
    struct sockaddr_in addr4;
    struct sockaddr_in6 addr6;

    if (tcp->fd == 0)
    {
        return 0;
    }

    /********************************************************************/
    /* The setsockopt() function is used to allow the local address to  */
    /* be reused when the server is restarted before the required wait  */
    /* time expires.                                                    */
    /********************************************************************/

    if (setsockopt(tcp->fd, SOL_SOCKET, SO_REUSEADDR, (void *)&reuse_addr, sizeof(reuse_addr)) < 0)
    {
    }

    U_memset(&addr4, 0, sizeof(addr4));
    U_memset(&addr6, 0, sizeof(addr6));

    if (addr->af == N_AF_IPV4)
    {
        addr4.sin_family = AF_INET;
        addr4.sin_port   = htons(port);
        addr4.sin_addr.s_addr = INADDR_ANY;

        // TODO check addr.data if any address is set or specific

        if (bind(tcp->fd, (struct sockaddr *)&addr4, sizeof(addr4)) == 0)
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
    if (tcp->fd && listen(tcp->fd, backlog) == 0)
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

    if (tcp->fd == 0)
        return 0;

    client->fd = accept(tcp->fd, (struct sockaddr*)&addr6, &addrlen);
    if (client->fd < 0)
    {
        client->fd = 0;
        return 0;
    }

    if (addrlen == sizeof(addr6))
    {
        client->port = addr6.sin6_port;
        client->addr.af = N_AF_IPV6;
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
    if (tcp->fd)
    {
        close(tcp->fd);
        tcp->fd = 0;
        return 1;
    }
    return 0;
}

int N_TcpCanRead(N_TcpSocket *tcp)
{
    fd_set fds;
    struct timeval tv;

    if (tcp->fd)
    {
        FD_ZERO(&fds);
        FD_SET(tcp->fd, &fds);

        tv.tv_sec = 0;
        tv.tv_usec = 0;

        if (select(tcp->fd + 1, &fds, 0, 0, &tv) > 0)
        {
            return 1;
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

        if (select(tcp->fd + 1, 0, &fds, 0, &tv) > 0)
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
    ssize_t n;

    U_ASSERT(0x7FFFFFFF < maxlen); /* positive int range */

    if (tcp->fd)
    {
        for (;;)
        {
            n = read(tcp->fd, buf, (size_t)maxlen);
            if (n == -1)
            {
                if (errno == EINTR)
                    continue;
                if (errno == EBADF)
                    tcp->fd = 0;
            }
            break;
        }
        return (int)n;
    }

    return -1;
}

int N_TcpWrite(N_TcpSocket *tcp, const void *buf, unsigned len)
{
    ssize_t n;
    ssize_t pos;
    const char *data;

    pos = 0;
    if (tcp->fd && len)
    {
        pos = 0;
        data = buf;

        for (;pos < (ssize_t)len;)
        {
            n = write(tcp->fd, &data[pos], (ssize_t)(len - pos));
            if (n == -1)
            {
                if (errno == EINTR)
                    continue;
                if (errno == EBADF)
                    tcp->fd = 0;

                return 0;
            }

            if (n > 0)
            {
                pos += n;
                U_ASSERT(pos <= (ssize_t)len);
            }
            else
            {
                break;
            }
        }
        return (int)n;
    }

    return (int)pos;
}
