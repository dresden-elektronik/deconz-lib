/*
 * Copyright (c) 2023-2026 Manuel Pietschmann.
 * All rights reserved.
 *
 * The software in this package is published under the terms of the BSD
 * style license a copy of which has been included with this distribution in
 * the LICENSE.txt file.
 *
 */

#include <dlfcn.h>
#include "u_platform.h"
#include "u_library.h"

void *U_library_open(const char *filename)
{
	int flags;
    unsigned len;
    unsigned dot_pos;
    char major;
    void *result;
    char path[512];

    flags = RTLD_LAZY;
    dot_pos = 0;
    result = 0;

    if (filename)
    {
        for (len = 0; filename[len]; len++)
        {
            if (dot_pos == 0 && filename[len] == '.')
                dot_pos = len;

            if ((len + 16) > sizeof(path))
            {
                /* error unexpected long path */
                return 0;
            }

            path[len] = filename[len];
        }

        path[len] = '\0';

        if (dot_pos == 0) /* add platform extension */
        {
#ifdef PL_MACOS
            path[len++] = '.';
            path[len++] = 'd';
            path[len++] = 'y';
            path[len++] = 'l';
            path[len++] = 'i';
            path[len++] = 'b';
            path[len] = '\0';
#endif

#ifndef PL_MACOS
#ifdef PL_UNIX
            path[len++] = '.';
            path[len++] = 's';
            path[len++] = 'o';
            path[len] = '\0';
#endif
#endif
        }

        result = dlopen(path, flags);

        /*  libname.so didn't work
            try libname.so.1 .. libname.so.9 */
        if (!result && dot_pos == 0)
        {
            dot_pos = len;
            /* NOTE this is restricted to small major version numbers */
            for (major = '0'; result == 0 && major < '9'; major++)
            {
                len = dot_pos;
                path[len++] = '.';
                path[len++] = major;
                path[len++] = '\0';

                result = dlopen(path, flags);
            }
        }

        return result;
    }

	return dlopen(filename, flags);
}

void U_library_close(void *handle)
{
    if (handle)
	   dlclose(handle);
}

void *U_library_symbol(void *handle, const char *symbol)
{
	return dlsym(handle, symbol);
}
