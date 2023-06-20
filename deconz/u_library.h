/*
 * Copyright (c) 2023 Manuel Pietschmann.
 * All rights reserved.
 *
 * The software in this package is published under the terms of the BSD
 * style license a copy of which has been included with this distribution in
 * the LICENSE.txt file.
 *
 *
 * Upstream repository: https://git.sr.ht/~cryo/u_library
 */

#ifndef _U_LIBRARY_H
#define _U_LIBRARY_H

#ifdef _WIN32
  #ifdef U_SHARED_LIB
    #define U_EXPORT __declspec(dllexport)
  #else
    #define U_EXPORT __declspec(dllimport)
  #endif
#else /* UNIX ... */
    #define U_EXPORT
#endif

#ifdef __cplusplus
extern "C" {
#endif

U_EXPORT void *U_library_open(const char *filename);
U_EXPORT void U_library_close(void *handle);
U_EXPORT void *U_library_symbol(void *handle, const char *symbol);

#ifdef __cplusplus
}
#endif

#endif /* _U_LIBRARY_H */

