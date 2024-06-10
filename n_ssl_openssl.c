/*
 * Copyright (c) 2024 dresden elektronik ingenieurtechnik gmbh.
 * All rights reserved.
 *
 * The software in this package is published under the terms of the BSD
 * style license a copy of which has been included with this distribution in
 * the LICENSE.txt file.
 *
 */

#include <openssl/bio.h>
#include <openssl/ssl.h>

#include "deconz/u_assert.h"
#include "deconz/u_library_ex.h"
#include "deconz/u_memory.h"
#include "n_ssl_openssl.h"

#define N_FLAG_HANDSHAKE_DONE 1
#define N_FLAG_HAS_SSL_READ_DATA 4
#define N_FLAG_HAS_TCP_READ_DATA 8

static SSL_CTX *(*libSSL_CTX_new)(const SSL_METHOD *meth);
static void (*libSSL_CTX_free)(SSL_CTX *);
static int (*libSSL_CTX_use_certificate_file)(SSL_CTX *ctx, const char *file, int type);
static int (*libSSL_CTX_use_PrivateKey_file)(SSL_CTX *ctx, const char *file, int type);
static SSL *(*libSSL_new)(SSL_CTX *ctx);
static void (*libSSL_free)(SSL *ssl);
static void (*libSSL_set_bio)(SSL *s, BIO *rbio, BIO *wbio);
static void (*libSSL_set_accept_state)(SSL *s);
static int (*libSSL_is_init_finished)(const SSL *s);
static int (*libSSL_do_handshake)(SSL *s);
static int (*libSSL_write)(SSL *ssl, const void *buf, int num);
static int (*libSSL_read)(SSL *ssl, void *buf, int num);
static int (*libSSL_peek)(SSL *ssl, void *buf, int num);
static int (*libSSL_get_error)(const SSL *s, int ret_code);
static const SSL_METHOD *(*libTLS_server_method)(void);
static const SSL_METHOD *(*libTLS_client_method)(void);
static BIO *(*libBIO_new)(const BIO_METHOD *type);
static const BIO_METHOD *(*libBIO_s_mem)(void);
static int (*libBIO_write)(BIO *b, const void *data, int dlen);
static int (*libBIO_read)(BIO *b, void *data, int dlen);

static int nOpenSslInitialized = 0;

typedef struct N_PrivOpenSslServer
{
    SSL_CTX *ctx;
    SSL *ssl;
    BIO *rbio;
    BIO *wbio;
    unsigned flags;
} N_PrivOpenSsl;

int N_SslInitOpenSsl(void)
{
    void *lib;

    lib = U_library_open_ex("libssl");
    if (!lib)
        goto err;

    libSSL_CTX_new = U_library_symbol(lib, "SSL_CTX_new");
    libSSL_CTX_free = U_library_symbol(lib, "SSL_CTX_free");
    libSSL_CTX_use_certificate_file = U_library_symbol(lib, "SSL_CTX_use_certificate_file");
    libSSL_CTX_use_PrivateKey_file = U_library_symbol(lib, "SSL_CTX_use_PrivateKey_file");
    libSSL_new = U_library_symbol(lib, "SSL_new");
    libSSL_free = U_library_symbol(lib, "SSL_free");
    libSSL_set_bio = U_library_symbol(lib, "SSL_set_bio");
    libSSL_set_accept_state = U_library_symbol(lib, "SSL_set_accept_state");
    libSSL_is_init_finished = U_library_symbol(lib, "SSL_is_init_finished");
    libSSL_do_handshake = U_library_symbol(lib, "SSL_do_handshake");
    libSSL_peek = U_library_symbol(lib, "SSL_peek");
    libSSL_read = U_library_symbol(lib, "SSL_read");
    libSSL_write = U_library_symbol(lib, "SSL_write");
    libSSL_get_error = U_library_symbol(lib, "SSL_get_error");
    libTLS_server_method = U_library_symbol(lib, "TLS_server_method");
    libTLS_client_method = U_library_symbol(lib, "TLS_client_method");
    libBIO_new = U_library_symbol(lib, "BIO_new");
    libBIO_s_mem = U_library_symbol(lib, "BIO_s_mem");
    libBIO_write = U_library_symbol(lib, "BIO_write");
    libBIO_read = U_library_symbol(lib, "BIO_read");

    if (!libSSL_CTX_new ||
        !libSSL_CTX_free ||
        !libSSL_CTX_use_certificate_file ||
        !libSSL_CTX_use_PrivateKey_file ||
        !libSSL_new ||
        !libSSL_free ||
        !libSSL_set_bio ||
        !libSSL_set_accept_state ||
        !libSSL_is_init_finished ||
        !libSSL_do_handshake ||
        !libSSL_peek ||
        !libSSL_read ||
        !libSSL_write ||
        !libSSL_get_error ||
        !libTLS_server_method ||
        !libTLS_client_method ||
        !libBIO_new ||
        !libBIO_s_mem ||
        !libBIO_write ||
        !libBIO_read)
    {
        goto err;
    }

    nOpenSslInitialized = 1;
    return 1;

err:
    return 0;
}

