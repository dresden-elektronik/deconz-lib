#include "stddef.h"
#include "deconz/u_assert.h"
#include "deconz/u_random.h"
#include "deconz/u_library.h"

static int (*getentropy_fp)(void*, size_t);

static int U_RandBytesGetEntropy(unsigned char *buf, unsigned bufsize)
{
    unsigned pos;
    size_t batch;

    U_ASSERT(bufsize != 0);
    U_ASSERT(getentropy_fp != 0);
    if (getentropy_fp == 0)
        return 0;

    /* getentropy supports max. 256 bytes */

    for (pos = 0; pos < bufsize;)
    {
        batch = ((bufsize - pos) <= 256) ? (bufsize - pos) : 256;

        if (getentropy_fp(&buf[pos], batch) != 0)
            return 0;

        pos += batch;
    }

    return 1;
}

U_rand_bytes_fp U_InitRandBytesGetEntropy(void)
{
    getentropy_fp = (int(*)(void*, size_t))U_library_symbol(0, "getentropy");
    if (getentropy_fp)
    {
        return U_RandBytesGetEntropy;
    }

    return 0;
}

