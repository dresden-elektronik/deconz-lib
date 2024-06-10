/*
 * Copyright (c) 2024 dresden elektronik ingenieurtechnik gmbh.
 * All rights reserved.
 *
 * The software in this package is published under the terms of the BSD
 * style license a copy of which has been included with this distribution in
 * the LICENSE.txt file.
 *
 */

#include "deconz/n_ssl.h"

#ifdef HAS_OPENSSL
    #include "n_ssl_openssl.h"
#endif


/*
 * Support multiple backends in future (WolfSSL, BoringSSL, etc.).
 */
enum N_SSL_Backend
{
    N_SSL_BACKEND_NONE,
    N_SSL_BACKEND_OPENSSL
};

static enum N_SSL_Backend n_ssl_backend = N_SSL_BACKEND_NONE;


int N_SslInit(void)
{
#ifdef HAS_OPENSSL
    if (N_SslInitOpenSsl())
    {
        n_ssl_backend = N_SSL_BACKEND_OPENSSL;
        return 1;
    }
#endif
    return 0;
}

int N_SslServerInit(N_SslSocket *sock, N_Address *addr, unsigned short port)
{
    if (n_ssl_backend == N_SSL_BACKEND_OPENSSL)
        return N_SslServerInitOpenSsl(sock, addr, port);

    return 0;
}

int N_SslAccept(N_SslSocket *srv, N_SslSocket *cli)
{
    if  (n_ssl_backend == N_SSL_BACKEND_OPENSSL)
        return N_SslAcceptOpenSsl(srv, cli);

    return 0;
}

int N_SslHandshake(N_SslSocket *sock)
{
    if  (n_ssl_backend == N_SSL_BACKEND_OPENSSL)
        return N_SslHandshakeOpenSsl(sock);

    return 0;
}

int N_SslWrite(N_SslSocket *sock, const void *buf, unsigned len)
{
    if  (n_ssl_backend == N_SSL_BACKEND_OPENSSL)
        return N_SslWriteOpenSsl(sock, buf, len);

    return 0;
}

int N_SslRead(N_SslSocket *sock, void *buf, unsigned len)
{
    if  (n_ssl_backend == N_SSL_BACKEND_OPENSSL)
        return N_SslReadOpenSsl(sock, buf, len);

    return 0;
}

int N_SslCanRead(N_SslSocket *sock)
{
    if  (n_ssl_backend == N_SSL_BACKEND_OPENSSL)
        return N_SslCanReadOpenSsl(sock);

    return 0;
}
