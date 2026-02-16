/*
 * Copyright (c) 2024-2025 dresden elektronik ingenieurtechnik gmbh.
 * All rights reserved.
 *
 * The software in this package is published under the terms of the BSD
 * style license a copy of which has been included with this distribution in
 * the LICENSE.txt file.
 *
 */

#include <stddef.h>
#include <openssl/bio.h>
#include <openssl/ssl.h>

/* cert creation */
#include <openssl/pem.h>
#include <openssl/x509.h>
#include <openssl/evp.h>
#include <openssl/rsa.h>
#include <openssl/err.h>

#include "deconz/u_assert.h"
#include "deconz/u_library_ex.h"
#include "deconz/u_memory.h"
#include "deconz/file.h"
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
static int (*libSSL_shutdown)(SSL *ssl);
static int (*libSSL_get_error)(const SSL *s, int ret_code);
static const SSL_METHOD *(*libTLS_server_method)(void);
static const SSL_METHOD *(*libTLS_client_method)(void);
static BIO *(*libBIO_new)(const BIO_METHOD *type);
static const BIO_METHOD *(*libBIO_s_mem)(void);
static int (*libBIO_write)(BIO *b, const void *data, int dlen);
static int (*libBIO_read)(BIO *b, void *data, int dlen);

static EVP_PKEY_CTX *(*libEVP_PKEY_CTX_new_id)(int id, ENGINE *e);
static int (*libEVP_PKEY_keygen_init)(EVP_PKEY_CTX *ctx);
static void (*libEVP_PKEY_CTX_free)(EVP_PKEY_CTX *ctx);
static int (*libEVP_PKEY_CTX_set_rsa_keygen_bits)(EVP_PKEY_CTX *ctx, int mbits);
static int (*libEVP_PKEY_keygen)(EVP_PKEY_CTX *ctx, EVP_PKEY **ppkey);
static void (*libEVP_PKEY_free)(EVP_PKEY *pkey);
static const EVP_MD *(*libEVP_sha256)(void);

static X509 *(*libX509_new)(void);
static void (*libX509_free)(X509 *a);
static int (*libX509_set_version)(X509 *x, long version);
static int (*libASN1_INTEGER_set)(ASN1_INTEGER *a, long v);
static ASN1_INTEGER *(*libX509_get_serialNumber)(X509 *x);
static ASN1_TIME *(*libX509_gmtime_adj)(ASN1_TIME *s, long adj);
static ASN1_TIME *(*libX509_getm_notBefore)(const X509 *x);
static ASN1_TIME *(*libX509_getm_notAfter)(const X509 *x);
static int (*libX509_set_pubkey)(X509 *x, EVP_PKEY *pkey);
static X509_NAME *(*libX509_get_subject_name)(const X509 *a);
static int (*libX509_NAME_add_entry_by_txt)(X509_NAME *name, const char *field, int type,
                               const unsigned char *bytes, int len, int loc,
                               int set);
static int (*libX509_set_issuer_name)(X509 *x, const X509_NAME *name);
static int (*libX509_sign)(X509 *x, EVP_PKEY *pkey, const EVP_MD *md);

static int (*libPEM_write_PrivateKey)(FILE *fp, EVP_PKEY *x, const EVP_CIPHER *enc,
                         unsigned char *kstr, int klen,
                         pem_password_cb *cb, void *u);
static int (*libPEM_write_X509)(FILE *fp, X509 *x);

static int nOpenSslInitialized = 0;

typedef struct N_PrivOpenSslServer
{
    SSL_CTX *ctx;
    SSL *ssl;
    BIO *rbio;
    BIO *wbio;
    unsigned flags;
} N_PrivOpenSsl;

/**
 * @brief Creates a self-signed X.509 certificate.
 *
 * This function generates an RSA private key and a corresponding self-signed
 * certificate, saving them to the specified PEM files.
 *
 * @param key_file Path to write the private key PEM file.
 * @param cert_file Path to write the certificate PEM file.
 * @param bits The number of bits for the RSA key (e.g., 2048 or 4096).
 * @param days The number of days the certificate should be valid.
 * @param common_name The Common Name (CN) for the certificate's subject.
 * @return 0 on success, -1 on failure.
 */
