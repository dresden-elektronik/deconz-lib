#ifndef U_TIMER_H
#define U_TIMER_H

/*
 * Copyright (c) 2012-2025 dresden elektronik ingenieurtechnik gmbh.
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

#define AM_ACTOR_ID_TIMERS 8

#define U_TIMER_REPEAT_INFINITE 0xFFFFFFFF

/*! Public interface */
U_LIBAPI int U_TimerStart(unsigned long actor_id, unsigned long timer_id, unsigned long timeout, unsigned long repeat);
U_LIBAPI int U_TimerStop(unsigned long actor_id, unsigned long timer_id);


/*! Following functions are only used by deCONZ core. */
U_LIBAPI void U_TimerInit(void *am_api_fn);
U_LIBAPI void U_TimerTick(long long diff);
U_LIBAPI void U_TimerDestroy(void);

#ifdef __cplusplus
}
#endif

#endif /* U_TIMER_H */