int N_SslServerInitOpenSsl(N_SslSocket *sock, N_Address *addr, unsigned short port)
{
    U_ASSERT(sock);
    U_ASSERT(addr);
    U_ASSERT(nOpenSslInitialized);

    N_TcpSocket *tcp;
    N_PrivOpenSsl *priv;
    const SSL_METHOD *method;
    const BIO_METHOD *biomethod;


    tcp =&sock->tcp;
    priv = (N_PrivOpenSsl*)&sock->_data[0];
    method = libTLS_server_method();
    biomethod = libBIO_s_mem();

    U_memset(priv, 0, sizeof(*priv));

    if (method == 0)
        return 0;

    if (biomethod == 0)
        return 0;

    if (N_TcpInit(tcp, addr->af) == 0)
        goto err;

    if (N_TcpBind(tcp, addr, port) == 0)
        goto err;

    if (N_TcpListen(&sock->tcp, 64) == 0)
        goto err;

    priv->ctx = libSSL_CTX_new(method);
    if (!priv->ctx)
        goto err;

    const char *cert = "/home/mpi/src/tmp_ssl/cert.pem";
    const char *key = "/home/mpi//src/tmp_ssl/key.pem";

    if (libSSL_CTX_use_certificate_file(priv->ctx, cert,  SSL_FILETYPE_PEM) != 1)
          goto err;


    if (libSSL_CTX_use_PrivateKey_file(priv->ctx, key, SSL_FILETYPE_PEM) != 1)
        goto err;

    return 1;

err:
    if (priv->ctx)
        libSSL_CTX_free(priv->ctx);

    U_memset(priv, 0, sizeof(*priv));
    N_TcpClose(tcp);

    return 0;
}


int N_SslAcceptOpenSsl(N_SslSocket *srv, N_SslSocket *cli)
{
    N_PrivOpenSsl *srvpriv;
    N_PrivOpenSsl *clipriv;
    const BIO_METHOD *biomethod;

    srvpriv = (N_PrivOpenSsl*)&srv->_data[0];
    clipriv = (N_PrivOpenSsl*)&cli->_data[0];

    if (!srvpriv->ctx)
        return 0;

    if (N_TcpCanRead(&srv->tcp) == 0)
        return 0;

    if (N_TcpAccept(&srv->tcp, &cli->tcp) == 0)
        return 0;

    U_memset(clipriv, 0, sizeof(*clipriv));

    biomethod = libBIO_s_mem();
    U_ASSERT(biomethod);

    clipriv->rbio = libBIO_new(biomethod);
    clipriv->wbio = libBIO_new(biomethod);
    U_ASSERT(clipriv->rbio);
    U_ASSERT(clipriv->wbio);

    clipriv->ssl = libSSL_new(srvpriv->ctx);
    U_ASSERT(clipriv->ssl);
    if (!clipriv->ssl)
        goto err;

    libSSL_set_accept_state(clipriv->ssl);
    libSSL_set_bio(clipriv->ssl, clipriv->rbio, clipriv->wbio);
    return 1;


err:
    if (clipriv->ssl)
        libSSL_free(clipriv->ssl);

    U_memset(clipriv, 0, sizeof(*clipriv));
    N_TcpClose(&cli->tcp);

    return 0;
}

