#ifndef U_RANDOM_H
#define U_RANDOM_H

/*
 * Copyright (c) 2024 dresden elektronik ingenieurtechnik gmbh.
 * All rights reserved.
 *
 * The software in this package is published under the terms of the BSD
 * style license a copy of which has been included with this distribution in
 * the LICENSE.txt file.
 *
 */

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

/*! Writes high quality random data into \p buf */
U_LIBAPI int U_RandomBytes(unsigned char *buf, unsigned bufsize);

#ifdef U_LIBAPI_PRIVATE
typedef int (*U_rand_bytes_fp)(unsigned char*, unsigned);
#endif

#ifdef __cplusplus
}
#endif

#endif /* U_RANDOM_H */
