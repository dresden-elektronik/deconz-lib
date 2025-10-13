/*
 * Copyright (c) 2024 dresden elektronik ingenieurtechnik gmbh.
 * All rights reserved.
 *
 * The software in this package is published under the terms of the BSD
 * style license a copy of which has been included with this distribution in
 * the LICENSE.txt file.
 *
 */

#include "deconz/u_sstream.h"
#include "deconz/u_library_ex.h"

void *U_library_open_ex(const char *filename)
{
    U_SStream ss;
    unsigned len;
    void *lib;

    if (!filename || filename[0] == '\0')
    {
        return U_library_open(filename);
    }

    for (len = 0; filename[len]; len++)
    {}

    U_sstream_init(&ss, (void*)filename, len);

    if (U_sstream_starts_with(&ss, "libssl"))
    {
#ifdef PL_MACOS
#ifdef DECONZ_DEBUG_BUILD
        filename = "/opt/homebrew/lib/libssl.3.dylib";
#else
        filename = "../Frameworks/libssl.3.dylib";
#endif
#elif defined PL_WINDOWS
      lib = U_library_open("libssl-3.dll");
      if (!lib)
        lib = U_library_open("libssl-3-x64.dll");

      return lib;
#endif

#ifndef PL_MACOS
#ifdef PL_UNIX
    lib = U_library_open("libssl.so");
    if (!lib)
        lib = U_library_open("libssl.so.3");
    if (!lib)
        lib = U_library_open("libssl.so.1.1");

    return lib;
#endif
#endif

    }
    else if (U_sstream_starts_with(&ss, "libcrypto"))
    {
#ifdef PL_MACOS
        filename = "../Frameworks/libcrypto.3.dylib";
#elif defined PL_WINDOWS
        lib = U_library_open("libcrypto-3.dll");
        if (!lib)
          lib = U_library_open("libcrypto-3-x64.dll");

        return lib;
#endif

#ifndef PL_MACOS
#ifdef PL_UNIX
    lib = U_library_open("libcrypto.so");
    if (!lib)
        lib = U_library_open("libcrypto.so.3");
    if (!lib)
        lib = U_library_open("libcrypto.so.1.1");

    return lib;
#endif
#endif
    }

    return U_library_open(filename);

}