static int n_SslCreateSelfSignedCert(const char *key_file, const char *cert_file, int bits, int days, const char *common_name)
{
// Philips Hue Pro Bridge uses the following
//  Server certificate:
//    subject: C=NL; O=Philips Hue; CN=C42996FFFEC5EA5B; OU=BSB003
//    start date: Feb 27 17:38:28 2025 GMT
//    expire date: Jan 19 03:14:07 2038 GMT
//    issuer: C=NL; O=Philips Hue; CN=root-bridge
//    Certificate level 0: Public key type EC/prime256v1 (256/128 Bits/secBits), signed using ecdsa-with-SHA256
//  SSL: certificate subject name 'C42996FFFEC5EA5B' does not match target hostname '192.168.178.32'


    // Phase 1: Key Generation
    // -----------------------
    EVP_PKEY *pkey = NULL;
    EVP_PKEY_CTX *ctx = libEVP_PKEY_CTX_new_id(EVP_PKEY_RSA, NULL);
    if (!ctx) {
        return -1;
    }

    if (libEVP_PKEY_keygen_init(ctx) <= 0) {
        libEVP_PKEY_CTX_free(ctx);
        return -1;
    }

    if (libEVP_PKEY_CTX_set_rsa_keygen_bits(ctx, bits) <= 0) {
        libEVP_PKEY_CTX_free(ctx);
        return -1;
    }

    if (libEVP_PKEY_keygen(ctx, &pkey) <= 0) {
        libEVP_PKEY_CTX_free(ctx);
        return -1;
    }
    libEVP_PKEY_CTX_free(ctx);

    // Phase 2: Certificate Creation and Configuration
    // -----------------------------------------------
    X509 *cert = libX509_new();
    if (!cert) {
        libEVP_PKEY_free(pkey);
        return -1;
    }

    // Set certificate version to v3 (value is 2)
    libX509_set_version(cert, 2);

    // Set a random serial number
    // TODO(mpi): use u_rand()
    libASN1_INTEGER_set(libX509_get_serialNumber(cert), (long)time(NULL));

    // Set validity period
    libX509_gmtime_adj(libX509_getm_notBefore(cert), 0); // Not before now
    libX509_gmtime_adj(libX509_getm_notAfter(cert), (long)days * 24 * 3600); // Not after 'days' from now

    // Set the public key for the certificate
    libX509_set_pubkey(cert, pkey);

    // Set the subject and issuer names (they are the same for a self-signed cert)
    X509_NAME *name = libX509_get_subject_name(cert);
    if (!name) {
        libX509_free(cert);
        libEVP_PKEY_free(pkey);
        return -1;
    }
    // Add Common Name (CN) entry
    libX509_NAME_add_entry_by_txt(name, "CN", MBSTRING_ASC, (const unsigned char *)common_name, -1, -1, 0);
    // You can add other fields like:
    // libX509_NAME_add_entry_by_txt(name, "C",  MBSTRING_ASC, (const unsigned char *)"US", -1, -1, 0);
    // libX509_NAME_add_entry_by_txt(name, "O",  MBSTRING_ASC, (const unsigned char *)"My Org", -1, -1, 0);

    libX509_set_issuer_name(cert, name);

    // Phase 3: Signing the Certificate
    // --------------------------------
    if (libX509_sign(cert, pkey, libEVP_sha256()) == 0) {
        libX509_free(cert);
        libEVP_PKEY_free(pkey);
        return -1;
    }

    // Phase 4: Writing to PEM files
    // -----------------------------
    FILE *key_fp = fopen(key_file, "wb");
    if (!key_fp) {
        libX509_free(cert);
        libEVP_PKEY_free(pkey);
        return -1;
    }
    if (!libPEM_write_PrivateKey(key_fp, pkey, NULL, NULL, 0, NULL, NULL)) {
        fclose(key_fp);
        libX509_free(cert);
        libEVP_PKEY_free(pkey);
        return -1;
    }
    fclose(key_fp);

    FILE *cert_fp = fopen(cert_file, "wb");
    if (!cert_fp) {
        libX509_free(cert);
        libEVP_PKEY_free(pkey);
        return -1;
    }
    if (!libPEM_write_X509(cert_fp, cert)) {
        fclose(cert_fp);
        libX509_free(cert);
        libEVP_PKEY_free(pkey);
        return -1;
    }
    fclose(cert_fp);

    libX509_free(cert);
    libEVP_PKEY_free(pkey);

    return 1;
}

