#ifndef U_TIME_H
#define U_TIME_H

/*
 * Copyright (c) 2012-2024 dresden elektronik ingenieurtechnik gmbh.
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

/*! Time from Epoch in milliseconds */
typedef signed long long U_Time;

#ifdef __cplusplus
extern "C" {
#endif

/*! Converts ISO8601 date time string to U_time. */
U_LIBAPI U_Time U_TimeFromISO8601(const char *str, unsigned len);

#ifdef __cplusplus
}
#endif

#endif /* U_TIME_H */
