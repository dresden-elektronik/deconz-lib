/*
 * Copyright (c) 2024 dresden elektronik ingenieurtechnik gmbh.
 * All rights reserved.
 *
 * The software in this package is published under the terms of the BSD
 * style license a copy of which has been included with this distribution in
 * the LICENSE.txt file.
 *
 */

#ifndef U_ASSERT_H
#define U_ASSERT_H

#ifdef DECONZ_DEBUG_BUILD
#if _MSC_VER
  #define U_ASSERT(c) if (!(c)) __debugbreak()
#elif __GNUC__
  #define U_ASSERT(c) if (!(c)) __builtin_trap()
#else
  #include <assert.h>
  #define U_ASSERT(c) assert((c))
#endif
#else // release build
  #define U_ASSERT(c)
#endif

#endif /* U_ASSERT_H */
