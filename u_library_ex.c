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
        filename = "../Frameworks/libssl.3.dylib";
#elif defined PL_WINDOWS
        filename = "libssl-1_1.dll";
#endif
    }
    else if (U_sstream_starts_with(&ss, "libcrypto"))
    {
#ifdef PL_MACOS
        filename = "../Frameworks/libcrypto.3.dylib";
#elif defined PL_WINDOWS
        filename = "libcrypto-1_1.dll";
#endif
    }

    return U_library_open(filename);

}
