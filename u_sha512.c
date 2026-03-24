#include <stddef.h>
#include "deconz/u_sha512.h"
#include "deconz/u_library_ex.h"



static unsigned char (*libSHA512)(const unsigned char *d, size_t n, unsigned char *md);

static int init_sha512_lib(void)
{
    void *libcrypto;

    if (!libSHA512)
    {
#ifdef HAS_OPENSSL
        libcrypto = U_library_open_ex("libcrypto");
        if (libcrypto)
            libSHA512 = (unsigned char (*)(const unsigned char*, size_t, unsigned char*))U_library_symbol(libcrypto, "SHA512");
#endif
    }

    return libSHA512 ? 1 : 0;
}

int U_Sha512(const void *data, unsigned size, unsigned char *hash)
{
    if (!init_sha512_lib())
        return 0;

    if (!data || size == 0 || !hash)
        return 0;

    libSHA512(data, size, hash);
    return 1;
}
