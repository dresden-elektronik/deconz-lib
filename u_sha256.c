#include "deconz/u_sha256.h"
#include "deconz/u_library.h"

#ifdef HAS_OPENSSL
#include <openssl/sha.h>

static unsigned char (*libSHA256)(const unsigned char *d, size_t n, unsigned char *md);

static int init_sha256_lib(void)
{
    void *libssl;

    if (!libSHA256)
    {
#ifdef PL_MACOS
        libssl = U_library_open("../Frameworks/libssl.3.dylib");
#elif defined(PL_WINDOWS)
        libssl = U_library_open("libcrypto-1_1.dll");
#else
        libssl = U_library_open("libssl");
#endif
        if (libssl)
            libSHA256 = (unsigned char (*)(const unsigned char*, size_t, unsigned char*))U_library_symbol(libssl, "SHA256");
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
