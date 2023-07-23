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

#ifndef U_LIBRARY_H
#define U_LIBRARY_H

#ifndef U_LIBAPI
#ifdef _WIN32
  #ifdef USE_ULIB_SHARED
    #define U_LIBAPI  __declspec(dllimport)
  #endif
  #ifdef BUILD_ULIB_SHARED
    #define U_LIBAPI  __declspec(dllexport)
  #endif
#endif
#endif /* ! defined(U_LIBAPI) */

#ifndef U_LIBAPI
#define U_LIBAPI
#endif

#ifdef __cplusplus
extern "C" {
#endif

U_LIBAPI void *U_library_open(const char *filename);
U_LIBAPI void U_library_close(void *handle);
U_LIBAPI void *U_library_symbol(void *handle, const char *symbol);

#ifdef __cplusplus
}
#endif

#endif /* U_LIBRARY_H */

