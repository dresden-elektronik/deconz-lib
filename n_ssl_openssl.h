/*
 * Copyright (c) 2024 dresden elektronik ingenieurtechnik gmbh.
 * All rights reserved.
 *
 * The software in this package is published under the terms of the BSD
 * style license a copy of which has been included with this distribution in
 * the LICENSE.txt file.
 *
 */

#ifndef N_SSL_OPENSSL_H
#define N_SSL_OPENSSL_H

#include "deconz/n_ssl.h"

int N_SslInitOpenSsl(void);
int N_SslServerInitOpenSsl(N_SslSocket *sock, N_Address *addr, unsigned short port);
int N_SslAcceptOpenSsl(N_SslSocket *srv, N_SslSocket *cli);
int N_SslHandshakeOpenSsl(N_SslSocket *sock);
int N_SslWriteOpenSsl(N_SslSocket *sock, const void *buf, unsigned len);
int N_SslReadOpenSsl(N_SslSocket *sock, void *buf, unsigned len);
int N_SslCanReadOpenSsl(N_SslSocket *sock);

#endif /* N_SSL_OPENSSL_H */

