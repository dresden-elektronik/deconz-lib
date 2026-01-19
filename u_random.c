#include "deconz/u_platform.h"
#include "deconz/u_assert.h"
#include "deconz/u_random.h"

/* support multiple implementations, select first available at runtime */

extern U_rand_bytes_fp U_InitRandBytesOpenssl(void);
extern U_rand_bytes_fp U_InitRandBytesGetEntropy(void);

static U_rand_bytes_fp rand_bytes_impl;

/* rudimentary test if it works */
static void test_rand_implementation(unsigned char *buf, unsigned bufsize)
{
    unsigned i;
    unsigned c;

    if (!rand_bytes_impl)
        return;

    for (i = 0; i < bufsize; i++)
        buf[i] = 1;

    rand_bytes_impl(buf, bufsize);

    for (c = 0, i = 0; i < bufsize; i++)
        c += buf[i];

    U_ASSERT(c != bufsize && "rand_bytes implementation suspicious");
    if (c == bufsize)
        rand_bytes_impl = 0;
}

int U_RandomBytes(unsigned char *buf, unsigned bufsize)
{
    U_ASSERT(buf);
    if (!buf || bufsize == 0)
        return 0;

    if (!rand_bytes_impl)
    {
        /*
         * Try to find a working implementation of OS or 3rd party library
         * high entropy random bytes function.
         */
#ifdef PL_UNIX
        if (!rand_bytes_impl)
        {
            rand_bytes_impl = U_InitRandBytesGetEntropy();
            test_rand_implementation(buf, bufsize);
        }
#endif

#ifdef HAS_OPENSSL
        if (!rand_bytes_impl)
        {
            rand_bytes_impl = U_InitRandBytesOpenssl();
            test_rand_implementation(buf, bufsize);
        }
#endif
        if (!rand_bytes_impl)
            goto no_impl;
    }

    return rand_bytes_impl(buf, bufsize);

no_impl:
    U_ASSERT(0 && "no rand_bytes implementation available");
    return 0;
}
