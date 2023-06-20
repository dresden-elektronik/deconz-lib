/*
 * Copyright (c) 2023 Manuel Pietschmann.
 * All rights reserved.
 *
 * The software in this package is published under the terms of the BSD
 * style license a copy of which has been included with this distribution in
 * the LICENSE.txt file.
 *
 */

#include <libloaderapi.h>
#include "u_library.h"

void *U_library_open(const char *filename)
{
    HMODULE h;
    h = LoadLibrary(filename);
    return (void*)h;
}

void U_library_close(void *handle)
{
    FreeLibrary((HMODULE)handle);
}

void *U_library_symbol(void *handle, const char *symbol)
{
    return (void*)GetProcAddress((HMODULE)handle, (LPCSTR)symbol);
}

