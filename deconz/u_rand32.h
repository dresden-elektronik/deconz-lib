#ifndef U_RAND32_H
#define U_RAND32_H

/*
 * Copyright (c) 2012-2025 dresden elektronik ingenieurtechnik gmbh.
 * All rights reserved.
 *
 * The software in this package is published under the terms of the BSD
 * style license a copy of which has been included with this distribution in
 * the LICENSE.txt file.
 *
 */

#ifndef U_LIBAP
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

#ifdef __cplusplus
extern "C" {
#endif

/*! Sets pseudo random number generator seed value. */
U_LIBAPI void U_rand32_seed(unsigned seed);


/*! Returns 32-bit random number.
    This must NOT be used for security related random data use u_random.h instead. */
U_LIBAPI unsigned U_rand32(void);

#ifdef __cplusplus
}
#endif

#endif /* U_RAND32_H */
