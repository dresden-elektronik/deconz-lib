#include "deconz/u_assert.h"
#include "deconz/u_random.h"
#include "deconz/u_library_ex.h"

static void (*RAND_bytes_openssl)(unsigned char*, int);

static int U_RandBytesOpenSSL(unsigned char *buf, unsigned bufsize)
{
    U_ASSERT(bufsize <= 32767); /* 32-bit INT_MAX */
    U_ASSERT(RAND_bytes_openssl != 0);
    if (!RAND_bytes_openssl || bufsize > 32767)
        return 0;

    RAND_bytes_openssl(buf, (int)bufsize);
    return 1;
}

U_rand_bytes_fp U_InitRandBytesOpenssl(void)
{
    void *lib;

    lib = U_library_open_ex("libcrypto");
    if (!lib)
        return 0;

    RAND_bytes_openssl = (void(*)(unsigned char*,int))U_library_symbol(lib, "RAND_bytes");
    if (RAND_bytes_openssl)
        return U_RandBytesOpenSSL;

    return 0;
}

