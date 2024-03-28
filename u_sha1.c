#include "deconz/u_sha1.h"
#include "deconz/u_library_ex.h"

#ifdef HAS_OPENSSL
#include <openssl/sha.h>

static unsigned char (*libSHA1)(const unsigned char *d, size_t n, unsigned char *md);

static int init_sha1_lib(void)
{
    void *libcrypto;

    if (!libSHA1)
    {
        libcrypto = U_library_open_ex("libcrypto");
        if (libcrypto)
            libSHA1 = (unsigned char (*)(const unsigned char*, size_t, unsigned char*))U_library_symbol(libcrypto, "SHA1");
    }

    return libSHA1 ? 1 : 0;
}

int U_Sha1(const void *data, unsigned size, unsigned char *hash)
{
    if (!init_sha1_lib())
        return 0;

    if (!data || size == 0 || !hash)
        return 0;

    libSHA1(data, size, hash);
    return 1;
}

#else

int U_Sha1(const void *data, unsigned size, unsigned char *hash)
{
    (void)data;
    (void)size;
    (void)hash;
    return 0;
}

#endif