int N_SslInitOpenSsl(void)
{
    void *lib;
    void *libCrypto;

    lib = U_library_open_ex("libssl");
    if (!lib)
        goto err;

    libCrypto = U_library_open_ex("libcrypto");
    if (!libCrypto)
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
    libSSL_shutdown = U_library_symbol(lib, "SSL_shutdown");
    libSSL_read = U_library_symbol(lib, "SSL_read");
    libSSL_write = U_library_symbol(lib, "SSL_write");
    libSSL_get_error = U_library_symbol(lib, "SSL_get_error");
    libTLS_server_method = U_library_symbol(lib, "TLS_server_method");
    libTLS_client_method = U_library_symbol(lib, "TLS_client_method");
    libBIO_new = U_library_symbol(libCrypto, "BIO_new");
    libBIO_s_mem = U_library_symbol(libCrypto, "BIO_s_mem");
    libBIO_write = U_library_symbol(libCrypto, "BIO_write");
    libBIO_read = U_library_symbol(libCrypto, "BIO_read");

    libEVP_PKEY_CTX_new_id = U_library_symbol(libCrypto, "EVP_PKEY_CTX_new_id");
    libEVP_PKEY_keygen_init = U_library_symbol(libCrypto, "EVP_PKEY_keygen_init");
    libEVP_PKEY_CTX_free = U_library_symbol(libCrypto, "EVP_PKEY_CTX_free");

    libEVP_PKEY_CTX_set_rsa_keygen_bits = U_library_symbol(libCrypto, "EVP_PKEY_CTX_set_rsa_keygen_bits");
    libEVP_PKEY_keygen = U_library_symbol(libCrypto, "EVP_PKEY_keygen");
    libEVP_PKEY_free = U_library_symbol(libCrypto, "EVP_PKEY_free");

    libEVP_sha256 = U_library_symbol(libCrypto, "EVP_sha256");
    libX509_new = U_library_symbol(libCrypto, "X509_new");
    libX509_free = U_library_symbol(libCrypto, "X509_free");
    libX509_set_version = U_library_symbol(libCrypto, "X509_set_version");
    libASN1_INTEGER_set = U_library_symbol(libCrypto, "ASN1_INTEGER_set");

    libX509_get_serialNumber = U_library_symbol(libCrypto, "X509_get_serialNumber");
    libX509_gmtime_adj = U_library_symbol(libCrypto, "X509_gmtime_adj");
    libX509_getm_notBefore = U_library_symbol(libCrypto, "X509_getm_notBefore");
    libX509_getm_notAfter = U_library_symbol(libCrypto, "X509_getm_notAfter");
    libX509_set_pubkey = U_library_symbol(libCrypto, "X509_set_pubkey");

    libX509_get_subject_name = U_library_symbol(libCrypto, "X509_get_subject_name");
    libX509_NAME_add_entry_by_txt = U_library_symbol(libCrypto, "X509_NAME_add_entry_by_txt");
    libX509_set_issuer_name = U_library_symbol(libCrypto, "X509_set_issuer_name");
    libX509_sign = U_library_symbol(libCrypto, "X509_sign");
    libPEM_write_PrivateKey = U_library_symbol(libCrypto, "PEM_write_PrivateKey");
    libPEM_write_X509 = U_library_symbol(libCrypto, "PEM_write_X509");

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
        !libSSL_shutdown ||
        !libSSL_read ||
        !libSSL_write ||
        !libSSL_get_error ||
        !libTLS_server_method ||
        !libTLS_client_method ||
        !libBIO_new ||
        !libBIO_s_mem ||
        !libBIO_write ||
        !libBIO_read ||
        !libEVP_PKEY_CTX_new_id ||
        !libEVP_PKEY_keygen_init ||
        !libEVP_PKEY_CTX_free ||
        !libEVP_PKEY_CTX_set_rsa_keygen_bits ||
        !libEVP_PKEY_keygen ||
        !libEVP_PKEY_free ||
        !libEVP_sha256 ||
        !libX509_new ||
        !libX509_free ||
        !libX509_set_version ||
        !libASN1_INTEGER_set ||
        !libX509_get_serialNumber ||
        !libX509_gmtime_adj ||
        !libX509_getm_notBefore ||
        !libX509_getm_notAfter ||
        !libX509_set_pubkey ||
        !libX509_get_subject_name ||
        !libX509_NAME_add_entry_by_txt ||
        !libX509_set_issuer_name ||
        !libX509_sign ||
        !libPEM_write_PrivateKey ||
        !libPEM_write_X509)
    {
        goto err;
    }

    nOpenSslInitialized = 1;
    return 1;

