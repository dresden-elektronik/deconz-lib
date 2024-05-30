#include <stddef.h>
#include "deconz/u_sha256.h"
#include "deconz/u_library_ex.h"

#define LONESHA256_STATIC
#include "lonesha256.h"

static unsigned char (*libSHA256)(const unsigned char *d, size_t n, unsigned char *md);

unsigned char libSha256LoneHash(const unsigned char *d, size_t n, unsigned char *hash)
{
    lonesha256(hash, d, n);
    return 1;
}

static int init_sha256_lib(void)
{
    void *libcrypto;

    if (!libSHA256)
    {
#ifdef HAS_OPENSSL
        libcrypto = U_library_open_ex("libcrypto");
        if (libcrypto)
            libSHA256 = (unsigned char (*)(const unsigned char*, size_t, unsigned char*))U_library_symbol(libcrypto, "SHA256");
#endif

        if (!libSHA256)
            libSHA256 = libSha256LoneHash;
    }

    return libSHA256 ? 1 : 0;
}

int U_Sha256(const void *data, unsigned size, unsigned char *hash)
{
    if (!init_sha256_lib())
        return 0;

    if (!data || size == 0 || !hash)
        return 0;

    libSHA256(data, size, hash);
    return 1;
}
