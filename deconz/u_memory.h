/*
 * Copyright (c) 2024-2025 dresden elektronik ingenieurtechnik gmbh.
 * All rights reserved.
 *
 * The software in this package is published under the terms of the BSD
 * style license a copy of which has been included with this distribution in
 * the LICENSE.txt file.
 *
 */

#ifndef U_MEMORY_H
#define U_MEMORY_H

#ifndef U_LIBAPI
#ifdef _WIN32
  #ifdef USE_ULIB_SHARED
    #define U_LIBAPI  __declspec(dllimport)
  #endif
  #ifdef BUILD_ULIB_SHARED
    #define U_LIBAPI  __declspec(dllexport)
  #endif
#elif defined(__GNUC__) /* Unix */
  #if defined(BUILD_ULIB_SHARED) || defined(USE_ULIB_SHARED)
    #define U_LIBAPI  __attribute__ ((visibility("default")))
  #endif
#endif
#endif /* ! defined(U_LIBAPI) */

#ifndef U_LIBAPI
#define U_LIBAPI
#endif


#define U_KILO_BYTES(n) ((n) * 1000)
#define U_MEGA_BYTES(n) ((n) * 1000 * 1000)

#ifdef __cplusplus
extern "C" {
#endif

U_LIBAPI void *U_memalign(void *p, unsigned align);
U_LIBAPI void *U_memset(void *p, int c, unsigned long n);
U_LIBAPI int U_memcmp(const void *a, const void *b, unsigned long n);
U_LIBAPI void *U_memcpy(void *dst, const void *src, unsigned long n);
U_LIBAPI unsigned U_strlen(const char *str);

U_LIBAPI void *U_Alloc(unsigned long size);
U_LIBAPI int U_Free(void *p);

#ifdef __cplusplus
}
#endif

#endif /* U_MEMORY_H */