err:
    return 0;
}

int N_SslServerInitOpenSsl(N_SslSocket *sock, N_Address *addr, unsigned short port, const char *certpath, const char *keypath)
{
    U_ASSERT(sock);
    U_ASSERT(addr);
    U_ASSERT(nOpenSslInitialized);

    N_TcpSocket *tcp;
    N_PrivOpenSsl *priv;
    const SSL_METHOD *method;
    const BIO_METHOD *biomethod;

    if ((0 == FS_FileExists(certpath)) || (0 == FS_FileExists(keypath)))
    {
        const int rsa_bits = 4096;
        const int validity_days = 365*42;
        const char *cn = "localhost";

        if (n_SslCreateSelfSignedCert(keypath, certpath, rsa_bits, validity_days, cn) != 1)
        {
          return 0;
        }
    }

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

    if (libSSL_CTX_use_certificate_file(priv->ctx, certpath,  SSL_FILETYPE_PEM) != 1)
          goto err;

    if (libSSL_CTX_use_PrivateKey_file(priv->ctx, keypath, SSL_FILETYPE_PEM) != 1)
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
        return -1;

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

    return -1;
}

int N_SslWriteOpenSsl(N_SslSocket *sock, const void *buf, unsigned len)
{
    int n;
    int n_ssl;
    int n_enc;
    unsigned outlen;
    unsigned written = 0;
    N_PrivOpenSsl *priv;
    const char *in = buf;
    char bbuf[4096]; /* TODO does this has to be so big? */

    priv = (N_PrivOpenSsl*)&sock->_data[0];

    if (len == 0)
        return 0;

    if (!priv->ssl)
        return 0;

    for (;written != len;)
    {
        if (N_TcpCanWrite(&sock->tcp) == 0)
            return 0;

        U_ASSERT(written <= len);
        outlen = len - written;
        if (outlen > sizeof(bbuf) / 2)
            outlen = sizeof(bbuf) / 2;

        n_ssl = libSSL_write(priv->ssl, &in[written], outlen);
        if (n_ssl <= 0)
            return 0;

        n_enc = libBIO_read(priv->wbio, bbuf, sizeof(bbuf));
        U_ASSERT(n_enc > 0);
        if (n_enc >= n_ssl) /* shouldn't be smaller */
        {
            n = N_TcpWrite(&sock->tcp, bbuf, n_enc);
            if (n != n_enc)
                return 0;

            written += n_ssl;
        }
        else
        {
            // TODO is this a thing? report error?
            U_ASSERT(0 && "unexpected libBIO_read() result");
            return 0;
        }
    }

    return (int)written;
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

    if ((priv->flags & N_FLAG_HANDSHAKE_DONE) == 0)
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

int N_SslCloseOpenSsl(N_SslSocket *sock)
{
    int n;
    char buf[2048];
    N_PrivOpenSsl *priv;

    priv = (N_PrivOpenSsl*)&sock->_data[0];

    if (!priv->ssl)
        return 0;

    if (priv->flags & N_FLAG_HANDSHAKE_DONE)
    {
        libSSL_shutdown(priv->ssl);
        n = libBIO_read(priv->wbio, buf, sizeof(buf));

        if (n > 0)
        {
            /* server has encrypted data to send */
            if (N_TcpWrite(&sock->tcp, buf, n) != n)
            {
                /* ignore and close TCP anyway */
            }
        }
    }

    if (priv->ssl)
        libSSL_free(priv->ssl);

    U_memset(priv, 0, sizeof(*priv));
    N_TcpClose(&sock->tcp);

    return 0;
}