int N_SslHandshakeOpenSsl(N_SslSocket *sock)
{
    int i;
    int n;
    int ret;
    char buf[2048];
    N_PrivOpenSsl *priv;

    priv = (N_PrivOpenSsl*)&sock->_data[0];

    if (!priv->ssl)
        return 0;

    if (priv->flags & N_FLAG_HANDSHAKE_DONE)
        return 1;

    for (i = 0; i < 64; i++)
    {
        if (libSSL_is_init_finished(priv->ssl) == 1)
        {
            priv->flags |= N_FLAG_HANDSHAKE_DONE;
            return 1;
        }

        ret = libSSL_do_handshake(priv->ssl);
        if (ret < 0)
        {
            ret = libSSL_get_error(priv->ssl, ret);
            if      (ret == SSL_ERROR_WANT_READ) {  }
            else if (ret == SSL_ERROR_WANT_WRITE) {  }
            else
            {
                goto err;
            }
        }

        n = libBIO_read(priv->wbio, buf, sizeof(buf));

        if (n > 0)
        {
            /* server has encrypted data to send */
            if (N_TcpWrite(&sock->tcp, buf, n) != n)
                goto err;


        }
        else if (N_TcpCanRead(&sock->tcp))
        {
            /* received encrypted data from client */
            n = N_TcpRead(&sock->tcp, buf, sizeof(buf));
            if (n <= 0)
                goto err; /* hung up? */

            libBIO_write(priv->rbio, buf, n);
        }
    }

    return 0;

err:
    if (priv->ssl)
        libSSL_free(priv->ssl);

    U_memset(priv, 0, sizeof(*priv));
    N_TcpClose(&sock->tcp);

    return 0;
}


int N_SslWriteOpenSsl(N_SslSocket *sock, const void *buf, unsigned len)
{
    int n_ssl;
    int n_enc;
    N_PrivOpenSsl *priv;
    char bbuf[4096]; /* TODO does this has to be so big? */

    priv = (N_PrivOpenSsl*)&sock->_data[0];

    if (len == 0)
        return 0;

    if (!priv->ssl)
        return 0;

    if (N_TcpCanWrite(&sock->tcp) == 0)
        return 0;

    if (len > sizeof(bbuf) / 2)
        len = sizeof(bbuf) / 2;

    n_ssl = libSSL_write(priv->ssl, buf, len);
    if (n_ssl <= 0)
        return 0;

    n_enc = libBIO_read(priv->wbio, bbuf, sizeof(bbuf));
    U_ASSERT(n_enc > 0);
    if (n_enc >= n_ssl) /* shouldn't be smaller */
    {
        if (N_TcpWrite(&sock->tcp, bbuf, n_enc) != n_enc)
            return 0;

        return n_ssl;
    }

    return 0;
}

int N_SslReadOpenSsl(N_SslSocket *sock, void *buf, unsigned len)
{
    int i;
    int n;
    int ret;

    N_PrivOpenSsl *priv;

    priv = (N_PrivOpenSsl*)&sock->_data[0];

    if (!priv->ssl)
        return 0;

    if ((priv->flags & 1) == 0)
        return 1;

    /* read from TCP socket only if no decrypted data is in SSL object */
    if (priv->flags & N_FLAG_HAS_TCP_READ_DATA)
    {
        priv->flags &= ~N_FLAG_HAS_TCP_READ_DATA;

        /* received encrypted data from client */
        n = N_TcpRead(&sock->tcp, buf, len);
        if (n <= 0)
            goto err; /* hung up? */

        if (libBIO_write(priv->rbio, buf, n) != n)
        {
            goto err;
        }

        priv->flags |= N_FLAG_HAS_SSL_READ_DATA;
    }

    if (priv->flags & N_FLAG_HAS_SSL_READ_DATA)
    {
        priv->flags &= ~N_FLAG_HAS_SSL_READ_DATA;
        n = libSSL_read(priv->ssl, buf, len);
        if (n > 0)
            return n;
    }

    return 0;

err:
    if (priv->ssl)
        libSSL_free(priv->ssl);

    priv->flags |= 2;
    U_memset(priv, 0, sizeof(*priv));
    N_TcpClose(&sock->tcp);

    return 0;
}

int N_SslCanReadOpenSsl(N_SslSocket *sock)
{
    N_PrivOpenSsl *priv;

    priv = (N_PrivOpenSsl*)&sock->_data[0];

    if (!priv->ssl)
        return 0;

    priv->flags &= ~N_FLAG_HAS_SSL_READ_DATA;
    priv->flags &= ~N_FLAG_HAS_TCP_READ_DATA;

    /* 1) already decrypted data in SSL object */
    char buf[1];
    if (libSSL_peek(priv->ssl, buf, sizeof(buf)) > 0)
    {
        priv->flags |= N_FLAG_HAS_SSL_READ_DATA;
        return 1;
    }

    /* 2) encrypted data available in TCP socket */
    if (N_TcpCanRead(&sock->tcp))
    {
        priv->flags |= N_FLAG_HAS_TCP_READ_DATA;
        return 1;
    }

    return 0;
}
