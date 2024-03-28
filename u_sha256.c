#include "deconz/u_sha256.h"
#include "deconz/u_library_ex.h"

#ifdef HAS_OPENSSL
#include <openssl/sha.h>

static unsigned char (*libSHA256)(const unsigned char *d, size_t n, unsigned char *md);

static int init_sha256_lib(void)
{
    void *libcrypto;

    if (!libSHA256)
    {
        libcrypto = U_library_open_ex("libcrypto");
        if (libcrypto)
            libSHA256 = (unsigned char (*)(const unsigned char*, size_t, unsigned char*))U_library_symbol(libcrypto, "SHA256");
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

#else

int U_Sha256(const void *data, unsigned size, unsigned char *hash)
{
    (void)data;
    (void)size;
    (void)hash;
    return 0;
}

#endif
